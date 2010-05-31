-- Substitute strings using the pattern [name]
function bind.fmt(str, tbl)
    local subst = {}
    for k,v in pairs(tbl) do
        subst["["..k.."]"] = v
    end
    local r = {string.gsub(str, "%[.-%]", subst)}
    return r[1]
end


-- --------------------------------------------------------------------------
-- Object class
-- --------------------------------------------------------------------------
local obj_mt = {}

function obj_mt:subclass(obj)
    local obj = obj or {}
    setmetatable(obj, self)
    self.__index = self
    return obj
end

function obj_mt:new(obj)
    local obj = obj or {}
    obj.parent = bind.class or bind.namespace
    setmetatable(obj, self)
    self.__index = self
    return obj
end


function obj_mt:isglobal()
    return false
end

-- Substitutions
function obj_mt:subst_prefix() return bind.prefix end
function obj_mt:subst_name()   return self.name end
function obj_mt:subst_type()   return self:get_qualified_name() end
function obj_mt:subst_var()    return self.name:lower() end
function obj_mt:subst_namespace() return self.parent:get_qualified_name() end
function obj_mt:subst_qname()  return self:get_qualified_name() end
function obj_mt:subst_api()    return bind.prefix:upper().."_API" end

-- Get the substituted name
function obj_mt:subst(str, t)
    if str == "" then return "[]" end
    -- Try the local table
    local ret
    if t then ret = t[str] end
    if ret then return ret end

    -- Try a member variable of the class instance
    local substname = 'subst_'..str

    ret = rawget(self, substname)
    if ret then return ret end

    -- Try a function from the class's metatable
    local func = self[substname]
    if func then
        ret = func(self)
        rawset(self, substname, ret) -- cache the result
        return ret
    end

    -- Try a prefixed function (pXXXX) = parent function
    if str:sub(1,1) == "p" and self.parent then
        ret = self.parent:subst(str:sub(2))
        if ret then
            rawset(self, substname, ret) -- cache the result
            return ret
        end
    end
end

function obj_mt:fmt(str, t)
    local t = t or {}
    local r = {string.gsub(str, "%[(.-)%]",
        function(str)
            return assert(self:subst(str, t), "Unable to substitute string: "..str)
        end)}
    return r[1]
end

function obj_mt:getfilename(obj)
    return self.parent:getfilename(self)
end

function obj_mt:gethppfilename(obj)
    return self:getfilename(obj)..".hpp"
end

function obj_mt:getcppfilename(obj)
    return self:getfilename(obj)..".cpp"
end

function obj_mt:get_qualified_name()
    if not self.parent or self.parent:isglobal() then
        return self.name
    end
    return self.parent:get_qualified_name() .. "::" .. self.name
end

bind.obj_mt = obj_mt


-- --------------------------------------------------------------------------
-- Base class (class and namespace)
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()

function mt:new(name)
    local obj = {
        name = name,
        classes = {},
        funcs = {},
        types = {},
        enums = {},
        typedefs = {},
        usertypes = {},
        namespaces = {},
    }
    return bind.obj_mt.new(self, obj)
end


function mt:get_hppheaders()
    local headers = {}
    -- Add the headers for the class
    if self.type then
        for _, header in pairs(self.type.headers) do
            headers[header] = true
        end
    end
    -- Add the headers for all enums
    if self.enums then
        for _, enum in pairs(self.enums) do
            for _, header in pairs(enum.type.headers) do
                headers[header] = true
            end
        end
    end
    -- Add the headers for all typedefs
    if self.typedefs then
        for _, td in pairs(self.typedefs) do
            for _, header in pairs(td.type.headers) do
                headers[header] = true
            end
        end
    end

    -- Remove our own file if it's there
    headers[self:gethppfilename()] = nil

    -- Make a sequential list
    local ret = {}
    for header, _ in pairs(headers) do
        -- Make non-absolute headers relative
        if header:sub(1,1) ~= "<" then
            table.insert(ret, string.format('"%s"', header))
        else
            table.insert(ret, header)
        end
    end
    return ret
end


