require 'date'
require 'lfs'
require 'luapuz'
local path = require 'pl.path'

local database = download.database

-- ----------------------------------------------------------------------------
-- Puzzle statistics
-- ----------------------------------------------------------------------------
-- Return started (bool), comlete (pct), time (secs)
local function puzzleStats(filename)
    local success, p = pcall(puz.Puzzle, filename)
    if not success then return end
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
    return started,
           math.floor(100 - blank / total * 100 + 0.5), -- Round
           time
end


-- ----------------------------------------------------------------------------
-- Cache a puzzle with the given filename
-- ----------------------------------------------------------------------------
function database.cachePuzzle(filename, source, date)
    -- Check to see if the file exists
    local attr = lfs.attributes(filename)
    if not attr or attr.mode ~= 'file' then
        return
    end

    local basename = path.basename(filename)

    -- Retrieve the record from the database
    local record = database.find(basename)
    -- If there is more than one, erase the others
    for i=2,#record do
        database.delete(record[i].id)
    end
    record = record[1]
    -- Check file modification time: if it has not changed, we're done
    if record and record.modified == attr.modification then
        return record
    end

    --local source, d = findSource(basename)
    --local display = source.display

    -- Try to load the puzzle
    local started, complete, time = puzzleStats(filename)
    if started == nil then return end

    -- Do the caching
    -- We aren't going to use the source and date columns in the database
    local stats = {
        filename = basename,
        source = source,
        date = date,
        modified = attr.modification,
        started = started,
        complete = complete or -1,
        time = time or -1,
    }

    if record then -- Update the existing record
        for k,v in pairs(stats) do
            record[k] = v
        end
        database.update(record)
        return record
    else -- Add a new record
        database.insert(stats)
        return database.last()
    end
end
