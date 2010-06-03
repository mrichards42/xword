-- ============================================================================
-- database
-- Works in a background thread to index already downloaded puzzles.
-- ============================================================================

require 'mtask'

database = {}
local P = database

require 'database.functions'
require 'database.messages'
require 'database.cache'


-- open the database on demand
local mt = {}
function mt:__index(name)
    if name == "__db" then
        return database.open()
    end
end
setmetatable(database, mt)

-- Start a background thread if this is the main thread
if task.id() == 1 then
    function database.getdbfilename()
        return xword.GetConfigDir() .. '/database/index.sqlite'
    end

    function database.start_task()
        if database.is_task_running() then
            return
        end
        database.task_id = task.create(
            xword.GetScriptsDir()..'/database/task.lua',
            { database.getdbfilename(),
              [[D:\C++\XWord\trunk\bin\Debug\puzzles]]} )
    end

    function database.is_task_running()
        return database.task_id and task.isrunning(database.task_id)
    end

    database.open()
    database.start_task()
else
    function database.is_task_running()
        return false
    end
end
