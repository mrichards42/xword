

//-----------------------------------------------------------------------------
// MyFrame overrides
//-----------------------------------------------------------------------------

%override wxLua_MyFrame_LoadPuzzle
//     bool LoadPuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
static int LUACALL wxLua_MyFrame_LoadPuzzle(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // const wxString filename
    const wxString filename = wxlua_getwxStringtype(L, 2);

    if (lua_gettop(L) < 3)
    {
        // call LoadPuzzle
        bool returns = self->LoadPuzzle(filename);
        lua_pushboolean(L, returns);
        return 1;
    }
    else
    {
        // Create a FileHandlerDesc
        luaL_checktype(L, 3, LUA_TFUNCTION);

        // Push the function for luapuz_Load_Puzzle.
        lua_pushvalue(L, 3);

        // Create the puz::Puzzle file handler
        puz::Puzzle::FileHandlerDesc desc;
        desc.data = L;
        desc.handler = luapuz_Load_Puzzle;

        bool returns = self->LoadPuzzle(filename, &desc);
        lua_pushboolean(L, returns);
        return 1;
    }
}
%end


%override wxLua_MyFrame_SavePuzzle
//     bool SavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
static int LUACALL wxLua_MyFrame_SavePuzzle(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // const wxString filename
    const wxString filename = wxlua_getwxStringtype(L, 2);

    if (lua_gettop(L) < 3)
    {
        // call LoadPuzzle
        bool returns = self->SavePuzzle(filename);
        lua_pushboolean(L, returns);
        return 1;
    }
    else
    {
        // Create a FileHandlerDesc
        luaL_checktype(L, 3, LUA_TFUNCTION);

        // Push the function for luapuz_Save_Puzzle.
        lua_pushvalue(L, 3);

        // Create the puz::Puzzle file handler
        puz::Puzzle::FileHandlerDesc desc;
        desc.data = L;
        desc.handler = luapuz_Save_Puzzle;

        bool returns = self->SavePuzzle(filename, &desc);
        lua_pushboolean(L, returns);
        return 1;
    }
}
%end


%override wxLua_MyFrame_GetPuzzle
// puz::Puzzle & GetPuzzle()
static int LUACALL wxLua_MyFrame_GetPuzzle(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::Puzzle & returns = self->GetPuzzle();
    luapuz_pushPuzzle(L, &returns);
    return 1;
}
%end


%override wxLua_MyFrame_GetFocusedSquare
// puz::Square * GetFocusedSquare()
static int LUACALL wxLua_MyFrame_GetFocusedSquare(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::Square * returns = self->GetFocusedSquare();
    luapuz_pushSquare(L, returns);
    return 1;
}
%end

%override wxLua_MyFrame_SetFocusedSquare
// puz::Square * SetFocusedSquare(puz::Square * square)
static int LUACALL wxLua_MyFrame_SetFocusedSquare(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::Square * square = luapuz_checkSquare(L, 2);
    puz::Square * returns = self->SetFocusedSquare(square);
    luapuz_pushSquare(L, returns);
    return 1;
}
%end



%override wxLua_MyFrame_GetFocusedDirection
// puz::GridDirection GetFocusedDirection()
static int LUACALL wxLua_MyFrame_GetFocusedDirection(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::GridDirection returns = self->GetFocusedDirection();
    luapuz_pushGridDirection(L, returns);
    return 1;
}
%end

%override wxLua_MyFrame_SetFocusedDirection
// void SetFocusedDirection(puz::GridDirection)
static int LUACALL wxLua_MyFrame_SetFocusedDirection(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::GridDirection direction = luapuz_checkGridDirection(L, 2);
    self->SetFocusedDirection(direction);
    return 0;
}
%end

%override wxLua_MyFrame_SetSquareText
// bool SetSquareText(puz::Square * square, const wxString & text)
static int LUACALL wxLua_MyFrame_SetSquareText(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    puz::Square * square = luapuz_checkSquare(L, 2);
    const wxString text = wxlua_getwxStringtype(L, 3);
    bool returns = self->SetSquareText(square, text);
    lua_pushboolean(L, returns);
    return 1;
}
%end


%override wxLua_MyFrame_GetFocusedWord
// %override [Lua table] MyFrame::GetFocusedWord()
// void GetFocusedWord(puz::Square * start, puz::Square * end)
static int LUACALL wxLua_MyFrame_GetFocusedWord(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // call GetFocusedWord
    puz::Square * start;
    puz::Square * end;
    self->GetFocusedWord(&start, &end);

    // Return nil if there is no focused word
    if (start == NULL || end == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    // Assemble a table of all squares in the focused word.

    lua_newtable(L);

    int i = 1; // lua indicies start with 1 not 0
    puz::GridDirection dir = self->GetFocusedDirection();
    end = end->Next(dir); // This has to be one past the last square.
    for (puz::Square * square = start; square != end; square = square->Next(dir))
    {
        lua_pushnumber(L, i++);
        luapuz_pushSquare(L, square);
        // t[i] = square
        lua_settable(L, -3); // -3 = third from the top of the stack
    }

    return 1; // One object on the stack for lua.
}
%end


%override wxLua_MyFrame_GetFocusedClue
// const puz::Puzzle::Clue * MyFrame::GetFocusedClue()
static int LUACALL wxLua_MyFrame_GetFocusedClue(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // call GetFocusedClue
    const puz::Puzzle::Clue * clue =  self->GetFocusedClue();

    // Return two nils if there is no focused clue
    if (clue == NULL)
    {
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    }

    // Return a pair (number, text)
    lua_pushnumber(L, clue->Number());
    lua_pushstring(L, clue->Text().c_str());
    return 2;
}
%end





//-----------------------------------------------------------------------------
// Global overrides
//-----------------------------------------------------------------------------

%override wxLua_function_GetFrame
// MyFrame * GetFrame()
// This is a lua-only function (it can't be accessed through the XWord C++ API)
int wxLua_function_GetFrame(lua_State *L)
{
    // Assume that the MyFrame pointer was given in the original wxLuaState
    // constructor
    wxLuaState wxlua(L);
    MyFrame * frame = wxDynamicCast(wxlua.GetEventHandler(), MyFrame);

    if (frame == NULL) // This should never happen
    {
        wxFAIL_MSG(_T("Frame pointer shouldn't be NULL"));
        lua_pushnil(L);
    }
    else
        wxluaT_pushuserdatatype(L, frame, wxluatype_MyFrame);
    return 1; // One object on the stack for lua.
}
%end

