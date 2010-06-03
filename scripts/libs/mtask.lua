-- ============================================================================
-- Modifications to the LuaTask library
--     Allow LuaTask to be used with multiple add-ons:
--         - task.post prepends the id of the calling task to the message for
--           identification.
--         - task.receive takes an additional task_id parameter, and will only
--           return messages from the given task.
--           if task_id is omitted or is -1, return the first message from a
--           thread using LuaTask directly (not through this post/receive API).
--
--     This is sort of hackish and should incur a small performance penalty,
--     but otherwise, only one add-on could use LuaTask without the message
--     queues getting tangled.
--
--    Preserve package.path and package.cpath when creating a new thread.
-- ============================================================================

require 'task'
require 'queue'
require 'table'
require 'serialize'

-- ============================================================================
-- local functions and data
-- ============================================================================

local task_post = task.post
local task_receive = task.receive
local task_create = task.create
local task_id = task.id
local task_isrunning = task.isrunning

-- Queue ids
task.UNUSED_QUEUE = -10
task.UNKNOWN_QUEUE = -1
task.DEBUG_QUEUE = -2
task.ERROR_QUEUE = -3


-- ============================================================================
-- Unique thread ids
-- ============================================================================
-- Functions on the posting end need to convert using find_id
-- Functions on the receiving end need to convert using find_name

task.task_aliases = {}
local task_aliases = task.task_aliases
local last_task_id = 1
local function new_id()
    last_task_id = last_task_id + 1
    return "mtask."..tostring(last_task_id)
end

local function find_id(name)
    if type(name) == "number" then return name end
    return task.find(name)
end

local function find_name(id)
    if type(id) == "string" then return id end
    return task_aliases[id]
end

local function register_name(id, name)
    task_aliases[id] = name
end

register_name(1, 1) -- main thread is always 1
register_name(task.UNKNOWN_QUEUE, "mtask.unknown")
register_name(task.DEBUG_QUEUE, "mtask.debug")
register_name(task.ERROR_QUEUE, "mtask.error")
register_name(task.UNUSED_QUEUE, "mtask.unused")

function task.isrunning(id)
    return task_isrunning(find_id(id))
end


-- ============================================================================
-- post functions
-- ============================================================================

-- Data is serialized before being posted.  When reconstructed, the data will
-- appear as follows: { 'id' = mtask_id, 'data' = the_data }
-- data can be a number, string, or non-cyclic table of any valid data.
task.post = function(id, data, flags)
    return task_post(find_id(id), serialize({id=task.id(), data=data}), flags)
end

-- Post with id 1 to the specified queue.
-- This is used to circumvent custom task.post behavior, which posts with
-- a given id to the queue designated for the current task.
task.post_to_queue = function(queue_id, data, flags)
    return task_post(1, serialize({id=queue_id, data=data}), flags)
end

function task.error(data) task.post_to_queue(task.ERROR_QUEUE, data) end

function task.debug(data) task.post_to_queue(task.DEBUG_QUEUE, data) end



-- ============================================================================
-- receive functions
-- ============================================================================

-- A table that holds the messages for each task as we receive them.
local queues = {}
task.queues = queues

-- Recieve the next message in the queue, but don't remove it
task.peek = function(timeout, task_id)
    local task_id = find_name(task_id) or task.UNKNOWN_QUEUE

    -- See if we already have a message in the queue for the specified task
    if not queues[task_id] then queues[task_id] = Queue:new() end

    local ret = queues[task_id]:get_last()
    -- Return the message if there is one
    if ret then
        return unpack(ret)
    end

    -- No message in the local queue, look to the global task queue.

    -- Find the first message (using the timeout if specified)
    local msg, flags, rc = task_receive(timeout)

    -- Read messages until we get one from the specified task, or we run
    -- out of messages.
    while rc == 0 do
        local success, data
        -- Serialized messages must begin with "return "
        if msg:sub(1, 7) == 'return ' then
            -- Read the serialized message
            success, data = pcall(loadstring(msg))
        end
        -- If the message cannot be deserialized, assume it came from an
        -- unknown thread.
        if type(data) ~= 'table' then
            data = {id = task.UNKNOWN_QUEUE, data = msg}
        end

        -- Add the message to the local queue
        data.id = find_name(data.id)
        if not queues[data.id] then queues[data.id] = Queue:new() end
        queues[data.id]:push({data.data, flags, rc})

        -- If this message is from the specified thread, return the message
        if data.id == task_id then
            return data.data, flags, rc
        end

        -- Get the next message
        msg, flags, rc = task_receive(0) -- No timeout
    end

    -- If we've gotten here, there are no messages from the specified thread,
    -- and all other messages have been read from the global queue into the
    -- local queue.
    return nil, nil, rc
end

-- Receive the next message in the queue and remove it
function task.receive(timeout, task_id)
    local task_id = find_name(task_id)
    local msg, flags, rc = task.peek(timeout, task_id)
    if rc == 0 then
        queues[task_id]:pop()
    end
    return msg, flags, rc
