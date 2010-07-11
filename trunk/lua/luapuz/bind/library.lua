require 'base'

function library(name, table)
    assert(not bind.library, "Only one library is allowed")
    bind.prefix = name
    bind.library = bind.library_mt:new(name)

    if table then
        bind.table = table
    end
end

-- --------------------------------------------------------------------------
-- Library class
-- --------------------------------------------------------------------------
local mt = bind.base_mt:subclass()

function mt:getfilename(obj)
    return self.name
end

function mt:subst_openfunc() return "luaopen_"..self.name end
function mt:subst_table()    return bind.table end

-- Get all headers used in this library
function mt:get_all_headers(obj)
    local obj = obj or bind.global_namespace
    local headers = {}

    -- Add the headers for any contained namespaces or classes
    for _, class in ipairs(obj.classes) do
        headers[class:gethppfilename()] = true
        -- Add contained headers
        for _, header in ipairs(self:get_all_headers(class)) do
            headers[header] = true
        end
    end
    for _, ns in ipairs(obj.namespaces) do
        headers[ns:gethppfilename()] = true
        -- Add contained headers
        for _, header in ipairs(self:get_all_headers(ns)) do
            headers[header] = true
        end
    end
    -- Make a sequential list
    local ret = {}
    for header, _ in pairs(headers) do
        table.insert(ret, header)
    end
    return ret
end

function mt:writehpp(f)
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// [name]
// ---------------------------------------------------------------------------

]]))

    for _, header in ipairs(self:get_all_headers()) do
        f:write(string.format('#include "%s"\n', header))
    end
    f:write("\n")

    f:write(self:fmt("[api] int [openfunc] (lua_State *L);\n"))
end

function mt:writecpp(f)
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// [name]
// ---------------------------------------------------------------------------

]]))
    bind.global_namespace:writecppheaders(f)

    f:write(self:fmt([[

// This needs to be extern "C" because of C++ name mangling
extern "C" {

[api] int [openfunc] (lua_State *L) {
    // For tracking userdata objects
    [prefix]_registerWeakTable(L, [prefix]_tracked_objects_key);
    // For registering enumerations
    [prefix]_registerTable(L, [prefix]_enumerations_key);
    // For tracking load/save functions
    [prefix]_registerTable(L, [prefix]_loadsave_key);

    // Register (sub)tables
]]))
    -- Register the namespaces
    for _, ns in ipairs(bind.global_namespace.namespaces) do
        if ns.register ~= false then
            f:write(ns:fmt("    [openfunc](L);\n"))
        end
    end

    f:write(self:fmt([[

    return 1;
}

} // extern "C"

]]))

end



bind.library_mt = mt
