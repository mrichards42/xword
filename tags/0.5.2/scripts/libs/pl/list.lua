--- Python-style list class.
-- Based on original code by Nick Trout. <br>
-- Please note: methods that change the list will return the list. <br>
-- This is to allow for method chaining, but please note that ls = ls:sort() <br>
-- does not mean that a new copy of the list is made. In-place (mutable) methods <br>
-- are marked as returning 'the list' in this documentation.
-- @class module
-- @name pl.list

-- See http://www.python.org/doc/current/tut/tut.html, section 5.1
-- Note:The comments before some of the functions are from the Python docs
-- and contain Python code.
-- Written for Lua version 4.0
-- Redone for Lua 5.1, Steve Donovan.

local tinsert,tremove,concat,tsort = table.insert,table.remove,table.concat,table.sort
local setmetatable, getmetatable,type,tostring,assert,string = setmetatable,getmetatable,type,tostring,assert,string
local write = io.write
local tablex = require 'pl.tablex'
local filter,imap,imap2,reduce,transform,tremovevalues = tablex.filter,tablex.imap,tablex.imap2,tablex.reduce,tablex.transform,tablex.removevalues
local utils = require 'pl.utils'
local function_arg = utils.function_arg
local is_type = utils.is_type
local split = utils.split

module ('pl.list')

local Multimap = utils.stdmt.MultiMap
-- metatable for our list objects
List = utils.stdmt.List
List.__index = List
List._name = "List"
List._class = List

-- we give the metatable its own metatable so that we can call it like a function!
setmetatable(List,{
    __call = function (tbl,arg)
        return List:new(arg)
    end,
})

local _List = List

local function makelist (t)
    return setmetatable(t,_List)
end

local function is_list(t)
    return getmetatable(t) == _List
end

local function simple_table(t)
  return type(t) == 'table' and not is_list(t) and #t > 0
end

--- Create a new list. Can optionally pass a table;
-- passing another instance of List will cause a copy to be created
-- we pass anything which isn't a simple table to iter() to work out
-- an appropriate iterator  @see iter
-- @param t An optional list-like table
-- @return a new List
-- @usage ls = List();  ls = List {1,2,3,4}
function List:new(t)
  if not t then t={}
  elseif not simple_table(t) then
    local tbl = t
    t = {}
    for v in iter(tbl) do
      tinsert(t,v)
    end
  end
  makelist(t,List)
  return t
end

---Add an item to the end of the list.
-- @param i An item
-- @return the list
function List:append(i)
  tinsert(self,i)
  return self
end

List.push = tinsert

--- Extend the list by appending all the items in the given list.
-- equivalent to 'a[len(a):] = L'.
-- @param L Another List
-- @return the list
function List:extend(L)
  assert(type(L)=="table","List:extend expecting a table")
  for i = 1,#L do tinsert(self,L[i]) end
  return self
end

--- Insert an item at a given position. i is the index of the
-- element before which to insert.
-- @param i index of element before whichh to insert
-- @param x A data item
-- @return the list
function List:insert(i, x)
  tinsert(self,i,x)
  return self
end

--- Insert an item at the begining of the list.
-- @param x a data item
-- @return the list
function List:put (x)
    return self:insert(1,x)
end

