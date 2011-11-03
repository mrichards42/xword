-- ============================================================================
-- Wikipedia search
--     A simple XWord add-on that searches wikipedia for the currently selected
--     word.  If the word has any blanks, alert the user.
-- ============================================================================

local base = 'http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search='

local function WikipediaSearch()
    search.search(base)
end

search.addEntry('Wikipedia', WikipediaSearch)
