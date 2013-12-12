--- A subclass of Task that simplifies task operations on queues
-- @classmod QueueTask

local task = require 'task'

local M = {
    -- Events
    APPEND = -200,
    PREPEND = -201,
    CLEAR = -202,
}
-- Return the events to secondary tasks
if not task.is_main then return M end

-- The original Task metatable
local Task = task.Task

M.__index = M
setmetatable(M, Task)

--- Create a new QueueTask.
-- Inherits from `Task`.  
-- @param opts An option table or script.  See `task.new` for most options.
-- @param opts.script A module name or '=' followed by a string chunk.  
--   Must return a function to process each item.
-- @param[opt=true] opts.unique Should items be unique?  See `Queue.new`.
-- @param opts.key If items are tables this key is used to determine equality.  
--   Tables are equal if `t1[key] == t2[key]`.
-- @return A new QueueTask
-- @usage
-- -- Create a task that posts back each item in the queue
-- t = QueueTask.new('=return function(item) task.debug(item) end')
-- -- Start the task with two items
-- t:start('hello', 'world')
-- Add some items to the queue
-- t:append('a', 'few', 'more', 'strings')
-- t:prepend('these', 'go', 'to', 'the', 'top', 'of', 'the', 'queue')
-- Clear any that have not yet been processed
-- t:clear()
function M.new(opts)
    local self = setmetatable(task.new(opts), M)
    if type(opts) == 'table' then
        if opts.unique == nil then opts.unique = true end
        self._unique = opts.unique
        self._key = opts.key
    else
        self._unique = true
    end
    -- self._check is used to check arguments for prepend and append
    local assert_arg = require 'pl.utils'.assert_arg
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
    self:post(M.APPEND, ...)
end

--- Prepend items to the front of the task's queue
-- @param ... Items
function M:prepend(...)
    self._check(...)
    self:post(M.PREPEND, ...)
end

--- Clear all items from the task's queue
function M:clear()
    self:post(M.CLEAR)
end

return M