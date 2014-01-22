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

#include "MyAuiMgr.hpp"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

// ----------------------------------------------------------------
// MyAuiManager
// ----------------------------------------------------------------

typedef std::list<wxAuiPaneInfo *> pane_list_t;
typedef std::list<wxAuiDockInfo *> dock_list_t;

BEGIN_EVENT_TABLE(MyAuiManager, wxAuiManager)
    EVT_AUI_PANE_BUTTON    (MyAuiManager::OnPaneButton)
    EVT_CONTEXT_MENU       (MyAuiManager::OnContextMenu)
    EVT_SIZE               (MyAuiManager::OnFrameSize)
    EVT_MOUSE_CAPTURE_LOST (MyAuiManager::OnCaptureLost)
END_EVENT_TABLE()

MyAuiManager::MyAuiManager(wxWindow* managed_wnd, unsigned int flags)
        : wxAuiManager(managed_wnd, flags),
          m_menu(NULL),
          m_contextPane(NULL),
          m_isEditing(false)
{
}

bool
MyAuiManager::FireCloseEvent(wxAuiPaneInfo & pane)
{
    wxAuiManagerEvent evt(wxEVT_AUI_PANE_CLOSE);
    evt.SetManager(this);
    evt.SetPane(&pane);
    ProcessMgrEvent(evt);
    return ! evt.GetVeto();
}

// ----------------------------------------------------------------------------
// Search functions
// ----------------------------------------------------------------------------

// Find the dock that this pane belongs to
wxAuiDockInfo &
MyAuiManager::FindDock(wxAuiPaneInfo & pane)
{
    for (size_t i = 0; i < m_docks.Count(); ++i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        if (dock.dock_layer == pane.dock_layer &&
            dock.dock_direction == pane.dock_direction &&
            dock.dock_row == pane.dock_row)
        {
            return dock;
        }
    }
    return wxAuiNullDockInfo;
}


// Find the pane that this window belongs to
wxAuiPaneInfo &
MyAuiManager::FindPane(wxWindow * window)
{
    for (;;)
    {
        // We're at the top of the hierarchy.
        if (! window || window == GetManagedWindow())
            return wxAuiNullPaneInfo;
        // Check this window
        wxAuiPaneInfo & pane = GetPane(window);
        if (pane.IsOk())
            return pane;
        // Next parent window
        window = window->GetParent();
    }
}

wxAuiPaneInfo &
MyAuiManager::FindPane(int id)
{
    return FindPane(wxWindow::FindWindowById(id, GetManagedWindow()));
}

wxAuiPaneInfo &
MyAuiManager::GetPaneByCaption(const wxString & caption)
{
    wxASSERT(! caption.IsEmpty());
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        if (pane.caption == caption)
            return pane;
    }
    return wxAuiNullPaneInfo;
}

bool HasCluePane(wxAuiDockInfo & dock)
{
    for (size_t i = 0; i < dock.panes.Count(); ++i)
        if (dock.panes.Item(i)->name.StartsWith("ClueList"))
            return true;
    return false;
}

// ----------------------------------------------------------------------------
// Layout
// ----------------------------------------------------------------------------

void MyAuiManager::OnFrameSize(wxSizeEvent & evt)
{
    evt.Skip();
    wxSize newSize = m_frame->GetClientSize();
    if (newSize != m_frameSize)
    {
        ResizeDocks(newSize - m_frameSize);
        m_frameSize = newSize;
        wxAuiManager::Update();
    }
}


