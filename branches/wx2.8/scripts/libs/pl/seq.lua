------------------------------------------
--- Manipulating sequences as iterators.
-- functions & algorithms
-- Steve Donovan, 2007-2009

local next,assert,type,pairs,tonumber,type,setmetatable,getmetatable,_G = next,assert,type,pairs,tonumber,type,setmetatable,getmetatable,_G
local strfind = string.find
local strmatch = string.match
local format = string.format
local mrandom = math.random
local remove,tsort,tappend = table.remove,table.sort,table.insert
local io = io
local utils = require 'pl.utils'
local function_arg = utils.function_arg
local _List = utils.stdmt.List
local _Map = utils.stdmt.Map

module("pl.seq")

local seq = _G.pl.seq

-- given a number, return a function(y) which returns true if y > x
-- @param x a number
function greater_than(x)
  return function(v)
    return tonumber(v) > x
  end
end

-- given a number, returns a function(y) which returns true if y < x
-- @param x a number
function less_than(x)
  return function(v)
    return tonumber(v) < x
  end
end

-- given any value, return a function(y) which returns true if y == x
-- @param x
function equal_to(x)
  if type(x) == "number" then
    return function(v)
      return tonumber(v) == x
    end
  else
    return function(v)
      return v == x
    end
  end
end

--- given a string, return a function(y) which matches y against the string.
-- @param a string
function matching(s)
  return function(v)
     return strfind(v,s)
  end
end

--- sequence adaptor for a table.   Note that if any generic function is
-- passed a table, it will automatically use seq.list()
-- @param t a list-like table
-- @usage sum(list(t)) is the sum of all elements of t
-- @usage for x in list(t) do...end
function list(t)
  local key,value
  return function()
    key,value = next(t,key)
    return value
  end
end

--- return the keys of the table.
-- @param t a list-like table
-- @return iterator over keys
function keys(t)
  local key,value
  return function()
    key,value = next(t,key)
    return key
  end
end

local function default_iter(iter)
  if type(iter) == 'table' then return list(iter)
  else return iter end
end

iter = default_iter

--- create an iterator over a numerical range. Like the standard Python function xrange.
-- @param start a number
-- @param finish a number greater than start
function range(start,finish)
  local i = start - 1
  return function()
      i = i + 1
      if i > finish then return nil
      else return i end
  end
end

-- count the number of elements in the sequence which satisfy the predicate
-- @param iter a sequence
-- @param condn a predicate function (must return either true or false)
-- @param optional argument to be passed to predicate as second argument.
function count(iter,condn,arg)
  local i = 0
  condn = function_arg(condn)
  for v in default_iter(iter) do
    if condn then
      if condn(v,arg) then i = i + 1 end
    else
      i = i + 1
    end
  end
  return i
end

--- return the minimum and the maximum value of the sequence.
-- @param iter a sequence
function minmax(iter)
  local vmin,vmax = 1e70,-1e70
  for v in default_iter(iter) do
    v = tonumber(v)
    if v < vmin then vmin = v end
    if v > vmax then vmax = v end
  end
  return vmin,vmax
end

--- return the sum and element count of the sequence.
-- @param iter a sequence
-- @param fn an optional function to apply to the values
function sum(iter,fn)
  local s = 0
  local i = 0
  for v in default_iter(iter) do
    if fn then v = fn(v) end
    s = s + v
    i = i + 1
  end
  return s,i
end

--- create a table from the sequence. (This will make the result a List.)
-- @param iter a sequence
-- @return a List
-- @usage copy(list(ls)) is equal to ls
-- @usage copy(list {1,2,3},List) == List{1,2,3}
function copy(iter)
    local res = {}
    for v in default_iter(iter) do
        tappend(res,v)
    end
    setmetatable(res,_List)
    return res
end

--- create a table of pairs from the double-valued sequence.
-- @param iter a double-valued sequence
-- @return a list-like table
function copy2 (iter,i1,i2)
    local res = {}
    for v1,v2 in iter,i1,i2 do
        tappend(res,{v1,v2})
    end
    return res
end

--- create a table of 'tuples' from a multi-valued sequence.
-- A generalization of copy2 above
-- @param iter a multiple-valued sequence
-- @return a list-like table
function copy_tuples (iter)
    iter = default_iter(iter)
    local res = {}
    local row = {iter()}
    while #row > 0 do
        tappend(res,row)
        row = {iter()}
    end
    return res
