//----------------------------------------------------------------------------
// lua interface to XWord
// Execute this to generate bindings:
//     lua -e"rulesFilename='xword_rules.lua'" genxwordbind.lua
//----------------------------------------------------------------------------

%include <vector>

%include "../MyFrame.hpp"
%include "../App.hpp"
%include "../paths.hpp"
%include "../../lua/luapuz/bind/luapuz.hpp"

//----------------------------------------------------------------------------
// Gui stuff
//----------------------------------------------------------------------------

%class MyFrame, wxFrame
    // %override puz::Puzzle & GetPuzzle()
    int GetPuzzle()

    void ShowPuzzle()
    void ShowGrid()
    void ShowClues()
    void ShowTitle()
    void ShowAuthor()
    void ShowCopyright()
    void ShowNotes()

    bool LoadPuzzle(const wxString & filename)
    bool SavePuzzle(wxString filename)
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
    // const puz::Puzzle::Clue * MyFrame::GetFocusedClue()
    int GetFocusedClue()

%endclass


// %override MyFrame * GetFrame()
%function MyFrame * GetFrame()

// %override bool MyApp::IsPortable()
%function bool IsPortable()

%function wxString GetUserDataDir()
%function wxString GetConfigDir()
%function wxString GetScriptsDir()
%function wxString GetImagesDir()
