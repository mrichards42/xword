// ---------------------------------------------------------------------------
// luapuz_puz_Square.hpp was generated by puzbind.lua
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

#ifndef luapuz_puz_Square_hpp
#define luapuz_puz_Square_hpp

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "../luapuz_functions.hpp"
#include "../luapuz_tracking.hpp"

// ---------------------------------------------------------------------------
// class Square
// ---------------------------------------------------------------------------

#include "puz/Square.hpp"

LUAPUZ_API extern const char * Square_meta;

// Square userdata
// Userdata member should_gc is used to suppress garbage collection of the
// actual data.
// Calling luapuz_newSquare() with default parameters enables
// garbage collection.
// Calling luapuz_pushSquare/Ref() with default parameters
// suppresses garbage collection.
struct LUAPUZ_API Square_ud
{
    puz::Square * square;
    bool should_gc;
};

// Get the userdata
inline Square_ud * luapuz_checkSquare_ud(lua_State * L, int index)
{
    return (Square_ud *)luaL_checkudata(L, index, Square_meta);
}

// Get the actual data
inline puz::Square * luapuz_checkSquare(lua_State * L, int index)
{
    Square_ud * ud = luapuz_checkSquare_ud(L, index);
    if (! ud->square)
        luaL_typerror(L, index, Square_meta);
    return ud->square;
}


// Check if this is the correct data type
inline bool luapuz_isSquare(lua_State *L, int index, const char *tname)
{
    return luapuz_isudata(L, index, tname);
}

// Create a new userdata with actual data and push it on the stack.
// The userdata will be tracked in the tracked objects table.
LUAPUZ_API void luapuz_newSquare(lua_State * L, puz::Square * square, bool should_gc = true);

// Push the actual data.
// If we have already tracked this userdata, push that userdata.
inline void luapuz_pushSquare(lua_State * L, puz::Square * square, bool should_gc = false)
{
    if (! square)
        lua_pushnil(L);
    else if (! luapuz_push_tracked_object(L, square))
        luapuz_newSquare(L, square, should_gc);
}


void luapuz_openSquarelib (lua_State *L);
#endif // luapuz_puz_Square_hpp
