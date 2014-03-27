--- A double-ended queue.
-- @classmod Queue

local assert_arg = require 'pl.utils'.assert_arg

local Queue = {}
Queue.__index = Queue

local UniqueQueue = {} -- The unique queue metatable
UniqueQueue.__index = UniqueQueue
setmetatable(UniqueQueue, Queue)

--- Create a new Queue.
-- @param[opt] opts An argument table
-- @param opts.unique If true, items will be unique.  
--   `Queue:push(item)` will only push if the item is not already in the queue.  
--   `Queue:pushfront(item)` will remove the item if it already exists in the queue.
-- @param opts.key If values are tables this key is used to determine equality.  
--   Tables are equal if `t1[key] == t2[key]`.
-- @return A Queue
-- @usage
-- Queue = require 'task.queue'
-- q = Queue()
-- for i=1,10 do q:push(i) end
-- -- print all values
-- for i in q:iter() do print(i) end
-- -- pop the first value value (1)
-- q:pop()
-- -- pop the last value value (10)
-- q:popback()
function Queue.new(opts)
    local self = {_front = 1, _back = 0}
    if opts and opts.unique then
        return UniqueQueue.new(self, opts)
    else
        return setmetatable(self, Queue)
    end
end

--- Retrieve an item by index.
function Queue:at(idx)
    return self[self._front + idx-1]
end

--- Retrieve the first item in the queue.
function Queue:front()
    return self[self._front]
end

--- Retrieve the last item in the queue.
function Queue:back()
    return self[self._back]
end

--- Remove and return the first item from the queue.
function Queue:pop()
    if self:empty() then return end
    local value = self:front()
    self[self._front] = nil
    self._front = self._front + 1
    return value
end

--- Remove and return the last item from the queue.
function Queue:popback()
    if self:empty() then return end
    local value = self:back()
    self[self._back] = nil
    self._back = self._back - 1
    return value
end

--- Add an item to the back of the queue.
-- @param item The item
function Queue:push(item)
    self._back = self._back + 1
    self[self._back] = item
end

--- Add an item to the front of the queue.
-- @param item The item
function Queue:pushfront(item)
    self._front = self._front - 1
    self[self._front] = item
end

--- Iterate over items from front to back.
function Queue:iter()
    local i = self._front - 1
    return function()
        i = i + 1
        return self[i]
    end
end

--- Remove all items from the queue.
function Queue:clear()
    for i=self._front,self._back do
        self[i] = nil
    end
    self._front = 1
    self._back = 0
end

--- Retrieve the length of the queue.
-- If using lua 5.2, Queue:__len is also defined, so the # operator can be used.
function Queue:length()
    return self._back - self._front + 1
end

if _VERSION:find(" 5.2") then
    Queue.__len = Queue.length
end

function Queue:empty()
    return self:length() == 0
end


-- The unique Queue.
-- This should only be called from Queue.new
function UniqueQueue.new(self, opts)
    local key = opts.key
    if key then
        self._hash = function(t)
            assert_arg(1, t, 'table', nil, nil, 4)
            return t[key]
        end
    else
        self._hash = function(t) return t end
    end
    -- Map a value's hash to its index in the queue
    self._map = {}
    -- We can't rely on self._front and self._back since values might be nil
    self._length = 0
    setmetatable(self, UniqueQueue)
    return self
end

local function post_insert(self, hash, idx)
    -- Compute and store the hash
    if hash == nil then return end
    -- If this value already exists, delete it
    local i = self._map[hash]
    if i then
        self[i] = nil
    else
        -- Add to length, unless we just removed an item
        self._length = self._length + 1
    end
    -- Store the hash
    self._map[hash] = idx
end

function UniqueQueue:push(v)
    if v == nil then return end
    -- Don't add items to the back of the queue if they already exist
    local hash = self._hash(v)
    if self._map[hash] then return end
    -- Push
    Queue.push(self, v)
    post_insert(self, hash, self._back)
end

function UniqueQueue:pushfront(v)
    if v == nil then return end
    Queue.pushfront(self, v)
    post_insert(self, self._hash(v), self._front)
end

function UniqueQueue:front()
    for i=self._front,self._back do
        if self[i] then return self[i] end
    end
end

function UniqueQueue:back()
    for i=self._back,self._front,-1 do
        if self[i] then return self[i] end
    end
end

-- Remove an item via pop or popback
local function do_remove(self, func)
    -- Get the first non-nil value
    if self:empty() then return end
    local v
    repeat v = func(self) until v ~= nil
    -- Remove from the hash map
    self._map[self._hash(v)] = nil
    -- Decrement length
    self._length = self._length - 1
    return v
end

function UniqueQueue:pop(hash)
    -- If we get a hash, find and remove just that value
    if hash then
        local idx = self._map[hash]
        local v = idx and self[idx]
        if (not idx) or (v == nil) then return end
        self._map[hash] = nil
        self[idx] = nil
        if idx == self._front then self._front = self._front + 1 end
        self._length = self._length - 1
        return v
    else
        return do_remove(self, Queue.pop)
    end
end

function UniqueQueue:popback()
    return do_remove(self, Queue.popback)
end

-- Return a value by hash
function UniqueQueue:get(hash)
    local idx = self._map[hash]
    return idx and self[idx]
end

function UniqueQueue:iter()
    -- If this queue has no nils, use the standard iteration function
    if self:isnormal() then
        return Queue.iter(self)
    end
    -- Otherwise iterate and remove nils
    local offset = 0
    local i=self._front - 1
    return function()
        local v
        while true do
            i = i + 1
            -- Check to see if we're done
            if i > self._back then
                -- Done: no nils, so we can reset self._back based on length
                self._back = self._front + self._length - 1
                return nil
            end
            v = self[i]
            if v ~= nil then
                -- We have a non-nil item
                if offset > 0 then
                    -- Remove nils between the previous item and this one
                    local new_idx = i-offset
                    self[new_idx] = v
                    self._map[self._hash(v)] = new_idx
                    self[i] = nil
                end
                return v
            else
                -- skip nils
                offset = offset + 1
            end
        end
    end
end

function UniqueQueue:normalize()
    if not self:isnormal() then
        for _ in self:iter() do end
    end
end

function UniqueQueue:isnormal()
    return Queue.length(self) == self._length
end

function UniqueQueue:clear()
    Queue.clear(self)
    for k,_ in pairs(self._map) do
        self._map[k] = nil
    end
    self._length = 0
end

function UniqueQueue:length()
    return self._length
end

-- Return the constructor
return Queue.new