// Make the grid as large as possible, then fill in the space with other panes.
void MyAuiManager::ResizeDocks(const wxSize & size_change)
{
    wxSize window_change = size_change;
    // Add docks
    // This happems in LayoutAll, but we can't control dock size from there
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        // Ignore pane best_size
        pane.best_size = wxDefaultSize;
        // Check for newly shown panes
        if (pane.IsShown() && ! pane.window->IsShown())
        {
            wxAuiDockInfo * dock = &FindDock(pane);
            if (! dock->IsOk())
            {
                // Create the dock
                wxAuiDockInfo d;
                d.dock_direction = pane.dock_direction;
                d.dock_layer = pane.dock_layer;
                d.dock_row = pane.dock_row;
                m_docks.Add(d);
                dock = &m_docks.Last();
            }
            dock->panes.Add(&pane);
        }
    }
    // Remove docks if they are empty
    // Calc min size for new docks
    for (int i = m_docks.Count()-1; i >= 0; --i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        // Does this dock have panes that are on screen?
        bool is_shown = false;
        for (size_t j = 0; j < dock.panes.Count(); ++j)
        {
            if (dock.panes.Item(j)->IsShown())
            {
                is_shown = true;
                break;
            }
        }
        if (! is_shown)
        {
            // If this was a fixed dock, add it to the window size
            if (dock.fixed)
            {
                if (dock.IsHorizontal())
                    window_change.y += dock.size;
                else
                    window_change.x += dock.size;
            }
            // Remove if no panes
            m_docks.RemoveAt(i);
        }
        else if (dock.size == 0)
        {
            // Calc size, min_size, and dock.fixed
            // Adapted from LayoutAll
            int size = 0;
            int min_size = 0;
            dock.fixed = true;
            for (size_t j = 0; j < dock.panes.Count(); ++j)
            {
                wxAuiPaneInfo& pane = *dock.panes.Item(j);
                wxSize pane_size = pane.min_size;
                if (pane_size == wxDefaultSize)
                    pane_size = pane.window->GetSize();

                wxSize pane_min_size = pane.min_size;
                if (pane_min_size == wxDefaultSize)
                    pane_min_size = pane.window->GetMinSize();

                if (dock.IsHorizontal())
                {
                    size = std::max(pane_size.y, size);
                    min_size = std::max(pane_min_size.y, min_size);
                }
                else
                {
                    size = std::max(pane_size.x, size);
                    min_size = std::max(pane_min_size.x, min_size);
                }
                // Check if this is a fixed pane
                if (! pane.IsFixed())
                    dock.fixed = false;
                if (pane.HasFlag(wxAuiPaneInfo::optionDockFixed))
                    dock.fixed = true;
            }
            dock.size = size;
            dock.min_size = min_size;
            // If this is a new fixed dock, subtract it from the window size
            if (dock.fixed)
            {
                if (dock.IsHorizontal())
                    window_change.y -= dock.size;
                else
                    window_change.x -= dock.size;
            }
        }
    }

    // Space calculations
    wxSize min_size(0,0); // min size of all docks
    wxSize dock_size(0,0); // current size of all docks
    dock_list_t vertical; // Docks to resize
    dock_list_t horizontal;
    int vertical_clue_size = 0, n_vertical_clues = 0; // Make clue docks the same size
    int horizontal_clue_size = 0, n_horizontal_clues = 0;
    for (size_t i = 0; i < m_docks.Count(); ++i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        // Fixed docks and the center dock dont't count
        if (dock.fixed || dock.dock_direction == wxAUI_DOCK_CENTER)
            continue;
        // Vertical == LEFT or RIGHT
        // Horizontal == BOTTOM or TOP
        if (dock.IsVertical())
        {
            if (HasCluePane(dock))
            {
                vertical_clue_size += dock.size;
                ++n_vertical_clues;
            }
            vertical.push_back(&dock);
            min_size.x += dock.min_size;
            dock_size.x += dock.size;
        }
        else
        {
            if (HasCluePane(dock))
            {
                horizontal_clue_size += dock.size;
                ++n_horizontal_clues;
            }
            horizontal.push_back(&dock);
            min_size.y += dock.min_size;
            dock_size.y += dock.size;
        }
    }
    // Average clue sizes
    if (n_horizontal_clues > 0)
        horizontal_clue_size /= n_horizontal_clues;
    if (n_vertical_clues > 0)
        vertical_clue_size /= n_vertical_clues;
    wxSize resizable = dock_size - min_size; // How much can the layout change?

    // Calculate the new grid size
    wxAuiPaneInfo & grid = GetPane("Grid");
    wxSize grid_size = grid.rect.GetSize() + window_change + resizable;
    // Make the grid square
    int grid_square = std::min(grid_size.x, grid_size.y);
    grid_size.x = grid_square;
    grid_size.y = grid_square;

    // Resize docks
    // Amount of space we need to adjust for the grid
    wxSize change = grid.rect.GetSize() - grid_size + window_change;
    // Change per dock in each direction
    int dx = vertical.size() > 0 ? change.x / (int)vertical.size() : 0;
    int dy = horizontal.size() > 0 ? change.y / (int)horizontal.size() : 0;
    // Check min size of each dock.
    // If we can't resize a given pane, set it to the min size and remove
    // the dock from our calculations.
    dock_list_t::iterator it = vertical.begin();
    while (it != vertical.end())
    {
        wxAuiDockInfo & dock = **it;
        if (HasCluePane(dock))
            dock.size = vertical_clue_size;
        if (dock.size + dx < dock.min_size) {
            change.x -= (dock.size - dock.min_size);
            dock.size = dock.min_size;
            it = vertical.erase(it);
        }
        else
            ++it;
    }
    it = horizontal.begin();
    while (it != horizontal.end())
    {
        wxAuiDockInfo & dock = **it;
        if (HasCluePane(dock))
            dock.size = horizontal_clue_size;
        if (dock.size + dy < dock.min_size) {
            change.y -= (dock.size - dock.min_size);
            dock.size = dock.min_size;
            it = horizontal.erase(it);
        }
        else
            ++it;
    }
    // Recalculate and resize docks
    dx = vertical.size() > 0 ? change.x / (int)vertical.size() : 0;
    dy = horizontal.size() > 0 ? change.y / (int)horizontal.size() : 0;
    for (it = vertical.begin(); it != vertical.end(); ++it)
        (*it)->size += dx;
    for (it = horizontal.begin(); it != horizontal.end(); ++it)
        (*it)->size += dy;
}


