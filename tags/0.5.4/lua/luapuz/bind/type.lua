require 'base'

-- --------------------------------------------------------------------------
-- Type class
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()

function mt:new(obj)
    assert(obj, "Type must be created with a table")
    assert(obj.name, "Type must be created with a name")
    assert(obj.ptr or obj.ref or obj.val,
           "Type must be created as a value, pointer, or reference")

    if obj.header then
        obj.headers = { obj.header }
        obj.header = nil
    end
    obj.headers = obj.headers or {}

    return bind.obj_mt.new(self, obj)
end

function mt:get_luaheader()
    if self.usertype then
        return self.usertype:gethppfilename()
    end
    return nil
end

function mt:__tostring()
    if self.val then return self.name end
    if self.ptr then return self.name.." *" end
    if self.ref then return self.name.." &" end
end

function mt:get_qualified_name()
    if self.val then return bind.obj_mt.get_qualified_name(self) end
    if self.ptr then return bind.obj_mt.get_qualified_name(self).." *" end
    if self.ref then return bind.obj_mt.get_qualified_name(self).." &" end
end


-- Substitutions
function mt:subst_name()  return tostring(self) end
function mt:subst_get()   return self.get end
function mt:subst_check() return self.get end
function mt:subst_push()  return self.push end
function mt:subst_query() return self.query end
function mt:subst_new()   return self.new end
function mt:subst_qname() return bind.obj_mt.get_qualified_name(self) end


bind.type_mt = mt

local function trim_ptr(name)
    return name:sub(0, -2):gsub(" +$", "")
end


-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------

-- Get the "real" type from the types table of this object or any class or
-- namespace underneath it.
function bind.find_type(t, obj)
    if not obj then
        -- Check the following, in order:
        -- Current class
        if bind.class then
            local ret = bind.find_type(t, bind.class)
            if ret then return ret end
        end
        -- Current namespace
        local ret = bind.find_type(t, bind.namespace)
        if ret then return ret end
        -- Global namespace
        if not bind.namespace:isglobal() then
            local ret = bind.find_type(t, bind.global_namespace)
            if ret then return ret end
        end
        return nil
    end

    local type = obj.types[tostring(t)]
    if type then
        return type
    end
    -- Search classes
    for _, cls in pairs(obj.classes) do
        local ret = bind.find_type(t, cls)
        if ret then return ret end
    end
    -- Search namespaces
    if obj.namespaces then
        for _, ns in pairs(obj.namespaces) do
            local ret = bind.find_type(t, ns)
            if ret then return ret end
        end
    end
end


-- Create and return the type object
function bind.new_type(opts)
    local obj
    if type(opts) == "string" then
        -- Parse the pointer or reference out
        if opts:sub(-1) == "*" then
            obj = mt:new{
                name = trim_ptr(opts),
                ptr = true
            }
        elseif opts:sub(-1) == "&" then
            obj = mt:new{
                name = trim_ptr(opts),
                ref = true
            }
        else
            obj = mt:new{
                name = opts,
                val = true
            }
        end
    else
        obj = mt:new{
            name = opts.name or opts[1],
            ref = opts.ref,
            ptr = opts.ptr,
            val = not (opts.ref or opts.ptr),
            headers = opts.headers or {opts.header},
            cppheaders = opts.cppheaders or {opts.cppheader},
            luatype = opts.luatype,
        }
    end
    return obj
end

-- Lookup and return the "real" type object
-- t("name")
-- t("name *")
-- t("name &")
-- t{"name", ptr=true}
-- t{"name", ref=true}
function t(opts)
    if getmetatable(opts) == bind.type_mt then
        return bind.find_type(opts)
    end
    local type = bind.new_type(opts)
    local ret = assert(bind.find_type(type), "Unknown type: "..tostring(type))
    return ret
end
