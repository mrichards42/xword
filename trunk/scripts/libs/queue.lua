-- Adapting the queue example from PiL as a class

Queue = {}
function Queue:new()
    local obj = {first = 0, last = -1}
    self.__index = self
    setmetatable(obj, self)
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
  self[last] = nil -- allow garbage collection
  self.last = last - 1
  return value
end

function Queue:clear()
  while self.first <= self.last do
    self[self.last] = nil -- garbage collect this value
    self.last = self.last - 1
  end
  self.first = 0
  self.last = -1
end

-- Using __len doesn't override the # operator for tables
function Queue:length()
    return self.last - self.first + 1
end
