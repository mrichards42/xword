require 'base'
require 'argument'

-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------

-- func{"TheName", returns="type", arg("name1","type1"), arg("name2","type2"), ...}
function func(opts)
    local enclosing_obj = bind.class or bind.namespace

    -- Get the name
    local name = assert(opts[1], "Function without name")
    print("New function: "..name)
    table.remove(opts, 1)
    opts.name = name

    -- Get the arguments (numerical indices)
    opts.args = {}
    opts.required_args = 0
    opts.optional_args = 0
    local has_default = false
    for _, arg in ipairs(opts) do
        assert(getmetatable(arg) == bind.argument_mt)
        if not arg.default then
            assert(not has_default, "non-default argument following default argument")
            opts.required_args = opts.required_args + 1
        else
            has_default = true
            opts.optional_args = opts.optional_args + 1
        end
        if arg.type.usertype then
            enclosing_obj.usertypes[arg.type] = true
        end
        table.insert(opts.args, arg)
    end
    opts.has_default = has_default

    -- Get the return value
    if opts.returns then
        opts.returns = var(opts.returns, "returns")
        if opts.returns.type.usertype then
            enclosing_obj.usertypes[opts.returns.type] = true
        end
    end

    opts.luafunc = opts.rename or opts.luafunc or opts.name
    opts.rename = nil
    -- Create the function
    local f = bind.function_mt:new(opts)
    if getmetatable(enclosing_obj) == bind.class_mt then
        f.is_method = true
    end
    f.is_static = opts.static
    if enclosing_obj.name == f.name
            and getmetatable(enclosing_obj) == bind.class_mt then
        f.is_constructor = true
        f.returns = var(t{enclosing_obj.type.name, ptr=true}, "returns")
        f.is_method = false
        enclosing_obj.constructor = f
    end
    f.overloads = {f}

    if enclosing_obj.funcs[f.luafunc] then
        -- Add an overload
        local obj = enclosing_obj.funcs[f.luafunc]
        f.overloads = obj.overloads
        table.insert(obj.overloads, f)
    else
        -- Add the function
        table.insert(enclosing_obj.funcs, f) -- array
        enclosing_obj.funcs[f.luafunc] = f        -- hash
    end
end


-- property{"type", "name", get=true, set=true}
-- If get or set is true, register functions "Get"..name and "Set"..name
-- If get or set is a string value, use that as the function name
-- e.g. property{"std::string", "Name"} allows the following:
--    obj()
--    obj:GetName() => name
--    obj:SetName(name)
function property(opts)
    local ptype = t(assert(opts[1]))
    local name = assert(opts[2], "Property requires a name")
    local get = opts.get
    if get == nil then get = true end
    local set = opts.set
    if set == nil then set = true end

    if not bind.class then error("No class") end
    if get then
        if type(get) ~= "string" then
            get = "Get"..name
        end
        func{get, returns=ptype}
    end
    if set then
        if type(set) ~= "string" then
            set = "Set"..name
        end
        func{set, arg(ptype, name:lower())}
    end
end


-- --------------------------------------------------------------------------
-- Function class
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()
bind.function_mt = mt

function mt:new(obj)
    assert(obj, "Function must be created with a table")
    assert(obj.name, "Function must be created with a name")
    assert(obj.args or obj.override, "Function must be created with an args table")

    return bind.obj_mt.new(self, obj)
end

function mt:is_overloaded()
    return #self.overloads > 1
end

function mt:arg_start()
    return ((self.is_method and not self.is_static) or self.is_constructor) and 2 or 1
end

function mt:__tostring()
    local t = {}
    if self.is_static then
        table.insert(t, "static ")
    end
    if self.returns then
        table.insert(t, tostring(self.returns.type))
    else
        table.insert(t, "void")
    end
    table.insert(t, " ")
    table.insert(t, self.name)
    table.insert(t, "(")
    local args = {}
    for _, arg in ipairs(self.args) do
        if arg.default then
            table.insert(args, arg.type:get_qualified_name().." "..arg.name.." = "..arg.default)
        else
            table.insert(args, arg.type:get_qualified_name().." "..arg.name)
        end
    end
    table.insert(t, table.concat(args, ", "))
    table.insert(t, ")")

    return table.concat(t)
end

-- Substitutions
function mt:subst_cfunc()
    if not self.parent then
        return self:subst('prefix').."_"..self.luafunc
    end
    return self.parent.name.."_"..self.luafunc
end
function mt:subst_luafunc()
    return self.luafunc
end

-- Write the executing code of the function
function mt:writeexec(f)

end

