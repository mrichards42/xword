#ifndef LUAPUZ_FUNCTIONS_H
#define LUAPUZ_FUNCTIONS_H

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include <iostream>
#include "puz/exceptions.hpp"
#include "puz/puzstring.hpp"


// Exceptions
static void luapuz_handleExceptions(lua_State * L)
{
    try {
        throw;
    }
    catch (std::exception & err) {
        lua_pushstring(L, err.what());
    }
    catch (...) {
        lua_pushstring(L, "Unknown error");
    }
}


// stronger type checking
static bool luapuz_isstring(lua_State * L, int index, bool strong = false)
{
    if (strong)
        return lua_type(L, index) == LUA_TSTRING;
    else
        return lua_isstring(L, index);
}

static bool luapuz_isstringstrong(lua_State * L, int index)
{
    return luapuz_isstring(L, index, true);
}

static bool luapuz_isnumber(lua_State * L, int index, bool strong = false)
{
    if (strong)
        return lua_type(L, index) == LUA_TNUMBER;
    else
        return lua_isstring(L, index);
}

static bool luapuz_isnumberstrong(lua_State * L, int index)
{
    return luapuz_isnumber(L, index, true);
}

// puz::string_t
static void luapuz_pushstring_t(lua_State * L, const puz::string_t & str)
{
    try
    {
        lua_pushstring(L, puz::encode_utf8(str).c_str());
        return;
    }
    catch (...)
    {
        luapuz_handleExceptions(L);
    }
    lua_error(L);
}

static puz::string_t luapuz_checkstring_t(lua_State * L, int index)
{
    try
    {
        return puz::decode_utf8(luaL_checkstring(L, index));
    }
    catch (...)
    {
        luapuz_handleExceptions(L);
    }
    lua_error(L);
}


// char
static void luapuz_pushchar(lua_State * L, const char ch)
{
    lua_pushlstring(L, &ch, 1);
}

static char luapuz_checkchar(lua_State * L, int index)
{
    size_t len;
    const char * str = luaL_checklstring(L, index, &len);
    if (len != 1)
        luaL_typerror(L, index, "character");
    return str[0];
}

static bool luapuz_ischar(lua_State * L, int index, bool strong = false)
{
    if (! luapuz_isstring(L, index, strong))
        return false;
    size_t len;
    lua_tolstring(L, index, &len);
    return len == 1;
}

static bool luapuz_ischarstrong(lua_State * L, int index)
{
    return luapuz_ischar(L, index, true);
}



// bool
static bool luapuz_checkboolean(lua_State * L, int index)
{
    if (! lua_isboolean(L, index))
        luaL_typerror(L, index, "boolean");
    return (bool)lua_toboolean(L, index);
}


// unsigned numbers
static lua_Number luapuz_checkuint(lua_State * L, int index)
{
    lua_Number num = luaL_checknumber(L, index);
    if (num < 0)
        luaL_typerror(L, index, "unsigned number");
    return num;
}

static bool luapuz_isuint(lua_State * L, int index, bool strong = false)
{
    if (! luapuz_isnumber(L, index, strong))
        return false;
    return lua_tonumber(L, index) >= 0;
}

static bool luapuz_isuintstrong(lua_State * L, int index)
{
    luapuz_isuint(L, index, true);
}


// userdata
static bool luapuz_isudata(lua_State *L, int index, const char *tname)
{
    void *p = lua_touserdata(L, index);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, index)) {  /* does it have a metatable? */
            lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
            if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
                lua_pop(L, 2);  /* remove both metatables */
                return true;
            }
            else {
                lua_pop(L, 2);  /* remove both metatables */
                return false;
            }
        }
    }
    return false;
}


// Register a constructor function to the table on the top of the stack
static void luapuz_registerConstructor(lua_State * L, lua_CFunction func)
{
    // Find or create a metatable
    if (! lua_getmetatable(L, -1))
    {
        lua_newtable(L);
/*
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);               // push metatable
        lua_settable(L, -3);                // metatable.__index = metatable
*/
    }

    lua_pushcfunction(L, func);
    lua_setfield(L, -2, "__call");

    // set the metatable
    lua_setmetatable(L, -2);
}




