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
    EVT_SIZE               (MyAuiManager::OnFrameSize)
    EVT_MOUSE_CAPTURE_LOST (MyAuiManager::OnCaptureLost)
END_EVENT_TABLE()

MyAuiManager::MyAuiManager(wxWindow* managed_wnd, unsigned int flags)
        : wxAuiManager(managed_wnd, flags),
          m_menu(NULL),
          m_isEditing(false),
          m_frameSize(-1,-1)
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

bool IsClueListDock(const wxAuiDockInfo & dock)
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
    if (! m_frameSize.IsFullySpecified())
        m_frameSize = newSize;
    else if (newSize != m_frameSize)
    {
        m_frameSize = newSize;
        wxAuiManager::Update();
    }
}


// Force ClueList docks in the same direction to be the same size
void MyAuiManager::ConstrainClueDocks()
{
    std::map<int, dock_list_t> clue_docks;
    for (int i = m_docks.Count()-1; i >= 0; --i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        if (IsClueListDock(dock))
            clue_docks[dock.dock_direction].push_back(&dock);
    }
    std::map<int, dock_list_t>::iterator it;
    for (it = clue_docks.begin(); it != clue_docks.end(); ++it)
    {
        dock_list_t & docks = it->second;

        // Sum the total dock size
        dock_list_t::iterator dock_it;
        int dock_size = 0;
        for (dock_it = docks.begin(); dock_it != docks.end(); ++dock_it)
        {
            dock_size += (**dock_it).size;
        }

        // Set each dock size to the average
        int dock_count = docks.size();
        int avg_dock_size = dock_size / dock_count;

        for (dock_it = docks.begin(); dock_it != docks.end(); ++dock_it)
        {
            (**dock_it).size = avg_dock_size;
        }
    }
}


void MyAuiManager::Update()
{
    ConstrainClueDocks();
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
            // Nothing here
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
// DetachPane removes the pane from our managed menu, and adds the pane to
// the cache.

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
    // Remove this window from our menu
    wxAuiPaneInfo & pane = GetPane(window);
    if (m_menu)
        RemoveFromMenu(pane);
    // Add this to the cache
    m_paneCache[pane.name] = SavePaneInfo(pane);
    // Remove the pane
    return wxAuiManager::DetachPane(window);
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
        pane.PaneBorder(true);         // Show borders
        pane.Dockable(pane.dock_direction != wxAUI_DOCK_CENTER);
        pane.Floatable(pane.dock_direction != wxAUI_DOCK_CENTER);
    }
    // Connect mouse events to the panes
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        ConnectRecursive(m_panes.Item(i).window, wxEVT_LEFT_DOWN,
            wxMouseEventHandler(MyAuiManager::OnLeftDown),  NULL, this);
        ConnectRecursive(m_panes.Item(i).window, wxEVT_SET_CURSOR,
            wxSetCursorEventHandler(MyAuiManager::OnSetCursor),  NULL, this);
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
        pane.Dockable(false);
        pane.Floatable(false);
        // Find the pane in the cache
        std::map<wxString, wxAuiPaneInfo>::iterator it
            = m_editCache.find(pane.name);
        if (it != m_editCache.end())
        {
            // Restore old settings
            wxAuiPaneInfo & oldPane = it->second;
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
