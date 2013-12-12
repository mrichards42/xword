-- This file is used to start a QueueTask
-- arg[1] = the script to run.
-- arg[2,3] = arguments passed to Queue{}
local script, unique, key = unpack(arg)

-- Messages sent to this task
local msg = require 'task.queue_task'

-- The message queue
local Queue = require 'task.queue'
local queue = Queue{unique=unique, key=key}

--- Process messages received from the calling thread
-- @param[opt=0] timeout -1 means wait forever
-- @return 'abort' if task.check_abort() returned true
local function process_messages(timeout)
    while true do
        if task.check_abort() then return 'abort' end
        local task_id, evt_id, data = task.receive(timeout or 0)
        if not task_id then return end -- No messages
        if evt_id == task.EVT_ABORT then
            return 'abort'
        elseif evt_id == msg.APPEND then
            for _, item in ipairs(data) do
                queue:push(item)
            end
        elseif evt_id == msg.PREPEND then
            -- Push prepended data backwards
            for i=#data,1,-1 do
                queue:pushfront(data[i])
            end
        elseif evt_id == msg.CLEAR then
            queue:clear()
        end
        timeout = 0 -- Check for more messages
    end
end

--- Loop through each item in the queue.
-- @param func The function used to process each item.
local function loop_through_queue(func)
    while true do
        -- Get the first item in the queue
        local data = queue:pop()
        if data then
            -- Do something with it
            local success, err = xpcall(function() func(data) end, debug.traceback)
            -- Report errors but continue processing
            if not success then
                task.error(err)
            end
            -- If the callback tried task.check_abort() we should break here
            if task.should_abort then
                return
            end
        else
            -- Nothing to do until the queue has data.
            -- Wait forever for new messages
            if process_messages(-1) == 'abort' then return end
        end
        -- Check for new messages after each item in the queue
        if process_messages() == 'abort' then return end
    end
end

-- Load the script, which should return a function for loop_through_queue
local func, err = task.load(script)
if func then
    loop_through_queue(func())
else
    task.error(err)
end