static int luapuz_index(lua_State * L)
{
    if (! lua_isuserdata(L, 1))
    {
        luaL_error(L, "Object is not a userdatum");
        return 0;
    }

    // Push the metatable for the object
    if (! lua_getmetatable(L, 1))
    {
        luaL_error(L, "Object does not have a metatable");
        return 0;
    }

    if (lua_isstring(L, 2))
    {
        // Get the index name
        const char * name = luaL_checkstring(L, 2);

        // rawget the index from the metatable and return it
        // if it exists.
        lua_pushstring(L, name);
        lua_rawget(L, -2);
        if (! lua_isnil(L, -1))
            return 1;
        lua_pop(L, 1);

        // Search for a GetXXX method and call it if it exists.
        lua_pushfstring(L, "%s%s", "Get", name);
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1))
        {
            // Push the original object as the argument to the
            // method.
            lua_pushvalue(L, 1);
            lua_call(L, 1, 1);
            return 1;
        }

        lua_pop(L, 1);

        // Look for an "_index" method (single underscore)
        // and try that one
        lua_pushstring(L, "_index");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1))
        {
            // Push the original object as the first argument
            lua_pushvalue(L, 1);
            // Push the argument to this function as the second argument
            lua_pushvalue(L, 2);
            // Call the index function
            if (lua_pcall(L, 2, 1, 0) == 0)
                return 1;
        }

        luaL_error(L, "method Get%s does not exist", name);
        return 0;
    }

    // Look for an "_index" method (single underscore)
    // and try that one
    lua_pushstring(L, "_index");
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1))
    {
        // Push the original object as the first argument
        lua_pushvalue(L, 1);
        // Push the argument to this function as the second argument
        lua_pushvalue(L, 2);
        // Call the index function
        lua_call(L, 2, 1);
        return 1;
    }
    else
    {
        luaL_typerror(L, 2, "string");
        return 0;
    }
}



static int luapuz_newindex(lua_State * L)
{
    if (! lua_isuserdata(L, 1))
    {
        luaL_error(L, "Object is not a userdatum");
        return 0;
    }

    // Push the metatable for the object
    if (! lua_getmetatable(L, 1))
    {
        luaL_error(L, "Object does not have a metatable");
        return 0;
    }

    if (lua_isstring(L, 2))
    {
        // Get the index name
        const char * name = luaL_checkstring(L, 2);

        // Search for a SetXXX method.
        lua_pushfstring(L, "%s%s", "Set", name);
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1))
        {
            // Push the original object as the first argument
            lua_pushvalue(L, 1);
            // Push the arguments
            lua_pushvalue(L, 3);
            lua_call(L, 2, 0);
            return 0;
        }
        else
        {
            lua_pop(L, 1);

            // Look for a "_newindex" method (single underscore)
            lua_pushstring(L, "_newindex");
            lua_rawget(L, -2);
            if (lua_isfunction(L, -1))
            {
                // Push the original object as the first argument
                lua_pushvalue(L, 1);
                // Push the arguments
                lua_pushvalue(L, 2);
                lua_pushvalue(L, 3);
                lua_call(L, 3, 0);
                return 0;
            }
            else
            {
                luaL_error(L, "method Set%s does not exist", name);
                return 0;
            }
        }
    }
    else
    {
        // Look for a "_newindex" method (single underscore)
        lua_pushstring(L, "_newindex");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1))
        {
            // Push the original object as the first argument
            lua_pushvalue(L, 1);
            // Push the arguments
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_call(L, 3, 0);
            return 0;
        }
        else
        {
            luaL_typerror(L, 2, "string");
            return 0;
        }
    }
}


#endif // LUAPUZ_FUNCTIONS_H
