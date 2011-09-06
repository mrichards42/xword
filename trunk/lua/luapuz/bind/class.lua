require 'base'

-- --------------------------------------------------------------------------
-- Global functions
-- --------------------------------------------------------------------------
bind.class = nil

-- Find a class and set the parent class as the current class
local function find_next_class(names, enclosing_obj)
    local enclosing_obj = enclosing_obj or bind.class or bind.namespace
    if #names == 1 then
        return enclosing_obj.classes[names[1]]
    end

    -- Set the current class and recurse
    bind.class = assert(enclosing_obj.classes[names[1]],
                        "Unknown class: "..tostring(names[1]))
    table.remove(names, 1)
    return find_next_class(names, enclosing_obj)
end

-- find_class{class, subclass, ...}
function bind.find_class(names)
    if type(names) ~= "table" then
        return bind.find_class{names}
    end
    bind.class = nil
    return find_next_class(names)
end

-- class()
-- class "name"
-- class {"class", "subclass"}
-- class {"name", headers = {"header1", "header2", ... }}
-- class {"name", header = "header" }
function class(opts)
    if not opts then
        print ("No class")
        bind.class = nil
        return
    end
    if type(opts) ~= "table" then
        return class{opts}
    end

    local cls = bind.find_class(opts)
    if not cls then
        -- Create a new class
        local name = opts[#opts]
        local headers = opts.headers or {opts.header}
        local cppheaders = opts.cppheaders or {opts.cppheader}
        cls = bind.class_mt:new(name)
        print("Creating class ".. name)
        -- Register this class with the current namespace or class
        local enclosing_obj = bind.class or bind.namespace
        table.insert(enclosing_obj.classes, cls) -- array
        enclosing_obj.classes[name] = cls        -- hash
        -- Register this class as a new type in the current namespace or class
        bind.register_user_type(name, cls, headers, cppheaders, enclosing_obj)
        cls.type = t(name)
    end
    -- Set the current class
    bind.class = cls
end



-- --------------------------------------------------------------------------
-- Class class
-- --------------------------------------------------------------------------
local mt = bind.base_mt:subclass()
bind.class_mt = mt

function mt:new(name)
    local obj = bind.base_mt.new(self, name)
    obj.needs_pointer = true
    return obj
end

function mt:getfilename()
    return bind.prefix.."_"..
           self:get_qualified_name():gsub("::", "_")
end

function mt:print()
    print("class "..self.name.." {")
    bind.base_mt.print(self)
    print("}; // class "..self.name)
end

-- Substitutions
function mt:subst_meta()    return self.name.."_meta" end
function mt:subst_ud()      return self.name.."_ud" end
function mt:subst_checkudfunc()  return self.type.get.."_ud" end
function mt:subst_checkfunc()    return self.type.get end
function mt:subst_pushfunc()     return self.type.push end
function mt:subst_queryfunc()     return self.type.query end
function mt:subst_newfunc()      return self:subst('prefix')..'_new'..self.name end
function mt:subst_constructor()  return self.constructor:subst('cfunc') end

function mt:writehpp(f)
    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// class [name]
// ---------------------------------------------------------------------------

]]))

    self:writehppheaders(f)

    f:write(self:fmt([[

[api] extern const char * [meta];

// [name] userdata
// Userdata member should_gc is used to suppress garbage collection of the
// actual data.
// Calling [newfunc]() with default parameters enables
// garbage collection.
// Calling [pushfunc]/Ref() with default parameters
// suppresses garbage collection.
struct [api] [ud]
{
    [type] * [var];
    bool should_gc;
};

// Get the userdata
inline [ud] * [checkudfunc](lua_State * L, int index)
{
    return ([ud] *)luaL_checkudata(L, index, [meta]);
}

// Get the actual data
inline [type] * [checkfunc](lua_State * L, int index)
{
    [ud] * ud = [checkudfunc](L, index);
    if (! ud->[var])
        luaL_typerror(L, index, [meta]);
    return ud->[var];
}


// Check if this is the correct data type
inline bool [queryfunc](lua_State *L, int index)
{
    return [prefix]_isudata(L, index, [meta]);
}

// Create a new userdata with actual data and push it on the stack.
// The userdata will be tracked in the tracked objects table.
[api] void [newfunc](lua_State * L, [type] * [var], bool should_gc = true);

// Push the actual data.
// If we have already tracked this userdata, push that userdata.
inline void [pushfunc](lua_State * L, [type] * [var], bool should_gc = false)
{
    if (! [var])
        lua_pushnil(L);
    else if (! [prefix]_push_tracked_object(L, [var]))
        [newfunc](L, [var], should_gc);
}


]]))

    bind.base_mt.writehpp(self, f)
