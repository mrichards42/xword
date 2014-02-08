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

#ifndef MY_AUI_MGR_H
#define MY_AUI_MGR_H

#include <wx/aui/aui.h>
#include <map>
#include <list>

// Enhancements to the wxAuiManager class:
// * When the Frame is resized, the layout is intelligently adapted by giving
//   the grid as much space as possible and filling the rest.
//   This should take care of older hacks to, e.g. keep clue panes the
//   same size.
// * LoadPerspective() caches panes that have not yet been added to the manager.
//     * AddPane() checks to see if this pane has been cached;
//       the wxAuiPaneInfo parameters can be thought of as default values.
// * The user can supply a menu that gets filled with panes and their state.
//     * public function UpdateMenu()
//     * UpdateMenu() is called in AddPane() and DetachPane()
// * An AuiPaneClose event is fired when panes are closed

// NB: Many of these changes require a hacked version of wxAuiManager
//     which is supplied in trunk/wxpatches
// Enhancements via wxAuiManager hacks:
// * Various wxAuiManager functions are made virtual

class MyAuiManager : public wxAuiManager
{
public:
    MyAuiManager(wxWindow* managed_wnd = NULL, unsigned int flags = wxAUI_MGR_DEFAULT);

    // Managed Pane Menu
    //------------------
    wxMenu * SetManagedMenu(wxMenu * menu)
    {
        wxMenu * oldmenu = m_menu;
        m_menu = menu;
        UpdateMenu();
        return oldmenu;
    }

    void UpdateMenu();

    // Pane Cache
    //-----------
    bool GetCachedPane(const wxString & name, wxAuiPaneInfo & pane);
    bool HasCachedPane(const wxString & name);

    // Instead of skipping unknown panes, we'll cache them
    // in m_paneMap.
    virtual bool LoadPerspective(const wxString & layout, bool update = true);
    virtual wxString SavePerspective();

    // If the pane is present in m_paneMap, use that pane info instead of
    // the supplied info; call UpdateMenu()
    virtual bool AddPane(wxWindow * window, const wxAuiPaneInfo & pane_info);
    // InsertPane uses AddPane in the implementation already

    // Edit Mode
    // ---------
    void StartEdit();
    void EndEdit();
    bool IsEditing() const { return m_isEditing; }

    // NB: This won't override anything unless wxAuiManager::Update is
    // made virtual (in include/wx/aui/framemanager.h)
    void Update();

    // Misc
    //-----

    // Clean up pane menu
    virtual bool DetachPane(wxWindow * window);

    wxAuiPaneInfo & GetPaneByCaption(const wxString & caption);

    // Find the pane that this window belongs to.  Window can be
    // any child of a pane.
    wxAuiPaneInfo & FindPane(wxWindow * window);
    wxAuiPaneInfo & FindPane(int id);

    // Find the dock this pane is in
    wxAuiDockInfo & FindDock(wxAuiPaneInfo & info);

protected:
    // Close Event
    bool FireCloseEvent(wxAuiPaneInfo & pane);
    void OnPaneButton(wxAuiManagerEvent & evt);

    // Menu
    wxMenu * m_menu;
    void AddToMenu(wxAuiPaneInfo & pane);
    void RemoveFromMenu(wxAuiPaneInfo & pane);
    void RemoveFromMenu(int id);
    void OnMenu(wxCommandEvent & evt);
    void OnUpdateUI(wxUpdateUIEvent & evt);

    // Cache
    std::map<wxString, wxString> m_paneCache;

    // Proportional resizing
    wxSize m_frameSize;
    void OnFrameSize(wxSizeEvent & evt);
    void ResizeDocks(bool is_frame_resize);

    // Edit Mode
    void OnLeftDown(wxMouseEvent & evt);
    void OnSetCursor(wxSetCursorEvent & evt);
    void OnCaptureLost(wxMouseCaptureLostEvent & evt);
    bool m_isEditing;
    std::map<wxString, wxAuiPaneInfo> m_editCache;

    DECLARE_EVENT_TABLE();
};

#endif // MY_AUI_MGR_H
