
//-----------------------------------------------------------------------------
// XPuzzle overrides
//-----------------------------------------------------------------------------

// Handle exceptions from the constructor
%override wxLua_XPuzzle_constructor
// XPuzzle(const wxString & filename = wxEmptyString)
static int LUACALL wxLua_XPuzzle_constructor(lua_State *L)
{
    try
    {
        // get number of arguments
        int argCount = lua_gettop(L);
        // const wxString filename = wxEmptyString
        const wxString filename = (argCount >= 1 ? wxlua_getwxStringtype(L, 1) : wxString(wxEmptyString));
        // Check to see if file exists
        if (! filename.IsEmpty() && ! wxFileExists(filename))
        {
            wxlua_pushwxString(L,
                wxString::Format(_T("File does not exist: %s"),
                                 filename.c_str()));
            lua_error(L);
        }
        // call constructor
        XPuzzle* returns = new XPuzzle(filename);
        // add to tracked memory list
        wxluaO_addgcobject(L, (void*)returns, new wxLua_wxObject_XPuzzle((XPuzzle*)returns));
        // push the constructed class pointer
        wxluaT_pushuserdatatype(L, returns, wxluatype_XPuzzle);

        return 1;
    }
    catch(...)
    {
        xword_handle_exceptions(L);
        return 0;
    }
}
%end

// Handle exceptions from XPuzzle::Load
%override wxLua_XPuzzle_Load
// bool Load(const wxString & filename, wxString ext = wxEmptyString)
static int LUACALL wxLua_XPuzzle_Load(lua_State *L)
{
    try
    {
        // get number of arguments
        int argCount = lua_gettop(L);
        // wxString ext = wxEmptyString
        wxString ext = (argCount >= 3 ? wxlua_getwxStringtype(L, 3) : wxString(wxEmptyString));
        // const wxString filename
        const wxString filename = wxlua_getwxStringtype(L, 2);
        // Check to see if file exists
        if (! wxFileExists(filename))
        {
            wxlua_pushwxString(L,
                wxString::Format(_T("File does not exist: %s"),
                                 filename.c_str()));
            lua_error(L);
        }
        // get this
        XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
        // call Load
        bool returns = (self->Load(filename, ext));
        // push the result flag
        lua_pushboolean(L, returns);

        return 1;
    }
    catch(...)
    {
        xword_handle_exceptions(L);
        return 0;
    }
}
%end


// Handle exceptions from XPuzzle::Save
%override wxLua_XPuzzle_Save
// bool Save(const wxString & filename, wxString ext = wxEmptyString)
static int LUACALL wxLua_XPuzzle_Save(lua_State *L)
{
    try
    {
        // get number of arguments
        int argCount = lua_gettop(L);
        // wxString ext = wxEmptyString
        wxString ext = (argCount >= 3 ? wxlua_getwxStringtype(L, 3) : wxString(wxEmptyString));
        // const wxString filename
        const wxString filename = wxlua_getwxStringtype(L, 2);
        // get this
        XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
        // call Save
        bool returns = (self->Save(filename, ext));
        // push the result flag
        lua_pushboolean(L, returns);

        return 1;
    }
    catch(...)
    {
        xword_handle_exceptions(L);
        return 0;
    }
}
%end


%override wxLua_XPuzzle_GetAcross
// XPuzzle::ClueList & GetAcross()
static int LUACALL wxLua_XPuzzle_GetAcross(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
    return xword_pushClueList(L, self->GetAcross());
}
%end



%override wxLua_XPuzzle_GetDown
// XPuzzle::ClueList & GetDown()
static int LUACALL wxLua_XPuzzle_GetDown(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
    return xword_pushClueList(L, self->GetDown());
}
%end




%override wxLua_XPuzzle_SetAcross
// void SetAcross(XSquare::ClueList & across) const
static int LUACALL wxLua_XPuzzle_SetAcross(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
    xword_table2ClueList(L, 2, &self->GetAcross());
    return 0;
}
%end




%override wxLua_XPuzzle_SetDown
// void SetDown(XSquare::ClueList & down) const
static int LUACALL wxLua_XPuzzle_SetDown(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
    xword_table2ClueList(L, 2, &self->GetDown());
    return 0;
}
%end




