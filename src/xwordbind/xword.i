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