--- Remove an element given its index.
-- (equivalent of Python's del s[i])
-- @param i the index
-- @return the list
function List:remove (i)
    tremove(self,i)
    return self
end

--- Remove the first item from the list whose value is given.
-- (This is called 'remove' in Python; renamed to avoid confusion
-- with table.remove)
-- Return nil if there is no such item.
-- @param x A data value
-- @return the list
function List:remove_value(x)
    for i=1,#self do
        if self[i]==x then tremove(self,i) return self end
    end
    return self
 end

--- Remove the item at the given position in the list, and return it.
-- If no index is specified, a:pop() returns the last item in the list.
-- The item is also removed from the list.
-- @param i An index
-- @return the item
function List:pop(i)
    if not i then i = #self end
    return tremove(self,i)
end

List.get = List.pop

--- Return the index in the list of the first item whose value is given.
-- Return nil if there is no such item.
-- @class function
-- @name List:index
-- @param x A data value
-- @param idx where to start search (default 1)
-- @return the index, or nil if not found.

local tfind = tablex.find
List.index = tfind

--- does this list contain the value?.
-- @param x A data value
-- @return true or false
function List:contains(x)
    return tfind(self,x) and true or false
end

--- Return the number of times value appears in the list.
-- @param x A data value
-- @return number of times x appears
function List:count(x)
  local cnt=0
  for i=1,#self do
    if self[i]==x then cnt=cnt+1 end
  end
  return cnt
end

--- Sort the items of the list, in place.
-- @param cmp an optional comparison function; '<' is used if not given.
-- @return the list
function List:sort(cmp)
  tsort(self,cmp)
  return self
end

--- Reverse the elements of the list, in place.
-- @return the list
function List:reverse()
    local t = self
    local n = #t
    local n2 = n/2
    for i = 1,n2 do
        local k = n-i+1
        t[i],t[k] = t[k],t[i]
    end
    return self
end

local function normalize_slice(self,first,last)
  local sz = #self
  if not first then first=1 end
  if first<0 then first=sz+first+1 end
  -- make the range _inclusive_!
  if not last then last=sz end
  if last < 0 then last=sz+1+last end
  return first,last
end

--- Emulate list slicing.  like  'list[first:last]' in Python.
-- If first or last are negative then they are relative to the end of the list
-- eg. _slice(-2)_ gives last 2 entries in a list,
-- eg. _slice(-4,-2)_ gives from -4th to -2nd
-- @param first An index
-- @param last An index
-- @return a new List
function List:slice(first,last)
  first,last = normalize_slice(self,first,last)
  local t=self:new()
  for i=first,last do tinsert(t,self[i]) end
  return t
end

--- empty the list.
-- @return the list
function List:clear()
  for i=1,#self do tremove(self,i) end
  return self
end

--- Emulate Python's range(x) function.
-- Include it in List table for tidiness
-- @param start A number
-- @param finish A number greater than start; if zero, then 0..start-1
-- @usage List.range(0,3) == List {0,1,2,3}
function List.range(start,finish)
  if not finish then
    start = 0
    finish = finish - 1
  end
  local  t = List:new()
  for i=start,finish do tinsert(t,i) end
  return t
end

--- list:len() is the same as #list.
function List:len()
  return #self
end

-- Extended operations --

--- Remove a subrange of elements.
-- equivalent to 'del s[i1:i2]' in Python.
-- @param i1 start of range
-- @param i2 end of range
-- @return the list
function List:chop(i1,i2)
    i1,i2 = normalize_slice(self,i1,i2)
    return tremovevalues(self,i1,i2)
end

--- Insert a sublist into a list
-- equivalent to 's[idx:idx] = list' in Python
-- @param idx index
-- @param list list to insert
-- @return the list
-- @usage  l = List{10,20}; l:splice(2,{21,22});  assert(l == List{10,21,22,20})
function List:splice(idx,list)
    idx = idx - 1
    for i = 1,#list do
        tinsert(self,i+idx,list[i])
    end
    return self
end

--- general slice assignment s[i1:i2] = seq.
-- @param i1  start index
-- @param i2  end index
-- @param seq a list
-- @return the list
function List:slice_assign(i1,i2,seq)
    i1,i2 = normalize_slice(self,i1,i2)
    if i2 >= i1 then self:chop(i1,i2) end
    self:splice(i1,seq)
    return self
end

--- concatenation operator .. .
-- @param L another List
-- @return a new list consisting of the list with the elements of the new list appended
function List:__concat(L)
    local ls = List(self)
    ls:extend(L)
    return ls
end

--- equality operator ==.  True iff all elements of two lists are equal.
-- @param L another List
-- @return true or false
function List:__eq(L)
    if #self ~= #L then return false end
    for i = 1,#self do
        if self[i] ~= L[i] then return false end
    end
    return true
end

--- join the elements of a list using a delimiter.<br>
-- This method uses tostring on all elements.
-- @param delim a delimiter string, can be empty.
-- @return a string
function List:join (delim)
    return concat(imap(tostring,self),delim)
end

--- join a list of strings. <br>
-- Uses table.concat directly.
-- @class function
-- @name List:concat
-- @param delim a delimiter
-- @return a string
List.concat = concat

--- how our list should be rendered as a string. Uses join().
-- @see pl.list.List:join
function List:__tostring()
    return '{'..self:join(',')..'}'
end

--[[
-- NOTE: this works, but is unreliable. If you leave the loop before finishing,
-- then the iterator is not reset.
--- can iterate over a list directly.
-- @usage for v in ls do print(v) end
function List:__call()
    if not self.key then self.key = 1 end
    local value = self[self.key]
    self.key = self.key + 1
    if not value then self.key = nil end
    return value
end
--]]

--- call the function for each element of the list.
-- @param fun a function or callable object
function List:foreach (fun,...)
    local t = self
    fun = function_arg(fun)
    for i = 1,#t do
        fun(t[i],...)
    end
end

--- create a list of all elements which match a function.
-- @param fun a boolean function
-- @return a new filtered list.
function List:filter (fun)
    return makelist(filter(self,fun))
end

--- split a string using a delimiter.
-- @param s the string
-- @param delim the delimiter (default spaces)
-- @return a List of strings
-- @see pl.utils.split
function List.split (s,delim)
    return makelist(split(s,delim))
end

--- apply a function to all elements.
-- Any extra arguments will be passed to the function
-- @param fun a function of at least one argument
-- @param arg1 an optional argument
-- @param ... arbitrary extra arguments.
-- @return a new list: {f(x) for x in self}
-- @see pl.tablex.imap
function List:map (fun,...)
    return makelist(imap(fun,self,...))
end

--- apply a function to all elementss, in-place.
-- Any extra arguments are passed to the function.
-- @param fun A function that takes at least one argument
-- @param t a table (applies to array part)
-- @param ... arbitrary extra arguments.
function List:transform (fun,t,...)
    return makelist(transform(fun,t,...))
end

--- apply a function to elements of two lists.
-- Any extra arguments will be passed to the function
-- @param fun a function of at least two arguments
-- @param ... arbitrary extra arguments.
-- @return a new list: {f(x,y) for x in self, for x in arg1}
-- @see pl.tablex.imap2
function List:map2 (fun,ls,...)
    return makelist(imap2(fun,self,ls,...))
end

--- apply a named meethod to all elements.
-- Any extra arguments will be passed to the method.
-- @param name name of method
-- @param ... extra arguments
-- @return a new list of the results
-- @see pl.seq.mapmethod
function List:mapm (name,...)
    local res = {}
    local t = self
    for i = 1,#t do
      local val = t[i]
      local fn = val[name]
      if not fn then error(type(val).." does not have method "..name) end
      res[i] = fn(val,...)
    end
    return makelist(res)
end

--- 'reduce' a list using a binary function.
-- @param fun a function of two arguments
-- @return result of the function
-- @see pl.tablex.reduce
function List:reduce (fun)
    return reduce(fun,self)
end

--- partition a list using a classifier function.
-- The function may return nil, but this will be converted to the string key '<nil>'.
-- @param fun a function of at least one argument
-- @param ... will also be passed to the function
-- @return a table where the keys are the returned values, and the values are Lists
-- of values where the function returned that key. It is given the type of Multimap.
-- @see pl.classx.MultiMap
function List:partition (fun,...)
    fun = function_arg(fun)
    local res = {}
    for i = 1,#self do
        local val = self[i]
        local klass = fun(val,...)
        if klass == nil then klass = '<nil>' end
        if not res[klass] then res[klass] = List() end
        res[klass]:append(val)
    end
    return setmetatable(res,Multimap)
end

function List:unique()

end

--- sum the elements. May provide an initial value, which is useful
-- if you are applying a generalized sum operator.
-- @param res initial value (default 0)
function List:sum (res)
    res = res or 0.0
    for i = 1,#self do
        sum = sum + self[i]
    end
    return sum,#self
end

--- calculate the minimum and maximum values.
-- @return minimum value
-- @return maximum value
function List:minmax()
    local vmin = self[1]
    local vmax = self[1]
    vmax = vmax or -1e70
    for i = 1,#self do
        local v = self[i]
        if v < vmin then vmin = v end
        if v > vmax then vmax = v end
    end
    return vmin,vmax
end

--- apply a function to all elements, in-place.
-- @param fun a function of at least one argument.
-- @param ... extra arguments for the function.
function List:transform (fun,...)
    fun = function_arg(fun)
    for i = 1,#sef do
        self[i] = fun(self[i],...)
    end
end

--- return an iterator over all values.
function List:iter ()
    return iter(self)
end

--- Create an iterator over a seqence.
-- This captures the Python concept of 'sequence'.
-- For tables, iterates over all values with integer indices.
-- @param seq a sequence; a string (over characters), a table, a file object (over lines) or an iterator function
-- @usage for x in iter {1,10,22,55} do io.write(x,',') end ==> 1,10,22,55
-- @usage for ch in iter 'help' do do io.write(ch,' ') end ==> h e l p
function iter(seq)
    if type(seq) == 'string' then
        local idx = 0
        local n = #seq
        local sub = string.sub
        return function ()
            idx = idx + 1
            if idx > n then return nil
            else
                return sub(seq,idx,idx)
            end
        end
    elseif type(seq) == 'table' then
        local idx = 0
        local n = #seq
        return function()
            idx = idx + 1
            if idx > n then return nil
            else
                return seq[idx]
            end
        end
    elseif type(seq) == 'function' then
        return seq
    elseif type(seq) == 'userdata' and io.type(seq) == 'file' then
        return seq:lines()
    end
end


