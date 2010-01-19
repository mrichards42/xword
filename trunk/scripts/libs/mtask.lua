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
-- ============================================================================

assert(require 'task')
assert(require 'queue')
assert(require 'table')


-- ============================================================================
-- Serialization
-- ============================================================================

-- Serialize function adapted from PiL 12.1.1
-- This will serialize simple non-cyclic tables (nested tables OK)
local function serialize (obj)
    local ret = {}
    if type(obj) == "number" then
        table.insert(ret, tostring(obj))
    elseif type(obj) == "string" then
        table.insert(ret, string.format("%q", obj))
    elseif type(obj) == "table" then
        table.insert(ret, '{')
        for k,v in pairs(obj) do
            table.insert(ret, '['..serialize(k)..']='..serialize(v)..',')
        end
        table.insert(ret, '}')
    else
        error("cannot serialize a " .. type(obj))
    end
    return table.concat(ret)
end

-- This of course has the potential to be horribly terribly unsafe.
-- However, in order to expliot this, the user would have to install an
-- untrusted add-on that includes a call to task.post() with a string
-- containing arbitrary code.  At that point, the add-on could just execute
-- whatever code it wants to, since we're using lua as a scripting language.
-- So effectively, this isn't a security hole.
local function deserialize(str)
    return loadstring('return '..str)()
end


-- ============================================================================
-- post function
-- ============================================================================

local task_post = task.post
local task_receive = task.receive

-- Data is serialized before being posted.  When reconstructed, the data will
-- appear as follows: { 'id' = task_id, 'data' = the_data }
-- data can be a number, string, or non-cyclic table of any valid data.
task.post = function(id, data, flags)
    return task_post(id, serialize({id=task.id(), data=data}), flags)
end

-- ============================================================================
-- receive function
-- ============================================================================

-- A table that holds the messages for each task as we receive them.
local queues = {}
task.queues = queues

task.receive = function(timeout, task_id)
    local task_id = task_id or -1

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
        -- Read the serialized message
        local success, data = pcall(deserialize, msg)
        -- If the message cannot be deserialized, assume it came from an
        -- unknown thread.
        if not success then
            data = {id = -1, data = msg}
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
