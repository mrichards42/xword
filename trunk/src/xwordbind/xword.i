//----------------------------------------------------------------------------
// lua interface to XWord
// Execute this to generate bindings:
//     lua -e"rulesFilename='xword_rules.lua'" genxwordbind.lua
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// The puz library
//----------------------------------------------------------------------------

%include "../puz/XPuzzle.hpp"
%include "../puz/XGrid.hpp"
%include "../puz/XSquare.hpp"
%include <vector>

// enum XGridFlag
%define XFLAG_NORMAL       XFLAG_NORMAL
%define XFLAG_NO_SOLUTION  XFLAG_NO_SOLUTION
%define XFLAG_SCRAMBLED    XFLAG_SCRAMBLED

// enum XGridType
%define XTYPE_NORMAL       XTYPE_NORMAL
%define XTYPE_DIAGRAMLESS  XTYPE_DIAGRAMLESS

//enum XFlag (GEXT flags)
%define XFLAG_CLEAR   XFLAG_CLEAR
%define XFLAG_PENCIL  XFLAG_PENCIL
%define XFLAG_BLACK   XFLAG_BLACK
%define XFLAG_X       XFLAG_X
%define XFLAG_RED     XFLAG_RED
%define XFLAG_CIRCLE  XFLAG_CIRCLE



// enum ClueFlag
%define NO_CLUE      NO_CLUE
%define ACROSS_CLUE  ACROSS_CLUE
%define DOWN_CLUE    DOWN_CLUE


// enum FindIncrement
%define FIND_PREV  FIND_PREV
%define FIND_NEXT  FIND_NEXT


// enum GridDirection
%define DIR_ACROSS  DIR_ACROSS
%define DIR_DOWN    DIR_DOWN

// enum CheckText
%define NO_CHECK_BLANK  NO_CHECK_BLANK
%define CHECK_BLANK     CHECK_BLANK



// Lua is not allowed to delete XSquares, because they are always owned by
// an XGrid.
%class %noclassinfo %encapsulate XSquare
    // Override GetCol / GetRow to reflect lua's 1-based indecies

    // %override lua_number XSquare::GetCol()
    // short GetCol() const
    int GetCol() const

    // %override lua_number XSquare::GetCol()
    // short GetRow() const
    int GetRow() const


    bool IsWhite() const
    bool IsBlack() const
    bool IsBlank() const


    // Text
    //-----
    const wxString & GetText() const

    // %override lua_string XSquare::GetPlainText() const
    // char GetPlainText() const
    int GetPlainText() const

    int GetTextSymbol() const

    bool HasTextRebus() const
    bool HasTextSymbol() const

    void SetText(const wxString & text)

    // Solution
    //---------
    const wxString & GetSolution() const

    // %override lua_string XSquare::GetPlainSolution() const
    // char GetPlainSolution() const
    int GetPlainSolution() const

    int GetSolutionSymbol() const

    bool HasSolutionRebus() const
    bool HasSolutionSymbol() const

    // %override void XSquare::SetSolution(lua_string, lua_string = "\0")
    // void SetSolution(const wxString &, wxChar) const
    int SetSolution()

    // %override void XSquare::SetPlainSolution() const
    // void SetPlainSolution(char) const
    int SetPlainSolution()

    bool Check(bool checkBlank = NO_CHECK_BLANK) const


    static bool IsValidChar(wxChar ch)
    static bool IsValidString(const wxString & str)

    // Clue
    //-----
    short GetNumber() const
    bool HasClue(bool direction) const
    bool HasClue() const


    // Flag (GEXT)
    //------------
    void   SetFlag     (wxByte flag)
    void   AddFlag     (wxByte flag)
    void   RemoveFlag  (wxByte flag)

    wxByte GetFlag     ()            const
    bool   HasFlag     (wxByte flag) const

    void   ReplaceFlag (wxByte flag1, wxByte flag2)


    // Linked-list
    //------------
    XSquare * Next(bool dir = DIR_ACROSS, bool inc = FIND_NEXT)
    XSquare * Prev(bool dir = DIR_ACROSS, bool inc = FIND_NEXT)
    XSquare * GetWordStart(bool dir)
    XSquare * GetWordEnd(bool dir)

    bool IsLast(bool direction, int increment = FIND_NEXT) const
    bool IsFirst(bool direction, int increment = FIND_NEXT) const
%endclass