%override wxLua_XPuzzle_GetClueList
// void GetClueList(std::vector<wxString> * clues)
static int LUACALL wxLua_XPuzzle_GetClueList(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);
    std::vector<wxString> clues;
    self->GetClueList(&clues);

    // Create a table (and push on to the stack)
    lua_newtable(L);

    int i = 1; // Starting index is always 1 in lua
    for (std::vector<wxString>::const_iterator it = clues.begin();
         it != clues.end();
         ++it)
    {
        lua_pushnumber(L, i++);
        xword_pushwxString(L, *it);
        // t[number] = text
        lua_settable(L, -3); // -3 = third from the top of the stack
    }

    return 1; // One object on the stack for lua.
}
%end


%override wxLua_XPuzzle_SetClueList
// bool SetClueList(const std::vector<wxString> & clues)
static int LUACALL wxLua_XPuzzle_SetClueList(lua_State *L)
{
    XPuzzle * self = (XPuzzle *)wxluaT_getuserdatatype(L, 1, wxluatype_XPuzzle);

    std::vector<wxString> clues;

    // Iterate the table on the stack
    lua_pushnil(L);  /* first key */
    while (lua_next(L, 2) != 0) // Table is the 2nd argument
    {
        // key is index -2
        // value is index -1
        clues.push_back(xword_getwxStringtype(L, -1));

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    lua_pushboolean(L, self->SetClueList(clues));

    return 1; // One object on the stack for lua.
}
%end


//-----------------------------------------------------------------------------
// XGrid overrides
//-----------------------------------------------------------------------------

// __index metatable methods

%override wxLua_XGrid___index
// %override [lua table] XGrid::operator[]({col, row})
static int LUACALL wxLua_XGrid___index(lua_State *L)
{
    // Get this
    XGrid * self = (XGrid *)wxluaT_getuserdatatype(L, 1, wxluatype_XGrid);

    // Make sure the second arg is a table with 2 items
    if (! lua_istable(L, 2) || lua_objlen(L, 2) != 2)
        wxlua_argerror(L, 2, wxT("a table {col, row}"));

    // Read col and row
    lua_pushnil(L); // First index of the table

    lua_next(L, 2); // Push key and value
    int col = wxlua_getnumbertype(L, -1);
    lua_pop(L, 1); // Pop key

    lua_next(L, 2); // Push key and value
    int row = wxlua_getnumbertype(L, -1);
    lua_pop(L, 1); // Pop key

    // Make col and row 0-based
    --col;
    --row;

    // Ensure that col and row are within the grid
    if (col < 0 || col >= self->GetWidth()
        || row < 0 || row >= self->GetHeight())
    {
        lua_pushnil(L);
        return 1;
    }

    wxluaT_pushuserdatatype(L, &self->At(col, row), wxluatype_XSquare);

    return 1;
}
%end


%override wxLua_XGrid___newindex
// %override [lua table] XGrid::operator[]({col, row})
static int LUACALL wxLua_XGrid___newindex(lua_State *L)
{
    // Get this
    XGrid * self = (XGrid *)wxluaT_getuserdatatype(L, 1, wxluatype_XGrid);

    // Make sure the second arg is a table with 2 items
    if (! lua_istable(L, 2) || lua_objlen(L, 2) != 2)
        wxlua_argerror(L, 2, wxT("a table {col, row}"));

    // Read col and row
    lua_pushnil(L); // First index of the table

    lua_next(L, 2); // Push key and value
    int col = wxlua_getnumbertype(L, -1);
    lua_pop(L, 1); // Pop key

    lua_next(L, 2); // Push key and value
    int row = wxlua_getnumbertype(L, -1);
    lua_pop(L, 1); // Pop key

    // Make col and row 0-based
    --col;
    --row;

    // Ensure that col and row are within the grid
    if (col < 0 || col >= self->GetWidth()
        || row < 0 || row >= self->GetHeight())
    {
        wxlua_error(L, wxString::Format(
                        _T("col and row must be within the bounds of the grid (%d <= col <= %d; %d <= row <= %d)"),
                        1, self->GetWidth(), 1, self->GetHeight()));
    }

    // Get new square
    XSquare * square = (XSquare *)wxluaT_getuserdatatype(L, 3, wxluatype_XSquare);

    // Set the square
    self->At(col, row) = *square;

    return 0;
}
%end



%override wxLua_XGrid_CountClues
// void CountClues(size_t * across, size_t * down) const
static int LUACALL wxLua_XGrid_CountClues(lua_State *L)
{
    XGrid * self = (XGrid *)wxluaT_getuserdatatype(L, 1, wxluatype_XGrid);
    size_t across, down;
    if (self->CountClues(&across, &down))
    {
        lua_pushnumber(L, across);
        lua_pushnumber(L, down);
    }
    else
    {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}
%end


//-----------------------------------------------------------------------------
// XSquare overrides
//-----------------------------------------------------------------------------
%override wxLua_XSquare_GetCol
// short GetCol() const
static int LUACALL wxLua_XSquare_GetCol(lua_State *L)
{
    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    short col = (self->GetCol()) + 1; // Add 1 because lua is 1-based
    lua_pushnumber(L, col);
    return 1;
}
%end

%override wxLua_XSquare_GetRow
// short GetCol() const
static int LUACALL wxLua_XSquare_GetRow(lua_State *L)
{
    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    short row = (self->GetRow()) + 1; // Add 1 because lua is 1-based
    lua_pushnumber(L, row);
    return 1;
}
%end

%override wxLua_XSquare_GetPlainText
// char GetPlainText() const
static int LUACALL wxLua_XSquare_GetPlainText(lua_State *L)
{
    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    char text = self->GetPlainText();
    lua_pushstring(L, &text);
    return 1;
}
%end

%override wxLua_XSquare_GetPlainSolution
// char GetPlainSolution() const
static int LUACALL wxLua_XSquare_GetPlainSolution(lua_State *L)
{
    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    char solution = self->GetPlainSolution();
    lua_pushstring(L, &solution);
    return 1;
}
%end


%override wxLua_XSquare_SetPlainSolution
// void SetPlainSolution(wxChar plain)
static int LUACALL wxLua_XSquare_SetPlainSolution(lua_State *L)
{
    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    wxString plain = xword_getwxStringtype(L, 2);
    self->SetPlainSolution(plain.at(0));
    return 0;
}
%end



%override wxLua_XSquare_SetSolution
// void SetSolution(const wxString & solution, wxChar plain)
static int LUACALL wxLua_XSquare_SetSolution(lua_State *L)
{
    // get number of arguments
    int argCount = lua_gettop(L);

    XSquare * self = (XSquare *)wxluaT_getuserdatatype(L, 1, wxluatype_XSquare);
    wxString solution = xword_getwxStringtype(L, 2);
    // Default argument for plain : '\0'
    wxString plain = (argCount >= 3 ? xword_getwxStringtype(L, 3) : _T("\0"));

    self->SetSolution(solution, plain.at(0));
    return 0;
}
%end




//-----------------------------------------------------------------------------
// MyFrame overrides
//-----------------------------------------------------------------------------

%override wxLua_MyFrame_GetFocusedWord
// void GetFocusedWord(XSquare * start, XSquare * end)
static int LUACALL wxLua_MyFrame_GetFocusedWord(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // call GetFocusedWord
    XSquare * start;
    XSquare * end;
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
    bool dir = self->GetFocusedDirection();
    end = end->Next(dir); // This has to be one past the last square.
    for (XSquare * square = start; square != end; square = square->Next(dir))
    {
        lua_pushnumber(L, i++);
        wxluaT_pushuserdatatype(L, square, wxluatype_XSquare);
        // t[i] = square
        lua_settable(L, -3); // -3 = third from the top of the stack
    }

    return 1; // One object on the stack for lua.
}
%end


%override wxLua_MyFrame_GetFocusedClue
// const XPuzzle::Clue * MyFrame::GetFocusedClue()
static int LUACALL wxLua_MyFrame_GetFocusedClue(lua_State *L)
{
    // get this
    MyFrame * self = (MyFrame *)wxluaT_getuserdatatype(L, 1, wxluatype_MyFrame);

    // call GetFocusedClue
    const XPuzzle::Clue * clue =  self->GetFocusedClue();

    // Return two nils if there is no focused clue
    if (clue == NULL)
    {
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    }

    // Return a pair (number, text)
    lua_pushnumber(L, clue->Number());
    xword_pushwxString(L, clue->Text());
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



%override wxLua_function_IsPortable
// bool MyApp::IsPortable()
int wxLua_function_IsPortable(lua_State *L)
{
    lua_pushboolean(L, wxGetApp().IsPortable());
    return 1;
}
%end
