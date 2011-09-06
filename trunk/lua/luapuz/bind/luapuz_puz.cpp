// ---------------------------------------------------------------------------
// luapuz_puz.cpp was generated by puzbind.lua
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

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

#include "luapuz_puz_Grid.hpp"
#include "luapuz_puz_Puzzle.hpp"
#include "luapuz_puz_Square.hpp"
#include "luapuz_puz.hpp"
// GridDirection ConstrainDirection(unsigned short dir)
static int puz_ConstrainDirection(lua_State * L)
{
    unsigned short dir = luapuz_checkuint(L, 1);
    puz::GridDirection returns = puz::ConstrainDirection(dir);
    luapuz_pushGridDirection(L, returns);
    return 1;
}
// unsigned short InvertDirection(unsigned short dir)
static int puz_InvertDirection(lua_State * L)
{
    unsigned short dir = luapuz_checkuint(L, 1);
    unsigned short returns = puz::InvertDirection(dir);
    lua_pushnumber(L, returns);
    return 1;
}
// bool IsHorizontal(unsigned short dir)
static int puz_IsHorizontal(lua_State * L)
{
    unsigned short dir = luapuz_checkuint(L, 1);
    bool returns = puz::IsHorizontal(dir);
    lua_pushboolean(L, returns);
    return 1;
}
// bool IsDiagonal(unsigned short dir)
static int puz_IsDiagonal(lua_State * L)
{
    unsigned short dir = luapuz_checkuint(L, 1);
    bool returns = puz::IsDiagonal(dir);
    lua_pushboolean(L, returns);
    return 1;
}
// bool IsVertical(unsigned short dir)
static int puz_IsVertical(lua_State * L)
{
    unsigned short dir = luapuz_checkuint(L, 1);
    bool returns = puz::IsVertical(dir);
    lua_pushboolean(L, returns);
    return 1;
}
// bool AreInLine(unsigned short dir1, unsigned short dir2)
static int puz_AreInLine(lua_State * L)
{
    unsigned short dir1 = luapuz_checkuint(L, 1);
    unsigned short dir2 = luapuz_checkuint(L, 2);
    bool returns = puz::AreInLine(dir1, dir2);
    lua_pushboolean(L, returns);
    return 1;
}
// unsigned short GetDirection(puz::Square & first, puz::Square & second)
static int puz_GetDirection(lua_State * L)
{
    puz::Square * first = luapuz_checkSquare(L, 1);
    puz::Square * second = luapuz_checkSquare(L, 2);
    unsigned short returns = puz::GetDirection(*first, *second);
    lua_pushnumber(L, returns);
    return 1;
}
static const luaL_reg puzlib[] = {
    {"ConstrainDirection", puz_ConstrainDirection},
    {"InvertDirection", puz_InvertDirection},
    {"IsHorizontal", puz_IsHorizontal},
    {"IsDiagonal", puz_IsDiagonal},
    {"IsVertical", puz_IsVertical},
    {"AreInLine", puz_AreInLine},
    {"GetDirection", puz_GetDirection},
    {NULL, NULL}
};


// enum GridDirection
//------------

const char * GridDirection_meta = "puz.GridDirection";

const luapuz_enumReg GridDirection_reg[] = {
    {"ACROSS", puz::ACROSS},
    {"DOWN", puz::DOWN},
    {"LEFT", puz::LEFT},
    {"RIGHT", puz::RIGHT},
    {"UP", puz::UP},
    {"NORTH", puz::NORTH},
    {"SOUTH", puz::SOUTH},
    {"EAST", puz::EAST},
    {"WEST", puz::WEST},
    {"DIAGONAL_SE", puz::DIAGONAL_SE},
    {"DIAGONAL_SW", puz::DIAGONAL_SW},
    {"DIAGONAL_NE", puz::DIAGONAL_NE},
    {"DIAGONAL_NW", puz::DIAGONAL_NW},
    {NULL, NULL}
};


// enum FindDirection
//------------

const char * FindDirection_meta = "puz.FindDirection";

const luapuz_enumReg FindDirection_reg[] = {
    {"PREV", puz::PREV},
    {"NEXT", puz::NEXT},
    {NULL, NULL}
};


// enum GextFlag
//------------

const char * GextFlag_meta = "puz.GextFlag";

const luapuz_enumReg GextFlag_reg[] = {
    {"FLAG_CLEAR", puz::FLAG_CLEAR},
    {"FLAG_PENCIL", puz::FLAG_PENCIL},
    {"FLAG_BLACK", puz::FLAG_BLACK},
    {"FLAG_X", puz::FLAG_X},
    {"FLAG_REVEALED", puz::FLAG_REVEALED},
    {"FLAG_CIRCLE", puz::FLAG_CIRCLE},
    {"ACROSS_LITE_MASK", puz::ACROSS_LITE_MASK},
    {"FLAG_COLOR", puz::FLAG_COLOR},
    {"FLAG_MISSING", puz::FLAG_MISSING},
    {NULL, NULL}
};