%class %delete %noclassinfo %encapsulate XGrid
    XGrid(size_t width = 0, size_t height = 0)

    void SetupGrid()

    // Size
    void SetSize(size_t width, size_t height)
    size_t GetWidth()  const
    size_t GetHeight() const
    int    LastRow()   const
    int    LastCol()   const

    bool IsEmpty() const
    void Clear()

    // XSquare access

    // Access to squares should be through grid[{col, row}]
    // This is accomplished by using the __index and __newindex
    // metatable methods.

    // %override XSquare XGrid::operator[{col, row}]()
    int __index()
    // %override XGrid::operator[{col, row}]() = XSquare
    int __newindex()

    XSquare & At(size_t col, size_t row);
    XSquare & At(wxPoint pt);

    XSquare * First()
    XSquare * Last()
    XSquare * FirstWhite()
    XSquare * LastWhite()

    bool IsBetween(const XSquare * square, const XSquare * start, const XSquare * end) const

    // %override [across, down] XGrid::CountClues()
    // if across or down are nil, the function failed
    int CountClues();

    bool IsScrambled() const
    bool HasSolution() const
    unsigned short GetFlag() const

    bool IsDiagramless() const
    unsigned short GetType() const

    bool ScrambleSolution(unsigned short key = 0)
    bool UnscrambleSolution(unsigned short key)
    bool CheckScrambledGrid()

    unsigned short GetKey() const
    unsigned short GetCksum() const
%endclass


%class %delete %noclassinfo %encapsulate XPuzzle
    // Override loading functions to catch exceptions and report them
    // as lua errors.  The error will be a table:
    // { "message", "exception type", fatal_true_false }

    // %override XPuzzle(const wxString & filename = wxEmptyString)
    XPuzzle()

    // %override bool Load(const wxString & filename, wxString ext = wxEmptyString)
    int Load()
    // %override bool Save(const wxString & filename, wxString ext = wxEmptyString)
    int Save()

    void Clear()
    bool IsOk()        const
    void SetOk(bool doit=true)
    bool IsScrambled() const
    short GetVersion() const

    const wxString & GetFilename() const
    void SetFilename(const wxString & filename)

    bool GetModified() const 
    void SetModified(bool modified) 

    int GetTime() const 
    void SetTime(int time) 

    const wxString & GetTitle() const 
    void SetTitle(const wxString & title) 

    const wxString & GetAuthor() const 
    void SetAuthor(const wxString & author) 

    const wxString & GetCopyright() const 
    void SetCopyright(const wxString & copyright) 

    const wxString & GetNotes() const 
    void SetNotes(const wxString & notes) 

    // %override [Lua table] XPuzzle::GetAcross() const
    // const XPuzzle::ClueList & GetAcross() const
    // Return a lua table { [cluenumber] = cluetext }
    int GetAcross() const

    // %override [Lua table] XPuzzle::GetDown() const
    // const XPuzzle::ClueList & GetDown() const
    // Return a lua table { [cluenumber] = cluetext }
    int GetDown() const


    // %override void XPuzzle::SetAcross(Lua table)
    // void SetAcross(const XPuzzle::ClueList &)
    // Set with a lua table { [cluenumber] = cluetext }
    int SetAcross()

    // %override void XPuzzle::SetDown(Lua table)
    // void SetDown(const XPuzzle::ClueList &)
    // Set with a lua table { [cluenumber] = cluetext }
    int SetDown()


    // %override [Lua table] XPuzzle::GetClueList() const
    // void GetClueList(std::vector<wxString> * clues) const
    int GetClueList() const

    // %override bool XPuzzle::SetClueList() const
    // bool SetClueList(const std::vector<wxString> & clues)
    int SetClueList() const

    void RenumberClues()

    %ungc XGrid & GetGrid()
    void SetGrid(const XGrid & grid)
%endclass




//----------------------------------------------------------------------------
// Gui stuff
//----------------------------------------------------------------------------
%include "../MyFrame.hpp"
%include "../App.hpp"

%class MyFrame, wxFrame
    %ungc XPuzzle & GetPuzzle()

    void ShowPuzzle()
    void ShowGrid()
    void ShowClues()
    void ShowTitle()
    void ShowAuthor()
    void ShowCopyright()
    void ShowNotes()

    bool LoadPuzzle(const wxString & filename, const wxString & ext = wxEmptyString)
    bool SavePuzzle(wxString filename, const wxString & ext = wxEmptyString)
    // Return true = puzzle is closed
    bool ClosePuzzle(bool prompt = true)
    void CheckPuzzle()

    void ShowPane(const wxString & name, bool show = true);
    void HidePane(const wxString & name)


    void SetStatus(const wxString & text)

    bool IsTimerRunning()
    void SetTime(int time)
    int  GetTime()
    void ResetTimer()
    void StartTimer()
    void StopTimer()
    void ToggleTimer()

    XSquare * GetFocusedSquare()
    XSquare * SetFocusedSquare(XSquare * square)

    // %override [Lua table] MyFrame::GetFocusedWord()
    // void MyFrame::GetFocusedWord(XSquare ** start, XSquare ** end)
    int GetFocusedWord()

    bool GetFocusedDirection() const
    void SetFocusedDirection(bool direction)

    // %override number, text MyFrame::GetFocusedClue()
    // const XPuzzle::Clue * MyFrame::GetFocusedClue()
    int GetFocusedClue()

%endclass


// %override MyFrame * GetFrame()
%function MyFrame * GetFrame()

// %override bool MyApp::IsPortable()
%function bool IsPortable()

