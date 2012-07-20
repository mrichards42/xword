-- OneAcross
-- Attemps to fill in the grid using oneacross

require 'luacurl'

local dlg

-- Words and URLS
local function make_pattern(word)
    local pattern = ''
    for _, square in ipairs(word) do
        if not square:IsBlank() then
            pattern = pattern .. square.Text
        else
            -- Blank squares are represented by question marks
            pattern = pattern .. '?'
        end
    end
    return pattern
end

local base = 'http://www.oneacross.com/cgi-bin/search_banner.cgi?'
local function make_url(clue, pattern)
    clue = string.gsub(clue, ' ', '+')
    local search = wx.wxURL('c0=' .. clue .. '&p0=' .. pattern)
    return base .. search:BuildURI()
end


local function get_word(number, direction)
    number = tostring(number)
    local s = g:FindSquare(function (s) return s.Number == number end)
    if not s then return end
    local word = {}
    while s and s:IsWhite() do
        table.insert(word, s)
        if s:IsLast(direction) then break end
        s = s:Next(direction)
    end
    return word
end

local function get_all_words()
    local p = xword.frame.Puzzle
    local g = p.Grid
    local across = p:GetClueList("Across")
    local down = p:GetClueList("Down")

    -- Make a list of all the words and clues
    local words = {}
    for _, clue in ipairs(across) do
        local t = {}
        table.insert(words, { word = get_word(clue.number, puz.ACROSS),
                              clue = clue.text })
    end
    for _, clue in ipairs(down) do
        table.insert(words, { word = get_word(clue.number, puz.DOWN),
                              clue = clue.text })
    end

    return words
end

-- Word scoring
local function score_word(word, suggestion, all_words)
    -- return the index of the square in the word
    local function find_square(word, square)
        for idx, s in ipairs(word) do
            if s == square then
                return idx
            end
        end
    end

    local score = suggestion.base_score

    -- Loop through the letters in this word and look for the crossing words.
    -- Multiply the score of this word by a crossing word factor (how well
    -- does this word fit into the grid?)
    local crossing_score = 0
    for i=1,#suggestion.word do
        local letter = suggestion.word:sub(i,i)
        local square = word.word[i]
        -- Find the crossing word.
        for _, w in ipairs(all_words) do
            local idx = find_square(w.word, square)
            if idx and w ~= word then
                -- For each crossing clue, find the ratio of crosses that match
                -- to possible crosses.
                local total_score = 0
                local matching_score = 0
                for _, cross in ipairs(w.suggestions) do
                    total_score = total_score + cross.base_score
                    if cross.word:sub(idx) == letter then
                        matching_score = matching_score + cross.base_score
                    end
                end
                if total_score > 0 then
                    crossing_score = crossing_score + (matching_score / total_score)
                end
                break
            end
        end
    end
    local cross_factor = crossing_score / #word.word + .25
    return score * cross_factor
end

local function make_base_score(word)
    -- Compute a weighted base score for each suggestion:
    -- Words that have high confidence get higher scores.
    -- Words with many blanks get lower scores.
    -- Words that have fewer suggestions get higher scores.
    local total_confidence = 0
    for _, suggestion in ipairs(word.suggestions) do
        total_confidence = total_confidence + suggestion.confidence
    end
    local _, blanks = word.pattern:gsub("?", "")
    for _, suggestion in ipairs(word.suggestions) do
        local confidence_ratio = suggestion.confidence / total_confidence
        suggestion.base_score = suggestion.confidence^2 * confidence_ratio / (blanks + 1) ^ .6
    end
end

local function score_words(all_words)
    -- Compute a score for each word (base_score is already computed)
    for _, word in ipairs(all_words) do
        for _, suggestion in ipairs(word.suggestions) do
            suggestion.score = score_word(word, suggestion, all_words)
        end
        table.sort(word.suggestions, function (s1, s2) return s1.score > s2.score end)
    end
end

