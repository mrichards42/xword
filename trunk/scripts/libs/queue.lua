-- Adapting the queue example from PiL as a class

Queue = {}
function Queue:new()
    local obj = {first = 0, last = -1}
    setmetatable(obj, self)
    self.__index = self
    return obj
end

function Queue:push(value)
  local first = self.first - 1
  self.first = first
  self[first] = value
end

function Queue:pop()
  local last = self.last
  if self.first > last then return nil end -- queue is empty
  local value = self[last]
  self[last] = nil         -- to allow garbage collection
  self.last = last - 1
  return value
end
