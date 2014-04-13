--- A subclass of Task that simplifies task operations on queues
-- @classmod QueueTask

local task = require 'task'

local M = {
    -- Events
    APPEND = -200,
    PREPEND = -201,
    CLEAR = -202,

    EVT_INTERNAL_START = -210, -- The internal item start event
    EVT_INTERNAL_END = -211, -- The internal item end event

    --- Sent by the main task to start an item.
    -- Callback is passed the item.
    EVT_ITEM_START = -220,

    --- Sent by the secondary thread to return results.
    -- Callback is pass the item and any values returned by the thread.
    EVT_ITEM_END = -221,

    --- Sent when the queue has been changed (append, prepend, or clear)
    EVT_QUEUE_UPDATED = -223
}
-- Return the events to secondary tasks
if not task.is_main then return M end

local Task = task.Task
local Queue = require 'task.queue'
local assert_arg = require 'pl.utils'.assert_arg

M.__index = M
function M:__tostring()
    return string.format(
        '%s (%s QueueTask).',
        tostring(self.name),
        self:is_running() and 'Running' or 'Not Running'
    )
end
setmetatable(M, Task)

--- Currently processing item.
-- @field M.current

--- Item queue.
-- @field M.queue

--- Create a new QueueTask.
-- Inherits from `Task`.  
-- @param opts An option table or script.  See `task.new` for most options.
-- @param opts.script A module name or '=' followed by a string chunk.  
--   Must return a function to process each item.
-- @param[opt=true] opts.unique Should items be unique?  See `Queue.new`.
-- @param opts.key If items are tables this key is used to determine equality.  
--   Tables are equal if `t1[key] == t2[key]`.
-- @param[opt=false] opts.queue Should this QueueTask keep a local queue?
--   Usually items are passed to the task, and the queue itself is kept
--   in a secondary thread.  If true, keep a copy of that queue in the
--   main thread
-- @return A new QueueTask
-- @usage
-- -- Create a task that posts back each item in the queue
-- t = QueueTask.new('=return function(item) task.debug(item) end')
-- -- Start the task with two items
-- t:start('hello', 'world')
-- -- Add some items to the queue
-- t:append('a', 'few', 'more', 'strings')
-- t:prepend('these', 'go', 'to', 'the', 'top', 'of', 'the', 'queue')
-- -- Clear any that have not yet been processed
-- t:clear()
function M.new(opts)
    local self = setmetatable(task.new(opts), M)
    if type(opts) == 'table' then
        if opts.unique == nil then opts.unique = true end
        self._unique = opts.unique
        self._key = opts.key
        self.queue = Queue{unique=opts.unique, key=opts.key}
    end
    -- self._check is used to check arguments for prepend and append
    if self._unique and self._key then
        local function has_key(t)
            return t[self._key] ~= nil
        end
        self._check = function(...)
            for i, item in ipairs({...}) do
                -- Assert that this is a table with the required key
                assert_arg(
                    i, item, 'table',
                    has_key, 'missing key: ' .. self._key,
                    4 -- level
                )
            end
        end
    else
        self._check = function() return end
    end
    self._userscript = self._script
    -- This is the helper script that is actually started via Task.start
    self._script = 'task.queue_task_create'
    -- Connect to internal events
    self:connect(M.EVT_INTERNAL_START, function(key)
        self.current = self.queue:get(key)
        if self.current ~= nil then
            self:send_event(M.EVT_ITEM_START, self.current)
        end
    end)
    self:connect(M.EVT_INTERNAL_END, function(key, ...)
        self.queue:pop(key)
        -- Call send_event *last* to make sure we avoid a wxYield race
        -- See Task:connect for details.
        local obj = self.current
        self.current = nil
        if obj ~= nil then
            self:send_event(M.EVT_ITEM_END, obj, ...)
        end
    end)
    return self
end

--- Create a new QueueTask.
-- @see Task:start
function M:start(...)
    -- Call Task:start with queue_task_create as the _script, and the script
    -- passed in the constructor as the first argument.
    -- The same as it we called task.new('queue_task_create'):start(script, ...)
    local ret = Task.start(self, self._userscript, self._unique, self._key)
    -- Append the first arguments
    if select('#', ...) > 0 then
        self:append(...)
    end
    return ret
end

--- Append items to the task's queue
-- @param ... Items
function M:append(...)
    self._check(...)
    for _, item in ipairs({...}) do
        self.queue:push(item)
    end
    self:post(M.APPEND, ...)
    self:send_event(M.EVT_QUEUE_UPDATED)

end

--- Prepend items to the front of the task's queue
-- @param ... Items
function M:prepend(...)
    self._check(...)
    local items = {...}
    for i=#items,1,-1 do
        self.queue:pushfront(items[i])
    end
    self:post(M.PREPEND, ...)
    self:send_event(M.EVT_QUEUE_UPDATED)
end

--- Clear all items from the task's queue.
function M:clear()
    self:post(M.CLEAR)
    self.queue:clear()
    self:send_event(M.EVT_QUEUE_UPDATED)
end

return M