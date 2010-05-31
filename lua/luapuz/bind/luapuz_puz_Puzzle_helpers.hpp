#ifndef luapuz_pua_Puzzle_helpers_hpp
#define luapuz_pua_Puzzle_helpers_hpp

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include "puz/Puzzle.hpp"

#include "luapuz_puz_Puzzle.hpp"
#include "../luapuz_functions.hpp"

static void luapuz_LoadSave_Puzzle(puz::Puzzle * puz,
                                   const std::string & filename,
                                   void * data);

struct luapuz_LoadSaveData
{
    lua_State * L;
    int funcRef;

    luapuz_LoadSaveData(lua_State * arg_L)
        : L(arg_L)
    {
        // Top of stack must be the function
        if (! lua_isfunction(L, -1))
            luaL_typerror(L, -1, "function");

        // Push the load/save table
        lua_getfield(L, LUA_REGISTRYINDEX, luapuz_loadsave_key);
        lua_pushvalue(L, -2); // Push the function
        // Create a reference to the function in the load/save table
        funcRef = luaL_ref(L, -2);
        // Pop the load/save table
        lua_pop(L, 1);
    }

    void unref()
    {
        // Push the load/save table
        lua_getfield(L, LUA_REGISTRYINDEX, luapuz_loadsave_key);
        // Free the reference
        luaL_unref(L, -1, funcRef);
        // Pop the load/save tablbe
        lua_pop(L, 1);
    }

    void call(puz::Puzzle * puz, const std::string & filename)
    {
        // Push the load/save table
        lua_getfield(L, LUA_REGISTRYINDEX, luapuz_loadsave_key);
        lua_rawgeti(L, -1, funcRef); // Push the function
        lua_remove(L, -2); // Remove the load save table
        luapuz_pushPuzzle(L, puz);  // Push the arguments
        luapuz_pushStdString(L, filename);
        lua_call(L, 2, 0); // Call the function
    }

    static const char * meta;
};

const char * luapuz_LoadSaveData::meta = "luapuz_LoadSaveData_meta";

struct luapuz_LoadSaveData_ud
{
    luapuz_LoadSaveData * data;
};

inline luapuz_LoadSaveData * luapuz_checkLoadSaveData(lua_State * L, int index)
{
    luapuz_LoadSaveData_ud * ud =
        (luapuz_LoadSaveData_ud *)
            luaL_checkudata(L, index, luapuz_LoadSaveData::meta);
    return ud->data;
}

// Remove the handler from the puz::Puzzle lists, then
// delete the data pointer.
static int luapuz_LoadSaveData_gc(lua_State * L)
{
    luapuz_LoadSaveData * data = luapuz_checkLoadSaveData(L, 1);
    // We don't know if this is a load or save handler, so try both.
    puz::Puzzle::RemoveLoadHandler(luapuz_LoadSave_Puzzle, data);
    puz::Puzzle::RemoveSaveHandler(luapuz_LoadSave_Puzzle, data);
    // Clean up the data
    data->unref();
    delete data;
    return 0;
}

static int luapuz_newLoadSaveData(lua_State * L)
{
    luapuz_LoadSaveData * data = new luapuz_LoadSaveData(L);
    // Wrap this in a userdata so it can be garbage collected
    luapuz_LoadSaveData_ud * ud =
        (luapuz_LoadSaveData_ud *)
            lua_newuserdata(L, sizeof(luapuz_LoadSaveData_ud));
    ud->data = data;

    // Push the load/save table
    lua_getfield(L, LUA_REGISTRYINDEX, luapuz_loadsave_key);
    lua_pushvalue(L, -2); // Push the userdata
    // Create a reference to the userdata in the load/save table
    luaL_ref(L, -2);
    // Pop the load/save table
    lua_pop(L, 1);

    // Set the metatable
    // newmetatable returns 1 if a new table was created
    // otherwise it returns 0 and pushes the metatable
    if (luaL_newmetatable(L, luapuz_LoadSaveData::meta) == 1)
    {
        // Garbage collection function.
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, luapuz_LoadSaveData_gc);
        lua_settable(L, -3);
    }
    lua_setmetatable(L, -2);
    return 1; // the userdata is on the stack
}

// Expects a lua function to be on the top of the stack
static void luapuz_LoadSave_Puzzle(puz::Puzzle * puz,
                                   const std::string & filename, void * data)
{
    // Cast to the correct type
    luapuz_LoadSaveData * d = (luapuz_LoadSaveData *)data;
    d->call(puz, filename);
}


#endif // luapuz_pua_Puzzle_helpers_hpp