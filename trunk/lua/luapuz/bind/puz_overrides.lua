overrides = {

-- ===================================================================
-- Load/Save functions
-- ===================================================================
Puzzle_Puzzle = [[
// Puzzle()
// Puzzle(const std::string & filename)
// Puzzle(const std::string & filename, const std::string & ext)
// Puzzle(const std::string & filename, FileHandlerDesc * desc)
int Puzzle_Puzzle(lua_State * L)
{
    try {
        puz::Puzzle * returns;

        // metatable is first argument
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            std::string filename = luapuz_checkStdString(L, 2);
            if (lua_isstring(L, 3))
            {
                std::string ext = luapuz_checkStdString(L, 3);
                returns = new puz::Puzzle(filename, ext);
            }
            else if (lua_isfunction(L, 3))
            {
                // make sure the function is on top of the stack
                lua_pushvalue(L, 3);
                luapuz_LoadSaveData luapuzdata(L);

                // Create the puz::Puzzle file handler
                puz::Puzzle::FileHandlerDesc desc;
                desc.data = &luapuzdata;
                desc.handler = luapuz_LoadSave_Puzzle;

                returns = new puz::Puzzle(filename, &desc);

                // Clean up our data pointer
                luapuzdata.unref();
            }
            else
            {
                luaL_typerror(L, 3, "string or function");
            }
        }
        else if (argCount >= 1)
        {
            std::string filename = luapuz_checkStdString(L, 2);
            returns = new puz::Puzzle(filename);

        }
        else if (argCount >= 0)
        {
            returns = new puz::Puzzle();
        }

        luapuz_newPuzzle(L, returns);
        if (returns->HasError())
        {
            lua_pushstring(L, returns->GetError().c_str());
            returns->ClearError();
            return 2;
        }
        else
        {
            return 1;
        }
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    return 0;
}
]],


Puzzle_Load = [[
// void Load(const std::string & filename)
// void Load(const std::string & filename, const std::string & ext)
// void Load(const std::string & filename, FileHandlerDesc * desc)
int Puzzle_Load(lua_State * L)
{
    puz::Puzzle * puzzle = luapuz_checkPuzzle(L, 1);
    std::string filename = luapuz_checkStdString(L, 2);
    try {
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            if (lua_isstring(L, 3))
            {
                std::string ext = luapuz_checkStdString(L, 3);
                puzzle->Load(filename, ext);
            }
            else if (lua_isfunction(L, 3))
            {
                // make sure the function is on top of the stack
                lua_pushvalue(L, 3);
                luapuz_LoadSaveData luapuzdata(L);

                // Create the puz::Puzzle file handler
                puz::Puzzle::FileHandlerDesc desc;
                desc.data = &luapuzdata;
                desc.handler = luapuz_LoadSave_Puzzle;

                puzzle->Load(filename, &desc);

                // Clean up our data pointer
                luapuzdata.unref();
            }
            else
            {
                luaL_typerror(L, 3, "string or function");
            }
        }
        else
        {
            puzzle->Load(filename);
        }

        if (puzzle->HasError())
        {
            lua_pushstring(L, puzzle->GetError().c_str());
            puzzle->ClearError();
            return 1;
        }
        else
        {
            return 0;
        }
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    return 0;
}]],

Puzzle_Save = [[
// void Save(const std::string & filename)
// void Save(const std::string & filename, const std::string & ext)
// void Save(const std::string & filename, FileHandlerDesc * desc)
int Puzzle_Save(lua_State * L)
{
    puz::Puzzle * puzzle = luapuz_checkPuzzle(L, 1);
    std::string filename = luapuz_checkStdString(L, 2);
    try {
        int argCount = lua_gettop(L) - 1;
        if (argCount >= 2)
        {
            if (lua_isstring(L, 3))
            {
                std::string ext = luapuz_checkStdString(L, 3);
                puzzle->Save(filename, ext);
            }
            else if (lua_isfunction(L, 3))
            {
                // make sure the function is on top of the stack
                lua_pushvalue(L, 3);
                luapuz_LoadSaveData luapuzdata(L);

                // Create the puz::Puzzle file handler
                puz::Puzzle::FileHandlerDesc desc;
                desc.data = &luapuzdata;
                desc.handler = luapuz_LoadSave_Puzzle;

                puzzle->Save(filename, &desc);

                // Clean up our data pointer
                luapuzdata.unref();
            }
            else
            {
                luaL_typerror(L, 3, "string or function");
            }
        }
        else
        {
            puzzle->Save(filename);
        }

        if (puzzle->HasError())
        {
            lua_pushstring(L, puzzle->GetError().c_str());
            puzzle->ClearError();
            return 1;
        }
        else
        {
            return 0;
        }
    }
    catch (...) {
        luapuz_handleExceptions(L);
    }
    return 0;
}
]],


-- ===================================================================
-- Add Load/Save functions
-- ===================================================================
Puzzle_AddLoadHandler = [[
// static void AddLoadHandler(FileHandler func, const char * ext, void * data)
int Puzzle_AddLoadHandler(lua_State * L)
{
    if (! lua_isfunction(L, 1))
        luaL_typerror(L, 1, "function");
    const char * ext = luaL_checkstring(L, 2);

    // Push the function for luapuz_LoadSaveData
    lua_pushvalue(L, 1);
    luapuz_newLoadSaveData(L);

    puz::Puzzle::AddLoadHandler(luapuz_LoadSave_Puzzle,
                                ext,
                                luapuz_checkLoadSaveData(L, -1));
    return 0;
}
]],

Puzzle_AddSaveHandler = [[
// static void AddSaveHandler(FileHandler func, const char * ext, void * data)
int Puzzle_AddSaveHandler(lua_State * L)
{
    if (! lua_isfunction(L, 1))
        luaL_typerror(L, 1, "function");
    const char * ext = luaL_checkstring(L, 2);

    // Push the function for luapuz_LoadSaveData
    lua_pushvalue(L, 1);
    luapuz_newLoadSaveData(L);

    puz::Puzzle::AddSaveHandler(luapuz_LoadSave_Puzzle,
                                ext,
                                luapuz_checkLoadSaveData(L, -1));
    return 0;
}
]],

-- ===================================================================
-- Grid index
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

-- ===================================================================
-- Typedef ClueList
-- ===================================================================
pushClueList = [[
int luapuz_pushClueList(lua_State * L, puz::Puzzle::ClueList * clues)
{
    // The clue table
    lua_newtable(L);

    for (puz::Puzzle::ClueList::const_iterator it = clues->begin();
         it != clues->end();
         ++it)
    {
        // t[number] = text
        lua_pushnumber(L, it->Number());
        luapuz_pushStdString(L, it->Text());
        lua_settable(L, -3);
    }

    return 1;
}
]],

checkClueList = [[
void luapuz_checkClueList(lua_State * L, int index, puz::Puzzle::ClueList * clues)
{
    luaL_checktype(L, index, LUA_TTABLE);

    clues->clear();

    // Iterate the table
    lua_pushnil(L);  /* first key */
    while (lua_next(L, index) != 0)
    {
        // key is index -2
        // value is index -1
        int number = luapuz_checkuint(L, -2);
        std::string text = luapuz_checkStdString(L, -1);
        clues->push_back(puz::Puzzle::Clue(number, text));

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    // Sort the clues, because there is no guarantee that the lua table is
    // sorted.
    std::sort(clues->begin(), clues->end());
}
]],



}
