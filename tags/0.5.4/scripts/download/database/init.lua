-- ============================================================================
-- database
-- Works in a background thread to index already downloaded puzzles.
-- ============================================================================

require 'wxtask'

-- So this can be used in a thread
if not download then download = {} end
download.database = {}
local database = download.database

require 'download.database.functions'
require 'download.database.cache'

-- Messages
database.CACHE_PUZZLE = 1
database.PAUSE = 2
database.RESUME = 3
database.DONE = 4


-- open the database on demand
local mt = {}
function mt:__index(name)
    if name == "__db" then
        return database.open()
    end
end
setmetatable(database, mt)

local join = require 'pl.path'.join
database.dbfilename = join(xword.configdir, 'download', 'index.sqlite')

-- Start a background thread if this is the main thread
if task.id() == 1 then
    -- maps basename to callback
    local cache_callbacks = {}
    local function on_cache_puzzle(record)
        local callback = cache_callbacks[record.basename]
        if callback then
            callback(record)
            cache_callbacks[record.basename] = nil
        end
    end

    function database.start_task()
        if database.is_task_running() then
            return
        end
        database.task_id = task.create(
            'download.database.task',
            { download.localfolder } )
        task.handleEvents(database.task_id, { [database.CACHE_PUZZLE] = on_cache_puzzle })
    end

    function database.end_task()
        if database.is_task_running() then
            task.abort(database.task_id)
        end
    end

    function database.is_task_running()
        return database.task_id and task.isrunning(database.task_id)
    end

    function database.cacheInBackground(basename, callback)
        database.start_task()
        cache_callbacks[basename] = callback
        task.post(database.task_id, basename, database.CACHE_PUZZLE)
    end

    -- Pause and resume the task so that we can do database transactions from
    -- the current thread
    function database.pauseTask()
        if database.is_task_running() then
            task.post(database.task_id, nil, database.PAUSE)
            database.__db:busy_timeout(500)
        end
    end
    function database.resumeTask()
        if database.is_task_running() then
            task.post(database.task_id, nil, database.RESUME)
        end
        database.__db:busy_handler(nil)
    end

    -- Register a cleanup function
    local function cleanup()
        database.end_task()
        database.close()
    end

    if xword then
        xword.OnCleanup(cleanup)
        function database.init()
            database.open()
            database.start_task()
        end
        function database.uninit()
            cleanup()
        end
    end
else
    function database.is_task_running()
        return false
    end
end

return database