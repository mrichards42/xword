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
    if (newSize != m_frameSize)
    {
        ResizeDocks(true);
        m_frameSize = newSize;
        wxAuiManager::Update();
    }
}

// this function is used to sort panes by dock position
static int PaneSortFunc(wxAuiPaneInfo** p1, wxAuiPaneInfo** p2)
{
    return ((*p1)->dock_pos < (*p2)->dock_pos) ? -1 : 1;
}


// Make the grid as large as possible, then fill in the space with other panes.
// If we are resizing the frame, all resizable docks are scaled.
// If we are updating the position of some panes, only clue lists are resized.
void MyAuiManager::ResizeDocks(bool is_frame_resize)
{
    int sash_size = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);
    int border_size = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);

    // Recalc all dock sizes.
    // This happems in LayoutAll, but we can't control dock size from there

    // Remove panes from all docks
    for (size_t i = 0; i < m_docks.Count(); ++i)
        m_docks.Item(i).panes.Empty();
    // Add panes to docks; create docks as needed
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        // Add panes to the dock
        wxAuiDockInfo * dock = &FindDock(pane);
        if (pane.IsShown() && pane.IsDocked())
        {
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
        else if (! pane.IsFixed() && pane.window->IsShown() && ! pane.frame)
        {
            // If the pane is about to be hidden or floated, save the
            // pane size
            wxSize pane_size = pane.rect.GetSize();
            if (pane.HasBorder()) // Add the border
                pane_size += wxSize(2*border_size, 2*border_size);
            // Change best size in the resizable direction
            if (pane.dock_direction == wxAUI_DOCK_BOTTOM || pane.dock_direction == wxAUI_DOCK_TOP)
                pane.best_size.y = pane_size.y;
            else
                pane.best_size.x = pane_size.x;
        }
    }
    // Calc sizes and remove empty docks
    wxSize dock_min_size(0,0); // min size of all docks
    wxSize old_dock_size(0,0); // current size of all docks
    dock_list_t vertical; // Docks to resize
    dock_list_t horizontal;
    for (int i = m_docks.Count()-1; i >= 0; --i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        if (dock.dock_direction == wxAUI_DOCK_CENTER)
            continue;
        bool is_shown = false; // Does this dock have panes that are on screen?
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
            // Remove empty docks
            m_docks.RemoveAt(i);
            continue;
        }
        // Recalc dock size
        // Adapted from LayoutAll
        int size = 0;
        int min_size = 0;
        dock.fixed = true;
        for (size_t j = 0; j < dock.panes.Count(); ++j)
        {
            wxAuiPaneInfo& pane = *dock.panes.Item(j);
            wxSize pane_size = pane.best_size;
            if (pane_size == wxDefaultSize)
                pane_size = pane.min_size;
            if (pane_size == wxDefaultSize)
                pane_size = pane.window->GetSize();

            wxSize pane_min_size = pane.min_size;
            // Use the window's best size as another min_size.
            // NB: This only works after Update(), since window sizes
            // won't get changed until the end of Update().
            // See MyAuiMgr::Update for the workaround.
            wxSize window_best_size = pane.IsFixed() ? pane.window->GetBestSize() : wxDefaultSize;

            if (dock.IsHorizontal())
            {
                size = std::max(pane_size.y, size);
                min_size = std::max(std::max(pane_min_size.y, window_best_size.y), min_size);
            }
            else
            {
                size = std::max(pane_size.x, size);
                min_size = std::max(std::max(pane_min_size.x, window_best_size.x), min_size);
            }
            // Check if this is a fixed dock
            if (! pane.IsFixed())
                dock.fixed = false;
        }
        // Update size of new or fixed docks
        if (dock.size == 0 || dock.fixed)
            dock.size = size;
        dock.min_size = min_size;
        if (dock.size < dock.min_size)
            dock.size = dock.min_size;
        // Adjust pane positions and sizes to fill fixed docks
        if (dock.fixed)
        {
            dock.panes.Sort(PaneSortFunc);
            wxArrayInt pane_positions, pane_sizes;
            GetPanePositionsAndSizes(dock, pane_positions, pane_sizes);
            int pane_count = dock.panes.Count();
            // Find the total size of panes, and the total dock size
            // NB: whereas earlier "size" meant the resizable direction,
            // here it means the opposite.
            int total_pane_size = 0;
            for (int j = 0; j < pane_count; ++j)
                total_pane_size += pane_sizes[j];
            int dock_size = dock.IsHorizontal() ? dock.rect.x : dock.rect.y;
            int pos = 0;
            if (dock_size > 0 && total_pane_size > 0)
            {
                double proportion = (double)dock_size / total_pane_size;
                // Adjust pane positions and size proportionate to the dock size
                for (int j = 0; j < pane_count; ++j)
                {
                    wxAuiPaneInfo& pane = *(dock.panes.Item(j));
                    int pane_size = pane_sizes[j] * proportion;
                    if (j == pane_count - 1) // Last pane gets the rest
                        pane_size = dock_size - pos;
                    if (dock.IsHorizontal())
                        pane.best_size.x = pane_size;
                    else
                        pane.best_size.y = pane_size;
                    pane.dock_pos = pos;
                    pos += pane_size;
                }
            }
            // Fixed docks sizes get reset during wxAuiManager::LayoutAll,
            // but we can trick LayoutAll() by setting dock.fixed = false here.
            // This will not affect any layout operations, since LayoutAll()
            // recalculates dock.fixed.
            dock.fixed = false;
        }
    }

    // Calculate the resizable area.

    wxSize resizable = m_frame->GetClientSize(); // Resizable area
    double frame_proportion_x, frame_proportion_y; // % change in frame size
    if (is_frame_resize)
    {
        wxSize new_size = m_frame->GetClientSize();
        frame_proportion_x = (double)new_size.x / m_frameSize.x;
        frame_proportion_y = (double)new_size.y / m_frameSize.y;
    }
    for (int i = m_docks.Count()-1; i >= 0; --i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        if (dock.dock_direction == wxAUI_DOCK_CENTER)
            continue;
        // Remove sashes from the resizable area.
        if (! dock.fixed)
        {
            if (dock.IsHorizontal())
                resizable.y -= sash_size;
            else
                resizable.x -= sash_size;
        }
        // Only docks with a ClueList are truly resizable.
        if (IsClueListDock(dock))
        {
            if (dock.IsHorizontal())
            {
                old_dock_size.y += dock.size;
                dock_min_size.y += dock.min_size;
                horizontal.push_back(&dock);
            }
            else
            {
                old_dock_size.x += dock.size;
                dock_min_size.x += dock.min_size;
                resizable.x -= sash_size;
                vertical.push_back(&dock);
            }
        }
        else
        {
            // If the frame is being resized, resize docks proportionate to
            // the new frame size.
            if (is_frame_resize)
                dock.size *= dock.IsHorizontal() ? frame_proportion_y : frame_proportion_x;
            // Otherwise, do not resize these docks.
            if (dock.IsHorizontal())
                resizable.y -= dock.size;
            else
                resizable.x -= dock.size;
        }
    }

    // Expand the grid to fill the resizable area.

    // Calculate the new grid size
    wxAuiPaneInfo & grid = GetPane("Grid");
    wxSize grid_size = resizable - dock_min_size;
    // Make the grid square
    int grid_square = std::min(grid_size.x, grid_size.y);
    grid_size.x = grid_square;
    grid_size.y = grid_square;

    // Resize the ClueList docks to take up remaining space.

    wxSize new_dock_size = resizable - grid_size;
    dock_list_t::iterator it;;
    for (it = vertical.begin(); it != vertical.end(); ++it)
    {
        wxAuiDockInfo & dock = **it;
        dock.size = new_dock_size.x / vertical.size();
    }
    for (it = horizontal.begin(); it != horizontal.end(); ++it)
    {
        wxAuiDockInfo & dock = **it;
        dock.size = new_dock_size.y / horizontal.size();
    }
}


void MyAuiManager::Update()
{
    // Save fixed pane sizes
    std::map<wxAuiPaneInfo *, int> sizes;
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        if (pane.IsFixed())
        {
            wxSize best_size = pane.window->GetBestSize();
            bool is_horizontal = pane.dock_direction == wxAUI_DOCK_BOTTOM
                                 || pane.dock_direction == wxAUI_DOCK_TOP;
            sizes[&pane] = is_horizontal ? best_size.y : best_size.x;
        }
    }
    // Run the custom layout engine
    ResizeDocks(false);
    // Update
    wxAuiManager::Update();
    // If sizes have changed, we need to update again to see changes.
    std::map<wxAuiPaneInfo *, int>::iterator it;
    for (it = sizes.begin(); it != sizes.end(); ++it)
    {
        wxAuiPaneInfo & pane = *it->first;
        int last_size = it->second;
        wxSize best_size = pane.window->GetBestSize();
        bool is_horizontal = pane.dock_direction == wxAUI_DOCK_BOTTOM
                             || pane.dock_direction == wxAUI_DOCK_TOP;
        if (last_size != (is_horizontal ? best_size.y : best_size.x))
        {
            Update();
            return;
        }
    }
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