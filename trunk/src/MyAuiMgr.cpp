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
    if (frameSize != m_frameSize)
        ResizeDocks(frameSize, m_frameSize);

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


// ----------------------------------------------------------------------------
// Update functions
// ----------------------------------------------------------------------------

void
MyAuiManager::Update()
{
    // Transfer pane size to pane best_size, so that the user's layout
    // changes are persistent.
    wxAuiPaneInfo & focused_pane = FindPane(wxWindow::FindFocus());
    for (size_t i = 0; i < m_panes.Count(); ++i)
        SavePaneSize(m_panes.Item(i));
    // Force Clue panes to be the same size
    std::list<wxAuiPaneInfo *> horizontal_clue_panes;
    std::list<wxAuiPaneInfo *> vertical_clue_panes;
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        if (pane.name.StartsWith(_T("ClueList")))
        {
            if (pane.dock_direction == wxAUI_DOCK_TOP ||
                pane.dock_direction == wxAUI_DOCK_BOTTOM)
            {
                horizontal_clue_panes.push_back(&pane);
            }
            else
            {
                vertical_clue_panes.push_back(&pane);
            }
        }
    }
    ConstrainPanes(horizontal_clue_panes);
    ConstrainPanes(vertical_clue_panes);

    wxAuiManager::Update();
}


void
MyAuiManager::SavePaneSize(wxAuiPaneInfo & pane)
{
    if (! pane.rect.IsEmpty())
    {
        pane.best_size = pane.rect.GetSize();
        if (! pane.floating_size.IsFullySpecified())
            pane.floating_size = pane.rect.GetSize();
    }
    else if (pane.floating_size.IsFullySpecified())
    {
        pane.best_size = pane.floating_size;
    }
    // Make sure the floating size is at least the min size.
    pane.floating_size.IncTo(pane.window->GetEffectiveMinSize());
}


// Make all panes in the given list the same size, assuming that all panes
// are in docks of the same direction.
// Docks that only contain panes in the list are made the same size.
// Panes in the same dock are given the same proportion.
void
MyAuiManager::ConstrainPanes(std::list<wxAuiPaneInfo *> & panes)
{
    // Map docks to the pane they contain
    typedef std::list<wxAuiPaneInfo *> pane_list_t;
    typedef std::map<wxAuiDockInfo *, pane_list_t> clue_map_t;
    clue_map_t clue_docks;
    // This is the sum of the average size of a pane in the each dock
    // with only clue panes.  Thus, when adjusting the individual dock sizes,
    // we will set dock_size = avg_dock_size * panes_in_this_dock
    int total_dock_size = 0;
    int dock_count = 0;
    {
        pane_list_t::iterator it;
        for (it = panes.begin(); it != panes.end(); ++it)
        {
            wxAuiPaneInfo * pane = *it;
            wxAuiDockInfo & dock = FindDock(*pane);
            pane_list_t & list = clue_docks[&dock];
            list.push_back(pane);
            // If all the panes in this dock are ClueLists, add the
            // size of this dock to the total
            if (list.size() == dock.panes.Count())
            {
                total_dock_size += (dock.size / list.size());
                ++dock_count;
            }
        }
    }

    // Adjust the dock size for docks with only ClueLists
    if (dock_count > 1)
    {
        clue_map_t::iterator it;
        int average_dock_size = total_dock_size / dock_count;
        for (it = clue_docks.begin(); it != clue_docks.end(); ++it)
        {
            if (it->first->panes.Count() == it->second.size())
                it->first->size = average_dock_size * it->second.size();
        }
    }

    // Adjust the proportion of panes in the docks
    clue_map_t::iterator it;
    for (it = clue_docks.begin(); it != clue_docks.end(); ++it)
    {
        const int pane_count = it->second.size();
        if (pane_count < 2)
            continue;
        pane_list_t::iterator pane;
        // Calculate the average proportion
        int total_proportion = 0;
        for (pane = it->second.begin(); pane != it->second.end(); ++pane)
            total_proportion += (*pane)->dock_proportion;
        // Set the average proportion
        int average_proportion = total_proportion / pane_count;
        for (pane = it->second.begin(); pane != it->second.end(); ++pane)
            (*pane)->dock_proportion = average_proportion;
    }
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
// Frame size
// ----------------------------------------------------------------------------

void
MyAuiManager::OnFrameSize(wxSizeEvent & evt)
{
    wxSize newSize = m_frame->GetClientSize();
    if (newSize != m_frameSize)
    {
        ResizeDocks(m_frameSize, newSize);
        m_frameSize = newSize;
        Update();
    }
    evt.Skip();
}

void
MyAuiManager::ResizeDocks(const wxSize & oldSize, const wxSize & newSize)
{
    // Calculate percent difference
    double x = double(newSize.x) / oldSize.x;
    double y = double(newSize.y) / oldSize.y;

    // Go through the clue docks and adjust their sizes
    for (size_t i = 0; i < m_docks.Count(); ++i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        // Look for docks with a clue list in them
        for (size_t j = 0; j < dock.panes.Count(); ++j)
        {
            if (dock.panes.Item(j)->name.StartsWith(_T("ClueList")))
            {
                // Resize the dock
                if (dock.IsVertical())
                    dock.size = floor(dock.size * x + 0.5);
                else
                    dock.size = floor(dock.size * y + 0.5);
                break;
            }
        }
    }
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
        m_action_part = part;
        m_action_start = pos;
        m_action_offset = pos - part->rect.GetTopLeft();
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
    m_last_mouse_move = wxPoint();
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