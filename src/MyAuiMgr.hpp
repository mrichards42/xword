// This file is part of XWord    
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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

// Enhancements to the wxAuiManager class:
// * Tabs along each side for hidden windows
//     * class MyAuiManagerTabs to draw tabs
//     * OnPaneButton() event for a custom Pin Button action
//     * ProcessDockResult() override to disallow docking outside tabs
// * Panes retain their size as best possible by setting best_size whenever
//   their docking / floating / hiding state changes.
//     * CreateFloatingFrame() override sets best_size
//     * OnPaneButton() event sets best_size on Close Button
//     * Panes hidden to tabs set best_size
// * Across and Down panes retain the same size whenever possible.
//     * OnRender() event sets dock_proportion, or dock.size
// * LoadPerspective() caches panes that have not yet been added to the manager.
//     * AddPane() checks to see if this pane has been cached;
//       the wxAuiPaneInfo parameters can be thought of as default values.
// * Context menus allow for docking, floating, hiding, and closing panes.
//     * SetContextWindow() sets the window that can be right clicked to
//       pop up a context menu.
//     * Context menus are always allowed on pane captions.
//     * Override DetachPane() so that RemoveContextWindow() is called
//       first.
// * The user can supply a menu that gets filled with panes and their state.
//     * public function UpdateMenu()
//     * UpdateMenu() is called in AddPane() and DetachPane()

class MyAuiManagerTabs;

class MyAuiManager : public wxAuiManager
{
    friend class MyAuiManagerTabs;
public:
    MyAuiManager(wxWindow* managed_wnd = NULL, unsigned int flags = wxAUI_MGR_DEFAULT);


    // Menus
    wxMenu * SetManagedMenu(wxMenu * menu)
    {
        wxMenu * oldmenu = m_menu;
        m_menu = menu;
        UpdateMenu();
        return oldmenu;
    }

    void UpdateMenu();

    // Instead of skipping unknown panes, we'll cache them
    // in m_paneMap.
    // I've hacked wx/aui/framemanager.hpp to make this virtual.
    virtual bool LoadPerspective(const wxString & layout, bool update = true);

    virtual wxString SavePerspective();

    // If the pane is present in m_paneMap, use that pane info instead of
    // the supplied info; call UpdateMenu()
    virtual bool AddPane(wxWindow * window, const wxAuiPaneInfo & pane_info);
    // InsertPane uses AddPane in the implementation already

    // Context menu windows
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

    // call RemoveContextWindow() and UpdateMenu()
    virtual bool DetachPane(wxWindow * window);

    // Find the pane that this window belongs to.  Window can be
    // any child of a pane.
    wxAuiPaneInfo & FindPane(wxWindow * window);
    wxAuiPaneInfo & FindPane(int id);

    bool GetCachedPane(const wxString & name, wxAuiPaneInfo & pane);
    bool HasCachedPane(const wxString & name);

    wxAuiPaneInfo & GetPaneByCaption(const wxString & caption);

    bool IsPaneClosed(const wxAuiPaneInfo & pane);

    virtual void Update();

protected:
    bool FireCloseEvent(wxAuiPaneInfo & pane);

    // Custom button actions:
    // * Pin button hides pane to a tab
    // * Close button sets best_size
    void OnPaneButton(wxAuiManagerEvent & evt);

    // Don't let panes dock outside the tabs.
    virtual bool ProcessDockResult(wxAuiPaneInfo& target,
                                   const wxAuiPaneInfo& new_pos);

    // Set floating_size and best_size
    wxAuiFloatingFrame* CreateFloatingFrame(wxWindow* parent,
                                            const wxAuiPaneInfo& pane_info);

    void SavePaneSize(wxAuiPaneInfo & pane);

    // Is this pane part of any action?
    bool IsPaneActive(wxAuiPaneInfo & pane);
    bool HasPane(wxAuiDockUIPart * part, wxAuiPaneInfo & pane);
    wxAuiPaneInfo & HitTestPane(int x, int y);

    // Tabs
    MyAuiManagerTabs * m_top;
    MyAuiManagerTabs * m_bottom;
    MyAuiManagerTabs * m_left;
    MyAuiManagerTabs * m_right;
    MyAuiManagerTabs * GetTabs(int direction);
    MyAuiManagerTabs * GetTabs(const wxAuiPaneInfo & pane) { return GetTabs(pane.dock_direction); }
    void AddToTabs(wxAuiPaneInfo & pane, bool hide = true);
    void RemoveFromTabs(wxAuiPaneInfo & pane, bool show = true);
    bool IsInTabs(wxAuiPaneInfo & pane);

    // Utils
    wxAuiDockInfo & FindDock(wxAuiPaneInfo & info);

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

    // Cached pane stuff
    struct CachedPane
    {
        CachedPane(const wxString & _perspective = wxEmptyString,
                   wxAuiManagerDock _tab = wxAUI_DOCK_NONE)
            : perspective(_perspective),
              tab(_tab)
        {}
        wxString perspective;
        wxAuiManagerDock tab;
    };

    std::map<wxString, CachedPane> m_paneCache;
};

#endif // MY_AUI_MGR_H
