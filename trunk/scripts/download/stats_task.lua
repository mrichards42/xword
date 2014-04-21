-- The task for computing puzzle stats
local lfs = require 'lfs'
local puz = require 'luapuz'
require 'import' -- extra file types

local stats = require 'download.stats'

-- Has this square had a value before?
local function has_entry(square)
    return square:HasFlag(puz.FLAG_X + puz.FLAG_REVEALED + puz.FLAG_BLACK)
end

-- Return the stats enum for this puzzle
local function get_solving_stats(p)
    local is_solving = false
    local square = p.Grid:First()
    while square do
        if square:IsWhite() then
            if square:IsBlank() then
                if is_solving or has_entry(square) then
                    return stats.SOLVING
                end
            elseif not is_solving then
                is_solving = true
            end
        end
        square = square:Next()
    end
    return is_solving and stats.COMPLETE or stats.EXISTS
end

-- Open the puzzle and return its stats enum
-- This function is called for each item in the stats queue
return function(filename)
    if lfs.attributes(filename, 'mode') ~= 'file' then
        return stats.MISSING
    end
    -- If we can't save this format, assume that the user hasn't started
    -- solving the puzzle
    -- If we can't load this format (e.g. PDF) obviously the user hasn't started
    -- solving either
    if not puz.Puzzle.CanSave(filename) or not puz.Puzzle.CanLoad(filename) then
        return stats.EXISTS
    end
    -- Try to load the puzzle
    local success, p = pcall(puz.Puzzle, filename)
    if not success then
        return stats.MISSING
    end
    local success, ret = pcall(get_solving_stats, p)
    if not success then
        task.error(ret .. '\n' .. require'serialize'.pprint(p))
    end
    -- Don't wait for the garbage collector
    p:__gc()
    return ret
end