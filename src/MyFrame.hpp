// This file is part of XWord
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef MY_FRAME_H
#define MY_FRAME_H


#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

#include <wx/aui/aui.h>


// XWord library
#include "puz/Puzzle.hpp"
class wxPuzEvent;

// Windows
class ClueListBox;
class CluePrompt;
class SizedText;
class CluePanel;
class LayoutDialog;
class PreferencesDialog;

class MyPrintout;

#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"


#include "utils/ToolManager.hpp"

#include <wx/fileconf.h> // Used to get the layouts directly from wxConfig

#ifdef XWORD_USE_LUA
// wxLua
#include "wxlua/include/wxlua.h"
#endif // XWORD_USE_LUA

//#define USE_AUI_TOOLBAR


//------------------------------------------------------------------------------
// The XWord frame class
//------------------------------------------------------------------------------

class MyFrame : public wxFrame
{
    friend class PreferencesDialog;
    friend class MyPrintout;
public:
    MyFrame();
    ~MyFrame();

    // XWord puzzle loading / saving
    //-----------
    bool LoadPuzzle(const wxString & filename);
    bool SavePuzzle(      wxString   filename);
    bool ClosePuzzle(bool prompt = true); // Return true = puzzle is closed
    void CheckPuzzle();

    static wxString GetSaveTypeString();
    static wxString GetLoadTypeString();

    // Gui display / updating
    void ShowPuzzle(); // Everything
    void ShowGrid();
    void ShowClues();
    void ShowAuthor();
    void ShowTitle();
    void ShowCopyright();
    void ShowNotes();

    puz::Puzzle & GetPuzzle() { return m_puz; }

    // Window Management
    //-----------
    void ShowPane(const wxString & name, bool show = true);
    void HidePane(const wxString & name) { return ShowPane(name, false); }

    void SaveLayout      (const wxString & name);
    bool LoadLayout      (const wxString & name, bool update = false);
    bool LoadLayoutString(const wxString & layout, bool update = false);
    void UpdateLayout();


    // Status bar
    //-----------
    void SetStatus(const wxString & text) { m_status->SetStatus(text); }


    // Timer
    //-----------
    bool IsTimerRunning() { return m_timer.IsRunning(); }
    void SetTime  (int time) { m_time = time; m_status->SetTime(time); }
    int GetTime() { return m_time; }
    void ResetTimer()     { SetTime(0); }
    void StartTimer();
    void StopTimer();
    void ToggleTimer()
        { if (IsTimerRunning()) StopTimer(); else StartTimer(); }

    void OnAppDeactivate();
    void OnAppActivate();

    // Access to the grid
    //-------------------
    puz::Square * GetFocusedSquare();
    puz::Square * SetFocusedSquare(puz::Square * square);
    void GetFocusedWord(puz::Square ** start, puz::Square ** end);
    puz::GridDirection GetFocusedDirection() const;
    void SetFocusedDirection(puz::GridDirection direction);
    const puz::Puzzle::Clue * GetFocusedClue();
    bool SetSquareText(puz::Square * square, const wxString & text = _T(""));

    // Lua
    void RunLuaScript(const wxString & filename);

private:
    // Windows
    //--------
    void CreateWindows();

#ifdef USE_AUI_TOOLBAR
    wxAuiToolBar * CreateAuiToolBar();
#else
    wxToolBar    * CreateToolBar();
#endif

    wxMenuBar    * CreateMenuBar();

    XGridCtrl *  m_XGridCtrl;

    CluePanel *  m_down;
    CluePanel *  m_across;
    CluePanel * GetCrossingClues();
    CluePanel * GetFocusedClues();

    SizedText *  m_title;
    SizedText *  m_author;
    SizedText *  m_copyright;
    CluePrompt *  m_cluePrompt;
    wxTextCtrl * m_notes;

#ifdef USE_AUI_TOOLBAR
    wxAuiToolBar * m_toolbar;
#else
    wxToolBar * m_toolbar;
#endif

    wxMenuBar * m_menubar;

    MyStatusBar * m_status;


    // Window Management
    //------------------
    void SetupWindowManager();
    void ManageWindows();

    wxAuiManager m_mgr;


    // Tool Management
    //----------------
    void SetupToolManager();
    void ManageTools();

    void EnableTools (bool enable = true);
    void DisableTools() { EnableTools(false); }

    // Incremental enabling/disabling . . .
    void EnableSave(bool enable = true);
    void EnableSaveAs(bool enable = true) { EnableSave(! enable); }
    void EnableGridSize(bool enable = true);
    void EnableScramble(bool enable = true);
    void EnableUnscramble(bool enable = true);
    void EnableCheck (bool enable = true);
    void EnableReveal(bool enable = true);

    void SetHasNotes(bool show = true);

    ToolManager m_toolMgr;


    // Timer
    //------
    wxTimer m_timer;
    int  m_time;
    bool m_isTimerRunning; // Used to start/stop timer on frame activate events.

    // The XWord puzzle
    //-----------------
    puz::Puzzle m_puz;
    wxString m_filename;
    bool m_isModified;


    // Config
    //-------
    void LoadConfig();
    void SaveConfig();

    wxFileConfig * GetConfig();

    PreferencesDialog * m_preferencesDialog;


private:
    // Load / save exception handling.
    void HandlePuzException();

    // Event Handlers
    //---------------

    // Menu and toolbar events
    // ------------------------

    // General
    void OnOpenPuzzle  (wxCommandEvent & WXUNUSED(evt));
    void OnSavePuzzle  (wxCommandEvent & WXUNUSED(evt));
    void OnSavePuzzleAs(wxCommandEvent & WXUNUSED(evt));
    void OnClosePuzzle (wxCommandEvent & WXUNUSED(evt))   { ClosePuzzle(true); }
    void OnQuit        (wxCommandEvent & WXUNUSED(evt))   { Close(); }