end

function mt:writecpp(f)
    self:writecppheaders(f)

    f:write(self:fmt([[
// ---------------------------------------------------------------------------
// class [name]
// ---------------------------------------------------------------------------


]]))

    f:write(self:fmt([[

const char * [meta] = "[luatype]";

// Create a new userdata with actual data and push it on the stack.
// The userdata will be tracked in the tracked objects table.
[api] void [newfunc](lua_State * L,
                               [type] * [var],
                               bool should_gc)
{
    // Create the userdata
    [ud] * ud =
        ([ud] *)lua_newuserdata(L, sizeof([ud]));

    ud->should_gc = should_gc;

    // Set the data member
    ud->[var] = [var];

    // Set the metatable
    luaL_getmetatable(L, [meta]);
    lua_setmetatable(L, -2);

    // Track the object
    [prefix]_track_object(L, [var]);
}



// Delete the data on garbage collection
int [name]_gc(lua_State * L)
{
#ifdef LUAPUZ_DEBUG
    std::cout << "Garbage collecting [type] ";
#endif
    [ud] * ud = [checkudfunc](L, 1);
    [prefix]_untrack_object(L, ud->[var]);
    if (ud->should_gc)
    {
        // If the user calls [name]:__gc() before this object
        // is garbage collected, we might try to delete ud->[var] twice.
        ud->should_gc = false;
        delete ud->[var];
#ifdef LUAPUZ_DEBUG
        std::cout << "and deleting data" << std::endl;
#endif
    }
    else
    {
#ifdef LUAPUZ_DEBUG
        //std::cout << "but not deleting data" << std::endl;
#endif
    }
    return 0;
}

// tostring() -> userdata 0xHHHHH ([luatype] 0xHHHH)
int [name]_tostring(lua_State * L)
{
    [ud] * ud = [checkudfunc](L, 1);
    lua_pushfstring(L, "userdata: %p (%s: %p gc = %s)", ud, [meta], ud->[var], ud->should_gc ? "true" : "false");
    return 1;
}

// [name] functions
//----------------
]]))

    -- Write functions and enumerations, and a registration table
    bind.base_mt.writecpp(self, f)

    -- Register the library
    f:write(self:fmt([[
const luaL_reg class[lib][] = {
    { "__gc",        [name]_gc },
    { "__index",     [prefix]_index },
    { "__newindex",  [prefix]_newindex },
    { "__tostring",  [name]_tostring },
    { NULL, NULL }
};

void [openfunc] (lua_State *L) {
    // The [name] table, and the metatable for [name] objects
    luaL_newmetatable(L, [meta]);

    // register metatable functions
    luaL_register(L, NULL, [lib]);
    luaL_register(L, NULL, class[lib]);
]]))
    -- Register the constructor
    if self.constructor then
        f:write(self:fmt([[
    // Register constructor
    [prefix]_registerConstructor(L, [constructor]);
]]))
    end
    -- Register the enumerations
    for _, enum in ipairs(self.enums) do
        f:write(enum:fmt("    [prefix]_registerEnum(L, [table], [reg]);\n"))
    end
    -- Register the classes
    for _, class in ipairs(self.classes) do
        f:write(class:fmt("    [openfunc](L);\n"))
    end

    f:write(self:fmt([[

    // [luatype] = the table
    lua_setfield(L, -2, "[name]");
}
]]))
end
