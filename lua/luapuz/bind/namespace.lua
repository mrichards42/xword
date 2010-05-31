require 'base'
require 'type'

-- --------------------------------------------------------------------------
-- Namespace class
-- --------------------------------------------------------------------------
local mt = bind.base_mt:subclass()
local GLOBAL_NAME = "__G"

function mt:isglobal()
    return self.name == GLOBAL_NAME
end

function mt:getfilename(obj)
    return bind.prefix.."_"..
           self:get_qualified_name():gsub("::", "_")
end


function mt:writehpp(f)
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// namespace [name]
// ---------------------------------------------------------------------------

]]))

    self:writehppheaders(f)

    bind.base_mt.writehpp(self, f)
end

function mt:writecpp(f)
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// namespace [name]
// ---------------------------------------------------------------------------

]]))

    bind.base_mt.writecppheaders(self, f)

    -- Write functions and enumerations
    bind.base_mt.writecpp(self, f)

    -- Register the library
    f:write(self:fmt([[
void [openfunc] (lua_State *L) {
    // register functions
    luaL_register(L, "[name]", [lib]);
]]))
    -- Register the enumerations
    for _, enum in ipairs(self.enums) do
        f:write(enum:fmt("    [prefix]_registerEnum(L, [table], [reg]);\n"))
    end
    -- Register the classes
    for _, class in ipairs(self.classes) do
        f:write(class:fmt("    [openfunc](L);\n"))
    end
    -- Register the namespaces
    if self.namespaces then
        for _, ns in ipairs(self.namespaces) do
            f:write(ns:fmt("    [openfunc](L);\n"))
        end
    end

    if self.parent:isglobal() then
        f:write("    lua_pop(L, 1);\n");
    else
        f:write(self:fmt([[
    // [luatype] = the table
    lua_setfield(L, -2, "[name]");
]]))
    end

    f:write("}\n")

end


function mt:print()
    if self:isglobal() then
        print("namespace {")
    else
        print("namespace "..self.name.." {")
    end
    for _, ns in ipairs(self.namespaces) do
        ns:print()
    end
    bind.base_mt.print(self)
    if self:isglobal() then
        print("} // global namespace")
    else
        print("} // namespace "..self.name)
    end
end


bind.namespace_mt = mt


-- --------------------------------------------------------------------------
-- The global namespace
-- --------------------------------------------------------------------------
bind.global_namespace = mt:new(GLOBAL_NAME)
bind.namespace = bind.global_namespace -- the current namespace

local types = {
    ["char *"] = {
        get  = "luaL_checkstring",
        push = "lua_pushstring",
        query = "lua_isstring",
        luatype = "LUA_TSTRING",
    },
    char = {
        get  = bind.prefix.."_checkchar",
        push = bind.prefix.."_pushchar",
        query = bind.prefix.."_ischar",
        luatype = "LUA_TSTRING"
    },
    ["std::string"] = {
        get  = bind.prefix.."_checkStdString",
        push = bind.prefix.."_pushStdString",
        query = "lua_isstring",
        luatype = "LUA_TSTRING"
    },

    bool = {
        get  = bind.prefix.."_checkboolean",
        push = "lua_pushboolean",
        query = "lua_isboolean",
        luatype = "LUA_TBOOLEAN"
    },

    lua_Number = {
        get  = "luaL_checknumber",
        push = "lua_pushnumber",
        query = "lua_isnumber",
        luatype = "LUA_TNUMBER",
    },
    ["unsigned int"] = {
        get  = bind.prefix.."_checkuint",
        push = "lua_pushnumber",
        query = bind.prefix.."_isuint",
        luatype = "LUA_TNUMBER",
    },
}

types["short"] = types.lua_Number
types["int"] = types.lua_Number
types["long"] = types.lua_Number

types["unsigned char"] = types["unsigned int"]
types["unsigned short"] = types["unsigned int"]
types["unsigned long"] = types["unsigned int"]
types["size_t"] = types["unsigned int"]

types["signed char"] = types["int"]
types["signed short"] = types["short"]
types["signed int"] = types["int"]
types["signed long"] = types["long"]

-- Read the types table into the global namespace:
-- types["name"] = type_object
-- e.g. types["std::string"].push => "function_name"
--      s = types['std::string']
--      s.val => true
--      s.get => "function_name"
for k,v in pairs(types) do
    local type = bind.new_type(k)
    for k, v in pairs(v) do
        type[k] = v
    end
    bind.register_type(type)
end


-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------

-- Find a namespace and set the parent namespace as the current namespace
local function find_next_namespace(names)
    if #names == 1 then
        return bind.namespace.namespaces[names[1]]
    end

    -- Set the current namespace and recurse
    bind.namespace = assert(bind.namespace.namespaces[names[1]],
                            "Unknown namespace: "..tostring(names[1]))
    table.remove(names, 1)
    return find_next_namespace(names)
end

-- find_namespace{ns, sub_ns, ...}
function bind.find_namespace(names)
    if type(names) ~= "table" then
        return bind.find_namespace{names}
    end
    bind.namespace = bind.global_namespace
    return find_next_namespace(names)
end

-- namespace()
-- namespace "name"
-- namespace {"ns", "subns"}
function namespace(opts)
    if not opts then
        bind.namespace = bind.global_namespace
        return
    end
    if type(opts) ~= "table" then
        return namespace{opts}
    end

    local ns = bind.find_namespace(opts)
    if not ns then
        local name = opts[#opts]
        -- Create a new namespace
        print("Creating namespace "..name)
        ns = mt:new(name)
        -- Register this namespace with the current namespace
        table.insert(bind.namespace.namespaces, ns) -- array
        bind.namespace.namespaces[name] = ns        -- hash
    end
    -- Set this as the current namespace
    bind.namespace = ns
    -- Clear the class
    bind.class = nil
end