end

--- useful to do Python-like assignments to sequences
-- @param iter a sequence
-- @usage x,y = unpack_copy(numbers '10 20')) leaves x==10, y==20.
function unpack_copy(iter)
    return upack(copy(iter))
end

--- return an iterator of random numbers.
-- @param n the length of the sequence
-- @param l same as the first optional argument to math.random
-- @param u same as the second optional argument to math.random
-- @return a sequnce
function random(n,l,u)
  local rand
  assert(type(n) == 'number')
  if u then
     rand = function() return mrandom(l,u) end
  elseif l then
     rand = function() return mrandom(l) end
  else
     rand = mrandom
  end

  return function()
     if n == 0 then return nil
     else
       n = n - 1
       return rand()
     end
  end
end

--- return an iterator to the sorted elements of a sequence.
-- @param iter a sequence
-- @param comp an optional comparison function (comp(x,y) is true if x < y)
function sort(iter,comp)
    local t = copy(iter)
    tsort(t,comp)
    return list(t)
end

--- return an iterator which returns elements of two sequences.
-- @param iter1 a sequence
-- @param iter2 a sequence
-- @usage for x,y in seq.zip(ls1,ls2) do....end
function zip(iter1,iter2)
    iter1 = default_iter(iter1)
    iter2 = default_iter(iter2)
    return function()
        return iter1(),iter2()
    end
end

--- A table where the key/values are the values and value counts of the sequence.
-- This version works with 'hashable' values like strings and numbers. <br>
-- pl.tablex.count_map is more general.
-- @return a map-like table
-- @return a table
-- @see pl.tablex.count_map
function count_map(iter)
    local t = {}
    local v
    for s1,s2 in default_iter(iter) do
          v = t[s1]
          s1 = s1 ~= nil and s1 or 1
          if v then t[s1] = v + s1
          else t[s1] = s1 end
    end
    return setmetatable(t,_Map)
end

-- given a sequence, return all the unique values in that sequence.
-- @param iter a sequence
-- @param returns_table true if we return a table, not a sequence
-- @return a sequence or a table; defaults to a sequence.
function unique(iter,returns_table)
  local t = count_map(iter)
  local res = {}
  for k in pairs(t) do tappend(res,k) end
  if returns_table then
    return res
  else
    return list(res)
  end
end

-- print out a sequence @iter, with a separator @sep (default space)
-- and maximum number of values per line @nfields (default 7)
-- @fmt is an optional format function to create a representation of each value.
function printall(iter,sep,nfields,fmt)
  local write = io.write
  if not sep then sep = ' ' end
  if not nfields then
      if sep == '\n' then nfields = 1e30
      else nfields = 7 end
  end
  if fmt then
    local fstr = fmt
    fmt = function(v) return format(fstr,v) end
  end
  local k = 1
  for v in default_iter(iter) do
     if fmt then v = fmt(v) end
     if k < nfields then
       write(v,sep)
       k = k + 1
    else
       write(v,'\n')
       k = 1
    end
  end
  write '\n'
end

-- return an iterator running over every element of two sequences (concatenation).
-- @param iter1 a sequence
-- @param iter2 a sequence
function splice(iter1,iter2)
  iter1 = default_iter(iter1)
  iter2 = default_iter(iter2)
  local iter = iter1
  return function()
    local ret = iter()
    if ret == nil then
      if iter == iter1 then
        iter = iter2
        return iter()
      else return nil end
   else
       return  ret
   end
 end
end

--- return a sequence where every element of a sequence has been transformed
-- by a function. If you don't supply an argument, then the function will
-- receive both values of a double-valued sequence, otherwise behaves rather like
-- tablex.map.
-- @param iter a sequence of one or two values
-- @param fn a function to apply to elements; may take two arguments
-- @param arg optional argument to pass to function.
function map(fn,iter,arg)
    fn = function_arg(fn)
    iter = default_iter(iter)
    return function()
        local v1,v2 = iter()
        if v1 == nil then return nil end
        if arg then return fn(v1,arg)
        else return fn(v1,v2)
        end
    end
end