void MyAuiManager::Update()
{
    ResizeDocks(wxSize(0,0));
    wxAuiManager::Update();
}


#include <wx/tokenzr.h>

// ----------------------------------------------------------------------------
// Perspectives
// ----------------------------------------------------------------------------

// Cache pane info strings and tab layouts if the expected pane
// isn't already part of the layout.  AddPane will check our
// cache and use the cached info string instead of the supplied
// wxAuiPaneInfo object, and add the pane to its tab if the cache exists.

wxString
MyAuiManager::SavePerspective()
{
    // This section is almost the same as wxAuiManager::SavePerspective()
    wxString result;
    result.Alloc(500);
    result = wxT("layout2|");

    int pane_i, pane_count = m_panes.GetCount();
    for (pane_i = 0; pane_i < pane_count; ++pane_i)
    {
        wxAuiPaneInfo& pane = m_panes.Item(pane_i);
        if (IsEditing())
        {
            // Find the pane in the edit cache
            std::map<wxString, wxAuiPaneInfo>::iterator it
                = m_editCache.find(pane.name);
            if (it != m_editCache.end())
            {
                // Restore old settings
                wxAuiPaneInfo & oldPane = it->second;
                pane.Resizable(oldPane.IsResizable());
                pane.PaneBorder(oldPane.HasBorder());
            }
        }
        result += SavePaneInfo(pane)+wxT("|");
    }

    // Save the cached panes
    std::map<wxString, wxString>::iterator it;
    for (it = m_paneCache.begin(); it != m_paneCache.end(); ++it)
        result += it->second + wxT("|");

    int dock_i, dock_count = m_docks.GetCount();
    for (dock_i = 0; dock_i < dock_count; ++dock_i)
    {
        wxAuiDockInfo& dock = m_docks.Item(dock_i);

        result += wxString::Format(wxT("dock_size(%d,%d,%d)=%d|"),
                                   dock.dock_direction, dock.dock_layer,
                                   dock.dock_row, dock.size);
    }

    // Save the frame size
    result += wxString::Format(_T("frame_size=%d,%d|"),
                               m_frameSize.x, m_frameSize.y);

    return result;
}

