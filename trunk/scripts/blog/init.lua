require 'date'

-- The package table
blog = {}

blog.entries = {}

function blog.add_entry(name, func)
    table.insert(blog.entries, {name = name, func = func})
end

function blog.open(url, d)
    wx.wxLaunchDefaultBrowser(d:fmt(url))
end


-- Try to figure out the date for this puzzle
local function get_date()
    -- Try the title
    local title = xword.frame.Puzzle.Title
    local str = title:match(".-(%S+ %d?%d,? %d%d%d%d).?")
    if str then
        return date(str)
    end
    -- TODO: try the filename
    xword.Error("Could not find a date in the puzzle title")
    return false
end

function blog.init()
    -- Blogs
    require 'blog.rexparker'
    require 'blog.crosswordfiend'
    require 'blog.wordplay'
    require 'blog.xwordinfo'

    for _, entry in ipairs(blog.entries) do
        -- Add to the menu
        xword.frame:AddMenuItem({'Tools', 'Blogs'}, entry.name,
            function(evt)
                -- Can't do anything unless we have a puzzle
                if not xword.HasPuzzle() then return end
                local d = get_date()
                if not d then return end
                entry.func(d)
            end
        )
    end
end

function blog.uninit()
    for _, entry in ipairs(blog.entries) do
        -- Remove from the menu
        xword.frame:RemoveMenuItem('Tools', 'Blogs', entry.name)
    end
    -- Remove the menu
    xword.frame:RemoveMenuItem('Tools', 'Blogs')
end

return blog
