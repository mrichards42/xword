-- A task that gathers stats for puzzles
require 'luapuz'
require 'lfs'
require 'download.messages'
require 'import'

-- Return started (bool), comlete (pct), time (secs)
-- If there is no puzzle, return a blank table
local function get_stats(filename)
    if lfs.attributes(filename, 'mode') ~= 'file' then
        return {}
    end

    -- If we can't save this format, assume that the user hasn't started
    -- solving the puzzle
    if not puz.Puzzle.CanSave(filename) then
        return {exists = true, started = false, complete = 0, time = 0}
    end

    local success, p = pcall(puz.Puzzle, filename)
    if not success then return {} end
    local blank = 0
    local total = 0
    local time = p.Time
    local started = time > 0
    local square = p.Grid:First()
    while square do
        if square:IsWhite() then
            total = total + 1
            if square:IsBlank() then
                blank = blank + 1
                if not started then
                    -- Flags that indicate that the user has entered something
                    -- in the square
                    started = square:HasFlag(puz.FLAG_X +
                                             puz.FLAG_REVEALED +
                                             puz.FLAG_BLACK)
                end
            elseif not started then
                started = true
            end
        end
        square = square:Next()
    end
    p:__gc()
    return {exists = true,
            started = started,
            complete = math.floor(100 - blank / total * 100 + 0.5), -- Round
            time = time}
end

local puzzle_queue = {}
-- Populate puzzle_queue with the arguments to this file
if type(arg) == 'table' then
    for _, filename in ipairs(arg) do
        table.insert(puzzle_queue, filename)
    end
elseif type(arg) == 'string' then
    table.insert(puzzle_queue, arg)
end

local function process_messages(timeout)
    -- Process messages recieved from the calling thread
    local filenames, flag, rc = task.receive(timeout or 0, 1)
    if rc == 0 then -- got a message
        if flag == task.ABORT then
            return 'abort'
        elseif flag == download.PREPEND then
            if type(filenames) == 'string' then
                filenames = {filenames}
            end
            if type(filenames == 'table') then
                for _, filename in ipairs(filenames) do
                    table.insert(puzzle_queue, 1, filename)
                end
            end
        elseif flag == download.APPEND then
            if type(filenames) == 'string' then
                filenames = {filenames}
            end
            if type(filenames == 'table') then
                for _, filename in ipairs(filenames) do
                    table.insert(puzzle_queue, filename)
                end
            end
        end
    end
end

-- Loop through puzzle_queue and retrieve the stats for each puzzle
while true do
    local filename = table.remove(puzzle_queue, 1)
    if not filename then
        -- Wait just a bit for new messages
        process_messages(500)
        if #puzzle_queue == 0 then
            break
        end
    else
        -- Get stats, post results
        --task.debug('Stats for: '.. filename:match('[\\/]([^\\/]+)$'))
        task.post(1, {filename, get_stats(filename)}, download.STATS)
    end
    -- Check for new messages
    if process_messages() == 'abort' then break end
end