// enum CheckTest
//------------

const char * CheckTest_meta = "puz.CheckTest";

const luapuz_enumReg CheckTest_reg[] = {
    {"NO_CHECK_BLANK", puz::NO_CHECK_BLANK},
    {"CHECK_BLANK", puz::CHECK_BLANK},
    {NULL, NULL}
};


// enum GridFlag
//------------

const char * GridFlag_meta = "puz.GridFlag";

const luapuz_enumReg GridFlag_reg[] = {
    {"FLAG_NORMAL", puz::FLAG_NORMAL},
    {"FLAG_NO_SOLUTION", puz::FLAG_NO_SOLUTION},
    {"FLAG_SCRAMBLED", puz::FLAG_SCRAMBLED},
    {NULL, NULL}
};


// enum GridType
//------------

const char * GridType_meta = "puz.GridType";

const luapuz_enumReg GridType_reg[] = {
    {"TYPE_NORMAL", puz::TYPE_NORMAL},
    {"TYPE_DIAGRAMLESS", puz::TYPE_DIAGRAMLESS},
    {NULL, NULL}
};


// enum FindOptions
//------------

const char * FindOptions_meta = "puz.FindOptions";

const luapuz_enumReg FindOptions_reg[] = {
    {"WRAP", puz::WRAP},
    {"NO_WRAP", puz::NO_WRAP},
    {"WHITE_SQUARES", puz::WHITE_SQUARES},
    {"FIND_IN_GRID", puz::FIND_IN_GRID},
    {"FIND_IN_WORD", puz::FIND_IN_WORD},
    {NULL, NULL}
};


// typedef ClueList
//------------

void luapuz_checkClueList(lua_State * L, int index, puz::ClueList * clues)
{
    luaL_checktype(L, index, LUA_TTABLE);

    clues->clear();

    // Iterate the table
    lua_pushnil(L);  /* first key */
    while (lua_next(L, index) != 0)
    {
        // Save the key for iteration . . .
        // for some reason lua doesn't like me to use luaL_checkstring()
        // on the key ? so make a copy here
        lua_pushvalue(L, -2);
        lua_insert(L, -3);

        // key is index -2
        // value is index -1
        puz::string_t number = luapuz_checkstring_t(L, -2);
        puz::string_t text;
        if (lua_istable(L, -1))
        {
            // Look for data:

            // number
            lua_getfield(L, -1, "number");
            if (! lua_isnil(L, -1))
                number = luapuz_checkstring_t(L, -1);
            lua_pop(L, 1);

            // text
            lua_getfield(L, -1, "text");
            if (! lua_isnil(L, -1))
                text = luapuz_checkstring_t(L, -1);
            lua_pop(L, 1);
        }
        else if (lua_isstring(L, -1))
        {
            text = luapuz_checkstring_t(L, -1);
        }
        else
        {
            luaL_error(L, "table or string expected for clue; got %s", luaL_typename(L, -1));
        }

        clues->push_back(puz::Clue(number, text));

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
        // Remove key as well, since we made a copy
        lua_pop(L, 1);
    }

    // Sort the clues, because there is no guarantee that the lua table is
    // sorted.
    std::sort(clues->begin(), clues->end());
}

int luapuz_pushClueList(lua_State * L, puz::ClueList * clues)
{
    // The clue table
    lua_newtable(L);

    int i = 1;
    for (puz::ClueList::iterator it = clues->begin();
         it != clues->end();
         ++it)
    {
        // t[i] = { number = number, text = text, square = square }
        lua_pushnumber(L, i++);

        lua_newtable(L);

        luapuz_pushstring_t(L, it->GetNumber());
        lua_setfield(L, -2, "number");

        luapuz_pushstring_t(L, it->GetText());
        lua_setfield(L, -2, "text");

        lua_settable(L, -3);
    }

    return 1;
}

void luapuz_openpuzlib (lua_State *L) {
    // register functions
    luaL_register(L, "puz", puzlib);
    luapuz_registerEnum(L, GridDirection_meta, GridDirection_reg);
    luapuz_registerEnum(L, FindDirection_meta, FindDirection_reg);
    luapuz_registerEnum(L, GextFlag_meta, GextFlag_reg);
    luapuz_registerEnum(L, CheckTest_meta, CheckTest_reg);
    luapuz_registerEnum(L, GridFlag_meta, GridFlag_reg);
    luapuz_registerEnum(L, GridType_meta, GridType_reg);
    luapuz_registerEnum(L, FindOptions_meta, FindOptions_reg);
    luapuz_openSquarelib(L);
    luapuz_openGridlib(L);
    luapuz_openPuzzlelib(L);
    lua_pop(L, 1);
}
