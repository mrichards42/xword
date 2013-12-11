-----------------------------------------------------
--- Functional helpers like composition,binding and placeholder expressions.
local type,select,setmetatable,getmetatable = type,select,setmetatable,getmetatable
local concat,append = table.concat,table.insert
local max = math.max
local print,tostring = print,tostring
local pairs,getfenv,ipairs,loadstring,rawget,unpack  = pairs,getfenv,ipairs,loadstring,rawget,unpack
local _G = _G
local utils = require 'pl.utils'
local tablex = require 'pl.tablex'
local map = tablex.map
local _DEBUG = rawget(_G,'_DEBUG')

module 'pl.func'

local mod = _G.pl.func

-- metatable for Placeholder Expressions (PE)
local _PEMT = {}

local function P (t)
    setmetatable(t,_PEMT)
    return t
end

mod.PE = P

local function isPE (obj)
    return getmetatable(obj) == _PEMT
end

mod.isPE = isPE

-- construct a placeholder variable (e.g _1 and _2)
local function PH (idx)
    return P {op='X',repr='_'..idx, index=idx}
end

-- construct a constant placeholder variable (e.g _C1 and _C2)
local function CPH (idx)
    return P {op='X',repr='_C'..idx, index=idx}
end

_1,_2,_3,_4,_5 = PH(1),PH(2),PH(3),PH(4),PH(5)
_0 = P{op='X',repr='...',index=0}

function Var (name)
    local ls = utils.split(name,',')
    local res = {}
    for _,n in ipairs(ls) do
        append(res,P{op='X',repr=n,index=0})
    end
    return unpack(res)
end

function _ (value)
    return P{op='X',repr=value,index='wrap'}
end

Nil = Var 'nil'

function _PEMT.__index(obj,key)
    return P{op='[]',obj,key}
end

function _PEMT.__call(fun,...)
    return P{op='()',fun,...}
end

function _PEMT.__tostring (e)
    return repr(e)
end

function _PEMT.__unm(arg)
    return P{op='-',arg}
end

function Not (arg)
    return P{op='not',arg}
end

function Len (arg)
    return P{op='#',arg}
end

local function binreg(context,t)
    for name,op in pairs(t) do
        context[name] = function(x,y)
            return P{op=op,x,y}
        end
    end
end

local function import_name (name,fun,context)
    context[name] = function(...)
        return P{op='()',fun,...}
    end
end

local imported_functions = {}

--- wrap a table of functions. This makes them available for use in
-- placeholder expressions.
-- @param tname a table name
-- @param context context to put results, defaults to local environment
function import(tname,context)
    local t = _G[tname]
    context = context or getfenv(2)
    for name,fun in pairs(t) do
        import_name(name,fun,context)
        imported_functions[fun] = name
    end
end

--- register a function for use in placeholder expressions.
-- @param fun a function
-- @return a placeholder functiond
function register (fun,name)
    if name then
        imported_functions[fun] = name
    end
    return function(...)
        return P{op='()',fun,...}
    end
end

function lookup_imported_name (fun)
    return imported_functions[fun]
end

local function _arg(...) return ... end

function Args (...)
    return P{op='()',_arg,...}
end

-- binary and unary operators, with their precedences (see 2.5.6)
local operators = {
    ['or'] = 0,
    ['and'] = 1,
    ['=='] = 2, ['~='] = 2, ['<'] = 2, ['>'] = 2,  ['<='] = 2,   ['>='] = 2,
    ['..'] = 3,
    ['+'] = 4, ['-'] = 4,
    ['*'] = 5, ['/'] = 5, ['%'] = 5,
    ['not'] = 6, ['#'] = 6, ['-'] = 6,
    ['^'] = 7
}

-- comparisons (as prefix functions)
binreg (mod,{And='and',Or='or',Eq='==',Lt='<',Gt='>',Le='<=',Ge='>='})

-- standard binary operators (as metamethods)
binreg (_PEMT,{__add='+',__sub='-',__mul='*',__div='/',__mod='%',__pow='^',__concat='..'})

binreg (_PEMT,{__eq='=='})

--- all elements of a table except the first.
-- @param ls a list-like table.
function tail (ls)
    local res = {}
    for i = 2,#ls do
        append(res,ls[i])
    end
    return res
end

