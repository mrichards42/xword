-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------

-- enum{"name", "var1", "var2", ...}
-- enum{"name", header="header", "var1", "var2", ...}
-- enum{"name", headers={"header1", "header2"}, "var1", "var2", ...}
function enum(opts)
    local name = opts[1]
    table.remove(opts, 1)
    local headers = opts.headers or { opts.header }
    opts.header = nil
    opts.headers = nil
    local cppheaders = opts.cppheaders or { opts.cppheader }
    opts.cppheader = nil
    opts.cppheaders = nil

    -- Create a new enumeration
    local e = bind.enumeration_mt:new{name=name, types=opts}
    print("Creating enumeration ".. name)
    -- Register this enumeration with the current namespace or class
    local enclosing_obj = bind.class or bind.namespace
    table.insert(enclosing_obj.enums, e) -- array
    enclosing_obj.enums[name] = e        -- hash
    -- Register this enumeration as a new type in the current namespace or class
    bind.register_user_type(name, e, headers, cppheaders, enclosing_obj)
    e.type = t(name)
end


-- --------------------------------------------------------------------------
-- Enumeration class
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()
bind.enumeration_mt = mt

function mt:print()
    print("enum "..self.name.." {")
    for _, name in ipairs(self.types) do
        print(name..',')
    end
    print("}; // enum "..self.name)
end


-- Substitutions
function mt:subst_table()     return self.name.."_meta" end
function mt:subst_ud()        return self.name.."_ud" end
function mt:subst_luatype()   return self:get_qualified_name():gsub("::", ".") end
function mt:subst_reg()       return self.name.."_reg" end
function mt:subst_namespace() return self.parent:get_qualified_name() end
function mt:subst_regfunc()   return self:subst('prefix').."_register"..self:subst('name') end

function mt:writehpp(f)
    f:write(self:fmt([[

// enum [name]
//------------

[api] extern const char * [table];

inline [type] [prefix]_check[name](lua_State * L, int index)
{
    return static_cast<[type]>([prefix]_checkEnum(L, index, [table]));
}

inline void [prefix]_push[name](lua_State * L, [type] [var])
{
    lua_pushnumber(L, [var]);
}

inline bool [prefix]_is[name](lua_State * L, int index)
{
    return [prefix]_isEnum(L, index, [table]);
}

]]))
end

function mt:writecpp(f)
    f:write(self:fmt([[

// enum [name]
//------------

const char * [table] = "[luatype]";

]]))

    -- Write the registration table
    f:write(self:fmt([[
const [prefix]_enumReg [reg][] = {
]]))
    for _, v in ipairs(self.types) do
        f:write(self:fmt('    {"[vname]", [namespace]::[vname]},\n', { vname = v } ))
    end
    f:write('    {NULL, NULL}\n')
    f:write('};\n\n')
end
