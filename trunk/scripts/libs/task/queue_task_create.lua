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
        -- Get an item, but keep it in the queue to prevent duplicates
        local item = queue:front()
        if item == 'abort' then -- Check for abort
            return
        elseif item then
            -- NB: QueueTask will convert INTERNAL_START and INTERNAL_END into
            -- the actual item, not the key.
            task.post(msg.EVT_INTERNAL_START, key and item[key] or item)
            -- Do something with the item
            local results = {xpcall(function() return func(item) end, debug.traceback)}
            local success = table.remove(results, 1)
            -- Post item[key] or item, and the results
            task.post(msg.EVT_INTERNAL_END, key and item[key] or item, unpack(results))
            -- Report errors
            if not success then
                task.error(results[1])
            end
            -- If the callback tried task.check_abort() we should break here
            if task.should_abort then
                return
            end
            -- Done with this item, now we can remove it.
            queue:pop()
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