bool
MyAuiManager::LoadPerspective(const wxString & layout, bool update)
{
    wxString escaped_layout = layout;
    // Escape
    escaped_layout.Replace(_T("\\|"), _T("\a"));
    escaped_layout.Replace(_T("\\;"), _T("\b"));

    // Get rid of frame_size section (and old "private" panes)
    wxString processed_layout;
    processed_layout.reserve(layout.size());
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

    wxSize frameSize = m_frame->GetClientSize();

    if (! wxAuiManager::LoadPerspective(processed_layout, false))
        return false;

    // Cache panes that are not currently present for later, and process frame_size
    m_paneCache.clear();
    wxStringTokenizer tok(escaped_layout, _T("|"), wxTOKEN_STRTOK);
    // Skip the first token, it's just the layout format "layout2"
    tok.GetNextToken();
    while (tok.HasMoreTokens())
    {
        wxString info_str = tok.GetNextToken();
        if (info_str.StartsWith(_T("dock_size")))
        {
            // Nothing to do here
        }
        else if (info_str.StartsWith(_T("frame_size")))
        {
            // Adjust the dock proportion for the current window size.
            long x, y;
            info_str = info_str.AfterFirst(_T('='));
            if (info_str.BeforeFirst(_T(',')).ToLong(&x)
                && info_str.AfterFirst(_T(',')).ToLong(&y))
            {
                frameSize = wxSize(x, y);
            }
        }
        else if (info_str.StartsWith(_T("__"))) // Old tabs
        {
            // Do nothing
        }
        else
        {
            // This is a pane info string
            processed_layout += info_str + _T("|");

            // Unescape
            info_str.Replace(wxT("\a"), wxT("|"));
            info_str.Replace(wxT("\b"), wxT(";"));
            // Parse the pane info string
            wxAuiPaneInfo pane;
            LoadPaneInfo(info_str, pane);
            // Cache the string if we don't have a pane with this name.
            if (! GetPane(pane.name).IsOk())
                m_paneCache[pane.name] = info_str;
        }
    }

    m_frameSize = m_frame->GetClientSize();

    UpdateMenu();
    // Reset the edit status to reflect change in borders, etc.
    if (IsEditing())
    {
        EndEdit();
        StartEdit();
    }
    if (update)
        Update();
    return true;
}

// ----------------------------------------------------------------------------
// Pane Cache
// ----------------------------------------------------------------------------

bool
MyAuiManager::GetCachedPane(const wxString & name, wxAuiPaneInfo & pane)
{
    // Look for our pane in m_paneCache
    std::map<wxString, wxString>::iterator it = m_paneCache.find(name);
    if (it != m_paneCache.end())
    {
        const wxString & perspective = it->second;
        // Load the cached pane
        if (! perspective.IsEmpty())
        {
            LoadPaneInfo(perspective, pane);
            return pane.name == name;
        }
    }
    return false;
}

bool
MyAuiManager::HasCachedPane(const wxString & name)
{
    wxAuiPaneInfo pane;
    if (! GetCachedPane(name, pane))
        return false;
    return pane.IsShown();
}


// Add and Deatach panes
//----------------------

// AddPane checks the pane cache and loads whatever it finds in the cache.
// AddPane also adds the pane to our managed menu.
// DetachPane removes the context window from the pane, removes the pane
// from our managed menu, and adds the pane to the cache.

bool
MyAuiManager::AddPane(wxWindow * window, const wxAuiPaneInfo & pane_info)
{
    // See if we have this pane cached already
    wxAuiPaneInfo cached_pane;
    if (GetCachedPane(pane_info.name, cached_pane))
    {
        // Copy over the caption.
        if (! pane_info.caption.IsEmpty())
            cached_pane.caption = pane_info.caption;
        // Try the cached pane, then the supplied pane if that
        // doesn't work.
        if (! wxAuiManager::AddPane(window, cached_pane))
            if (! wxAuiManager::AddPane(window, pane_info))
                return false;
        m_paneCache.erase(pane_info.name);
    }
    else // No cached pane
    {
        if (! wxAuiManager::AddPane(window, pane_info))
            return false;
    }
    // Add this pane to the menu
    if (m_menu)
        AddToMenu(GetPane(window));
    return true;
}


bool
MyAuiManager::DetachPane(wxWindow * window)
{
    // Remove this window from our menu, and remove the context handler
    wxAuiPaneInfo & pane = GetPane(window);
    RemoveContextWindow(pane);
    if (m_menu)
        RemoveFromMenu(pane);
    // Add this to the cache
    m_paneCache[pane.name] = SavePaneInfo(pane);
    // Remove the pane
    if (! wxAuiManager::DetachPane(window))
        return false;
    return true;
}


// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------

void
MyAuiManager::OnPaneButton(wxAuiManagerEvent & evt)
{
    wxAuiPaneInfo & pane = *evt.GetPane();
    switch (evt.GetButton()) {
        case wxAUI_BUTTON_CLOSE:
            if (FireCloseEvent(pane))
            {
                if (GetPane(pane.window).IsOk())
                    ClosePane(pane);
                Update();
            }
            // Don't skip
            break;

        default:
            evt.Skip();
            break;
    }
}


// ----------------------------------------------------------------------------
// Menu
// ----------------------------------------------------------------------------

// The pane menu is set with SetManagedMenu, and is altered via
// AddPane and DetachPane.  The user can update the menu state using
// UpdateMenu().
// It is a list of check menu items that show the shown/hidden state
// of each pane.

void
MyAuiManager::UpdateMenu()
{
    if (! m_menu)
        return;
    // Destroy all menu items
    while (m_menu->GetMenuItemCount() > 0)
        RemoveFromMenu(m_menu->FindItemByPosition(0)->GetId());
    // Add menu items
    for (size_t i = 0; i < m_panes.size(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        AddToMenu(pane);
    }
}

void
MyAuiManager::AddToMenu(wxAuiPaneInfo & pane)
{
    wxASSERT(m_menu);
    wxMenuItem * item = m_menu->AppendCheckItem(wxID_ANY, pane.caption);
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyAuiManager::OnMenu));
    Connect(item->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MyAuiManager::OnUpdateUI));
    item->Check(pane.IsShown());
}

void
MyAuiManager::RemoveFromMenu(wxAuiPaneInfo & pane)
{
    RemoveFromMenu(m_menu->FindItem(pane.caption));
}

void
MyAuiManager::RemoveFromMenu(int id)
{
    wxASSERT(m_menu->FindItem(id));
    wxASSERT(m_menu);
    Disconnect(id, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyAuiManager::OnMenu));
    Disconnect(id, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MyAuiManager::OnUpdateUI));
    m_menu->Destroy(id);
}

void
MyAuiManager::OnMenu(wxCommandEvent & evt)
{
    wxMenuItem * item = m_menu->FindItem(evt.GetId());
    wxCHECK_RET(item, _T("Missing wxMenuItem"));
    wxAuiPaneInfo & pane = GetPaneByCaption(item->GetItemLabel());
    wxCHECK_RET(pane.IsOk(), _T("Missing wxAuiPaneInfo"));

    pane.Show(evt.IsChecked());
    Update();
}

void 
MyAuiManager::OnUpdateUI(wxUpdateUIEvent & evt)
{
    wxMenuItem * item = m_menu->FindItem(evt.GetId());
    wxCHECK_RET(item, _T("Missing wxMenuItem"));
    wxAuiPaneInfo & pane = GetPaneByCaption(item->GetItemLabel());
    wxCHECK_RET(pane.IsOk(), _T("Missing wxAuiPaneInfo"));

    evt.Enable(pane.HasCloseButton());
    evt.Check(pane.IsShown());
}


// ----------------------------------------------------------------------------
// Context menu
// ----------------------------------------------------------------------------

// Set a window that recieves the context menu event.  Pane captions always
// get a context menu event.
void
MyAuiManager::SetContextWindow(wxAuiPaneInfo & info, wxWindow * window)
{
    wxASSERT(info.IsOk() && GetPane(info.window).IsOk());
    if (! window)
        RemoveContextWindow(info);
    else
        m_contextWindows[window] = &info;
}

void
MyAuiManager::RemoveContextWindow(wxAuiPaneInfo & info)
{
    wxASSERT(info.IsOk() && GetPane(info.window).IsOk());
    std::map<wxWindow *, wxAuiPaneInfo *>::iterator it;
    for (it = m_contextWindows.begin(); it != m_contextWindows.end(); ++it)
    {
        if (it->second->window == info.window)
        {
            m_contextWindows.erase(it);
            return;
        }
    }
}

void
MyAuiManager::RemoveContextWindow(wxWindow * window)
{
    m_contextWindows.erase(window);
}