end

-- Print all messages from a given queue
function task.dump_queue(queue_id)
    task.flush_queues()
    for val in queues[find_name(queue_id)]:iter() do
        local msg, flags, rc = unpack(val)
        print(msg)
    end
    task.clear_queue(queue_id)
end

-- Clear all messages from a given queue
function task.clear_queue(queue_id)
    task.queues[find_name(queue_id)]:clear()
end

-- Push all messages into their respecitve queues
function task.flush_queues()
    -- task.receive will read messages until it gets a message from the
    -- given queue / thread or it runs out of messages.  There should never
    -- be messages in the UNUSED_QUEUE, so this just reads until we run
    -- out of all messages.
    msg, flags, rc = task.receive(0, task.UNUSED_QUEUE)
end

-- Flush messages and return the count of messages for a given queue
function task.pending(queue_id)
    task.flush_queues()
    local q = task.queues[find_name(queue_id)]
    if q then return q:length() else return 0 end
end

-- Dump the debug queue
function task.dump_debug()
    print '===============      DEBUG       ==============='
    task.dump_queue(task.DEBUG_QUEUE)
    print '===============   END OF DEBUG   ==============='
end

-- Dump the error queue
function task.dump_errors()
    print '===============      ERRORS      ==============='
    task.dump_queue(task.ERROR_QUEUE)
    print '===============   END OF ERRORS  ==============='
end




-- ============================================================================
-- create thread function
-- ============================================================================

task.create = function(s, args)
    local task_name = new_id()
    args = args or {}
    s = s or ""

    local command = [[=
        local success, err = pcall(function()
        -- Set the package.paths
        package.path = arg[1]
        package.cpath = arg[2]

        require "mtask"

        -- Set this task's unique id
        function task.id()
            return "]]..task_name..[["
        end
        task.register(task.id())

        -- Set the script source argument
        arg[0] = arg[3]
    ]]

    -- Load the script
    if s:sub(1,1) == '=' then -- Script is a string
        -- Remove the equals sign
        s = s:sub(2)
        command = command .. [[
            -- Load the string that was passed to task.create
           mtask._mtask_func, mtask._mtask_load_err = loadstring(arg[3])
        ]]
    else -- Script is a file name
        command = command .. [[
            -- Load the file that was passed to task.create
            mtask._mtask_func, mtask._mtask_load_err = loadfile(arg[3])
        ]]
    end

    -- Run the script with the remaining arguments
    command = command .. [[
            if not mtask._mtask_func then error(mtask._mtask_load_err) end

            -- Remove the extraneous arguments (path, cpath, and script)
            table.remove(arg, 3)
            table.remove(arg, 2)
            table.remove(arg, 1)

            -- Execute the script with the remaining args
            mtask._mtask_func(unpack(arg))

        end)-- end of pcall function

        -- Report any errors
        if not success then
            task.error(err)
        end
        ]]

    -- Insert our extra args into the args table
    table.insert(args, 1, package.path)
    table.insert(args, 2, package.cpath)
    table.insert(args, 3, s)

    local id = task_create(command, args)
    if id > 0 then
        register_name(id, task_name)
        return task_name
    end
    -- If the task did not create, return the error code
    return id
end


-- ============================================================================
-- Frame closing
-- ============================================================================
task.ABORT = -100

if task.id() == 1 then
    function task.abort(id)
        task.post(id, "", task.ABORT)
    end

    if xword.frame then
        xword.frame:Connect(wx.wxEVT_CLOSE_WINDOW,
            function(evt)
                local sw = wx.wxStopWatch()
                local isMsgShown = false
                while true do
                    if not isMsgShown and sw:Time() > 2000 then
                        isMsgShown = true
                        wx.wxBusyInfo("Waiting for threads to complete...")
                    end
                    -- Give our threads some time to exit before we kill them.
                    if sw:Time() > 5000 then break end
                    local shouldBreak = true
                    for id, _ in pairs(task.list()) do
                        if id ~= 1 then
                            shouldBreak = false
                            task.abort(id)
                        end
                    end
                    -- Main thread is the only one left
                    if shouldBreak then break end
                end
                evt:Skip()
            end
        )
    end
else
    function task.checkAbort(timeout, cleanupFunc)
        local function doCheck(msg, flag, rc)
            if rc == 0 and flag == task.ABORT then
                if cleanupFunc then
                    cleanupFunc()
                end
                error("ABORTING THREAD")
            end
        end

        -- Check the first message in the queue (with a timeout)
        local msg, flag, rc = task.peek(timeout or 0, 1)
        doCheck(msg, flag, rc)

        -- Check all other messages in the queue
        task.flush_queues()
        for val in task.queues[1]:iter() do
            local msg, flag, rc = unpack(val)
            doCheck(msg, flag, rc)
        end
    end
end