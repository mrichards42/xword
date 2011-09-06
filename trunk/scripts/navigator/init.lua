local tablex = require 'pl.tablex'
local path = require 'pl.path'
local seq = require 'pl.seq'

-- Return an iterator over all files that have a known extension (and the given
-- filename)
local function iter(filename)
    local d = path.dirname(filename)
    -- Valid extensions
    return seq(lfs.dir(d))
        :map(function(f) return path.join(d, f) end)
end

-- Return an iterator over files after this one
local function FindNextPuzzle(filename)
    local it = iter(filename)
    for fn in it do
        if fn == filename then
            return it
        end
    end
end

-- Return an iterator over files before this one
local function FindPrevPuzzle(filename)
    local prev = {}
    for fn in iter(filename) do
        if fn == filename then
            -- Return an iterator over prev reversed
            local i = #prev + 1
            return function()
                i = i - 1
                return prev[i]
            end
        else
            table.insert(prev, fn)
        end
    end
end

-- Load the first valid file
local function load_first(file_iterator)
    -- Temp puzzle so that we can figure out the file that we should load
    local p = puz.Puzzle()
    for filename in file_iterator do
        if puz.Puzzle.CanLoad(filename) and pcall(puz.Puzzle.Load, p, filename) then
            p:__gc()
            xword.frame:LoadPuzzle(filename)
            return
        end
    end
end

local function init()
    -- Common part of both 
    local function openpuzzle(func)
        -- Can't do anything unless we have a puzzle
        if not xword.HasPuzzle() then return end
        local filename = xword.frame.Filename
        if #filename == 0 then return end
        local file_iterator = func(filename)
        if file_iterator then
            load_first(file_iterator)
        end
    end

    xword.frame:AddMenuItem({'Tools', 'Navigate'}, 'Next Puzzle\tCtrl+Shift+=',
        function(evt)
            openpuzzle(FindNextPuzzle)
        end
    )
    xword.frame:AddMenuItem({'Tools', 'Navigate'}, 'Previous Puzzle\tCtrl+Shift+-',
        function(evt)
            openpuzzle(FindPrevPuzzle)
        end
    )
end

local function uninit()
    xword.frame:RemoveMenuItem("Tools", "Navigate", "Next Puzzle")
    xword.frame:RemoveMenuItem("Tools", "Navigate", "Previous Puzzle")
    xword.frame:RemoveMenu("Tools", "Navigate")
end

return { init, uninit }