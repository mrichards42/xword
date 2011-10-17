bind = {}
bind.prefix = "luapuz"

function bind.register_user_type(name, obj, headers, cppheaders, enclosing_obj)
    local enclosing_obj = enclosing_obj or bind.global_namespace
    local headers = headers or {}
    local type_obj

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
    local get = type(obj.get) == "string" and obj.get or bind.prefix.."_check"..name
    local push = type(obj.push) == "string" and obj.push or bind.prefix.."_push"..name
    local query = type(obj.query) == "string" and obj.query or bind.prefix.."_is"..name
    local new = type(obj.new) == "string" and obj.new or bind.prefix.."_new"..name
    -- register the value type
    type_obj = maketype{val=true}
    type_obj.get  = get
    type_obj.push = push
    type_obj.query = query
    bind.register_type(type_obj, enclosing_obj)

    -- register the pointer type
    type_obj = maketype{ptr=true}
    type_obj.get  = get
    type_obj.push = push
    type_obj.query = query
    type_obj.new = new
    bind.register_type(type_obj, enclosing_obj)

    -- register the reference type
    type_obj = maketype{ref=true}
    type_obj.get  = get
    type_obj.push = push
    type_obj.query = query
    bind.register_type(type_obj, enclosing_obj)
end


function bind.register_type(type_obj, enclosing_obj)
    local enclosing_obj = enclosing_obj or bind.global_namespace
    enclosing_obj.types[tostring(type_obj)] = type_obj
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