// The context menu displays the pane caption and a list of items:
//   * Float
//   * Dock
//   -------
//   X Resizable
//   -------
//   X Border
//   -------
//     Close
//   -------

enum MyAuiManagerContextId
{
    ID_AUI_CONTEXT_FLOATING = wxID_HIGHEST,
    ID_AUI_CONTEXT_DOCKED,
    ID_AUI_CONTEXT_RESIZABLE,
    ID_AUI_CONTEXT_BORDER,
    ID_AUI_CONTEXT_CLOSE
};

wxMenu *
MyAuiManager::NewContextMenu(wxAuiPaneInfo & pane)
{
    wxMenu * menu = new wxMenu(pane.caption);
    wxMenuItem * item;

    item = menu->AppendRadioItem(ID_AUI_CONTEXT_FLOATING, _T("Floating"));
    item->Check(pane.IsFloating());
    item->Enable(pane.IsFloatable());

    item = menu->AppendRadioItem(ID_AUI_CONTEXT_DOCKED, _T("Docked"));
    item->Check(pane.IsDocked());
    item->Enable(pane.HasFlag(wxAuiPaneInfo::optionLeftDockable |
                              wxAuiPaneInfo::optionRightDockable |
                              wxAuiPaneInfo::optionTopDockable |
                              wxAuiPaneInfo::optionBottomDockable));

    menu->AppendSeparator();

    item = menu->AppendCheckItem(ID_AUI_CONTEXT_RESIZABLE, _T("Resizable"));
    if (! IsEditing())
        item->Check(pane.IsResizable());
    else
        item->Check(m_editCache[pane.name].IsResizable());
    item->Enable(pane.IsDocked());

    menu->AppendSeparator();

    item = menu->AppendCheckItem(ID_AUI_CONTEXT_BORDER, _T("Border"));
    if (! IsEditing())
        item->Check(pane.HasBorder());
    else
        item->Check(m_editCache[pane.name].HasBorder());

    menu->AppendSeparator();

    item = menu->Append(ID_AUI_CONTEXT_CLOSE, _T("Close"));
    item->Enable(pane.HasCloseButton());

    return menu;
}

// Show a context menu for this pane, process the event, and delete the menu.
void
MyAuiManager::ShowContextMenu(wxAuiPaneInfo & pane)
{
    wxASSERT(pane.IsOk());
    wxMenu * menu = NewContextMenu(pane);
    m_contextPane = &pane;
    pane.window->Connect(wxEVT_COMMAND_MENU_SELECTED,
                         wxCommandEventHandler(MyAuiManager::OnContextMenuClick),
                         NULL, this);
    pane.window->PopupMenu(menu);
    pane.window->Disconnect(wxEVT_COMMAND_MENU_SELECTED,
                            wxCommandEventHandler(MyAuiManager::OnContextMenuClick),
                            NULL, this);
    m_contextPane = NULL;
    delete menu;
}


void
MyAuiManager::OnContextMenuClick(wxCommandEvent & evt)
{
    wxCHECK_RET(m_contextPane, _T("Context menu needs an AUI Pane"));
    wxAuiPaneInfo & pane = *m_contextPane;

    switch (evt.GetId()) {
        case ID_AUI_CONTEXT_FLOATING:
            pane.Float();
            Update();
            break;

        case ID_AUI_CONTEXT_DOCKED:
            pane.Dock();
            Update();
            break;

        case ID_AUI_CONTEXT_RESIZABLE:
            if (! IsEditing())
                pane.Resizable(evt.IsChecked());
            else
                m_editCache[pane.name].Resizable(evt.IsChecked());
            Update();
            break;

        case ID_AUI_CONTEXT_BORDER:
            if (! IsEditing())
                pane.PaneBorder(evt.IsChecked());
            else
                m_editCache[pane.name].PaneBorder(evt.IsChecked());
            Update();
            break;

        case ID_AUI_CONTEXT_CLOSE:
            if (FireCloseEvent(pane))
            {
                // close the pane, but check that it
                // still exists in our pane array first
                // (the event handler above might have removed it)
                if (GetPane(pane.window).IsOk())
                    ClosePane(pane);
                Update();
            }
            break;

        default:
            // The "title" menu item was clicked.
            break;
    }
}


