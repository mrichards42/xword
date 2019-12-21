-- The package table
import = {}

-- Theme algorithm
import.mark_theme_squares = require 'import.theme'


--[[
handler table format:
{
    {
        ext = extension,
        load = function(puz, filename) end,
        desc = description/label
    },
    [...]
}
]]
import.handlers = {
}

-- ============================================================================
-- Add a handler to the import handlers table.

-- loadfunc should take a puzzle and filename as parameteters.  It should
-- return true on succes, raise an error on failure.

-- ============================================================================
function import.addHandler(loadfunc, ext, label)
    assert(label and ext and loadfunc)
    ext = ext:lower()

    -- Add to the handlers table
    table.insert(import.handlers,
                 { desc = label, ext = ext, load = loadfunc })
end


function import.init()
end

function import.uninit()
end

-- Import the handlers
require 'import.uclick'
require 'import.xwordinfo'
require 'import.newsday'
require 'import.rowsgarden'

-- ============================================================================
-- Replace loading functions with import.load
-- ============================================================================
local puzzle_load = puz.Puzzle.Load
local tablex = require 'pl.tablex'
local path = require 'pl.path'
-- Try all handlers on a puzzle.
function import.load(p, filename)
    if not path.exists(filename) then
        error("File does not exist: "..filename, 2)
    end
     -- Extension without the dot
    local ext = path.extension(filename):lower():sub(2)

    -- Try all handlers, but prefer those with a matching extension
    local handlers = tablex.filter(import.handlers,
                                   function (h) return h.ext == ext end)
    -- load = nil means Puzzle.Load will be called normally
    table.insert(handlers, {})
    tablex.insertvalues(
        handlers,
        tablex.filter(import.handlers,
                      function (h) return h.ext ~= ext end)
    )

    for _, handler in ipairs(handlers) do
        p:Clear()

        -- Load the puzzle (prevent line information from being added
        local success, err
        if handler.load ~= nil then
            success, err = xpcall(
                function () puzzle_load(p, filename, handler.load) end,
                function (err) return err end
            )
        else
            success, err = xpcall(function () puzzle_load(p, filename) end,
                                  function (err) return err end)
        end

        -- Check the return value
        if success then
            return true
        elseif err:sub(1,12) == "Not a valid " or err == "Unknown file type" or err == "Wrong file type" then
            -- Try again . . . wrong handler for this file.
        else
            -- By setting the error level to 2, "import/init.lua line xx" is
            -- omitted from the error report!
            error(err, 2) -- Correct handler, bad file
        end
    end
    error("Unknown file type", 2)
end


-- Puzzle.Load
puz.Puzzle.Load = import.load

-- Puzzle constructor
local puzzle_constructor = getmetatable(puz.Puzzle).__call
getmetatable(puz.Puzzle).__call = function(p, filename, handler)
    if filename then
        return puzzle_constructor(p, filename, handler or import.load)
    else
        return puzzle_constructor(p)
    end
end

-- Puzzle.CanLoad
local canload = puz.Puzzle.CanLoad
puz.Puzzle.CanLoad = function(filename)
    if canload(filename) then
        return true
    else
        -- Try custom extensions
        local extension = path.extension(filename):sub(2):lower()
        for _, desc in ipairs(import.handlers) do
            if desc.ext == extension then
                return true
            end
        end
    end
    return false
end

return import
