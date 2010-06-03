require 'mtask'
require 'database'
require 'lfs'

local dbfilename = arg[1]
table.remove(arg, 1)
local directories = arg

-- Redefine the database open function to use the supplied filename
local database_open = database.open
database.open = function()
    return database_open(dbfilename)
end

local function onAbort()
    database.close()
end

-- Check message queue for messages and deal with them
local function processMessages()
    task.checkAbort(0, onAbort)
    local msg, flag, rc = task.receive(0, 1) -- no timeout; from main thread
    if rc ~= 0 then return end
    if flag == database.CACHE_PUZZLE then
        database.cachePuzzle(msg)
    end
end

-- ----------------------------------------------------------------------------
-- Main thread execution
-- ----------------------------------------------------------------------------

-- Check for puzzles in the supplied directories
for _, dir in ipairs(directories) do
    for f in lfs.dir(dir) do
        -- Check the message queue before each file
        processMessages()
        task.debug(dir .. '/' .. f)
        database.cachePuzzle(dir .. '/' .. f)
    end
end

-- When everything is done, run a message loop forever
while true do
    -- wait forever; from main thread
    task.checkAbort(-1, onAbort)
    local msg, flag, rc = task.receive(-1, 1)
    if rc == 0 then
        task.debug("MESSAGE LOOP: "..flag.."; "..tostring(msg))
        if flag == database.CACHE_PUZZLE then
            task.debug("Caching puzzle from task: "..msg)
            database.cachePuzzle(msg)
        end
    end
end
