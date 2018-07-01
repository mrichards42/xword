// This file is part of XWord
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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


// XWord library
#include "puz/Puzzle.hpp"
class wxPuzEvent;

// Windows
class ClueListBox;
class CluePrompt;
class MetadataCtrl;
class CluePanel;
class LayoutDialog;
class PreferencesDialog;
class wxHtmlWindow;
class wxHtmlLinkEvent;
class NotesPanel;

class MyPrintout;
struct PrintInfo;
class ConfigManager;

#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"

#include "utils/ToolManager.hpp"

#include <wx/fileconf.h> // Used to get the layouts directly from wxConfig
#include <wx/docview.h> // wxFileHistory

#include <map>

//#define USE_AUI_TOOLBAR
#define USE_MY_AUI_MANAGER 1

#if USE_MY_AUI_MANAGER
#   include "MyAuiMgr.hpp"
#else
#   include <wx/aui/aui.h>
#   include <wx/tokenzr.h>
#endif


//------------------------------------------------------------------------------
// The XWord frame class
//------------------------------------------------------------------------------

class MyFrame : public wxFrame
{
    friend class PreferencesDialog;
    friend class MyPrintout;
    friend class ConfigManager;
public:
    MyFrame();
    ~MyFrame();

    // XWord puzzle loading / saving
    //-----------
    bool LoadPuzzle(const wxString & filename);
    bool LoadPuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler);
    bool SavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler = NULL);
    bool ClosePuzzle(bool prompt = true, bool update = true); // Return true = puzzle is closed
    void CheckPuzzle();

    static wxString GetSaveTypeString();
    static wxString GetLoadTypeString();

    // Gui display / updating
    void ShowPuzzle(bool update = true); // Everything
    void ShowGrid();
    void ShowClues();
    void ShowMetadata();
    void ShowNotes();

    puz::Puzzle & GetPuzzle() { return m_puz; }
    const wxString & GetFilename() const { return m_filename; }
    void SetFilename(const wxString & filename) { m_filename = filename; }

    // Window Management
    //-----------
    void ShowPane(const wxString & name, bool show = true);
    void HidePane(const wxString & name) { return ShowPane(name, false); }
    void AddPane(wxWindow * window, const wxAuiPaneInfo & info);

    void SaveLayout      (const wxString & name);
    bool LoadLayout      (const wxString & name, bool update = true);
    void RemoveLayout    (const wxString & name);
    bool LoadPerspective (const wxString & perspective, bool update = true);

    // Status bar
    //-----------
    void SetStatus(const wxString & text) { m_status->SetStatus(text); }
    void SetShowCompletionStatus(bool show);

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
    void SetAutoStartTimer(bool doit = true) { m_autoStartTimer = doit; }
    bool AutoStartTimer() const { return m_autoStartTimer; }

    // AutoSave after 5 sec inactivity
    void SetAutoSaveInterval(int seconds = 5) { m_autoSaveInterval = seconds; }
    int AutoSaveInterval() const { return m_autoSaveInterval; }

    // File history
    void SetSaveFileHistory(bool doit = true);
    const wxFileHistory & GetFileHistory() const { return m_fileHistory; }
    wxFileHistory & GetFileHistory() { return m_fileHistory; }

    void OnAppDeactivate();
    void OnAppActivate();

    // Access to the grid
    //-------------------
    puz::Square * GetFocusedSquare();
    puz::Word * GetFocusedWord();
    puz::Clue * GetFocusedClue();
    short GetFocusedDirection() const;

    puz::Square * SetFocusedSquare(puz::Square * square);
    void SetFocusedWord(puz::Word * word);
    void SetFocusedClue(puz::Clue * clue);
    void SetFocusedDirection(short direction);
    bool SetSquareText(puz::Square * square, const wxString & text = _T(""));

    // Printing
    bool Print(const PrintInfo & info, puz::Puzzle * puz = NULL, bool promt=true);
    void PrintPreview(const PrintInfo & info, puz::Puzzle * puz = NULL);

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

    wxFileHistory m_fileHistory;

    XGridCtrl *  m_XGridCtrl;

    std::map<wxString, CluePanel *> m_clues;

    CluePrompt *  m_cluePrompt;
    NotesPanel * m_notes;

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
    void OnPaneClose(wxAuiManagerEvent & evt);

