local database = require 'download.database'
require 'lfs'

local directories = arg
local isPaused = false

-- Check message queue for messages and deal with them
local function process_messages(timeout)
    if task.checkAbort(timeout or 0) then
        database.close()
        error()
    end
    local msg, flag, rc = task.receive(timeout or 0, 1)
    if rc ~= 0 then return end
    if flag == database.CACHE_PUZZLE then
        if type(msg) == "table" then
            database.cachePuzzle(unpack(msg))
        else
            database.cachePuzzle(msg)
        end
    elseif flag == database.PAUSE then
        task.post(1, nil, database.PAUSE)
        isPaused = true
    elseif flag == database.RESUME then
        isPaused = false
    end
    if isPaused then
        -- wait for database.RESUME
        process_messages(-1)
    end
end

-- ----------------------------------------------------------------------------
-- Main thread execution
-- ----------------------------------------------------------------------------

-- This caching all puzzles in the background business is a bit much,
-- especially when the directories start accumulating puzzles
if false then
do
    -- Cache puzzles in the supplied directories
    local cached_puzzles = {}
    for _, dir in ipairs(directories) do
        for f in lfs.dir(dir) do
            -- Check the message queue before each file
            process_messages()
            local result = database.cachePuzzle(dir .. '/' .. f)
            if type(result) == 'table' then
                cached_puzzles[result.filename] = true
            end
        end
    end
    -- Remove database entries that do not exist
    for id, filename in database.__db:urows("SELECT ID, FILENAME from puz_index") do
        if not cached_puzzles[filename] then
            --task.debug("Deleting from database: "..tostring(id)..", "..tostring(filename))
            database.delete(id)
        end
    end
end
end

-- Done with the main part of the task
task.post(1, nil, database.DONE)

-- When everything is done, run a message loop forever
while true do
    process_messages(-1) -- wait forever
end
