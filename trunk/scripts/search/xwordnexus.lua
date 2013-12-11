-- ============================================================================
-- CrosswordNexus search
-- ============================================================================

local base = 'http://crosswordnexus.com/finder.php?'

local function makeURL(clue, pattern)
    clue = string.gsub(clue, ' ', '+')
    local search = wx.wxURL('clue=' .. clue .. '&pattern=' .. pattern)
    -- BuildURI escapes characters such as '?'
    return base .. search:BuildURI()
end

local function CrosswordNexusSearch()
    -- Get the current clue
    local number, clue = xword.frame:GetFocusedClue()
    local pattern = search.makePattern()
    wx.wxLaunchDefaultBrowser(makeURL(clue, pattern))
end

search.addEntry('CrosswordNexus', CrosswordNexusSearch)