--- create a string representation of a placeholder expression.
-- @param e a placeholder expression
function repr (e,lastpred)
    if isPE(e) then
        local pred = operators[e.op]
        local ls = map(repr,e,pred)
        if pred then --unary or binary operator
            if #ls ~= 1 then
                local s = concat(ls,' '..e.op..' ')
                if lastpred and lastpred > pred then
                    s = '('..s..')'
                end
                return s
            else
                return e.op..' '..ls[1]
            end
        else -- either postfix, or a placeholder
            if e.op == '[]' then
                return ls[1]..'['..ls[2]..']'
            elseif e.op == '()' then
                local fn
                if ls[1] ~= _args then
                    fn = ls[1]
                else
                    fn = ''
                end
                return fn..'('..concat(tail(ls),',')..')'
            else
                return e.repr
            end
        end
    elseif type(e) == 'string' then
        return '"'..e..'"'
    elseif type(e) == 'function' then
        local name = lookup_imported_name(e)
        if name then return name else return tostring(e) end
    else
        return tostring(e) --should not really get here!
    end
end

-- collect all the non-PE values in this PE into vlist, and replace each occurence
-- with a constant PH (_C1, etc). Return the maximum placeholder index found.
function collect_values (e,vlist)
    if isPE(e) then
        if e.op ~= 'X' then
            local m = 0
            for i,subx in ipairs(e) do
                local pe = isPE(subx)
                if pe then
                    if subx.op == 'X' and subx.index == 'wrap' then
                        subx = subx.repr
                        pe = false
                    else
                        m = max(m,collect_values(subx,vlist))
                    end
                end
                if not pe then
                    append(vlist,subx)
                    e[i] = CPH(#vlist)
                end
            end
            return m
        else -- was a placeholder, it has an index...
            return e.index
        end
    else -- plain value has no placeholder dependence
        return 0
    end
end

--- instantiate a PE into an actual function. First we find the largest placeholder used,
-- e.g. _2; from this a list of the formal parameters can be build. Then we collect and replace
-- any non-PE values from the PE, and build up a constant binding list.
-- Finally, the expression can be compiled, and e.__PE_function is set.
-- @param e a placeholder expression
-- @return a function
function instantiate (e)
    local consts,values,parms = {},{},{}
    local rep
    local n = collect_values(e,values)
    for i = 1,#values do
        append(consts,'_C'..i)
        if _DEBUG then print(i,values[i]) end
    end
    for i =1,n do
        append(parms,'_'..i)
    end
    consts = concat(consts,',')
    parms = concat(parms,',')
    rep = repr(e)
    fstr = ('return function(%s) return function(%s) return %s end end'):format(consts,parms,rep)
    if _DEBUG then print(fstr) end
    fun,err = loadstring(fstr,'fun')
    if not fun then return nil,err end
    fun = fun()  -- get wrapper
    fun = fun(unpack(values)) -- call wrapper (values could be empty)
    e.__PE_function = fun
    return fun
end

--- instantiate a PE unless it has already been done.
-- @param e a placeholder expression
-- @return the function
function I(e)
    if rawget(e,'__PE_function')  then
        return e.__PE_function
    else return instantiate(e)
    end
end

utils.add_function_factory(_PEMT,I)

--- bind the first parameter of the function to a value.
-- @class function
-- @name curry
-- @param fn a function of one or more arguments
-- @param p a value
-- @return a function of one less argument
-- @usage (curry(math.max,10))(20) == math.max(10,20)
curry = utils.bind1

--- create a function which chains two functions.
-- @param f a function of at least one argument
-- @param g a function of at least one argument
-- @return a function
-- @usage printf = compose(io.write,string.format)
function compose (f,g)
    return function(...) return f(g(...)) end
end

--- bind the arguments of a function to given values.
-- bind(fn,v,_2) is equivalent to curry(fn,v).
-- @param fn a function of at least one argument
-- @param ... values or placeholder variables
-- @return a function
-- @usage (bind(f,_1,a))(b) == f(a,b)
-- @usage (bind(f,_2,_1))(a,b) == f(b,a)
function bind(fn,...)
    local args,n = utils.args(...)
    local holders,parms,bvalues,values = {},{},{'fn'},{}
    local nv,maxplace,varargs = 1,0,false
    for i = 1,n do
        local a = args[i]
        if isPE(a) and a.op == 'X' then
            append(holders,a.repr)
            maxplace = max(maxplace,a.index)
            if a.index == 0 then varargs = true end
        else
            local v = '_v'..nv
            append(bvalues,v)
            append(holders,v)
            append(values,a)
            nv = nv + 1
        end
    end
    for np = 1,maxplace do
        append(parms,'_'..np)
    end
    if varargs then append(parms,'...') end
    bvalues = concat(bvalues,',')
    parms = concat(parms,',')
    holders = concat(holders,',')
    local fstr = ([[
return function (%s)
    return function(%s) return fn(%s) end
end
]]):format(bvalues,parms,holders)
    if _DEBUG then print(fstr) end
    local res,err = loadstring(fstr)
    res = res()
    return res(fn,unpack(values))
end


