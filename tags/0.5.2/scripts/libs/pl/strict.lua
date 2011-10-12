--- checks uses of undeclared global variables.
-- All global variables must be 'declared' through a regular assignment
-- (even assigning nil will do) in a main chunk before being used
-- anywhere or assigned to inside a function.
-- @class module
-- @name pl.strict

local getinfo, error, rawset, rawget = debug.getinfo, error, rawset, rawget
local handler,hooked

local mt = getmetatable(_G)
if mt == nil then
  mt = {}
  setmetatable(_G, mt)
elseif mt.hook then
    hooked = true
end

mt.__declared = {}

local function what ()
  local d = getinfo(3, "S")
  return d and d.what or "C"
end

mt.__newindex = function (t, n, v)
  if not mt.__declared[n] then
    local w = what()
    if w ~= "main" and w ~= "C" then
      error("assign to undeclared variable '"..n.."'", 2)
    end
    mt.__declared[n] = true
  end
  rawset(t, n, v)
end

handler = function(t,n)
  if not mt.__declared[n] and what() ~= "C" then
    error("variable '"..n.."' is not declared", 2)
  end
  return rawget(t, n)
end

if not hooked then
    mt.__index = handler
else
    mt.hook(handler)
end


