-- ============================================================================
-- Wikipedia search
--     A simple XWord add-on that searches wikipedia for the currently selected
--     word.  If the word has any blanks, alert the user.
-- ============================================================================

local base = 'http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search='


local function makeURL(pattern)
    -- BuildURI escapes characters such as '?'
    return base .. wx.wxURL(pattern):BuildURI()
end

local function WikipediaSearch()
    -- Read the currently selected word
    local word = xword.frame:GetFocusedWord()
    local pattern = ''
    for _, square in ipairs(word) do
        -- Can't search wikipedia if the word contains blanks
        if #square.Text == 0 then
            xword.Message('Wikipedia search entry cannot contain blank squares.')
            return
        end
        pattern = pattern .. square.Text
    end

    wx.wxLaunchDefaultBrowser(makeURL(pattern))
end

search.addEntry('Wikipedia', WikipediaSearch)
