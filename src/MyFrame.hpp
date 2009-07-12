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
// - Most importantly, organize MyFrame.hpp
//    and MyFrame.cpp (including comments)
//   - Figure out what needs to go in the config


#ifndef MY_FRAME_H
#define MY_FRAME_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wx/aui/aui.h>

// XWord library
#include "puz/XPuzzle.hpp"
class wxPuzEvent;

// Windows
class ClueListBox;
class SizedText;
class CluePanel;
class PerspectiveDialog;

#include "GridCtrl.hpp"
#include "MyStatusBar.hpp"

#include "utils/ToolManager.hpp"

// Drag and drop
class XWordFileDropTarget;

// Config headers
#include <wx/wfstream.h>    // for wxFileInputStream and wxFileOutputStream
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>


// Frame menu and toolbar ids
enum
{
    ID_OPEN = wxID_HIGHEST,
    ID_SAVE,
    ID_CLOSE,

    ID_QUIT,

    ID_ZOOM_IN,
    ID_ZOOM_FIT,
    ID_ZOOM_OUT,
    ID_CHECK_LETTER,
    ID_CHECK_WORD,
    ID_CHECK_GRID,

    ID_SCRAMBLE,
    ID_UNSCRAMBLE,

    ID_LAYOUT_PANES,
    ID_LOAD_PERSPECTIVE,
    ID_SAVE_PERSPECTIVE,

    ID_SHOW_NOTES,
    ID_SHOW_NOTES_NEW,

    ID_TIMER,

#ifdef __WXDEBUG__
    ID_DUMP_STATUS
#endif
};

class MyFrame
    : public wxFrame
{
    friend class PerspectiveDialog;
public:
    MyFrame();
    ~MyFrame();

    bool LoadPuzzle(const wxString & filename,
                    const wxString & ext = wxEmptyString);

    bool SavePuzzle(const wxString & filename,
                    const wxString & ext = wxEmptyString);

    // Return true if no puzzle is opened after the function
    bool ClosePuzzle();

    void SetStatus(const wxString & text) { m_status->SetStatus(text); }


protected:
    // General use functions
    //------------------------

    void SavePerspective      (const wxString & name);
    bool LoadPerspective      (const wxString & name, bool update = false);
    bool LoadPerspectiveString(const wxString & name, bool update = false);

    void EnableTools(bool enable = true);

    // Timer convenience functions
    void SetTime  (int time) { m_time = time; m_status->SetTime(time); }
    void StartTimer(bool start = true)
        { if (start != m_timer.IsRunning()) ToggleTimer(); }
    void ResetTimer()  { SetTime(0); }
    void StopTimer()   { StartTimer(false); }
    void ToggleTimer() { wxCommandEvent evt; OnTimer(evt); }
    bool IsTimerRunning() { return m_timer.IsRunning(); }

private:
    // Member variables
    //------------------------

    // An XWord puzzle
    XPuzzle m_puz;

    // Location of the config file
    wxString m_configFile;

    // Windows
    // -------------------------

    // Grid
    GridCtrl * m_grid;

    // Clues
    CluePanel    * m_downPanel;
    ClueListBox  * m_down;

    CluePanel    * m_acrossPanel;
    ClueListBox  * m_across;

    SizedText  * m_title;
    SizedText  * m_author;
    SizedText  * m_copyright;
    SizedText  * m_cluePrompt;
    wxTextCtrl * m_notes;

    wxAuiToolBar * m_toolbar;
    wxMenuBar * m_menubar;

    MyStatusBar * m_status;


    // Window Manager
    wxAuiManager m_mgr;

    // Tool Manager
    ToolManager m_toolMgr;

    // Timer
    wxTimer m_timer;
    int m_time;
    bool m_timerRunning;

private:
    // Frame setup functions
    void CreateWindows();
    void SetupManager();
    void ManageWindows();
    wxAuiToolBar * MakeAuiToolBar();
    wxMenuBar    * MakeMenuBar();

    // Config
    void LoadConfig();
    void SaveConfig();

    // Frame Management
    void ShowPane(const wxString & name, bool show = true);
    void HidePane(const wxString & name) { return ShowPane(name, false); }
    void UpdateLayout();

    // Called from LoadPuzzle
    void ShowPuzzle();
    void CheckPuzzle();

protected:
    // Event Handlers
    //----------------------------------------------

    // Menus and toolbars
    void OnOpenPuzzle (wxCommandEvent & WXUNUSED(evt));
    void OnSavePuzzle (wxCommandEvent & WXUNUSED(evt));
    void OnClosePuzzle(wxCommandEvent & WXUNUSED(evt))
        { ClosePuzzle(); ShowPuzzle(); }
    void OnQuit       (wxCommandEvent & WXUNUSED(evt))   { Close(); }

    void OnZoomIn (wxCommandEvent & WXUNUSED(evt));
    void OnZoomFit(wxCommandEvent & WXUNUSED(evt));
    void OnZoomOut(wxCommandEvent & WXUNUSED(evt));

    void OnCheckGrid  (wxCommandEvent & evt)  { m_grid->CheckGrid(); }
    void OnCheckWord  (wxCommandEvent & evt)  { m_grid->CheckWord(); }
    void OnCheckLetter(wxCommandEvent & evt)  { m_grid->CheckLetter(); }

    void OnScramble(wxCommandEvent & WXUNUSED(evt));
    void OnUnscramble(wxCommandEvent & WXUNUSED(evt));

    void OnLayout(wxCommandEvent & WXUNUSED(evt));
    void OnLoadPerspective(wxCommandEvent & WXUNUSED(evt));
    void OnSavePerspective(wxCommandEvent & WXUNUSED(evt));
    void OnShowNotes      (wxCommandEvent & WXUNUSED(evt));

    void OnTimer          (wxCommandEvent & WXUNUSED(evt));
    void OnTimerNotify    (wxTimerEvent   & WXUNUSED(evt));

    // From the puzzle's controls
    void OnGridFocus (wxPuzEvent & evt);
    void OnGridLetter(wxPuzEvent & evt);
    void OnClueFocus (wxPuzEvent & evt);

    // Frame Events
    void OnActivate(wxActivateEvent & evt);
    void OnClose(wxCloseEvent & evt);

#ifdef __WXDEBUG__
    void OnDumpStatus(wxCommandEvent & WXUNUSED(evt));
#endif

    DECLARE_EVENT_TABLE()
};