#if USE_MY_AUI_MANAGER
    MyAuiManager m_mgr;
#else
    wxAuiManager m_mgr;
#endif


    // Tool Management
    //----------------
    void SetupToolManager();
    void ManageTools();

    void EnableTools (bool enable = true);
    void DisableTools() { EnableTools(false); }

    // Incremental enabling/disabling . . .
    void EnableSave(bool enable = true);
    void EnableGridSize(bool enable = true);
    void EnableScramble(bool enable = true);
    void EnableUnscramble(bool enable = true);
    void EnableCheck (bool enable = true);
    void EnableReveal(bool enable = true);
    void EnableDiagramless(bool enable = true);

    void SetHasNotes(bool show = true);

    ToolManager m_toolMgr;


    // Timer
    //------
    wxTimer m_timer;
    int  m_time;
    bool m_isTimerRunning; // Used to start/stop timer on frame activate events.
    bool m_autoStartTimer;

    // AutoSave
    int m_autoSaveInterval;
    wxTimer m_autoSaveTimer;

    // Status
    bool m_showCompletionStatus;

    // The XWord puzzle
    //-----------------
    puz::Puzzle m_puz;
    wxString m_filename;
    bool m_isModified;

    // Config
    //-------
    void LoadConfig();
    void SaveConfig();
    void SaveWindowConfig();

    wxFileConfig * GetConfig();


private:
    // Load / save exception handling.
    void DoSavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler = NULL);
    void HandlePuzException();

    // Event Handlers
    //---------------

    // Menu and toolbar events
    // ------------------------

    // General
    void OnOpenPuzzle  (wxCommandEvent & WXUNUSED(evt));
    void OnOpenRecentPuzzle(wxCommandEvent & evt);
    void OnSavePuzzle  (wxCommandEvent & WXUNUSED(evt));
    void OnSavePuzzleAs(wxCommandEvent & WXUNUSED(evt));
    void OnClosePuzzle (wxCommandEvent & WXUNUSED(evt))   { ClosePuzzle(true); }
    void OnDeletePuzzle(wxCommandEvent & WXUNUSED(evt));
    void OnQuit        (wxCommandEvent & WXUNUSED(evt))   { Close(); }

    // Edit
    void OnCopy        (wxCommandEvent & WXUNUSED(evt));
    void OnCopySquare  (wxCommandEvent & WXUNUSED(evt));
    void OnPaste       (wxCommandEvent & WXUNUSED(evt));
    void OnPasteRebus  (wxCommandEvent & WXUNUSED(evt));

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
    void OnConvertToNormal(wxCommandEvent & WXUNUSED(evt));

    void OnEraseGrid      (wxCommandEvent & WXUNUSED(evt));
    void OnEraseUncrossed (wxCommandEvent & WXUNUSED(evt));

    void OnRebusEntry     (wxCommandEvent & evt);

    // Scrambling
    void OnScramble   (wxCommandEvent & WXUNUSED(evt));
    void OnUnscramble (wxCommandEvent & WXUNUSED(evt));

    // Window layout
    void OnEditLayout (wxCommandEvent & WXUNUSED(evt));
    std::map<wxString, wxAuiPaneInfo> m_paneCache;
    void OnLoadLayout (wxCommandEvent & WXUNUSED(evt));
    void OnSaveLayout (wxCommandEvent & WXUNUSED(evt));
    void OnShowNotes  (wxCommandEvent & WXUNUSED(evt));

    // Menus / toolbar stuff
    void OnUpdateUI   (wxUpdateUIEvent & evt);

    // Timer
    void OnTimer      (wxCommandEvent & WXUNUSED(evt));
    void OnResetTimer (wxCommandEvent & WXUNUSED(evt));
    void OnTimerNotify(wxTimerEvent   & WXUNUSED(evt));

    // AutoSave
    void OnAutoSaveNotify(wxTimerEvent   & WXUNUSED(evt));

    // Preferences
    void OnPreferences(wxCommandEvent & WXUNUSED(evt));

    // Printing
    void OnPageSetup(wxCommandEvent & WXUNUSED(evt));
    void OnPrintBlank(wxCommandEvent & WXUNUSED(evt));
    void OnPrintSolution(wxCommandEvent & WXUNUSED(evt));
    void OnPrintCurrent(wxCommandEvent & WXUNUSED(evt));
    void OnPrintTwoPages(wxCommandEvent & WXUNUSED(evt));
    void OnPrintCustom(wxCommandEvent & WXUNUSED(evt));

    // Help
    void OnAbout(wxCommandEvent & WXUNUSED(evt));
    void OnLicense(wxCommandEvent & WXUNUSED(evt));
