require 'base'
require 'type'

-- --------------------------------------------------------------------------
-- Variable class
-- --------------------------------------------------------------------------
local mt = bind.obj_mt:subclass()

function mt:new(obj)
    assert(obj, "Variable must be created with a table")
    assert(obj.name, "Variable must be created with a name")
    assert(obj.type, "Variable must be created with a type")

    return bind.obj_mt.new(self, obj)
end

function mt:needs_pointer()
    return self.type.usertype and self.type.usertype.needs_pointer
end

function mt:add_pointer()
    return self:needs_pointer() and not self.type.ptr
end

-- Substitutions
function mt:subst_type()
    if self:needs_pointer() then
        return bind.obj_mt.get_qualified_name(self.type).." *"
    else
        return self.type:get_qualified_name()
    end
end

function mt:subst_new()      return self.type.new end
function mt:subst_var()      return self.name end
function mt:subst_qname()    return self.type:subst_qname() end
function mt:subst_get()      return self.type.get end
function mt:subst_push()     return self.type.push end
function mt:subst_query()      return self.type.query end
function mt:subst_strongquery()     return self.type.strongquery or self.type.query end


function mt:write(f)
end


bind.variable_mt = mt

-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------
-- var(type, "name")
-- var("typestr", "name")
-- var({"typename", ref=true}, "name")
-- var({"typename", ptr=true}, "name")
-- var({"typename", val=true}, "name")
function var(type, name)
    local obj
    if getmetatable(type) == bind.type_mt then
        obj = mt:new{name = name, type = type}
    else
        obj = mt:new{name = name, type = t(type)}
    end

    return obj
end