// Determine if we should show a context menu

void
MyAuiManager::OnFloatingContextMenu(wxContextMenuEvent &evt)
{
    // Find the frame that this window belongs to
    wxWindow * window = wxDynamicCast(evt.GetEventObject(), wxWindow);
    wxCHECK_RET(window, _T("OnFloatingContextMenu has no window"));
    wxFrame * frame = wxDynamicCast(wxGetTopLevelParent(window), wxFrame);
    wxCHECK_RET(frame, _T("OnFloatingContextMenu has no frame"));

    // The user clicked on part of the floating frame (the caption)
    if (window == wxDynamicCast(frame, wxWindow))
    {
        // Find the wxAuiPaneInfo inside the frame
        for (size_t i = 0; i < m_panes.Count(); ++i)
        {
            wxAuiPaneInfo & pane = m_panes.Item(i);
            if (pane.frame == frame)
                ShowContextMenu(pane);
        }
    }
    // The user clicked the pane itself
    else
    {
        // This will pass through to OnContextMenu.
        evt.Skip();
    }
}

void
MyAuiManager::OnContextMenu(wxContextMenuEvent &evt)
{
    wxASSERT(GetManagedWindow());

    wxWindow * window = wxDynamicCast(evt.GetEventObject(), wxWindow);
    wxCHECK_RET(window, _T("OnFloatingContextMenu has no window"));

    if (window == GetManagedWindow())
    {
        // The user clicked on a pane sizer or caption, etc.
        // Figure out which pane this belongs to.
        wxPoint pos = evt.GetPosition();
        if (pos == wxDefaultPosition)
            pos = wxGetMousePosition();
        pos = window->ScreenToClient(pos);
        wxAuiPaneInfo & pane = HitTestPane(pos.x, pos.y);
        if (pane.IsOk())
            ShowContextMenu(pane);
        else
            evt.Skip();
    }
    else
    {
        // Walk the window hierarchy until we get to a window that
        // has been supplied through SetContextWindow.
        for (;;)
        {
            // Top of the window hierarchy
            if (! window)
            {
                evt.Skip();
                return;
            }
            // The window we've been looking for
            if (m_contextWindows.find(window) != m_contextWindows.end())
                break;
            window = window->GetParent();
        }
        wxAuiPaneInfo & pane = *m_contextWindows[window];
        if (pane.IsOk())
            ShowContextMenu(pane);
        else
            evt.Skip();
    }
}

// Add a context menu to the floating pane
wxAuiFloatingFrame *
MyAuiManager::CreateFloatingFrame(wxWindow* parent, const wxAuiPaneInfo & pane)
{
    // Create the frame
    wxAuiFloatingFrame * frame = wxAuiManager::CreateFloatingFrame(parent, pane);
    // Attach a ContextMenu event
    frame->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(MyAuiManager::OnFloatingContextMenu), NULL, this);
    return frame;
}


// ----------------------------------------------------------------------------
// Pane state and position
// ----------------------------------------------------------------------------

wxAuiPaneInfo &
MyAuiManager::HitTestPane(int x, int y)
{
    wxAuiDockUIPart * part = HitTest(x, y);
    if (part && part->pane)
        return *(part->pane);
    return wxAuiNullPaneInfo;
}

// ----------------------------------------------------------------------------
// Edit Mode
// ----------------------------------------------------------------------------

void ConnectRecursive(wxWindow * w,
    wxEventType eventType, wxObjectEventFunction function,
    wxObject * userData = NULL, wxEvtHandler * eventSink = NULL)
{
    w->Connect(eventType, function, userData, eventSink);
    wxWindowList & children = w->GetChildren();
    for (wxWindowList::iterator c = children.begin(); c != children.end(); ++c)
    {
        ConnectRecursive(*c, eventType, function, userData, eventSink);
    }
}

void DisconnectRecursive(wxWindow * w,
    wxEventType eventType, wxObjectEventFunction function,
    wxObject * userData = NULL, wxEvtHandler * eventSink = NULL)
{
    w->Disconnect(eventType, function, userData, eventSink);
    wxWindowList & children = w->GetChildren();
    for (wxWindowList::iterator c = children.begin(); c != children.end(); ++c)
    {
        DisconnectRecursive(*c, eventType, function, userData, eventSink);
    }
}

