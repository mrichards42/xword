#include "luapuz_tracking.hpp"

#include <iostream>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


void luapuz_registerTable(lua_State * L, const char * name)
{
    // Create a table
    lua_newtable(L);
    // registry.name = table
    lua_setfield(L, LUA_REGISTRYINDEX, name);
}



void luapuz_registerWeakTable(lua_State * L, const char * name)
{
    // Create a table
    lua_newtable(L);

    // Set the mode of the table to use weak values (using a metatable)
        lua_newtable(L);                    // metatable
            lua_pushstring(L, "v");
            lua_setfield(L, -2, "__mode");  // mt.__mode = "v"
        lua_setmetatable(L, -2);            // setmetatable(table, mt)

    // registry.name = table
    lua_setfield(L, LUA_REGISTRYINDEX, name);
}

// ---------------------------------------------------------------------------
// Tracked userdata objects
// ---------------------------------------------------------------------------

const char * luapuz_tracked_objects_key = "luapuz tracked objects table";

// Add the object at the top of the stack to the tracked objects table
// using objptr as the key.  i.e.  trackedobjects[objptr] = (top of stack)
// Note that we need the actual lua object, not a pointer to the userdata.
// If we stored the userdata, it would lose its metatable.
LUAPUZ_API void luapuz_track_object(lua_State * L, void * objptr)
{

#ifdef LUAPUZ_DEBUG
    if (! lua_isuserdata(L, -1))
        std::cerr << "Top of stack is not a userdata!" << std::endl;
#endif

    // Push the tracked objects table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_tracked_objects_key);

#ifdef LUAPUZ_DEBUG
    // Make sure this isn't already in the table
    if (luapuz_is_tracked_object(L, objptr))
        std::cerr << "Object is already tracked: " << objptr << std::endl;
#endif

    // table[objptr] = ud
    lua_pushlightuserdata(L, objptr);
    lua_pushvalue(L, -3); // Copy the userdata to the top of the stack
    lua_rawset(L, -3);

    // Pop the tracked objects table
    lua_pop(L, 1);
}


LUAPUZ_API void luapuz_untrack_object(lua_State * L, void * objptr)
{
    // Push the tracked objects table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_tracked_objects_key);

#ifdef LUAPUZ_DEBUG
    // Make sure this is already in the table
    if (! luapuz_is_tracked_object(L, objptr))
        std::cerr << "Object is not tracked: " << objptr << std::endl;
#endif

    // table[objptr] = nil
    lua_pushlightuserdata(L, objptr);
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Remove the tracked objects table
    lua_pop(L, 1);
}


LUAPUZ_API bool luapuz_is_tracked_object(lua_State * L, void * objptr)
{
    // Push the tracked objects table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_tracked_objects_key);

    // Push table[objptr]
    lua_pushlightuserdata(L, objptr);
    lua_rawget(L, -2);

    bool isTracked = ! lua_isnil(L, -1);
    lua_pop(L, 1); // Remove the result

    // Remove the tracked objects table
    lua_pop(L, 1);                  
    return isTracked;
}


// Push the tracked userdata.
// return false and pop the nil if the object is not tracked
LUAPUZ_API bool luapuz_push_tracked_object(lua_State * L, void * objptr)
{
    // Push the tracked objects table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_tracked_objects_key);

    // Push table[objptr]
    lua_pushlightuserdata(L, objptr);
    lua_rawget(L, -2);

    if (lua_isnil(L, -1))
    {
        lua_pop(L, 2); // Pop the tracked objects table and the nil.
        return false;
    }

    // Remove the tracked objects table but leave the userdata
    lua_remove(L, -2);

    return true;
}


// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

const char * luapuz_enumerations_key = "luapuz enumerations table";

// Register an enumeration to the table at the top of the stack.
void luapuz_registerEnum(lua_State * L,
                         const char * name,
                         const luapuz_enumReg * e)
{
    // Create a lookup table for the enumeration (enum_table)
    lua_newtable(L);

    for (; e->label; ++e)
    {
        // top_of_stack[e.label] = e.value
        lua_pushstring(L, e->label);
        lua_pushnumber(L, e->value);
        lua_settable(L, -4);

        // enum_table[e.value] = e.label
        lua_pushnumber(L, e->value);
        lua_pushstring(L, e->label);
        lua_settable(L, -3);
    }

    // global_enumerations_table.name = enum_table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_enumerations_key);
    lua_pushstring(L, name);
    lua_pushvalue(L, -3); // push enum_table
    lua_settable(L, -3);

    lua_pop(L, 2); // Pop enum_table and global enums table
}

// Check whether the value at the given index belongs to the specified enumeration
// and return it.
LUAPUZ_API int luapuz_checkEnum(lua_State * L, int idx, const char * enum_name)
{
    // push the value at the index
    lua_pushvalue(L, idx);

    // push the enumerations table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_enumerations_key);

    // push this enumeration's table
    lua_getfield(L, -1, enum_name);
    if (lua_isnil(L, -1))
    {
        lua_pushfstring(L, "Enumeration \"%s\" does not exist", enum_name);
        lua_error(L);
    }

    // enumerations_table.enum_name[value]
    lua_pushvalue(L, -3);
    lua_gettable(L, -2);
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 4); // nil, enum table, global enum table, copy of value
        luaL_typerror(L, idx, enum_name);
    }
    lua_pop(L, 4); // nil, enum table, global enum table, copy of value

    return static_cast<int>(lua_tonumber(L, idx));
}

// Check whether the value at the given index belongs to the specified enumeration
// and return it.
LUAPUZ_API bool luapuz_isEnum(lua_State * L, int idx, const char * enum_name)
{
    // push the value at the index
    lua_pushvalue(L, idx);

    // push the enumerations table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_enumerations_key);

    // push this enumeration's table
    lua_getfield(L, -1, enum_name);
    if (lua_isnil(L, -1))
    {
        lua_pushfstring(L, "Enumeration \"%s\" does not exist", enum_name);
        lua_error(L);
    }

    // enumerations_table.enum_name[value]
    lua_pushvalue(L, -3);
    lua_gettable(L, -2);
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 4); // nil, enum table, global enum table, copy of value
        return false;
    }
    lua_pop(L, 4); // nil, enum table, global enum table, copy of value
    return true;
}


// ---------------------------------------------------------------------------
// Load Save
// ---------------------------------------------------------------------------

const char * luapuz_loadsave_key = "luapuz load save table";
