//----------------------------------------------------------------------------
// lua interface to XWord
// Execute this to generate bindings:
//     lua -e"rulesFilename='xword_rules.lua'" genxwordbind.lua
//----------------------------------------------------------------------------

//#include <vector>

#include "../MyFrame.hpp"
#include "../PuzEvent.hpp"
#include "../App.hpp"
#include "../paths.hpp"
#include "../../lua/luapuz/bind/luapuz.hpp"
#include "../../lua/luapuz/bind/luapuz_puz_Puzzle_helpers.hpp"
#include "../../lua/wxbind/include/wxaui_bind.h"
#include "../dialogs/PrintDialog.hpp"


//----------------------------------------------------------------------------
// Printing
//----------------------------------------------------------------------------
class PrintInfo
{
    PrintInfo(int options = 0);
    
    bool clues;
    bool grid;
    int grid_options;
    bool two_pages;
    bool author;
    bool title;
    bool notes;
};

// Printing flags
#define DRAW_USER_TEXT  0x01
#define DRAW_SOLUTION   0x02
#define DRAW_NUMBER     0x04
#define DRAW_FLAG       0x08
#define DRAW_X          0x10
#define DRAW_CIRCLE     0x20
#define DRAW_OUTLINE    0x40
#define DRAW_BLANK_DIAGRAMLESS 0x80
#define DRAW_THEME      0x100



//----------------------------------------------------------------------------
// Gui stuff
//----------------------------------------------------------------------------

class MyFrame : public wxFrame
{
    // %override puz::Puzzle & GetPuzzle()
    int GetPuzzle()
    const wxString & GetFilename()
    void SetFilename(const wxString & filename)

    // Override so that Puzzle::TestOk is called first
    // %override void ShowPuzzle()
    void ShowPuzzle()
    // %override void ShowGrid()
    void ShowGrid()
    // %override void ShowClues()

    void ShowClues()
    void ShowMetadata()
    void ShowNotes()

    // %override bool LoadPuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler = NULL)
    int LoadPuzzle()
    // %override bool SavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler = NULL)
    int SavePuzzle()

    // Return true = puzzle is closed
    bool ClosePuzzle(bool prompt = true)
    void CheckPuzzle()

    void ShowPane(const wxString & name, bool show = true);
    void HidePane(const wxString & name)
    void AddPane(wxWindow * window, const wxAuiPaneInfo & info)


    void SetStatus(const wxString & text)

    bool IsTimerRunning()
    void SetTime(int time)
    int  GetTime()
    void ResetTimer()
    void StartTimer()
    void StopTimer()
    void ToggleTimer()

    // %override puz::Square * GetFocusedSquare()
    int GetFocusedSquare()

    // %override puz::Square * SetFocusedSquare(puz::Square * square)
    int SetFocusedSquare()

    // %override bool SetSquareText(puz::Square * square, const wxString & text)
    int SetSquareText()

    // %override [Lua table] MyFrame::GetFocusedWord()
    // void MyFrame::GetFocusedWord(puz::Square ** start, puz::Square ** end)
    int GetFocusedWord()

    // %override puz::GridDirection MyFrame::GetFocusedDirection()
    int GetFocusedDirection()
    // %override void MyFrame::SetFocusedDirection(puz::GridDirection)
    int SetFocusedDirection()


    // %override number, text MyFrame::GetFocusedClue()
    // const puz::Clue * MyFrame::GetFocusedClue()
    int GetFocusedClue()

    // %override bool Print(const PrintInfo & info, puz::Puzzle * puz = NULL, bool prompt = true)
    int Print()
    // %override void PrintPreview(const PrintInfo & info, puz::Puzzle * puz = NULL)
    int PrintPreview()
};


// %override MyFrame * GetFrame()
MyFrame * GetFrame()

// %override void logerror()
void logerror()


//----------------------------------------------------------------------------
// Events
//----------------------------------------------------------------------------

class wxPuzEvent : public wxCommandEvent
{
    // %override puz::Square * GetSquare()
    int GetSquare()

    %wxEventType wxEVT_PUZ_GRID_FOCUS
    %wxEventType wxEVT_PUZ_CLUE_FOCUS
    %wxEventType wxEVT_PUZ_LETTER
};

typedef wxCommandEvent wxluataskevent {
    %wxEventType EVT_LUATASK
};