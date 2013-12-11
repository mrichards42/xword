-- A double-ended queue

local Queue = {}
Queue.__index = Queue

function Queue.new()
    local obj = {_front = 1, _back = 0}
    setmetatable(obj, Queue)
    return obj
end

function Queue:front()
    return self[self._front]
end

function Queue:back()
    return self[self._back]
end

function Queue:push(v)
    self._back = self._back + 1
    self[self._back] = v
end

function Queue:pop()
    if self:empty() then return end
    local value = self:front()
    self[self._front] = nil
    self._front = self._front + 1
    return value
end

function Queue:pushfront(v)
    self._front = self._front - 1
    self[self._front] = v
end

function Queue:popback()
    if self:empty() then return end
    local value = self:back()
    self[self._back] = nil
    self._back = self._back - 1
    return value
end

function Queue:iter()
    local i = self._front - 1
    return function()
        i = i + 1
        return self[i]
    end
end

function Queue:clear()
    for i=self._first,self._last do
        self[i] = nil
    end
    self._front = 1
    self._back = 0
end

-- NB: __len doesn't override the # operator for tables until 5.2
function Queue:length()
    return self._back - self._front + 1
end

function Queue:empty()
    return self._front > self._back
end

return Queue.new
