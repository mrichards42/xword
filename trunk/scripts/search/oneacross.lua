-- ============================================================================
-- OneAcross search
--     A slightly more complicated XWord add-on (compared to Wikipedia search)
--     that searches OneAcross with the currently selected clue and word.
-- ============================================================================

local base = 'http://www.oneacross.com/cgi-bin/search_banner.cgi?'

local function makeURL(clue, pattern)
    clue = string.gsub(clue, ' ', '+')
    local search = wx.wxURL('c0=' .. clue .. '&p0=' .. pattern)
    -- BuildURI escapes characters such as '?'
    return base .. search:BuildURI()
end

local function OneAcrossSearch()
    -- Get the current clue
    local number, clue = xword.frame:GetFocusedClue()

    -- Assemble the search pattern for the current word
    local word = xword.frame:GetFocusedWord()
    local pattern = ''
    for _, square in ipairs(word) do
        if not square:IsBlank() then
            pattern = pattern .. square.Text
        else
            -- Blank squares are represented by question marks
            pattern = pattern .. '?'
        end
    end

    wx.wxLaunchDefaultBrowser(makeURL(clue, pattern))
end

search.addEntry('OneAcross', OneAcrossSearch)