-- Write the executing code of the function
function mt:writeexec(f, name)
    f:write("// "..tostring(self).."\n")

    -- methods are named [class]_[function];
    -- free functions are named [prefix]_[function]
    -- methods take as their first parameter the class object.

    if self.is_method and not self.is_static then
        f:write(self:fmt([[
static int [cfunc](lua_State * L)
{
    [ptype] * [pvar] = [pcheckfunc](L, 1);
]], { cfunc = name }))
    else
        f:write(self:fmt([[
static int [cfunc](lua_State * L)
{
]], { cfunc = name }))
    end

    -- Collect the arguments
    local index = self:arg_start()
    local fargs = {}
    if self.has_default then
        f:write("    int argCount = lua_gettop(L);\n")
    end
    for _, arg in ipairs(self.args) do
        -- Typedefs are treated special
        if arg.type.usertype and getmetatable(arg.type.usertype) == bind.typedef_mt then
            assert(not arg.default, "typedefs can't have defaults arguments")
            f:write(arg.type:fmt([[
    [qname] [var];
    [get](L, [idx], &[var]);
]], { idx = index }))
            if arg.type.ptr then
                table.insert(fargs, "*"..arg.name)
            else
                table.insert(fargs, arg.name)
            end
        else
            if arg:add_pointer() then
                table.insert(fargs, "*"..arg.name)
            else
                table.insert(fargs, arg.name)
            end
            if arg.default then
                f:write(arg:fmt([[
    [type] [var] = (argCount >= [idx] ? [get](L, [idx]) : [default]);
]], { idx = index }))
            else
                f:write(arg:fmt([[
    [type] [var] = [get](L, [idx]);
]], { idx = index }))
            end
        end
        index = index + 1
    end

    -- Call the function
    local fstring = ""
    if self.is_constructor then
        fstring = self.parent:fmt("new [namespace]::")
    elseif self.is_method then
        if not self.is_static then
            fstring = self:fmt("[pvar]->")
        else
            fstring = self:fmt("[namespace]::")
        end
    end

    fstring = fstring..string.format("%s(%s)",
                                     self.name,
                                     table.concat(fargs, ", "))
    if self.returns then
        if self.returns:add_pointer() then
            fstring = "&"..fstring
        end

        if self.throws then
            f:write(self.returns:fmt("    [type] returns;\n"))
            fstring = "returns = "..fstring
        else
            fstring = self.returns:fmt("[type] returns = ")..fstring
        end
    end

    if self.throws then
        f:write("    try {\n")
        f:write("        "..fstring..";\n")
        if self.is_constructor then
            f:write(self.returns:fmt([[
        [new](L, returns);
        return 1;
]]))
        elseif self.returns then
            f:write(self.returns:fmt([[
        [push](L, returns);
        return 1;
]]))
        else
            f:write([[
        return 0;
]])
        end
        f:write(self:fmt([[
    }
    catch (...) {
        [prefix]_handleExceptions(L);
        return 0;
    }
]]))
    else -- No throw
        f:write("    "..fstring..";\n")

        if self.is_constructor then
            f:write(self.returns:fmt([[
    [new](L, returns);
    return 1;
]]))
        elseif self.returns then
            f:write(self.returns:fmt([[
    [push](L, returns);
    return 1;
]]))
        else
            f:write([[
    return 0;
]])
        end
    end

    f:write("}\n")
end


function mt:writecpp(f)
    if self.override then
        f:write(self.override)
        return
    end

    if not self:is_overloaded() then
        self:writeexec(f)
        return
    end

    -- Write executing code for all of the overloaded functions
    -- Each function is suffixed with the index in the overloads table
    for i, func in ipairs(self.overloads) do
        func.subst_cfunc_overload = func:subst('cfunc')..i
        func:writeexec(f, func.subst_cfunc_overload)
    end

    -- Figure out which overload we should use
    f:write(self:fmt([[
// [name] overload resolution
static int [cfunc](lua_State * L)
{

]]))

    if self.is_constructor then
        f:write(self:fmt([[
    // metatable is first argument
]]))
    elseif self.is_method then
        f:write(self:fmt([[
    [pcheckfunc](L, 1);
]]))
    end

    -- Return a list of functions given a valid condition and optionally
    -- a table of functions
    local find_funcs = function(condition, func_list)
        local funcs = {}
        for _, func in ipairs(func_list or self.overloads) do
            if condition(func) then
                table.insert(funcs, func)
            end
        end
        return funcs
    end

    -- Helper function for find_funcs
    local has_luatype = function(argnum, luatype)
        return function(func)
            return func.args[argnum].type.luatype == luatype
        end
    end

    -- Helper function for find_funcs
    local has_type = function(argnum, type)
        return function(func)
            return func.args[argnum].type == type
        end
    end

    local find_type_function = function(get_func)
    end

    -- Return a list of argument types for the given functions
    local find_types = function(func_list, get)
        local get = get or function(arg) return arg.type end
        local types = {}
        for _, func in ipairs(func_list) do
            for nargs=1, #func.args do
                if not types[nargs] then
                    types[nargs] = {}
                end
                local type = get(func.args[nargs])
                if not types[nargs][type] then
                    types[nargs][type] = true -- hash
                    table.insert(types[nargs], type) -- array
                end
            end
        end
        return types
    end

    -- Return a list of argument luatypes for the given functions
    local find_luatypes = function(func_list)
        return find_types(func_list, function(arg) return arg.type.luatype end)
    end

    -- Table of possible arguments
    local possible_args = {}
    local used_args = {}
    for _, func in ipairs(self.overloads) do
        for nargs=func.required_args, #func.args do
            if not used_args[nargs] then
                table.insert(possible_args, nargs)
                used_args[nargs] = true
            end
        end
    end
    table.sort(possible_args, function(a,b) return a > b end)

    f:write("    int argCount = lua_gettop(L)")
    if self:arg_start() ~= 1 then
        f:write(" - "..self:arg_start()-1)
    end
    f:write(";\n")


    local check_args -- so that we can call this recursively
    check_args = function(funcs, arg, max_arg, lines, indent)
        local types = find_luatypes(funcs)
        local indent = indent or 2
        local insert = function(str, idt)
            table.insert(lines, string.rep(" ", (indent+(idt or 0))*4)..str)
        end

        -- If ever there is only one possibility, call the function
        if #funcs == 1 then
            insert("// Only one function")
            insert(funcs[1]:fmt("return [cfunc_overload](L);"))
            return
        end

        -- Otherwise we need to check the arguments
        for i=arg, max_arg do
            -- If we have only one type, don't bother with the switch statement.
            if #types[i] > 1 then
                break
            end
            -- Handle userdata
            if types[i][1] == "LUA_TUSERDATA" then
                -- Figure out which userdata
                local index = i + self:arg_start() - 1
                local udtypes = find_types(funcs)
                if #udtypes[i] == 1 then
                    insert("// Argument "..i.." == "..udtypes[i][1].name:subst('type'))
                else
                    for ud_idx, udtype in ipairs(udtypes[i]) do
                        insert((ud_idx == 1 and "if" or "else if")..
                               udtype:fmt(" ([query]").."(L, "..index..")")
                        insert("{")
                        local funcs = find_funcs(has_type(arg, udtype), funcs)
                        check_args(funcs, arg+1, max_arg, lines, indent+1)
                        insert("}")
                    end
                end
                return
            else
                insert("// Argument "..i.." == "..types[i][1])
            end
            arg = arg + 1
        end
        if arg > max_arg then return end

        -- Multiple argument types, use a switch
        types = types[arg]
        local index = arg + self:arg_start() - 1
        insert("// Argument "..arg)
        insert("switch(lua_type(L, "..index..")) {")
        indent = indent + 1

        -- Userdata
        if types["LUA_TUSERDATA"] then
            types["LUA_TUSERDATA"] = false -- Remove this from the list
            insert("case LUA_TLIGHTUSERDATA:")
            insert("case LUA_TUSERDATA:")
                -- Let the userdata bit in the single argument type handle
                -- this.
                local funcs = find_funcs(has_luatype(arg, "LUA_TUSERDATA"), funcs)
                check_args(funcs, arg, max_arg, lines, indent+1)
            insert("break;")
        end

        -- Numbers and strings
        if types["LUA_TSTRING"] and not types["LUA_TNUMBER"] then
            types["LUA_TSTRING"] = false -- Remove from the list
            insert("case LUAT_NUMBER: // fall through (always a string)")
            insert("case LUAT_STRING:")
                local funcs = find_funcs(has_luatype(arg, "LUA_TSTRING"), funcs)
                check_args(funcs, arg+1, max_arg, lines, indent+1)
            insert("break;")
        elseif types["LUA_TNUMBER"] and not types["LUA_TSTRING"] then
            insert("case LUA_TSTRING: // sometimes a number")
            insert("if (! lua_isnumber(L, "..index.."))", 1)
            insert("break;", 2)
            insert("// else fall through", 1)
        end
        -- It's way too hard to be perfectly accurate when we have
        -- both a number and string argument, so we'll just treat
        -- those types strictly (as follows).

        -- All other types (strict type checking)
        for _, type in ipairs(types) do
            if types[type] then -- Check the hash part (we've been deleting the hash part)
                insert("case "..type..":")
                    local funcs = find_funcs(has_luatype(arg, type), funcs)
                    check_args(funcs, arg+1, max_arg, lines, indent+1)
                insert("break;")
            end
        end
        indent = indent - 1
        insert("}")
    end

    local ifs = {}
    for _, nargs in ipairs(possible_args) do
        local lines = {}
        local funcs = find_funcs(function(func)
                                     return nargs >= func.required_args and
                                            nargs <= #func.args
                                 end)

        table.insert(lines, "if (argCount >= "..nargs..")\n    {")
        check_args(funcs, 1, nargs, lines)
        table.insert(lines, "    }")

        table.insert(ifs, table.concat(lines, "\n"))
    end

    f:write("    "..table.concat(ifs, "\n    else "))
    f:write("\n")

    f:write([[
    // Else we didn't find a function
    luaL_error(L, "Unable to find overload");
    return 0;
}

]])
end


function mt:print()
    print(tostring(self))
end
