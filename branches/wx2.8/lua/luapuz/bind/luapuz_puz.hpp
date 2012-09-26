// ---------------------------------------------------------------------------
// luapuz_puz.hpp was generated by puzbind.lua
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

#ifndef luapuz_puz_hpp
#define luapuz_puz_hpp

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "../luapuz_functions.hpp"
#include "../luapuz_tracking.hpp"

// ---------------------------------------------------------------------------
// namespace puz
// ---------------------------------------------------------------------------

#include "puz/Grid.hpp"
#include <algorithm>
#include "puz/Clue.hpp"
#include "puz/Square.hpp"

// enum GridDirection
//------------

LUAPUZ_API extern const char * GridDirection_meta;

inline puz::GridDirection luapuz_checkGridDirection(lua_State * L, int index)
{
    return static_cast<puz::GridDirection>(luapuz_checkEnum(L, index, GridDirection_meta));
}

inline void luapuz_pushGridDirection(lua_State * L, puz::GridDirection griddirection)
{
    lua_pushnumber(L, griddirection);
}

inline bool luapuz_isGridDirection(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, GridDirection_meta);
}


// enum FindDirection
//------------

LUAPUZ_API extern const char * FindDirection_meta;

inline puz::FindDirection luapuz_checkFindDirection(lua_State * L, int index)
{
    return static_cast<puz::FindDirection>(luapuz_checkEnum(L, index, FindDirection_meta));
}

inline void luapuz_pushFindDirection(lua_State * L, puz::FindDirection finddirection)
{
    lua_pushnumber(L, finddirection);
}

inline bool luapuz_isFindDirection(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, FindDirection_meta);
}


// enum GextFlag
//------------

LUAPUZ_API extern const char * GextFlag_meta;

inline puz::GextFlag luapuz_checkGextFlag(lua_State * L, int index)
{
    return static_cast<puz::GextFlag>(luapuz_checkEnum(L, index, GextFlag_meta));
}

inline void luapuz_pushGextFlag(lua_State * L, puz::GextFlag gextflag)
{
    lua_pushnumber(L, gextflag);
}

inline bool luapuz_isGextFlag(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, GextFlag_meta);
}


// enum CheckTest
//------------

LUAPUZ_API extern const char * CheckTest_meta;

inline puz::CheckTest luapuz_checkCheckTest(lua_State * L, int index)
{
    return static_cast<puz::CheckTest>(luapuz_checkEnum(L, index, CheckTest_meta));
}

inline void luapuz_pushCheckTest(lua_State * L, puz::CheckTest checktest)
{
    lua_pushnumber(L, checktest);
}

inline bool luapuz_isCheckTest(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, CheckTest_meta);
}


// enum GridFlag
//------------

LUAPUZ_API extern const char * GridFlag_meta;

inline puz::GridFlag luapuz_checkGridFlag(lua_State * L, int index)
{
    return static_cast<puz::GridFlag>(luapuz_checkEnum(L, index, GridFlag_meta));
}

inline void luapuz_pushGridFlag(lua_State * L, puz::GridFlag gridflag)
{
    lua_pushnumber(L, gridflag);
}

inline bool luapuz_isGridFlag(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, GridFlag_meta);
}


// enum GridType
//------------

LUAPUZ_API extern const char * GridType_meta;

inline puz::GridType luapuz_checkGridType(lua_State * L, int index)
{
    return static_cast<puz::GridType>(luapuz_checkEnum(L, index, GridType_meta));
}

inline void luapuz_pushGridType(lua_State * L, puz::GridType gridtype)
{
    lua_pushnumber(L, gridtype);
}

inline bool luapuz_isGridType(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, GridType_meta);
}


// enum FindOptions
//------------

LUAPUZ_API extern const char * FindOptions_meta;

inline puz::FindOptions luapuz_checkFindOptions(lua_State * L, int index)
{
    return static_cast<puz::FindOptions>(luapuz_checkEnum(L, index, FindOptions_meta));
}

inline void luapuz_pushFindOptions(lua_State * L, puz::FindOptions findoptions)
{
    lua_pushnumber(L, findoptions);
}

inline bool luapuz_isFindOptions(lua_State * L, int index)
{
    return luapuz_isEnum(L, index, FindOptions_meta);
}


// typedef ClueList
//-------------

LUAPUZ_API void luapuz_checkClueList(lua_State * L, int index, puz::ClueList * cluelist);

LUAPUZ_API int luapuz_pushClueList(lua_State * L, puz::ClueList * cluelist);

void luapuz_openpuzlib (lua_State *L);
#endif // luapuz_puz_hpp
