#ifndef luapuz_puz_Puzzle_helpers_hpp
#define luapuz_puz_Puzzle_helpers_hpp

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include "puz/Puzzle.hpp"
#include "puz/puzstring.hpp"

#include "luapuz_puz_Puzzle.hpp"
#include "../luapuz_functions.hpp"


// These expect the function to be on the stack.

static void luapuz_LoadSave_Puzzle(puz::Puzzle * puz,
                                   const std::string & filename,
                                   void * data)
{
    // Cast to the correct type
    lua_State * L = (lua_State*)data;

    luaL_checktype(L, -1, LUA_TFUNCTION);

    // Push the function with arguments
    lua_pushvalue(L, -1);
    luapuz_pushPuzzle(L, puz);
    lua_pushstring(L, filename.c_str());

    // Call the function, check for errors.
    // If an error is raised, throw a LoadError.
    // If a function returns false, throw a FileTypeError
    if (lua_pcall(L, 2, 1, 0) == 0)
    {
        if (lua_isboolean(L, -1) && ! lua_toboolean(L, -1))
            throw puz::FileTypeError();
    }
    else // Error
    {
        if (lua_isstring(L, -1))
            throw puz::LoadError(lua_tostring(L, -1));
        else
            throw puz::LoadError("Error loading file");
    }
}

// Variations on LoadSave_Puzzle for error checking.
static void luapuz_Load_Puzzle(puz::Puzzle * puz,
                               const std::string & filename,
                               void * data)
{
    luapuz_LoadSave_Puzzle(puz, filename, data);
}


static void luapuz_Save_Puzzle(puz::Puzzle * puz,
                               const std::string & filename,
                               void * data)
{
    luapuz_LoadSave_Puzzle(puz, filename, data);
}


#endif // luapuz_puz_Puzzle_helpers_hpp