void
MyAuiManager::StartEdit()
{
    m_isEditing = true;
    // Save the pane states before edit.
    m_editCache.clear();
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        m_editCache[pane.name] = pane; // Cache the old wxAuiPaneInfos
        pane.Resizable(true);          // Make all panes resizable
        pane.PaneBorder(true);         // Show borders
    }
    // Connect mouse events to the panes
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        ConnectRecursive(m_panes.Item(i).window, wxEVT_LEFT_DOWN,
            wxMouseEventHandler(MyAuiManager::OnLeftDown),  NULL, this);
        ConnectRecursive(m_panes.Item(i).window, wxEVT_SET_CURSOR,
            wxSetCursorEventHandler(MyAuiManager::OnSetCursor),  NULL, this);
        ConnectRecursive(m_panes.Item(i).window, wxEVT_CONTEXT_MENU,
            wxContextMenuEventHandler(MyAuiManager::OnEditContextMenu),  NULL, this);
    }
}

void
MyAuiManager::EndEdit()
{
    m_isEditing = false;
    if (m_frame->HasCapture())
        m_frame->ReleaseMouse();
    // Restore the pane states after edit.
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        // Find the pane in the cache
        std::map<wxString, wxAuiPaneInfo>::iterator it
            = m_editCache.find(pane.name);
        if (it != m_editCache.end())
        {
            // Restore old settings
            wxAuiPaneInfo & oldPane = it->second;
            pane.Resizable(oldPane.IsResizable());
            pane.PaneBorder(oldPane.HasBorder());
        }
    }
    m_editCache.clear();
    // Disconnect mouse events
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        DisconnectRecursive(m_panes.Item(i).window, wxEVT_LEFT_DOWN,
            wxMouseEventHandler(MyAuiManager::OnLeftDown),  NULL, this);
        DisconnectRecursive(m_panes.Item(i).window, wxEVT_SET_CURSOR,
            wxSetCursorEventHandler(MyAuiManager::OnSetCursor),  NULL, this);
        DisconnectRecursive(m_panes.Item(i).window, wxEVT_CONTEXT_MENU,
            wxContextMenuEventHandler(MyAuiManager::OnEditContextMenu),  NULL, this);
    }
}

void
MyAuiManager::OnLeftDown(wxMouseEvent & evt)
{
    wxPoint pos = m_frame->ScreenToClient(wxDynamicCast(evt.GetEventObject(), wxWindow)->ClientToScreen(evt.GetPosition()));
    wxAuiDockUIPart * part = HitTest(pos.x, pos.y);
    if (part && part->pane && ! part->pane->IsFloating() && part->pane->IsFloatable())
    {
        // Start dragging the pane
        m_action = actionClickCaption;
        m_actionPart = part;
        m_actionStart = pos;
        m_actionOffset = pos - part->rect.GetTopLeft();
        if (! m_frame->HasCapture())
            m_frame->CaptureMouse();
    }
    else
    {
        evt.Skip();
    }
}

void
MyAuiManager::OnEditContextMenu(wxContextMenuEvent & evt)
{
    wxAuiPaneInfo & pane = FindPane(wxDynamicCast(evt.GetEventObject(), wxWindow));
    if (pane.IsOk())
        ShowContextMenu(pane);
    else
        evt.Skip();
}


void
MyAuiManager::OnCaptureLost(wxMouseCaptureLostEvent & evt)
{
    if (m_frame->HasCapture())
        m_frame->ReleaseMouse();
    // Reset the action variables
    m_action = actionNone;
    m_lastMouseMove = wxPoint();
    EndEdit();
}

void
MyAuiManager::OnSetCursor(wxSetCursorEvent & evt)
{
    wxAuiPaneInfo & pane = FindPane(wxDynamicCast(evt.GetEventObject(), wxWindow));
    if (! pane.IsFloating() && pane.IsFloatable())
        evt.SetCursor(wxCursor(wxCURSOR_SIZING));
    else
        evt.SetCursor(wxNullCursor);
}