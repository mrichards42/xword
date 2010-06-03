require 'date'
require 'lfs'
require 'luapuz'

-- ----------------------------------------------------------------------------
-- Puzzle statistics
-- ----------------------------------------------------------------------------
-- Return started (bool), comlete (pct), time (secs)
local function puzzleStats(filename)
    local success, p = pcall(puz.Puzzle, filename)
    if not success then return end
    local blank = 0
    local total = 0
    local started = p.Time > 0
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
                                             puz.FLAG_RED +
                                             puz.FLAG_BLACK)
                end
            elseif not started then
                started = true
            end
        end
        square = square:Next()
    end
    return started,
           math.floor(100 - blank / total * 100 + 0.5), -- Round
           p.Time
end


-- ----------------------------------------------------------------------------
-- Cache a puzzle with the given filename
-- ----------------------------------------------------------------------------
function database.cachePuzzle(filename, source, date)
    -- Check to see if the file exists
    local attr = lfs.attributes(filename)
    if not attr then return end

    -- Check to see if this is a file or a directory
    filename = filename:gsub('\\', '/')
    local basename = filename:match(".*/([^/%.]+)%..*")
    if not basename then return end

    -- Check to see if we can load the file
    if not puz.Puzzle.CanLoad(filename) then return end

    -- Check file modification time: if it has not changed, we're done
    local record = database.find_one(basename)
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
        record:commit()
        return record
    else -- Add a new record
        return database.insert(stats)
    end
end
