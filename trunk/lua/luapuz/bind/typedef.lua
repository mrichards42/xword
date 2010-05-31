-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------

-- typedef{"name", luatype="typename", check="code", push="code", query="code"}
function typedef(opts)
    opts.name = opts[1]
    local name = opts.name
    table.remove(opts, 1)
    assert(opts.luatype)
    assert(opts.check or opts.push or opts.query)

    local headers = opts.headers or { opts.header }
    opts.header = nil
    opts.headers = nil
    local cppheaders = opts.cppheaders or { opts.cppheader }
    opts.cppheader = nil
    opts.cppheaders = nil

    -- Create a new typedef
    local td = bind.typedef_mt:new(opts)
    print("Creating typedef ".. name)
    -- Register this typdef with the current namespace or class
    local enclosing_obj = bind.class or bind.namespace
    table.insert(enclosing_obj.typedefs, td) -- array
    enclosing_obj.typedefs[name] = td        -- hash
    -- Register this typedef as a new type in the current namespace or class
    bind.register_user_type(name, td, headers, cppheaders, enclosing_obj)
    td.type = t(name)
end


-- --------------------------------------------------------------------------
-- Typedef class
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()
bind.typedef_mt = mt

function mt:new(opts)
    local obj = bind.obj_mt.new(self, opts)
    obj.needs_pointer = true
    return obj
end

-- Substitutions
function mt:subst_checkfunc()    return self.type.get end
function mt:subst_pushfunc()     return self.type.push end
function mt:subst_queryfunc()    return self.type.query end

function mt:writehpp(f)
    f:write(self:fmt([[

// typedef [name]
//-------------

]]))
    if self.check then
        f:write(self:fmt([[
[api] void [checkfunc](lua_State * L, int index, [type] * [var]);

]]))
    end

    if self.push then
        f:write(self:fmt([[
[api] int [pushfunc](lua_State * L, [type] * [var]);

]]))
    end

    if self.query then
        f:write(self:fmt([[
[api] bool [queryfunc](lua_State * L, int index);

]]))
    end
end

function mt:writecpp(f)
    f:write(self:fmt([[

// typedef [name]
//------------

]]))

    if self.check then
        f:write(self.check)
        f:write("\n")
    end
    if self.push then
        f:write(self.push)
        f:write("\n")
    end
    if self.query then
        f:write(self.query)
        f:write("\n")
    end
end
