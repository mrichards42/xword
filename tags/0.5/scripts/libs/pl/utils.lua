---------------------------------------------------
--- Generally useful routines.
-- @class module
-- @name pl.utils
local _G = _G
local type,getfenv,rawget,pairs,ipairs,getmetatable,require,setmetatable = type,getfenv,rawget,pairs,ipairs,getmetatable,require,setmetatable
local select,unpack = select,unpack
local io = io
local format,gsub,byte = string.format,string.gsub,string.byte
local clock = os.clock
local stdout = io.stdout
local append = table.insert
local exit = os.exit

local collisions = {}

module ('pl.utils')

--- end this program gracefully.
-- @param code The exit code
-- @param msg A message to be printed
-- @param ... extra arguments for fprintf
-- @see pl.utils.fprintf
function quit(code,msg,...)
    if type(code) == 'string' then
        msg = code
        code = -1
    end
    fprintf(io.stderr,msg,...)
    io.stderr:write('\n')
    exit(code)
end

--- print an arbitrary number of arguments using a format.
--  @param fmt The format (see string.format)
function printf(fmt,...)
    fprintf(stdout,fmt,...)
end

--- write an arbitrary number of arguments to a file using a format.
-- @param fmt The format (see string.format)
function fprintf(f,fmt,...)
    f:write(format(fmt,...))
end

local function import_symbol(T,k,v,libname)
    local key = rawget(_G,k)
    -- warn about collisions!
    if key and k ~= '_M' and k ~= '_NAME' and k ~= '_PACKAGE' and k ~= '_VERSION' then
        printf("warning: '%s.%s' overrides existing symbol\n",libname,k)
    end
    T[k] = v
end

local function lookup_lib(T,t)
    for k,v in pairs(T) do
        if v == t then return k end
    end
    return '?'
end

local already_imported = {}

--- take a table and 'inject' it into the local namespace.
-- @param t The Table
-- @param T An optional destination table (defaults to callers environment)
function import(t,T)
    T = T or getfenv(2)
    t = t or _G.pl.utils
    if type(t) == 'string' then
        t = require (t)
    end
    local libname = lookup_lib(T,t)
    if already_imported[t] then return end
    already_imported[t] = libname
    for k,v in pairs(t) do
        import_symbol(T,k,v,libname)
    end
end

patterns = {
    FLOAT = '[%+%-%d]%d*%.?%d*[eE]?[%+%-]?%d*',
    INTEGER = '[+%-%d]%d*',
    IDEN = '[%a_][%w_]*',
    FILE = '[%a%.\\][:%][%w%._%-\\]*'
}

--- escape any 'magic' characters in a string
-- @param s The input string
function escape(s)
    return (s:gsub('[%-%.%+%[%]%(%)%$%^%%%?%*]','%%%1'))
end

--- return either of two values, depending on a condition.
-- @param cond A condition
-- @param value1 Value returned if cond is true
-- @param value2 Value returned if cond is false (can be optional)
function choose(cond,value1,value2)
    if cond then return value1
    else return value2
    end
end

--- return the contents of a file as a string
-- @param filename The file path
-- @return file contents
function readfile(filename)
    local f,err,std
    if filename == nil then
        f = io.stdin
        std = true
    elseif type(filename) == 'string' then
        f,err = io.open(filename,'r')
    else
        f,err = nil,'filename as string required'
    end
    if not f then return nil,err end
    local res = f:read('*a')
    if not std then f:close() end
    return res
end

--- write a string to a file
-- @param filename The file path
-- @param str The string
function writefile(filename,str)
    local f,err,std
    if not str then
        f = io.stdout
        str = filename
        std = true
    elseif type(filename) == 'string' then
        f,err = io.open(filename,'w')
    else
        f,err = nil,'filename as string required'
    end
    if not f then return nil,err end
    f:write(str)
    if not std then f:close() end
    return true
end

--- return the contents of a file as a list of lines
-- @param filename The file path
-- @return file contents as a table
function readlines(filename)
    local f,err = io.open(filename,'r')
    if not f then return nil,err end
    local res = {}
    for line in f:lines() do
        append(res,line)
    end
    f:close()
    return res
end

---- split a string into a list of strings separated by a delimiter.
-- @param s The input string
-- @param re A regular expression; defaults to spaces
-- @return a list-like table
function split(s,re)
    local i1 = 1
    local ls = {}
    if not re then re = '%s+' end
    if re == '' then return {s} end
    while true do
        local i2,i3 = s:find(re,i1)
        if not i2 then
            local last = s:sub(i1)
            if last ~= '' then append(ls,last) end
            if #ls == 1 and ls[1] == '' then
                return {}
            else
                return ls
            end
        end
        append(ls,s:sub(i1,i2-1))
        i1 = i3+1
    end
end


--- split a string into a number of values.
-- @param s the string
-- @param re the delimiter, default space
-- @return n values
-- @usage first,next = splitv('hello:dolly',':')
-- @see split
function splitv (s,re)
    return unpack(split(s,re))
end

--- split a string into a list of strings separated by either spaces or commas.
-- @param s The input string
-- @return a list-like table
function splitl(s)
    return split(s,'[%s,]+')
end

--- take an arbitrary set of arguments and make into a table.
-- This returns the table and the size; works fine for nil arguments
-- @param ... arguments
-- @return table
-- @return table size
-- @usage local t,n = utils.args(...)
function args (...)
    return {...},select('#',...)
end


--- 'memoize' a function (cache returned value for next call).
-- This is useful if you have a function which is relatively expensive,
-- but you don't know in advance what values will be required, so
-- building a table upfront is wasteful/impossible.
-- @param func a function of at least one argument
-- @return a function with at least one argument, which is used as the key.
function memoize(func)
    return setmetatable({}, {
        __index = function(self, k, ...)
            local v = func(k,...)
            self[k] = v
            return v
        end,
        __call = function(self, k) return self[k] end
    })
end

--- is the object either a function or a callable object?.
function is_callable (obj)
    return type(obj) == 'function' or getmetatable(obj) and getmetatable(obj).__call
end

--- is the object of the specified type?.
-- If the type is a string, then use type, otherwise compare with metatable
-- @param obj an object
-- @param tp a type
function is_type (obj,tp)
    if type(tp) == 'string' then return type(obj) == tp end
    local mt = getmetatable(obj)
    return tp == mt
end

stdmt = { List = {}, Map = {}, Set = {}, MultiMap = {} }


local _function_factories = {}

function add_function_factory (mt,fun)
    _function_factories[mt] = fun
end

local ops

--- process a function argument. <br>
-- This is used throughout Penlight and defines what is meant by a function: <br>
-- Something that is_callable, or an operator string as defined by pl.operator, <br>
-- such as '>' or '#'.
-- @param f a function, operator string, or callable object
-- @return a callable
function function_arg (f)
    local tp = type(f)
    if tp == 'function' then return f end
    if tp == 'string' then
        if not ops then ops = require 'pl.operator'.optable end
        local fn = ops[f]
        if fn then return fn end
    end
    local mt = getmetatable(f)
    if not mt then return f end
    local ff = _function_factories[mt]
    if not ff then return f end
    return ff(f)
end

--- bind the first argument of the function to a value.
-- @param fn a function of at least two values
-- @param p a value
-- @return a function such that f(x) is fn(p,x)
-- @see pl.func.curry
function bind1 (fn,p)
	fn = function_arg(fn)
    return function(...) return fn(p,...) end
end


