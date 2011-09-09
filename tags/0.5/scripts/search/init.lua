-- ============================================================================
-- Search
--     Adds a search menu
--     Each search web page requires a separate lua script that must be placed
--     in the search directory.  In order to add an entry to the search menu,
--     the script should call search.addEntry() described below.
-- ============================================================================

-- The package table
search = {}

search.entries = {}

function search.addEntry(name, func)
    table.insert(search.entries, {name = name, func = func})
end

function search.init()
    -- Search handlers
    require 'search.wikipedia'
    require 'search.oneacross'

    for _, entry in ipairs(search.entries) do
        -- Add to the menu
        xword.frame:AddMenuItem({'Tools', 'Search'}, entry.name,
            function(evt)
                -- Can't do anything unless we have a puzzle
                if not xword.HasPuzzle() then return end
                entry.func()
            end
        )
    end
end

function search.uninit()
    for _, entry in ipairs(search.entries) do
        -- Remove from the menu
        xword.frame:RemoveMenuItem('Tools', 'Search', entry.name)
    end
    -- Remove the search menu
    xword.frame:RemoveMenuItem('Tools', 'Search')
end

return search
