require 'variable'

-- --------------------------------------------------------------------------
-- Argument class
-- --------------------------------------------------------------------------
local mt = bind.variable_mt:subclass()

-- Substitutions
function mt:subst_default()  return self.default end

function mt:write(f)

end


bind.argument_mt = mt

-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------
-- arg(type, name)
-- arg(type, name, default)
function arg(type, name, default)
    local v = var(type, name)
    return mt:new{name = v.name, type = v.type, default = default}
end
