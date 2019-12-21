overrides = {

-- ===================================================================
-- Puzzle functions
-- ===================================================================
Puzzle_Puzzle = [[
// Puzzle()
// Puzzle(const std::string & filename)
// Puzzle(const std::string & filename, const FileHandlerDesc * desc)
int Puzzle_Puzzle_try(lua_State * L)
{
    try {
        puz::Puzzle * returns;

        // metatable is first argument
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            const char * filename = luaL_checkstring(L, 2);
            luaL_checktype(L, 3, LUA_TFUNCTION);

            // Push the function for luapuz_Load_Puzzle.
            lua_pushvalue(L, 3); // Function

            // Create the puz::Puzzle file handler
            puz::Puzzle::FileHandlerDesc desc;
            desc.data = L;
            desc.handler = luapuz_Load_Puzzle;

            // Call the constructor
            returns = new puz::Puzzle(filename, &desc);
        }
        else if (argCount >= 1)
        {
            const char * filename = luaL_checkstring(L, 2);
            returns = new puz::Puzzle(filename);
        }
        else if (argCount >= 0)
        {
            returns = new puz::Puzzle();
        }

        luapuz_newPuzzle(L, returns);
        return 1;
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    return -1; // Error on the stack
}

int Puzzle_Puzzle(lua_State * L)
{
    int code = Puzzle_Puzzle_try(L);
    if (code == -1)
        lua_error(L);
    return code;
}
]],


Puzzle_Load = [[
// void Load(const std::string & filename)
// void Load(const std::string & filename, FileHandlerDesc * desc)
int Puzzle_Load_try(lua_State * L)
{
    puz::Puzzle * puzzle = luapuz_checkPuzzle(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    try {
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            luaL_checktype(L, 3, LUA_TFUNCTION);

            // Push the function for luapuz_Load_Puzzle.
            lua_pushvalue(L, 3); // Function

            // Create the puz::Puzzle file handler
            puz::Puzzle::FileHandlerDesc desc;
            desc.data = L;
            desc.handler = luapuz_Load_Puzzle;

            // Call Load()
            puzzle->Load(filename, &desc);
        }
        else
        {
            puzzle->Load(filename);
        }
        return 0;
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    return -1; // Error on the stack
}

int Puzzle_Load(lua_State * L)
{
    int code = Puzzle_Load_try(L);
    if (code == -1)
        lua_error(L);
    return code;
}
]],

Puzzle_Save = [[
// void Save(const std::string & filename)
// void Save(const std::string & filename, FileHandlerDesc * desc)
int Puzzle_Save(lua_State * L)
{
    puz::Puzzle * puzzle = luapuz_checkPuzzle(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    try {
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            luaL_checktype(L, 3, LUA_TFUNCTION);

            // Push the function for luapuz_Save_Puzzle.
            lua_pushvalue(L, 3); // Function

            // Create the puz::Puzzle file handler
            puz::Puzzle::FileHandlerDesc desc;
            desc.data = L;
            desc.handler = luapuz_Save_Puzzle;

            // Call Save()
            puzzle->Save(filename, &desc);
        }
        else
        {
            puzzle->Save(filename);
        }
        return 0;
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    lua_error(L); // We should have returned by now
    return 0;
}
]],

-- ===================================================================
-- Typedef puz::Puzzle::metamap_t
-- ===================================================================

push_metamap_t = [[
// NB: setting keys on this metadata table won't actually do anything.
// [lua table] GetMetadata()
int luapuz_push_metamap_t(lua_State * L, puz::Puzzle::metamap_t * meta)
{
    // The table
    lua_newtable(L);
    puz::Puzzle::metamap_t::iterator it;
    for (it = meta->begin(); it != meta->end(); ++it)
    {
        if (! it->first.empty())
        {
            // t[key] = value
            luapuz_pushstring_t(L, it->second);
            lua_setfield(L, -2, puz::encode_utf8(it->first).c_str());
        }
    }
    return 1;
}
]],

-- ===================================================================
-- Grid
-- ===================================================================
Grid__index = [[
// puz::Square * Grid[{col, row}]
static int Grid__index(lua_State * L)
{
    // Grid can be indexed using Grid[{col, row}]
    // Indices start from 1 (as expected in lua)

    puz::Grid * grid = luapuz_checkGrid(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);

    lua_pushnumber(L, 1);
    lua_gettable(L, 2);
    size_t col = luapuz_checkuint(L, -1);

    lua_pushnumber(L, 2);
    lua_gettable(L, 2);
    size_t row = luapuz_checkuint(L, -1);

    try
    {
        // Adjust the lua indices to C indices (1-based to 0-based).
        puz::Square * returns = &grid->At(col - 1, row - 1);
        luapuz_pushSquare(L, returns);
        return 1;
    }
    catch (std::exception &)
    {
        luaL_error(L,
                   "Grid indices out of bounds: "
                   "1 <= col <= %d and 1 <= row <= %d",
                   grid->LastCol() + 1,
                   grid->LastRow() + 1);
        return 0;
    }
}
]],


Grid__newindex = [[
// void Grid[{col, row}] = puz::Square *
static int Grid__newindex(lua_State * L)
{
    // Grid squares can be set as follows:
    //     Grid[{col, row}] = Square
    // Indices start from 1 (as expected in lua)

    puz::Grid * grid = luapuz_checkGrid(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);

    puz::Square * square = luapuz_checkSquare(L, 3);

    lua_pushnumber(L, 1);
    lua_gettable(L, 2);
    size_t col = luapuz_checkuint(L, -1);

    lua_pushnumber(L, 2);
    lua_gettable(L, 2);
    size_t row = luapuz_checkuint(L, -1);

    try
    {
        // Adjust the lua indices to C indices (1-based to 0-based).
        grid->At(col - 1, row - 1) = *square;
        return 0;
    }
    catch (std::exception &)
    {
        luaL_error(L,
                   "Grid indices out of bounds: "
                   "1 <= col <= %d and 1 <= row <= %d",
                   grid->LastCol() + 1,
                   grid->LastRow() + 1);
        return 0;
    }
}
]],


Grid_CountClues = [[
// size_t across, size_t down = CountClues()
static int Grid_CountClues(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    size_t across, down;
    try {
        grid->CountClues(&across, &down);
        lua_pushnumber(L, across);
        lua_pushnumber(L, down);
        return 2;
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    lua_error(L); // We should have returned by now
    return 0;
}
]],

Grid_LastRow = [[
// int LastRow()
static int Grid_LastRow(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    int returns = grid->LastRow() + 1; // Lua indices are 1-based
    lua_pushnumber(L, returns);
    return 1;
}
]],

Grid_LastCol = [[
// int LastCol()
static int Grid_LastCol(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    int returns = grid->LastCol() + 1; // Lua indices are 1-based
    lua_pushnumber(L, returns);
    return 1;
}
]],


Grid_CheckGrid = [[
// { puz::Square*, ... } CheckGrid(bool checkBlank = false, bool strictRebus = false)
static int Grid_CheckGrid(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    int argCount = lua_gettop(L);
    bool checkBlank = (argCount >= 2 ? luapuz_checkboolean(L, 2) : false);
    bool strictRebus = (argCount >= 3 ? luapuz_checkboolean(L, 3) : false);
    std::vector<puz::Square*> returns;
    grid->CheckGrid(&returns, checkBlank, strictRebus);
    luapuz_pushSquareVector(L, &returns);
    return 1;
}
]],

Grid_CheckWord = [[
// { puz::Square*, ... } CheckWord(puz::Square * start, puz::Square * end, bool checkBlank = false, bool strictRebus = false)
static int Grid_CheckWord(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    int argCount = lua_gettop(L);
    puz::Square * start = luapuz_checkSquare(L, 2);
    puz::Square * end = luapuz_checkSquare(L, 3);
    bool checkBlank = (argCount >= 4 ? luapuz_checkboolean(L, 4) : false);
    bool strictRebus = (argCount >= 5 ? luapuz_checkboolean(L, 5) : false);
    try {
        std::vector<puz::Square*> returns;
        grid->CheckWord(&returns, start, end, checkBlank, strictRebus);
        luapuz_pushSquareVector(L, &returns);
        return 1;
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    lua_error(L); // We should have returned by now
    return 0;
}
]],


Grid_FindSquare = [[
// Helper for FindSquare
struct luapuz_FindSquare_Struct
{
    luapuz_FindSquare_Struct(lua_State * L)
        : m_L(L)
    {}

    lua_State * m_L;

    bool operator() (puz::Square * square)
    {
        // Push the function (on top of the stack)
        // lua_call() pops this function, so we'll make a copy
        // for further iterations.
        lua_pushvalue(m_L, -1);
        luapuz_pushSquare(m_L, square);
        lua_call(m_L, 1, 1);
        bool result = luapuz_checkboolean(m_L, -1);
        lua_pop(m_L, 1); // Pop the result (stack is balanced).
        return result;
    }
};


// FindSquare overloads:
//----------------------
// puz::Square * FindSquare(puz::Square * start, function findFunc,
//                          puz::GridDirection direction = puz::ACROSS,
//                          unsigned int options = puz::FIND_IN_GRID)
//
// puz::Square * FindSquare(function findFunc,
//                          puz::GridDirection direction = puz::ACROSS,
//                          unsigned int options = puz::FIND_IN_GRID)
//
static int Grid_FindSquare(lua_State * L)
{
    puz::Grid * grid = luapuz_checkGrid(L, 1);
    int argCount = lua_gettop(L);

    puz::Square * returns;

    if (luapuz_isSquare(L, 2))  // First overload
    {
        puz::Square * start = luapuz_checkSquare(L, 2);
        luaL_checktype(L, 3, LUA_TFUNCTION);
        puz::GridDirection direction = (argCount >= 4 ? luapuz_checkGridDirection(L, 4) : puz::ACROSS);
        unsigned int options = (argCount >= 5 ? luapuz_checkuint(L, 5) : puz::FIND_IN_GRID);

        // Push the function on the stack for luapuz_FindSquare_Struct
        lua_pushvalue(L, 3);
        luapuz_FindSquare_Struct func(L);

        returns = grid->FindSquare(start, func, direction, options);
    }
    else  // Second overload
    {
        luaL_checktype(L, 2, LUA_TFUNCTION);
        puz::GridDirection direction = (argCount >= 3 ? luapuz_checkGridDirection(L, 3) : puz::ACROSS);
        unsigned int options = (argCount >= 4 ? luapuz_checkuint(L, 4) : puz::FIND_IN_GRID);

        // Push the function on the stack for luapuz_FindSquare_Struct
        lua_pushvalue(L, 2);
        luapuz_FindSquare_Struct func(L);

        returns = grid->FindSquare(func, direction, options);
    }

    luapuz_pushSquare(L, returns);
    return 1;
}
]],

-- ===================================================================
-- Square row / col
-- ===================================================================
Square_GetCol = [[
// short GetCol()
static int Square_GetCol(lua_State * L)
{
    puz::Square * square = luapuz_checkSquare(L, 1);
    // Adjust to a lua index (1-based)
    short returns = square->GetCol() + 1;
    lua_pushnumber(L, returns);
    return 1;
}
]],

Square_GetRow = [[
// short GetRow()
static int Square_GetRow(lua_State * L)
{
    puz::Square * square = luapuz_checkSquare(L, 1);
    // Adjust to a lua index (1-based)
    short returns = square->GetRow() + 1;
    lua_pushnumber(L, returns);
    return 1;
}
]],

Square_HasClue = [[
// bool HasClue()
// bool HasClue(puz::GridDirection dir)
static int Square_HasClue(lua_State * L)
{
    puz::Square * square = luapuz_checkSquare(L, 1);
    bool returns;
    int argCount = lua_gettop(L);
    if (argCount >= 2)
    {
        puz::GridDirection dir = luapuz_checkGridDirection(L, 2);
        returns = square->HasClue(dir);
    }
    else
    {
        returns = square->HasClue();
    }
    lua_pushboolean(L, returns);
    return 1;
}
]],


-- ===================================================================
-- Typedef puz::Clues
-- ===================================================================
pushClues = [[
int luapuz_pushClues(lua_State * L, puz::Clues * clues)
{
    // The clue table
    lua_newtable(L);

    int i = 1;
    puz::Clues::iterator it;
    for (it = clues->begin(); it != clues->end(); ++it)
    {
        luapuz_pushClueList(L, &it->second);
        lua_setfield(L, -2, puz::encode_utf8(it->first).c_str());
    }

    return 1;
}
]],


-- ===================================================================
-- Typedef puz::ClueList
-- ===================================================================
pushClueList = [[
int luapuz_pushClueList(lua_State * L, puz::ClueList * clues)
{
    // The clue table
    lua_newtable(L);

    int i = 1;
    puz::ClueList::iterator it;
    for (it = clues->begin(); it != clues->end(); ++it)
    {
        luapuz_pushClue(L, &*it);
        lua_rawseti(L, -2, i++);
    }

    return 1;
}
]],

checkClueList = [[
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

        if (luapuz_isClue(L, -1))
        {
            clues->push_back(*luapuz_checkClue(L, -1));
        }
        else
        {
            // key is index -2
            // value is index -1
            puz::string_t number = luapuz_checkstring_t(L, -2);
            puz::string_t text;
            puz::Word word;
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

                // word
                lua_getfield(L, -1, "word");
                if (! lua_isnil(L, -1))
                    luapuz_checkWord(L, -1, &word);
                lua_pop(L, 1);
            }
            else if (lua_isstring(L, -1))
            {
                text = luapuz_checkstring_t(L, -1);
            }
            else
            {
                luaL_error(L, "puz::Clue, table, or string expected for clue; got %s", luaL_typename(L, -1));
            }

            clues->push_back(puz::Clue(number, text, word));
        }

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
        // Remove key as well, since we made a copy
        lua_pop(L, 1);
    }

    // Sort the clues, because there is no guarantee that the lua table is
    // sorted.
    std::sort(clues->begin(), clues->end());
}
]],

-- ===================================================================
-- Typedef puz::Word
-- ===================================================================
pushWord = [[
int luapuz_pushWord(lua_State * L, puz::Word * word)
{
    // The squares table
    lua_newtable(L);

    int i = 1;
    puz::Word::iterator it;
    for (it = word->begin(); it != word->end(); ++it)
    {
        // t[number] = square
        luapuz_pushSquare(L, &*it);
        lua_rawseti(L, -2, i);
        ++i;
    }

    return 1;
}
]],

checkWord = [[
#include "luapuz_puz_Square.hpp"

void luapuz_checkWord(lua_State * L, int index, puz::Word * word)
{
    luaL_checktype(L, index, LUA_TTABLE);

    lua_pushvalue(L, index);

    // Iterate the table
    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0)
    {
        // key is index -2
        // value is index -1
        int number = luapuz_checkuint(L, -2);
        puz::Square * square = luapuz_checkSquare(L, -1);
        word->push_back(square);

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}
]],

-- ===================================================================
-- Typedef std::vector<Square*>
-- ===================================================================
pushSquareVector = [[
int luapuz_pushSquareVector(lua_State * L, std::vector<puz::Square*> * squares)
{
    // The squares table
    lua_newtable(L);

    int i = 1;
    for (std::vector<puz::Square*>::iterator it = squares->begin();
         it != squares->end();
         ++it)
    {
        // t[number] = square
        luapuz_pushSquare(L, *it);
        lua_rawseti(L, -2, i);
        ++i;
    }

    return 1;
}
]],

checkSquareVector = [[
#include "luapuz_puz_Square.hpp"

void luapuz_checkSquareVector(lua_State * L, int index, std::vector<puz::Square*> * squares)
{
    luaL_checktype(L, index, LUA_TTABLE);

    lua_pushvalue(L, index);

    // Iterate the table
    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0)
    {
        // key is index -2
        // value is index -1
        int number = luapuz_checkuint(L, -2);
        puz::Square * square = luapuz_checkSquare(L, -1);
        squares->push_back(square);

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}
]],

}
