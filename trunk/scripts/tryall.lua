-- ============================================================================
-- Try A-Z
--     A simple XWord add-on that presents the current clue with the focused
--     letter replaced by all possible letters.
-- ============================================================================


local function TryAZ()
    -- Read the currently selected word
    local word = xword.frame:GetFocusedWord()
    local focusedSquare = xword.frame:GetFocusedSquare()
    local _, clue = xword.frame:GetFocusedClue()
    local pattern = ''
    for _, square in ipairs(word) do
        if square == focusedSquare then
            pattern = pattern .. '~'
        elseif #square.Text == 0 then
            pattern = pattern .. '?'
        else
            pattern = pattern .. square.Text
        end
    end

    local wordstable = {}
    local order = 'AEIOUY BCDFGHJKLMNPQRSTVWXZ'
    for letter in string.gmatch(order, '.') do
        if letter == ' ' then
            table.insert(wordstable, string.rep('-', #word))
        else
            local word = string.gsub(pattern, '~', letter)
            table.insert(wordstable, word)
        end
    end
    local sel = wx.wxGetSingleChoiceIndex(clue, 'Try A-Z', wordstable) + 1
    if sel > 0 then
        local letter = order:sub(sel, sel)
        if letter ~= ' ' then
            assert(#letter == 1)
            xword.frame:SetSquareText(focusedSquare, letter)
        end
    end
end

local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Try A-Z',
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end
            TryAZ()
        end
    )
end

init()
