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


// TODO:
//   - Figure out what needs to go in the config

#ifndef MY_FRAME_H
#define MY_FRAME_H


#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

#include <wx/aui/aui.h>


// XWord library
#include "puz/XPuzzle.hpp"
class wxPuzEvent;

// Windows
class ClueListBox;
class SizedText;
class CluePanel;
class LayoutDialog;
class PropertiesDialog;

#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"


#include "utils/ToolManager.hpp"

// Config headers
#include <wx/wfstream.h>    // for wxFileInputStream and wxFileOutputStream
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>


//#define USE_AUI_TOOLBAR


//------------------------------------------------------------------------------
// The XWord frame class
//------------------------------------------------------------------------------

class MyFrame : public wxFrame
{
    friend class PropertiesDialog;
public:
    MyFrame();
    ~MyFrame();

    // XWord puzzle loading / saving
    //-----------
    bool LoadPuzzle(const wxString & filename, const wxString & ext = _T(""));
    bool SavePuzzle(      wxString   filename, const wxString & ext = _T(""));
    bool ClosePuzzle(bool prompt = true); // Return true = puzzle is closed
    void CheckPuzzle();
    void ShowPuzzle();  // Displays the actual puzzle

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
    void ResetTimer()     { SetTime(0); }
    void StartTimer()     { if (! IsTimerRunning()) m_timer.Start(); }
    void StopTimer()      { if (IsTimerRunning()) m_timer.Stop(); }
    void ToggleTimer()
        { if (IsTimerRunning()) m_timer.Stop(); else m_timer.Start(); }

private:
    // Windows
    //--------
    void CreateWindows();

#ifdef USE_AUI_TOOLBAR
    wxAuiToolBar * MakeAuiToolBar();
#else
    wxToolBar    * MakeToolBar();
#endif

    wxMenuBar    * MakeMenuBar();

    XGridCtrl *  m_gridCtrl;

    CluePanel *  m_down;
    CluePanel *  m_across;
    CluePanel * GetCrossingClues();
    CluePanel * GetFocusedClues();

    SizedText *  m_title;
    SizedText *  m_author;
    SizedText *  m_copyright;
    SizedText *  m_cluePrompt;
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
    XPuzzle m_puz;


    // Config
    //-------
    void LoadConfig();
    void SaveConfig();

    wxFileConfig * GetConfig();

    PropertiesDialog * m_propertiesDialog;


private:
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

    // XGridCtrl interaction
    void OnZoomIn      (wxCommandEvent & WXUNUSED(evt));
    void OnZoomFit     (wxCommandEvent & WXUNUSED(evt));
    void OnZoomOut     (wxCommandEvent & WXUNUSED(evt));

    void OnCheckGrid   (wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckGrid(); }
    void OnCheckWord   (wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckWord(); }
    void OnCheckLetter (wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckLetter(); }

    void OnRevealGrid   (wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckGrid(REVEAL_ANSWER | CHECK_ALL); }
    void OnRevealIncorrect(wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckGrid(REVEAL_ANSWER); }
    void OnRevealWord  (wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckWord(REVEAL_ANSWER); }
    void OnRevealLetter(wxCommandEvent & WXUNUSED(evt))
        { m_gridCtrl->CheckLetter(REVEAL_ANSWER); }

    // XWord puzzle stuff
    void OnScramble   (wxCommandEvent & WXUNUSED(evt));
    void OnUnscramble (wxCommandEvent & WXUNUSED(evt));

    // Window management
    void OnLayout     (wxCommandEvent & WXUNUSED(evt));
    void OnLoadLayout (wxCommandEvent & WXUNUSED(evt));
    void OnSaveLayout (wxCommandEvent & WXUNUSED(evt));
    void OnShowNotes  (wxCommandEvent & WXUNUSED(evt));

    // Timer
    void OnTimer      (wxCommandEvent & WXUNUSED(evt));
    void OnTimerNotify(wxTimerEvent   & WXUNUSED(evt));

    // Config
    void OnOptions    (wxCommandEvent & WXUNUSED(evt));
    void OnPropertiesDialogCancel(wxCommandEvent & evt);


    // XGridCtrl and CluePanel events
    //------------------------------
    void OnGridFocus  (wxPuzEvent & evt);
    void OnGridLetter (wxPuzEvent & evt);
    void OnClueFocus  (wxPuzEvent & evt);


    // Frame events
    //-------------
    void OnActivate   (wxActivateEvent & evt);
    void OnClose      (wxCloseEvent & evt);


private:
    // Debugging
    //----------

#ifdef __WXDEBUG__

    void ShowDebugDialog(const wxString & title, const wxString & str);
    void OnDumpLayout (wxCommandEvent & WXUNUSED(evt));
    void OnDumpStatus (wxCommandEvent & WXUNUSED(evt));

#endif

    DECLARE_EVENT_TABLE()
};



//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------

inline
CluePanel *
MyFrame::GetFocusedClues()
{
    return (m_gridCtrl->GetDirection() == DIR_ACROSS ?
             m_across :
             m_down);
}

inline
CluePanel *
MyFrame::GetCrossingClues()
{
    return (m_gridCtrl->GetDirection() == DIR_DOWN ?
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


inline bool
MyFrame::LoadLayoutString(const wxString & layout, bool update)
{
    // Save the toolbar size so it isn't cut off
    //wxSize tbSize = m_toolbar->GetMinSize();

    if (! m_mgr.LoadPerspective(layout, false))
        return false;

    // Restore toolbar size
    //m_mgr.GetPane(m_toolbar).BestSize(tbSize);

    if (update)
        m_mgr.Update();

    return true;
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