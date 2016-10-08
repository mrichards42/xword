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
    local pprint = require 'serialize'.pprint
    local log = {}
    local is_solving = false
    local square = p.Grid:First()
    table.insert(log, pprint(p))
    table.insert(log, pprint(p.Grid))
    while square do
        if not pcall(function () square:IsWhite() end) then
            -- TODO:
            -- This appears to be a bug in luapuz where the square pointer
            -- is returned with the wrong metadata.  It seems like an old
            -- pointer (with the same address as the square) is being stored in
            -- the tracked objects table.
            table.insert(log, pprint(square) .. " (" .. square:GetText() .. ")")
            task.log("square:IsWhite() missing?\n" .. table.concat(log, '\n'))
            require 'task.debug'() -- Try to sort this out next time
            error("square:IsWhite() missing?\n" .. table.concat(log, '\n'))
        else
            table.insert(log, pprint(square) .. " (" .. square:GetSolution() .. ")")
        end
        if square:IsWhite() then
            if square:IsBlank() then
                if square:IsSolutionBlank() then
                    -- Nothing
                elseif is_solving or has_entry(square) then
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
        task.error(ret .. '\n' .. filename)
        return stats.ERROR, ret
    else
        -- Don't wait for the garbage collector
        p:__gc()
        return ret
    end
end