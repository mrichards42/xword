-- This is a generic task template
-- Use this template for tasks that use a list to queue operations

require 'download.messages'
local OrderedSet = require 'download.ordered_set'

local queue = OrderedSet()
queue:append(unpack(arg))

local equal = function (a, b)
    return a == b
end

local function process_messages(timeout)
    -- Process messages recieved from the calling thread
    while true do
        if task.checkAbort() then return 'abort' end
        local data, flag, rc = task.receive(timeout or 0, 1)
        if rc ~= 0 then return end -- No messages
        if flag == task.ABORT then
            return 'abort'
        elseif flag == download.CLEAR then
            queue:clear()
        elseif flag == download.PREPEND then
            queue:prepend(unpack(data))
        elseif flag == download.APPEND then
            queue:append(unpack(data))
        end
        timeout = 0 -- Respect the timeout just the first time
    end
end

-- Loop through the queue calling callback with each piece of data
-- Call
function loop_through_queue(callback, hash)
    if hash then
        queue.hash = hash
    end
    while true do
        -- Get the top item from the queue
        local data = table.remove(queue, 1)
        if data then
            -- Do something with it
            local success, err = xpcall(
                function () callback(data) end,
                -- error handler . . . separates aborts
                function (e)
                    if type(e) == 'table' and e[1] == 'abort' then
                        return 'abort'
                    else
                        return debug.traceback(e)
                    end
                end)
            if not success then
                if err == 'abort' then
                    break
                else
                    task.error(err)
                    break
                end
            end
        else
            -- Wait forever for new messages
            if process_messages(-1) == 'abort' then break end
        end
        -- Check for new messages
        if process_messages() == 'abort' then break end
    end
end
