#ifndef luapuz_puz_Puzzle_helpers_hpp
#define luapuz_puz_Puzzle_helpers_hpp

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include "puz/Puzzle.hpp"

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
    luapuz_pushStdString(L, filename);

    // Call the function, check for errors.
    if (lua_pcall(L, 2, 0, 0) != 0)
    {
        // Throw an exception
        if (! lua_isstring(L, -1))
            throw puz::FatalFileError();
        else
            throw puz::FatalFileError(lua_tostring(L, -1));
    }
}

// Variations on LoadSave_Puzzle for error checking.
static void luapuz_Load_Puzzle(puz::Puzzle * puz,
                              const std::string & filename,
                              void * data)
{
    luapuz_LoadSave_Puzzle(puz, filename, data);
    puz->TestOk();
}


static void luapuz_Save_Puzzle(puz::Puzzle * puz,
                              const std::string & filename,
                              void * data)
{
    puz->TestOk();
    luapuz_LoadSave_Puzzle(puz, filename, data);
}


#endif // luapuz_puz_Puzzle_helpers_hpp
