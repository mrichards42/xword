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

-- Queue ids
task.UNUSED_QUEUE = -10
task.UNKNOWN_QUEUE = 0
task.DEBUG_QUEUE = -1
task.ERROR_QUEUE = -2



-- ============================================================================
-- post functions
-- ============================================================================

-- Data is serialized before being posted.  When reconstructed, the data will
-- appear as follows: { 'id' = task_id, 'data' = the_data }
-- data can be a number, string, or non-cyclic table of any valid data.
task.post = function(id, data, flags)
    return task_post(id, serialize({id=task.id(), data=data}), flags)
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

task.receive = function(timeout, task_id)
    local task_id = task_id or task.UNKNOWN_QUEUE

    -- See if we already have a message in the queue for the specified task
    if not queues[task_id] then queues[task_id] = Queue:new() end

    local ret = queues[task_id]:pop()
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

        -- If this message is from the specified thread, return
        if data.id == task_id then
            return data.data, flags, rc
        end

        -- Otherwise, add it to the local queue
        if not queues[data.id] then queues[data.id] = Queue:new() end
        queues[data.id]:push({data.data, flags, rc})

        -- Get the next message
        msg, flags, rc = task_receive(0) -- No timeout
    end

    -- If we've gotten here, there are no messages from the specified thread,
    -- and all other messages have been read from the global queue into the
    -- local queue.
    return nil, nil, rc
end


-- Print all messages from a given queue
function task.dump_queue(queue_id)
    msg, flags, rc = task.receive(0, queue_id)
    while rc == 0 do
        print(msg)
        msg, flags, rc = task.receive(0, queue_id)
    end
end

-- Clear all messages from a given queue
function task.clear_queue(queue_id)
    task.queues[queue_id]:clear()
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
    local q = task.queues[queue_id]
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
    args = args or {}
    s = s or ""

    command = [[=
        local success, err = pcall(function()
        -- Set the package.paths
        package.path = arg[1]
        package.cpath = arg[2]

        -- Set the script source argument
        arg[0] = arg[3]
    ]]

    -- Load the script
    if s:sub(1,1) == '=' then -- Script is a string
        -- Remove the equals sign
        s = s:sub(2)
        command = command .. [[
            -- Load the string that was passed to task.create
           local  _mtask_func, _mtask_load_err = loadstring(arg[3])
        ]]
    else -- Script is a file name
        command = command .. [[
            -- Load the file that was passed to task.create
            local _mtask_func, _mtask_load_err = loadfile(arg[3])
        ]]
    end

    -- Run the script with the remaining arguments
    command = command .. [[
            if not _mtask_func then error(_mtask_load_err) end

            -- Remove the extraneous arguments (path, cpath, and script)
            table.remove(arg, 3)
            table.remove(arg, 2)
            table.remove(arg, 1)

            -- Execute the script with the remaining args
            _mtask_func(unpack(arg))

        end)-- end of pcall function

        -- Report any errors
        if not success then
            -- If the script has required mtask, task.post will not post to the
            -- correct thread id, so we need this work-around.
            if task.error then
                task.error(err)
            else
                -- Otherwise we need to manually serialize the data in a format
                -- that our version of task.receive will understand.
                task.post(1,
                    "return { id = -2, data = " ..
                    string.format("%q", tostring(err)) ..
                    " }")
            end
        end
        ]]

    -- Insert our extra args into the args table
    table.insert(args, 1, package.path)
    table.insert(args, 2, package.cpath)
    table.insert(args, 3, s)

    return task_create(command, args)
end