function mt:get_cppheaders()
    local headers = {}
    -- Start with our own header
    if not self:isglobal() then
        headers[self:gethppfilename()] = true
    end
    -- Add the lua headers for all used types
    if self.usertypes then
        for type, _ in pairs(self.usertypes) do
            headers[type:get_luaheader()] = true
        end
    end

    -- Add the headers for any contained namespaces or classes
    for _, class in ipairs(self.classes) do
        headers[class:gethppfilename()] = true
    end
    for _, ns in ipairs(self.namespaces) do
        headers[ns:gethppfilename()] = true
    end

    -- Add the headers for the class
    if self.type then
        for _, header in pairs(self.type.cppheaders) do
            headers[header] = true
        end
    end
    -- Add the headers for all enums
    if self.enums then
        for _, enum in pairs(self.enums) do
            for _, header in pairs(enum.type.cppheaders) do
                headers[header] = true
            end
        end
    end
    -- Add the headers for all typedefs
    if self.typedefs then
        for _, td in pairs(self.typedefs) do
            for _, header in pairs(td.type.cppheaders) do
                headers[header] = true
            end
        end
    end

    -- Make a sequential list
    local ret = {}
    for header, _ in pairs(headers) do
        -- Make non-absolute headers relative
        if header:sub(1,1) ~= "<" then
            table.insert(ret, string.format('"%s"', header))
        else
            table.insert(ret, header)
        end
    end
    return ret
end




function mt:print()
    for _, enum in ipairs(self.enums) do
        enum:print()
    end
    for _, func in ipairs(self.funcs) do
        func:print()
    end
    for _, class in ipairs(self.classes) do
        class:print()
    end
end


function mt:newhppfile()
    local filename = self:gethppfilename()
    local f = io.open(filename, 'w')
    local defname = string.gsub(filename, "%.", "_")
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// [filename] was generated by puzbind.lua
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

#ifndef [defname]
#define [defname]

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "../[prefix]_functions.hpp"
#include "../[prefix]_tracking.hpp"

]], {
        filename=filename,
        defname=defname,
    }))

    getmetatable(f).endfile = function()
        f:write(string.format([[
#endif // %s
]], defname))
        f:close()
    end
    return f
end


function mt:newcppfile()
    local filename = self:getcppfilename()
    local f = io.open(filename, 'w')
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// [filename] was generated by puzbind.lua
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "../[prefix]_functions.hpp"
#include "../[prefix]_tracking.hpp"

]], {
        filename=filename,
    }))

    getmetatable(f).endfile = function()
        f:close()
    end
    return f
end

-- Substitutions
function mt:subst_lib()     return self.name.."lib" end
function mt:subst_openfunc() return self:subst('prefix')..'_open'..self:subst('lib') end
function mt:subst_luatype() return self:get_qualified_name():gsub("::", ".") end

function mt:writefiles()
    local f
    if not self:isglobal() then
        -- Write hpp file
        f = self:newhppfile()
        self:writehpp(f)
        f.endfile()
        -- Write cpp file
        f = self:newcppfile()
        self:writecpp(f)
        f.endfile()
    end

    -- Write contained classes
    if self.classes then
        for _, class in ipairs(self.classes) do
            class:writefiles()
        end
    end

    -- Write contained namespaces
    if self.namespaces then
        for _, ns in ipairs(self.namespaces) do
            ns:writefiles()
        end
    end
end

function mt:writehppheaders(f)
    -- Figure out the files we need to include
    for _, inc in ipairs(self:get_hppheaders()) do
        f:write(string.format([[#include %s]], inc)..'\n')
    end
end

function mt:writecppheaders(f)
    -- Figure out the files we need to include
    for _, inc in ipairs(self:get_cppheaders()) do
        f:write(string.format([[#include %s]], inc)..'\n')
    end
end

function mt:writehpp(f)

    -- write enumerations
    for _, enum in ipairs(self.enums) do
        enum:writehpp(f)
    end

    -- write typedefs
    for _, td in ipairs(self.typedefs) do
        td:writehpp(f)
    end

    f:write(self:fmt("void [openfunc] (lua_State *L);\n"))
end

function mt:writecpp(f)
    -- Write the functions
    for _, func in ipairs(self.funcs) do
        func:writecpp(f)
    end

    -- Write the registration table
    f:write(self:fmt("static const luaL_reg [lib][] = {\n"))
    for _, func in ipairs(self.funcs) do
        if not func.is_constructor then
            f:write(func:fmt('    {"[luafunc]", [cfunc]},\n'))
        end
    end
    f:write('    {NULL, NULL}\n')
    f:write('};\n\n')

    -- Write the enumerations
    for _, enum in ipairs(self.enums) do
        enum:writecpp(f)
    end

    -- write typedefs
    for _, td in ipairs(self.typedefs) do
        td:writecpp(f)
    end
end

bind.base_mt = mt