-- The GUI
local function makeDialog()
    if dlg then return dlg end
    dlg = wx.wxDialog(
        xword.frame, wx.wxID_ANY, "OneAcross Grid Filler",
        wx.wxDefaultPosition, wx.wxSize(600,400),
        wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )

    -- Controls
    dlg.list = wx.wxListCtrl(dlg, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLC_REPORT)
    dlg.button = wx.wxButton(dlg, wx.wxID_ANY, "Update")

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        sizer:Add(dlg.list, 1, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(dlg.button, 0, wx.wxALL, 5)
    dlg:SetSizer(sizer)

    -- List functions
    dlg.columns = {
        "Direction",
        "#",
        "Clue",
        "Pattern",
        "Suggestions",
        "Best",
        "Confidence",
        "Base Score",
        "Score"
    }
    function dlg.init_list()
        dlg.list:ClearAll()
        print "Finding all words in the puzzle"
        dlg.words = get_all_words()
        for i, name in ipairs(dlg.columns) do
            dlg.list:InsertColumn(i-1, name)
            dlg.columns[name:lower():gsub(" ", "_")] = i-1 -- map name to column number
        end
        local across = xword.frame.Puzzle:GetClueList("Across")
        local down = xword.frame.Puzzle:GetClueList("Down")
        local nacross = #across
        for idx, word in ipairs(dlg.words) do
            local direction = (idx <= nacross and "Across" or "Down")
            dlg.list:InsertItem(idx-1, direction)
            local number = direction == "Across" and across[idx].number or down[idx - nacross].number
            dlg.list:SetItem(idx-1, dlg.columns["#"], number)
            dlg.list:SetItem(idx-1, dlg.columns.clue, word.clue)
        end
    end

    function dlg.update_list()
        for idx, word in ipairs(dlg.words) do
            dlg.list:SetItem(idx-1, dlg.columns.pattern, word.pattern)
            dlg.update_list_suggestion(idx)
        end
        -- Resize
        for i, _ in ipairs(dlg.columns) do
            dlg.list:SetColumnWidth(i-1, -1)
        end
    end

    function dlg.update_list_suggestion(idx)
        local word = dlg.words[idx]
        if word.suggestions and #word.suggestions > 0 then
            dlg.list:SetItem(idx-1, dlg.columns.suggestions, tostring(#word.suggestions))
            dlg.list:SetItem(idx-1, dlg.columns.best, word.suggestions[1].word)
            dlg.list:SetItem(idx-1, dlg.columns.confidence, tostring(word.suggestions[1].confidence))
            dlg.list:SetItem(idx-1, dlg.columns.base_score, string.format("%.2f", word.suggestions[1].base_score))
            dlg.list:SetItem(idx-1, dlg.columns.score, word.suggestions[1].score and string.format("%.2f", word.suggestions[1].score) or "")
            -- Resize
            for i, _ in ipairs(dlg.columns) do
                dlg.list:SetColumnWidth(i-1, -1)
            end
            local _, blanks = word.pattern:gsub("?", "")
            if blanks == 0 then
                -- Grey the completed words
                dlg.list:SetItemBackgroundColour(idx-1, wx.wxLIGHT_GREY)
            elseif word.suggestions[1].score then
                -- Highlight the best scores
                local score = word.suggestions[1].score
                if score > 1 then
                    dlg.list:SetItemBackgroundColour(idx-1, wx.wxColour(math.min(math.floor(100 + (score / 4)*155), 255), 0, 0))
                end
            end
        else
            dlg.list:SetItem(idx-1, dlg.columns.suggestions, "")
            dlg.list:SetItem(idx-1, dlg.columns.best, "")
            dlg.list:SetItem(idx-1, dlg.columns.confidence, "")
            dlg.list:SetItem(idx-1, dlg.columns.base_score, "")
            dlg.list:SetItem(idx-1, dlg.columns.score, "")
        end
    end

    -- Events
    dlg.words = {}
    function dlg.fetch_words()
        if #dlg.words == 0 then
            dlg.init_list()
        end
        local args = {}
        for idx, word in ipairs(dlg.words) do
            for _, suggestion in ipairs(word.suggestions or {}) do
                suggestion.score = nil
            end
            -- Only update words that have changed and that are not
            -- completely filled in.
            local pattern = make_pattern(word.word)
            if pattern ~= word.pattern then
                word.pattern = pattern
                if pattern:match("?") then
                    args[idx] = make_url(word.clue, pattern)
                end
            end
        end
        print("Downloading word suggestions . . . ")
        dlg.update_list()
        dlg.task_id = task.create('filler.task', args)
        local e = task.handleEvents(dlg.task_id, dlg.callbacks, dlg)
    end

    dlg.button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function (evt)
            dlg.init_list()
            dlg.fetch_words()
        end
    )

    dlg.callbacks = {
        [10] = function(args)
            local idx, suggestions = unpack(args)
            local word = dlg.words[idx]
            print(string.format("Got %d suggestions for %s (%s)", #suggestions, word.clue, word.pattern))
            word.suggestions = suggestions
            make_base_score(word)
            dlg.update_list_suggestion(idx)
        end,

        -- Place the words
        [task.END] = function()
            print "Download complete"
            print "Scoring words"
            score_words(dlg.words)
            print "Done"
            -- Find the top score from unplaced words
            local top_score = -1
            local unplaced = {}
            for _, word in ipairs(dlg.words) do
                local _, blanks = word.pattern:gsub("?", "")
                if blanks > 0 then
                    local best = word.suggestions[1]
                    if best and best.score > top_score then
                        top_score = best.score
                    end
                    table.insert(unplaced, word)
                end
            end
            if top_score == -1 then
                print "All words have been placed!"
            elseif top_score < 2 then
                print "top_score < 2 . . . I'm just guessing at this point."
            else
                print(string.format("Placing words with score: %.2f", top_score))
                for _, word in ipairs(unplaced) do
                    local best = word.suggestions[1]
                    if best and best.score == top_score then
                        print(string.format("Placing word: %s (%s)", best.word, word.clue))
                        for i=1,#word.word do
                            word.word[i]:SetText(best.word:sub(i, i))
                        end
                        xword.frame:Refresh()
                    end
                end
                dlg.fetch_words()
            end
            dlg.update_list()
        end
    }

    -- Popup menu
    dlg.list:Connect(wx.wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
        function(evt)
            local word = dlg.words[evt:GetIndex()+1]
            local function fill_word(text)
                return function()
                    for i=1,#word.word do
                        word.word[i]:SetText(text:sub(i, i))
                    end
                    xword.frame:Refresh()
                end
            end
            if word.suggestions then
                -- Create a popup menu
                local menu = wx.wxMenu()
                -- Best Word
                local best = word.suggestions[1]
                if best then
                    local item = menu:Append(wx.wxID_ANY, string.format("Fill best word: %s (%.2f)", best.word, best.base_score))
                    dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
                        fill_word(best.word)
                    )
                end
                -- All words
                if #word.suggestions > 1 then
                    local submenu = wx.wxMenu()
                    for _, suggestion in ipairs(word.suggestions) do
                        local item = submenu:Append(wx.wxID_ANY, string.format("%s (%.2f)", suggestion.word, suggestion.base_score))
                        dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
                            fill_word(suggestion.word)
                        )
                    end
                    menu:Append(wx.wxID_ANY, "All suggestions", submenu)
                end
                -- Show the menu
                dlg.list:PopupMenu(menu)
            end
        end
    )

    return dlg
end

local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Grid Filler',
        function(evt)
            the_dlg = makeDialog()
            the_dlg:Show()
        end
    )
end

return init
