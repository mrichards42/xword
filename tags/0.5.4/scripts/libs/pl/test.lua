--------------------------------------------
--- Useful test utilities.
local tablex = require 'pl.tablex'
local utils = require 'pl.utils'
local pretty = require 'pl.pretty'
local path = require 'pl.path'
local print,type = print,type
local clock = os.clock
local io,debug = io,debug
module 'pl.test'

local function complain (x,y)
    local i = debug.getinfo(3)
    io.stderr:write('assertion failed at '..path.basename(i.short_src)..':'..i.currentline..'\n')
    print("x:",pretty.write(x,' ',true))
    print("y:",pretty.write(y,' ',true))
    utils.quit(1,"these values were not equal")
end

--- like assert, except takes two arguments that must be equal.
-- If they are plain tables, it will use tablex.deepcompare.
-- @param x any value
-- @param y a value equal to x
function asserteq (x,y)
    if x ~= y then
        local res = false
        if type(x) == 'table' and type(y) == 'table' then
            res = tablex.deepcompare(x,y)
        end
        if not res then
            complain(x,y)
        end
    end
end

--- a version of asserteq that takes two pairs of values.
-- @param x1 any value
function asserteq2 (x1,x2,y1,y2)
    if x1 ~= y1 then complain(x1,y1) end
    if x2 ~= y2 then complain(x2,y2) end
end

--- Time a function. Call the function a given number of times, and report the number of seconds taken,
-- together with a message.  Any extra arguments will be passed to the function.
-- @param msg a descriptive message
-- @param n number of times to call the function
-- @param fun the function
function timer(msg,n,fun,...)
    local start = clock()
    for i = 1,n do fun(...) end
    utils.printf("%s: took %7.2f sec\n",msg,clock()-start)
end

