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
// * Panes retain their size as best possible by setting best_size on Update()
//     * Update() override sets best_size
// * CluePanel panes retain the same size whenever possible.
//     * Update() sets dock_proportion or dock.size
// * LoadPerspective() caches panes that have not yet been added to the manager.
//     * AddPane() checks to see if this pane has been cached;
//       the wxAuiPaneInfo parameters can be thought of as default values.
// * Context menus allow for docking, floating, and closing panes.
//     * SetContextWindow() sets the window that can be right clicked to
//       pop up a context menu.
//     * Context menus are always allowed on pane captions.
//     * Override DetachPane() so that RemoveContextWindow() is called
//       first.
//     * CreateFloatingFrame() override adds a context window
// * The user can supply a menu that gets filled with panes and their state.
//     * public function UpdateMenu()
//     * UpdateMenu() is called in AddPane() and DetachPane()
// * AuiManager keeps track of the frame size and resizes panes proportionally
//   when the frame is resized.
// * An AuiPaneClose event is fired when panes are closed

// NB: Many of these changes require a hacked version of wxAuiManager
//     which is supplied in trunk/wxpatches
// Enhancements via wxAuiManager hacks:
// * wxAuiPaneInfo ::resizable and ::fixed are independent values.
//     * Fixed panes work with absolute coordinates, while non-Resizable
//       panes merely omit the resize sash, and maintain relative coordinates.
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

    // Context Menu
    //-------------
    void SetContextWindow(wxWindow * pane_window, wxWindow * window)
    {
        SetContextWindow(GetPane(pane_window), window);
    }
    void SetContextWindow(const wxString & name, wxWindow * window)
    {
        SetContextWindow(GetPane(name), window);
    }
    void SetContextWindow(wxAuiPaneInfo & info, wxWindow * window);

    void RemoveContextWindow(wxWindow * window);
    void RemoveContextWindow(wxAuiPaneInfo & info);

    // Proportional resizing
    //--------------------
    virtual void Update();

    // Edit Mode
    // ---------
    void StartEdit();
    void EndEdit();
    bool IsEditing() const { return m_isEditing; }

    // Misc
    //-----

    // Clean up context and pane menus
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

    // Context Menu
    wxAuiPaneInfo & HitTestPane(int x, int y);
    wxAuiFloatingFrame* CreateFloatingFrame(wxWindow* parent,
                                            const wxAuiPaneInfo& pane_info);

    // Menu
    wxMenu * m_menu;
    void AddToMenu(wxAuiPaneInfo & pane);
    void RemoveFromMenu(wxAuiPaneInfo & pane);
    void RemoveFromMenu(int id);
    void OnMenu(wxCommandEvent & evt);
    void OnUpdateUI(wxUpdateUIEvent & evt);

    // Context menus
    std::map<wxWindow *, wxAuiPaneInfo *> m_contextWindows;
    wxAuiPaneInfo * m_contextPane;
    wxMenu * NewContextMenu(wxAuiPaneInfo & pane);
    void OnContextMenu(wxContextMenuEvent & evt);
    void OnFloatingContextMenu(wxContextMenuEvent & evt);
    void ShowContextMenu(wxAuiPaneInfo & pane);
    void OnContextMenuClick(wxCommandEvent & evt);

    // Cache
    std::map<wxString, wxString> m_paneCache;

    // Proportional resizing
    wxSize m_frameSize;
    void OnFrameSize(wxSizeEvent & evt);
    void ResizeDocks(const wxSize & oldSize, const wxSize & newSize);
    void ConstrainPanes(std::list<wxAuiPaneInfo *> & panes);

    void SavePaneSize(wxAuiPaneInfo & pane);

    // Edit Mode
    void OnLeftDown(wxMouseEvent & evt);
    void OnEditContextMenu(wxContextMenuEvent & evt);
    void OnSetCursor(wxSetCursorEvent & evt);
    void OnCaptureLost(wxMouseCaptureLostEvent & evt);
    bool m_isEditing;
    std::map<wxString, wxAuiPaneInfo> m_editCache;

    DECLARE_EVENT_TABLE();
};

#endif // MY_AUI_MGR_H