--- filter a sequence using a predicate function
-- @param iter a sequence of one or two values
-- @param pred a boolean function; may take two arguments
-- @param arg optional argument to pass to function.
function filter (iter,pred,arg)
    pred = function_arg(pred)
    return function ()
        local v1,v2
        while true do
            v1,v2 = iter()
            if v1 == nil then return nil end
            if arg then
                if pred(v1,arg) then return v1,v2 end
            else
                if pred(v1,v2) then return v1,v2 end
            end
        end
    end
end

--- 'reduce' a sequence using a binary function.
-- @param seq a sequence
-- @param fun a function of two arguments
-- @usage seq.reduce(operator.add,seq.list{1,2,3,4}) == 10
function reduce (fun,seq,oldval)
    if not oldval then
        seq = default_iter(seq)
        oldval = seq()
        fun = function_arg(fun)
    end
    local val = seq()
    if val==nil then return oldval
    else return fun(oldval,reduce(fun,seq,val))
    end
end

--- take the first n values from the sequence.
-- @param iter a sequence
-- @param n number of values to take
-- @return a sequence of at most n values
function take (iter,n)
    local i = 1
    iter = default_iter(iter)
    return function()
        if i > n then return end
        local val = iter()
        if not val then return end
        i = i + 1
        return val
    end
end

--- a sequence with a sequence count and the original value. <br>
-- enum(copy(ls)) is a roundabout way of saying ipairs(ls).
-- @param iter a single or double valued sequence
-- @return sequence of (i,v), i = 1..n and v is from iter.
function enum (iter)
    local i = 0
    iter = default_iter(iter)
    return function  ()
        local val1,val2 = iter()
        if not val1 then return end
        i = i + 1
        return i,val1,val2
    end
end

--- map using a named method over a sequence.
-- @param iter a sequence
-- @param name the method name
-- @param arg1 optional first extra argument
-- @param arg1 optional second extra argument
function mapmethod (iter,name,arg1,arg2)
    iter = default_iter(iter)
    return function()
        local val = iter()
        if not val then return end
        local fn = val[name]
        if not fn then error(type(val).." does not have method "..name) end
        return fn(val,arg1,arg2)
    end
end

--- a sequence of (last,current) values from another sequence.
--  This will return S(i-1),S(i) if given S(i)
-- @param iter a sequence
function last (iter)
    iter = default_iter(iter)
    local l = iter()
    if l == nil then return nil end
    return function ()
        local val,ll
        val = iter()
        if val == nil then return nil end
        ll = l
        l = val
        return val,ll
    end
end

--- call the function on each element of the sequence.
-- @param iter a sequence with up to 3 values
-- @param fn a function
function foreach(iter,fn)
  fn = function_arg(fn)
  for i1,i2,i3 in default_iter(iter) do fn(i1,i2,i3) end
end

---------------------- Sequence Adapters ---------------------

local SMT
local callable = utils.is_callable

local function SW (iter,...)
    if callable(iter) then
        return setmetatable({iter=iter},SMT)
    else
        return iter,...
    end
end


-- can't directly look these up in seq because of the wrong argument order...
local overrides = {
    map = function(self,fun,arg)
        return map(fun,self,arg)
    end,
    reduce = function(self,fun)
        return reduce(fun,self)
    end
}

SMT = {
    __index = function (tbl,key)
        local s = overrides[key] or seq[key]
        if s then
            return function(sw,...) return SW(s(sw.iter,...)) end
        else
            return function(sw,...) return SW(mapmethod(sw.iter,key,...)) end
        end
    end,
    __call = function (sw)
        return sw.iter()
    end,
}

setmetatable(seq,{
    __call = function(tbl,iter)
        if not callable(iter) then
            if type(iter) == 'table' then iter = list(iter)
            else return iter
            end
        end
        return setmetatable({iter=iter},SMT)
    end
})

--- create a wrapped iterator over all lines in the file.
-- @param f either a filename or nil (for standard input)
-- @return a sequence wrapper
function lines (f)
    local iter = f and io.lines(f) or io.lines()
    return SW(iter)
end

function import ()
    _G.debug.setmetatable(function() end,{
        __index = function(tbl,key)
            local s = overrides[key] or seq[key]
            if s then return s
            else
                return function(s,...) return mapmethod(s,key,...) end
            end
        end
    })
end

