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
    -- If we can't save this format, assume that the user hasn't started
    -- solving the puzzle
    if not puz.Puzzle.CanSave(filename) then
        return false, 0, 0
    end

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

-- NB: These need to be updated every time we add a new file format.
-- Ideally this would be taken care of in luapuz.
local save_extensions = { 'puz', 'xpf', 'xml', 'jpz' }
local load_extensions = { 'ipuz' }

local function get_last_modified(basename)
    -- Look for files that we can save (i.e. files that could be partially
    -- solved), and find the most recently modified.
    local base = path.join(download.localfolder, basename)
    local last_attr = { modification = 0 }
    local last_filename
    for _, ext in ipairs(save_extensions) do
        local attr = lfs.attributes(base..'.'..ext)
        if attr and attr.modification > last_attr.modification then
            last_filename = base..'.'..ext
            last_attr = attr
        end
    end
    return last_filename, last_attr
end



function database.cachePuzzle(basename)
    local filename, attr = get_last_modified(basename)

    if not filename then
        local base = path.join(download.localfolder, basename)
        -- Look for other puzzles that we can load but not save.
        -- Assume that files we can load but not save have not been solved.
        for _, ext in ipairs(load_extensions) do
            local attr = lfs.attributes(base..'.'..ext)
            if attr and attr.mode == 'file' then
                -- Return empty stats
                return {
                    basename = basename,
                    filename = base..'.'..ext,
                    modified = attr.modification,
                    started = false,
                    complete = -1,
                    time = -1,
                }
            end
        end
    end

    -- Check to see if the file exists
    if not attr or attr.mode ~= 'file' then
        return { basename = basename }
    end

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

    -- Try to load the puzzle
    local started, complete, time = puzzleStats(filename)
    if started == nil then
        return { basename = basename }
    end

    -- Do the caching
    local stats = {
        basename = basename,
        filename = filename,
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
