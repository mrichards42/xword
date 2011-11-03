-- ============================================================================
-- Google search
--     A simple XWord add-on that searches google for the currently selected
--     word.  If the word has any blanks, alert the user.
-- ============================================================================

local base = 'http://www.google.com/search?q='

local function GoogleSearch()
    search.search(base)
end

search.addEntry('Google', GoogleSearch)
