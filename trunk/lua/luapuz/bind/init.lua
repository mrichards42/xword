bind = {}
bind.prefix = "luapuz"

function bind.register_user_type(name, obj, headers, cppheaders, enclosing_obj)
    local enclosing_obj = enclosing_obj or bind.global_namespace
    local headers = headers or {}
    local type

    local maketype = function(opts)
        opts.name = name
        opts.headers = headers
        opts.cppheaders = cppheaders
        if not obj.luatype then
            if getmetatable(obj) == bind.class_mt then
                opts.luatype = "LUA_TUSERDATA"
            elseif getmetatable(obj) == bind.enumeration_mt then
                opts.luatype = "LUA_TNUMBER"
            else
                error "Unknown type"
            end
        end
        local t = bind.new_type(opts)
        t.usertype = obj
        return t
    end
    -- register the value type
    type = maketype{val=true}
    type.get  = bind.prefix.."_check"..name
    type.push = bind.prefix.."_push"..name
    type.query = bind.prefix.."_is"..name
    bind.register_type(type, enclosing_obj)

    -- register the pointer type
    type = maketype{ptr=true}
    type.get  = bind.prefix.."_check"..name
    type.push = bind.prefix.."_push"..name
    type.query = bind.prefix.."_is"..name
    type.new = bind.prefix.."_new"..name
    bind.register_type(type, enclosing_obj)

    -- register the reference type
    type = maketype{ref=true}
    type.get  = bind.prefix.."_check"..name
    type.push = bind.prefix.."_push"..name
    type.query = bind.prefix.."_is"..name
    bind.register_type(type, enclosing_obj)
end


function bind.register_type(type, enclosing_obj)
    local enclosing_obj = enclosing_obj or bind.global_namespace
    enclosing_obj.types[tostring(type)] = type
end

require 'library'
require 'namespace'
require 'class'
require 'function'
require 'enumeration'
require 'typedef'

function bind.run()
    bind.global_namespace:writefiles()
    bind.library:writefiles()
end