    // Zoom
    void OnZoomIn      (wxCommandEvent & WXUNUSED(evt));
    void OnZoomFit     (wxCommandEvent & WXUNUSED(evt));
    void OnZoomOut     (wxCommandEvent & WXUNUSED(evt));

    // Check / Reveal
    void OnCheckGrid      (wxCommandEvent & WXUNUSED(evt));
    void OnCheckSelection (wxCommandEvent & WXUNUSED(evt));
    void OnCheckWord      (wxCommandEvent & WXUNUSED(evt));
    void OnCheckLetter    (wxCommandEvent & WXUNUSED(evt));
    void OnRevealGrid     (wxCommandEvent & WXUNUSED(evt));
    void OnRevealSelection(wxCommandEvent & WXUNUSED(evt));
    void OnRevealIncorrectSelection(wxCommandEvent & WXUNUSED(evt));
    void OnRevealIncorrect(wxCommandEvent & WXUNUSED(evt));
    void OnRevealWord     (wxCommandEvent & WXUNUSED(evt));
    void OnRevealLetter   (wxCommandEvent & WXUNUSED(evt));

    // Scrambling
    void OnScramble   (wxCommandEvent & WXUNUSED(evt));
    void OnUnscramble (wxCommandEvent & WXUNUSED(evt));

    // Window layout
    void OnLayout     (wxCommandEvent & WXUNUSED(evt));
    void OnLoadLayout (wxCommandEvent & WXUNUSED(evt));
    void OnSaveLayout (wxCommandEvent & WXUNUSED(evt));
    void OnShowNotes  (wxCommandEvent & WXUNUSED(evt));
    void OnAuiPaneClose(wxAuiManagerEvent & evt);

    // Timer
    void OnTimer      (wxCommandEvent & WXUNUSED(evt));
    void OnTimerNotify(wxTimerEvent   & WXUNUSED(evt));

    // File conversion
    void OnConvert    (wxCommandEvent & WXUNUSED(evt));


    // Preferences
    void OnPreferences(wxCommandEvent & WXUNUSED(evt));
    void OnPreferencesDialogCancel(wxCommandEvent & evt);

    // Printing
    void OnPageSetup(wxCommandEvent & WXUNUSED(evt));
    void OnPrintPreview(wxCommandEvent & WXUNUSED(evt));
    void OnPrint(wxCommandEvent & WXUNUSED(evt));

    // Help
    void OnAbout(wxCommandEvent & WXUNUSED(evt));
    void OnLicense(wxCommandEvent & WXUNUSED(evt));

    // XGridCtrl and CluePanel events
    //------------------------------
    void OnGridFocus  (wxPuzEvent & evt);
    void OnGridLetter (wxPuzEvent & evt);
    void OnClueFocus  (wxPuzEvent & evt);


    // Frame events
    //-------------
    void OnActivate   (wxActivateEvent & evt);
    void OnClose      (wxCloseEvent & evt);

#ifdef XWORD_USE_LUA
    void OnLuaScript  (wxCommandEvent & WXUNUSED(evt));
    void OnLuaPrint   (wxLuaEvent & evt);
    void OnLuaError   (wxLuaEvent & evt);
    wxLuaState m_lua;
    void LuaInit();
#endif // XWORD_USE_LUA

private:
    // Debugging
    //----------

#ifdef __WXDEBUG__

    void ShowDebugDialog(const wxString & title, const wxString & str);
    void OnDumpLayout (wxCommandEvent & WXUNUSED(evt));
    void OnDumpStatus (wxCommandEvent & WXUNUSED(evt));
    void OnBruteForceUnscramble (wxCommandEvent & WXUNUSED(evt));

#endif // __WXDEBUG__

    DECLARE_EVENT_TABLE()
};



//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------

inline
CluePanel *
MyFrame::GetFocusedClues()
{
    return (m_XGridCtrl->GetDirection() == puz::ACROSS ?
             m_across :
             m_down);
}

inline
CluePanel *
MyFrame::GetCrossingClues()
{
    return (m_XGridCtrl->GetDirection() == puz::DOWN ?
             m_across :
             m_down);
}


// Config
//-------

inline
wxFileConfig *
MyFrame::GetConfig()
{
    wxFileConfig * config = dynamic_cast<wxFileConfig*>(wxFileConfig::Get());
    wxASSERT_MSG(config != NULL, _T("Did you forget to set up a config?"));
    return config;
}



// Window Management
//------------------

inline void
MyFrame::UpdateLayout()
{
    // Save the toolbar size so it isn't cut off
    //wxSize tbSize = m_toolbar->GetMinSize();
    //m_mgr.GetPane(m_toolbar).BestSize(tbSize);
    m_mgr.Update();
}


inline void
MyFrame::SaveLayout(const wxString & name)
{
    GetConfig()->Write(wxString::Format(_T("/Layouts/%s"), name.c_str()),
                       m_mgr.SavePerspective());
}



inline bool
MyFrame::LoadLayout(const wxString & name, bool update)
{
    wxString perspective;
    if (! GetConfig()->Read(wxString::Format(_T("/Layouts/%s"), name.c_str()),
                            &perspective) )
        return false;

    return LoadLayoutString(perspective, update);
}


inline void
MyFrame::ShowPane(const wxString & name, bool show)
{
    wxAuiPaneInfo & info = m_mgr.GetPane(name);
    wxASSERT(info.IsOk());

    info.Show(show);
    UpdateLayout();
}

#endif // MY_FRAME_H