----------------------------------------------
--- Python-style string library.
-- see 3.6.1 of the Python reference. <br> <br>
-- If you want to make these available as string methods, then say
-- <code>stringx.import()</code> to bring them into the standard <code>string</code>
-- table.
-- @class module
-- @name pl.stringx
local string = string
local find = string.find
local type,setmetatable,getmetatable,ipairs,unpack = type,setmetatable,getmetatable,ipairs,unpack
local error,tostring = error,tostring
local gsub = string.gsub
local rep = string.rep
local sub = string.sub
local concat = table.concat
local utils = require 'pl.utils'
local escape = utils.escape
local _G = _G

module ('pl.stringx')

--- does s only contain alphabetic characters?.
function isalpha(s)
    return find(s,'^%a+$') == 1
end

--- does s only contain digits?.
function isdigit(s)
    return find(s,'^%d+$') == 1
end

--- does s only contain alphanumeric characters?.
function isalnum(s)
    return find(s,'^%d+$') == 1
end

--- does s only contain spaces?.
function isspace(s)
    return find(s,'^%s+$') == 1
end

--- does s only contain lower case characters?.
function islower(s)
    return find(s,'^%l+$') == 1
end

--- does s only contain upper case characters?.
function isupper(s)
    return find(s,'^%u+$') == 1
end

--- concatenate the strings using this string as a delimiter.
-- @param seq a table of strings or numbers
-- @usage (' '):join {1,2,3} == '1 2 3'
function join (self,seq)
    return concat(seq,self)
end

--- does string start with the substring?.
-- @param s2 a string
function startswith(self,s2)
    return find(self,s2,1,true) == 1
end

local function _find_all(s,sub,first,last)
    local i1,i2 = find(s,sub,first,true)
    local res
    local k = 0
    while i1 do
        res = i1
        k = k + 1
        i1,i2 = find(s,sub,i2+1,true)
        if last and i1 > last then break end
    end
    return res,k
end

--- does string end with the given substring?.
-- @param s a substring or a table of suffixes
function endswith(self,s,first,last)
    first = first or 1
    if type(s) == 'string' then
        local i1 = _find_all(self,s,first,last)
        return i1 == #self - #s + 1
    elseif type(s) == 'table' then
        for _,suffix in ipairs(s) do
            if endswith(self,suffix,first,last) then return true end
        end
        return false
    end
end

-- break string into a list of lines
function splitlines (self,keepends)
    return split(self,'\n')
end

--- replace all tabs in s with n spaces. If not specified, n defaults to 8.
-- @param n number of spaces to expand each tab
function expandtabs(self,n)
    n = n or 8
    local tab = rep(' ',n)
    return (gsub(s,'\t',tab))
end

--- find index of first instance of sub in s from the left.
-- @param sub substring
-- @param  i1 start index
function lfind(self,sub,i1,i2)
    local idx = find(self,sub,i1,true)
    if idx then return idx else return -1 end
end

--- find index of first instance of sub in s from the right.
-- @param sub substring
-- @param first first index
-- @param last last index
function rfind(self,sub,first,last)
    local idx = _find_all(self,sub,first,last)
    if idx then return idx else return -1 end
end

--- replace up to n instances of old by new in the string s.
-- if n is not present, replace all instances.
function replace(s,old,new,n)
    return gsub(s,escape(old),new,n)
end

--- split a string into a list of strings using a pattern.
-- @class function
-- @name split
-- @param self the string
-- @param re a Lua string pattern (defaults to whitespace)
-- @usage #(('one two'):split()) == 2
split = utils.split

--- split a string using a pattern. Note that at least one value will be returned!
-- @param self the string
-- @param re a Lua string pattern (defaults to whitespace)
-- @return the parts of the string
-- @usage  a,b = line:splitv('=')
function splitv (self,re)
    return unpack(split(self,re))
end

local function copy(self)
    return self..''
end

-- capitalize the string
function capitalize(self)
    return self:sub(1,1):upper()..self:sub(2)
end

--- count all instances of subtring in string.
-- @param sub substring
function count(self,sub)
    local i,k = _find_all(self,sub,1)
    return k
end

function _just(s,w,ch,left,right)
    local n = #s
    if w > n then
        if not ch then ch = ' ' end
        local f1,f2
        if left and right then
            f1 = rep(ch,(w-n)/2)
            f2 = f1
        elseif left then
            f1 = rep(ch,w-n)
            f2 = ''
        else
            f2 = rep(ch,w-n)
            f1 = ''
        end
        return f1..s..f2
    else
        return copy(s)
    end
end

--- left-justify s with width w.
-- @param w width of justification
-- @param ch padding character, default ' '
function ljust(self,w,ch)
    return _just(self,w,ch,true,false)
end

--- right-justify s with width w.
-- @param w width of justification
-- @param ch padding character, default ' '
function rjust(s,w,ch)
    return _just(s,w,ch,false,true)
end

--- center-justify s with width w.
-- @param w width of justification
-- @param ch padding character, default ' '
function center(s,w,ch)
    return _just(s,w,ch,true,true)
end

local function _strip(s,chrs,left,right)
    if left then
        local i1,i2 = find(s,'^%s*')
        if i2 >= i1 then
            s = sub(s,i2+1)
        end
    end
    if right then
        local i1,i2 = find(s,'%s*$')
        if i2 >= i1 then
            s = sub(s,1,i1-1)
        end
    end
    return s
end

--- trim any whitespace on the left of s.
function lstrip(self,chrs)
    return _strip(self,chrs,true,false)
end

--- trim any whitespace on the right of s.
function rstrip(s,chrs)
    return _strip(s,chrs,false,true)
end

--- trim any whitespace on both left and right of s.
function strip(self,chrs)
    return _strip(self,chrs,true,true)
end

-- The partition functions split a string  using a delimiter into three parts:
-- the part before, the delimiter itself, and the part afterwards
local function _partition(p,delim,fn)
    local i1,i2 = fn(p,delim)
    if not i1 or i1 == -1 then
        return p,'',''
    else
        if not i2 then i2 = i1 end
        return sub(p,1,i1-1),sub(p,i1,i2),sub(p,i2+1)
    end
end

--- partition the string using first occurance of a delimiter
-- @param ch delimiter
-- @return part before ch, ch, part after ch
function partition(self,ch)
    return _partition(self,ch,lfind)
end

--- partition the string p using last occurance of a delimiter
-- @param ch delimiter
-- @return part before ch, ch, part after ch
function rpartition(self,ch)
    return _partition(self,ch,rfind)
end

--- return the 'character' at the index.
-- @param self the string
-- @param idx an index (can be negative)
-- @return a substring of length 1 if successful, empty string otherwise.
function at(self,idx)
    return sub(self,idx,idx)
end

--- return an interator over all lines in a string
-- @param self the string
-- @return an iterator
function lines (self)
    local s = self
    if not s:find '\n$' then s = s..'\n' end
    return self:gfind('([^\n]*)\n')
end

function import(dont_overload)
    utils.import(_G.pl.stringx,string)
end