#ifdef __WXMSW__
    void OnHelp(wxCommandEvent & WXUNUSED(evt));
#endif // __WXMSW__

    // XGridCtrl and CluePanel events
    //------------------------------
    void OnGridFocus  (wxPuzEvent & evt);
    void OnGridLetter (wxPuzEvent & evt);
    void OnClueFocus  (wxPuzEvent & evt);
    void UpdateClues();


    // Frame events
    //-------------
    void OnActivate    (wxActivateEvent & evt);
    void SetFocusOnIdle(wxIdleEvent & evt);
    bool m_isIdleConnected;
    void OnClose       (wxCloseEvent & evt);
    void OnLinkClicked (wxHtmlLinkEvent & evt);
    void OnSize        (wxSizeEvent & evt);

private:
    // Debugging
    //----------

#ifdef _DEBUG

    void ShowDebugDialog(const wxString & title, const wxString & str);
    void OnDumpLayout (wxCommandEvent & WXUNUSED(evt));
    void OnDumpStatus (wxCommandEvent & WXUNUSED(evt));
    void OnBruteForceUnscramble (wxCommandEvent & WXUNUSED(evt));

#endif // _DEBUG

    DECLARE_EVENT_TABLE()
};



//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------

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
MyFrame::SaveLayout(const wxString & name)
{
    GetConfig()->Write(wxString(_T("/Layouts/")) + name,
                       m_mgr.SavePerspective());
}

inline void
MyFrame::RemoveLayout(const wxString & name)
{
    GetConfig()->DeleteEntry(wxString(_T("/Layouts/")) + name);
}


inline bool
MyFrame::LoadLayout(const wxString & name, bool update)
{
    wxString perspective;
    if (! GetConfig()->Read(wxString(_T("/Layouts/")) + name,
                            &perspective) )
        return false;

    return LoadPerspective(perspective, update);
}

inline bool
MyFrame::LoadPerspective(const wxString & perspective, bool update)
{
#if USE_MY_AUI_MANAGER
    return m_mgr.LoadPerspective(perspective, update);
#else // ! USE_MY_AUI_MANAGER
    // Strip the sections wxAuiManager can't read
    wxString escaped_layout = perspective;
    // Escape
    escaped_layout.Replace(_T("\\|"), _T("\a"));
    escaped_layout.Replace(_T("\\;"), _T("\b"));

    // Get rid of frame_size section (and old "private" panes)
    wxString processed_layout;
    processed_layout.reserve(perspective.size());
    {
        wxStringTokenizer tok(escaped_layout, _T("|"), wxTOKEN_RET_DELIMS);
        while (tok.HasMoreTokens())
        {
            wxString part = tok.GetNextToken();
            if (! part.StartsWith(_T("frame_size"))
                && ! part.StartsWith(_T("__"))) // Old tabs
                processed_layout += part;
        }
        // Unescape
        processed_layout.Replace(_T("\a"), _T("\\|"));
        processed_layout.Replace(_T("\b"), _T("\\;"));
    }
    return m_mgr.LoadPerspective(processed_layout, update);
#endif // ! USE_MY_AUI_MANAGER
}


inline void
MyFrame::ShowPane(const wxString & name, bool show)
{
    wxAuiPaneInfo & info = m_mgr.GetPane(name);
    wxASSERT(info.IsOk());

    info.Show(show);
    m_mgr.Update();
}

#endif // MY_FRAME_H