inline void
MyFrame::UpdateLayout()
{
    // Save the toolbar size so it isn't cut off
    wxSize tbSize = m_toolbar->GetMinSize();
    m_mgr.GetPane(m_toolbar).BestSize(tbSize);
    m_mgr.Update();
}


inline void
MyFrame::SavePerspective(const wxString & name)
{
    wxConfigBase * config = wxFileConfig::Get();
    wxASSERT(config != NULL);
    config->Write(wxString::Format(_T("Perspectives/%s"), name),
                  m_mgr.SavePerspective());
}

inline bool
MyFrame::LoadPerspectiveString(const wxString & perspective, bool update)
{
    // Save the toolbar size so it isn't cut off
    wxSize tbSize = m_toolbar->GetMinSize();
    if (m_mgr.LoadPerspective(perspective, false))
    {
        m_mgr.GetPane(m_toolbar).BestSize(tbSize);
        if (update)
            m_mgr.Update();
        return true;
    }
    return false;
}


inline bool
MyFrame::LoadPerspective(const wxString & name, bool update)
{
    wxConfigBase * config = wxFileConfig::Get();
    wxASSERT(config != NULL);
    wxString perspective;
    if (config->Read(wxString::Format(_T("Perspectives/%s"), name),
                     &perspective))
    {
        return LoadPerspectiveString(perspective, update);
    }

    return false;
}


inline void
MyFrame::ShowPane(const wxString & name, bool show)
{
    wxAuiPaneInfo & info = m_mgr.GetPane(name);
    wxASSERT(info.IsOk());

    info.Show(show);
    UpdateLayout();
}



inline void
MyFrame::EnableTools(bool enable)
{
    m_toolMgr.Enable(ID_SAVE,         enable);
    m_toolMgr.Enable(ID_CLOSE,        enable);

    m_toolMgr.Enable(ID_ZOOM_IN,      enable);
    m_toolMgr.Enable(ID_ZOOM_OUT,     enable);
    m_toolMgr.Enable(ID_ZOOM_FIT,     enable);

    m_toolMgr.Enable(ID_SCRAMBLE,     enable);
    m_toolMgr.Enable(ID_UNSCRAMBLE,   enable);

    m_toolMgr.Enable(ID_CHECK_LETTER, enable);
    m_toolMgr.Enable(ID_CHECK_WORD,   enable);
    m_toolMgr.Enable(ID_CHECK_GRID,   enable);

    m_toolMgr.Enable(ID_SHOW_NOTES,   enable);

    m_toolMgr.Enable(ID_TIMER,        enable);
}


#endif // MY_FRAME_H