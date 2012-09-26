-- A task that gathers stats for puzzles
require 'download.queue_task'
require 'luapuz'
require 'lfs'
require 'import'
require 'download.messages'

-- return puzzle status
local function get_status(filename)
    if lfs.attributes(filename, 'mode') ~= 'file' then
        return download.MISSING
    end

    -- If we can't save this format, assume that the user hasn't started
    -- solving the puzzle
    -- If we can't load this format (e.g. PDF) obviously the user hasn't started
    -- solving either
    if not puz.Puzzle.CanSave(filename) or not puz.Puzzle.CanLoad(filename) then
        return download.EXISTS
    end

    local success, p = pcall(puz.Puzzle, filename)
    if not success then
        return download.MISSING
    end
    -- See if the user has started the puzzle but not finished it
    local function get_solving_status()
        local solving = p.Time > 0
        local square = p.Grid:First()
        while square do
            if square:IsWhite() then
                if square:IsBlank() then
                    if solving then
                        return download.SOLVING
                    elseif square:HasFlag(puz.FLAG_X + puz.FLAG_REVEALED +
                                                           puz.FLAG_BLACK) then
                        return download.SOLVING
                    end
                elseif not solving then
                    solving = true
                end
            end
            square = square:Next()
        end
        return solving and download.COMPLETE or download.EXISTS
    end
    local status = get_solving_status()
    p:__gc()
    return status
end

loop_through_queue(function(filename)
    task.post(1, {filename, get_status(filename)}, download.STATS)
end)
