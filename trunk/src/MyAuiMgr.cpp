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
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

// "private" panes start with a double-underscore
inline bool IsPrivatePane(const wxAuiPaneInfo & pane)
{
    return pane.name.StartsWith(_T("__"));
}

// ----------------------------------------------------------------
// MyAuiManagerTabs
// ----------------------------------------------------------------
// A pane with notebook tabs representing hidden panes.
// There should be one set of tabs for each side of the window.


class MyAuiManagerTabs : public wxControl
{
    friend class MyAuiManager;
public:
    static const int dock_layer = 1000;
    static const int height_padding = 2;

    // Timer ids
    static const int ID_PANE_TIMEOUT = 101;

    MyAuiManagerTabs(MyAuiManager * mgr, wxDirection direction)
        : wxControl(mgr->GetManagedWindow(), wxID_ANY,
                    wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
          m_mgr(mgr),
          m_direction(direction),
          m_padding(5,5),
          m_previewPane(-1),
          m_activeTab(-1),
          m_isPreviewFocused(false),
          m_timer(this, ID_PANE_TIMEOUT)
    {
        Connect(wxEVT_PAINT, wxPaintEventHandler(MyAuiManagerTabs::OnPaint));
        Connect(wxEVT_MOTION, wxMouseEventHandler(MyAuiManagerTabs::OnMouseMove));
        Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(MyAuiManagerTabs::OnMouseOut));
        Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(MyAuiManagerTabs::OnLeftDown));
        Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(MyAuiManagerTabs::OnLeftDblClick));
        Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(MyAuiManagerTabs::OnContextMenu));
        Connect(ID_PANE_TIMEOUT, wxEVT_TIMER, wxTimerEventHandler(MyAuiManagerTabs::OnPaneTimeout));

        // A little hackish
        if (IsHorizontal())
            SetInitialSize(wxSize(-1, 17 + height_padding + GetPadding().y));
        else
            SetInitialSize(wxSize(17 + height_padding + GetPadding().y, -1));
    }

    // Add and remove panes
    //---------------------
    void AddPane(wxAuiPaneInfo & pane)
    {
        m_panes.push_back(&pane);
        m_rects.push_back(wxRect());
        m_mgr->GetPane(this).Show();
        // Force a layout
        InvalidateTabRects();
        Refresh();
    }

    bool RemovePane(wxAuiPaneInfo & pane)
    {
        return RemovePane(pane.name);
    }

    bool RemovePane(const wxString & name)
    {
        int i = FindPane(name);
        if (i == -1)
            return false;
        return RemovePane(i);
    }

    bool RemovePane(int i)
    {
        wxCHECK_MSG(i >= 0 && i < m_panes.size(), false,
                    _T("RemovePane index out of bounds"));
        // Adjust our preview pane / active tab
        if (m_activeTab == i)
            m_activeTab = -1;
        else if (m_activeTab > i)
            --m_activeTab;
        if (m_previewPane == i)
        {
            // HidePreviewPane is going to Hide this pane,
            // so save the current state.
            wxAuiPaneInfo & pane = GetPane(i);
            bool isShown = pane.IsShown();
            HidePreviewPane();
            pane.Show(isShown);
        }
        else if (m_previewPane > i)
            --m_previewPane;
        // Remove the pane
        m_panes.erase(m_panes.begin() + i);
        m_rects.erase(m_rects.begin() + i);
        // Hide our own pane if we are out of tabs
        if (m_panes.empty())
            m_mgr->GetPane(this).Hide();
        // Force a layout
        InvalidateTabRects();
        Refresh();
        return true;
    }

    void ClearPanes()
    {
        m_activeTab = -1;
        HidePreviewPane();
        m_panes.clear();
        m_rects.clear();
        m_mgr->GetPane(this).Hide();
        // Force a layout
        InvalidateTabRects();
        Refresh();
    }

    // Return the index of this pane, or -1 if not found
    int FindPane(const wxAuiPaneInfo & pane)
    {
        return FindPane(pane.name);
    }

    int FindPane(const wxString & name)
    {
        const size_t count = m_panes.size();
        for (size_t i = 0; i < count; ++i)
            if (GetPane(i).name == name)
                return i;
        return -1;
    }

    bool HasPane(const wxAuiPaneInfo & pane)
    {
        return FindPane(pane) != -1;
    }

    // Return a reference the pane (panes are stored as pointers).
    wxAuiPaneInfo & GetPane(size_t i)
    {
        wxASSERT_MSG(i >= 0 && i < m_panes.size(),
                     _T("GetPane index out of bounds"));
        return *m_panes.at(i);
    }

    wxAuiPaneInfo & GetPane(const wxString & name)
    {
        int i = FindPane(name);
        if (i == -1)
            return wxAuiNullPaneInfo;
        return GetPane(i);
    }


    bool IsHorizontal() const { return ((wxTOP  | wxBOTTOM) & m_direction) != 0; }
    bool IsVertical()   const { return ((wxLEFT | wxRIGHT)  & m_direction) != 0; }

    const wxSize & GetPadding() const { return m_padding; }
    void SetPadding(const wxSize & padding) { m_padding = padding; }

protected:
    MyAuiManager * m_mgr;
    std::vector<wxAuiPaneInfo *> m_panes;
    std::vector<wxRect> m_rects;
    wxDirection m_direction;
    wxSize m_padding;

    int m_activeTab;
    wxTimer m_timer;
    int m_previewPane;
    bool m_isPreviewFocused;

    virtual wxBorder GetDefaultBorder() const { return wxBORDER_NONE; }

    wxAuiDockArt * GetArtProvider() { return m_mgr->GetArtProvider(); }

    // Tab size functions
    // ------------------

    wxPoint GetFirstTabPosition()
    {
        wxClientDC dc(this);
        dc.SetFont(GetArtProvider()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
        int height = dc.GetCharHeight();
        height += 2 * GetPadding().y;
        if (IsHorizontal())
            return wxPoint(height + height_padding, // Offset from corner
                           m_direction == wxTOP ? 0 : GetClientSize().y - height);
        else
            return wxPoint(m_direction == wxLEFT ? 0 : GetClientSize().x - height,
                           height_padding); // Offset from corner
    }

    void CalculateTabRects(wxDC & dc)
    {
        if (HasTabRects())
            return;
        DoCalculateTabRects(dc);
    }

    void CalculateTabRects()
    {
        if (HasTabRects())
            return;
        wxClientDC dc(this);
        DoCalculateTabRects(dc);
    }

    // Calculate where all the tabs should be
    virtual void DoCalculateTabRects(wxDC & dc)
    {
        wxAuiDockArt * art = GetArtProvider();
        dc.SetFont(art->GetFont(wxAUI_DOCKART_CAPTION_FONT));

        wxPoint pos = GetFirstTabPosition();
        for (size_t i = 0; i < m_panes.size(); ++i)
        {
            // Calculate the tab size assuming the layout is horizontal.
            // We'll swap the size if it's not horizontal.
            wxAuiPaneInfo & pane = GetPane(i);

            // Text
            wxSize size = dc.GetTextExtent(pane.caption);

            size += GetPadding() * 2;

            if (IsVertical())
                size = wxSize(size.y, size.x);

            m_rects.at(i).SetPosition(pos);
            m_rects.at(i).SetSize(size);

            // Next tab; increment the position
            if (IsHorizontal())
                pos.x += size.x;
            else
                pos.y += size.y;
        }
    }

    bool HasTabRects() const
    {
        return ! m_rects.empty() && ! m_rects.at(0).IsEmpty();
    }
    void InvalidateTabRects()
    {
        if (! m_rects.empty())
            m_rects.at(0).SetSize(wxSize(0,0));
        InvalidateBestSize();
    }

    wxRect GetTabsExtent()
    {
        CalculateTabRects();
        if (m_rects.empty())
            return wxRect();
        return wxRect(m_rects.front().GetTopLeft(),
                      m_rects.back().GetBottomRight());
    }

    virtual wxSize DoGetBestSize() const
    {
        MyAuiManagerTabs * non_const_this = wx_const_cast(MyAuiManagerTabs *, this);
        wxRect rect = non_const_this->GetTabsExtent();
        wxSize best = rect.GetSize();
        if (IsHorizontal())
            best += wxSize(2 * rect.x, height_padding);
        else
            best += wxSize(height_padding, 2 * rect.y);
        CacheBestSize(best);
        return best;
    }


    // Drawing functions
    // -----------------

    void DrawTab(wxDC & dc, size_t i)
    {
        DrawTab(dc, GetPane(i), m_rects.at(i), i == m_activeTab);
    }

    void DrawTab(wxDC & dc, size_t i, bool active)
    {
        CalculateTabRects(dc);
        DrawTab(dc, GetPane(i), m_rects.at(i), active);
    }

    void DrawTab(wxDC & dc, wxAuiPaneInfo & pane,
                 const wxRect & rect, bool active = false)
    {
        wxAuiDockArt * art = GetArtProvider();

        // Tab background
        dc.GradientFillLinear(rect,
                              art->GetColor(active
                                                ? wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR
                                                : wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR), 
                              art->GetColor(active
                                                ? wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR
                                                : wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR),
                              m_direction);

        // Tab border
        dc.SetPen(wxPen(art->GetColor(wxAUI_DOCKART_BORDER_COLOUR)));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(rect);

        // Tab text
        dc.SetTextForeground(art->GetColor(active
                                               ? wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR
                                               : wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR));

        dc.SetFont(art->GetFont(wxAUI_DOCKART_CAPTION_FONT));
        wxRect text_rect(rect.GetTopLeft() + GetPadding(), dc.GetTextExtent(pane.caption));
        if (IsHorizontal())
        {
            dc.DrawText(pane.caption, text_rect.CenterIn(rect, wxVERTICAL).GetTopLeft());
        }
        else
        {
            text_rect.SetSize(wxSize(text_rect.height, text_rect.width));
            dc.DrawRotatedText(pane.caption,
#ifdef __WXMSW__
                               // For some reason DrawRotatedText is off by 3 pixels . . . 
                               text_rect.CenterIn(rect, wxHORIZONTAL).GetTopRight() + wxPoint(3,0),
#else
                               text_rect.CenterIn(rect, wxHORIZONTAL).GetTopRight(),
#endif
                               -90);
        }
    }

    void OnPaint(wxPaintEvent & WXUNUSED(evt))
    {
        wxPaintDC dc(this);

        wxAuiDockArt * art = GetArtProvider();

        wxRect background(GetClientSize());
        background.Inflate(art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE));
        art->DrawBackground(dc, this, m_direction, background);

        CalculateTabRects(dc);

        for (size_t i = 0; i < m_panes.size(); ++i)
            DrawTab(dc, i);
    }


    // Mouse
    // -----
    // When the user mouses over a tab, activate the tab and show the pane.
    // As long as the mouse remains over the tab or the pane, keep the pane
    // displayed.  As soon as another tab is activated, switch panes.  If the
    // mouse is out of the window for a period of time, hide the pane.  If the
    // pane is focused at any point, or if the user clicks on a tab, show
    // the pane until another window receives focus.  A double-click unhides
    // the pane.

    int HitTest(wxPoint pos)
    {
        CalculateTabRects();
        // Anywhere on the tabs?
        if (! GetTabsExtent().Contains(pos))
            return -1;
        // Find the tab
        for (size_t i = 0; i < m_panes.size(); ++i)
            if (m_rects.at(i).Contains(pos))
                return i;
        wxFAIL_MSG(_T("Mouse should be on a tab"));
        return -1;
    }

    bool IsMouseOnPane(wxAuiPaneInfo & pane)
    {
        int x, y;
        wxGetMousePosition(&x, &y);
        m_mgr->GetManagedWindow()->ScreenToClient(&x, &y);

        // Find the dock part the mouse is over.
        wxAuiDockUIPart * part = m_mgr->HitTest(x, y);
        return part && m_mgr->HasPane(part, pane);
    }

    void ShowPreviewPane(int index)
    {
        if (index == m_previewPane)
            return;
        // Hide the previous preview pane
        if (m_previewPane != -1)
        {
            GetPane(m_previewPane).Show(false);
            // Disconnect events
            m_mgr->Disconnect(wxEVT_CHILD_FOCUS,
                              wxChildFocusEventHandler(MyAuiManagerTabs::OnChildFocus),
                              NULL, this);
        }
        m_previewPane = index;
        // Show the new preview pane
        if (m_previewPane != -1)
        {
            GetPane(m_previewPane).Show(true);
            // Connect events
            m_mgr->Connect(wxEVT_CHILD_FOCUS,
                           wxChildFocusEventHandler(MyAuiManagerTabs::OnChildFocus),
                           NULL, this);
        }
        // Preview pane does not start out focused
        m_isPreviewFocused = false;
    }

    void HidePreviewPane() { ShowPreviewPane(-1); }

    // Show the pane with the mouse over it.
    void OnMouseMove(wxMouseEvent & evt)
    {
        int pane_index = HitTest(evt.GetPosition());
        if (pane_index != -1)
        {
            StopTimer();

            // Set and redraw the active tab
            if (m_activeTab != pane_index)
            {
                wxClientDC dc(this);
                if (m_activeTab != -1)
                    DrawTab(dc, m_activeTab, false);
                DrawTab(dc, pane_index, true);
                m_activeTab = pane_index;
            }

            // Change the preview pane
            if (m_previewPane != pane_index)
            {
                ShowPreviewPane(pane_index);
                m_mgr->Update();
            }
        }
        else
        {
            wxMouseEvent e;
            OnMouseOut(e);
        }
        evt.Skip();
    }

    void OnMouseOut(wxMouseEvent & evt)
    {
        // Start the pane hide timeout
        if (m_previewPane != -1 && ! m_isPreviewFocused)
            StartTimer();
        // Unset and redraw the active tab
        if (m_activeTab != -1)
        {
            wxClientDC dc(this);
            DrawTab(dc, m_activeTab, false);
            m_activeTab = -1;
        }
        evt.Skip();
    }

    void OnLeftDown(wxMouseEvent & evt)
    {
        // Panes that are focused never timeout, so
        // kill the timer.
        StopTimer();
        int pane_index = HitTest(evt.GetPosition());
        if (pane_index != -1)
        {
            // Set focus to the new preview pane
            ShowPreviewPane(pane_index);
            GetPane(pane_index).window->SetFocus();
        }
        else
        {
            HidePreviewPane();
        }
        m_mgr->Update();
    }

    void OnLeftDblClick(wxMouseEvent & evt)
    {
        // OnLeftDown is called first, so we should already
        // have a preview pane
        if (m_previewPane == -1)
            return;
        // Remove the pane from our tabs and show it (same as
        // selecting "Dock" from the context menu).
        GetPane(m_previewPane).Show();
        RemovePane(m_previewPane);
        m_mgr->Update();
    }

    // If the focus is given to our preview pane, stop the timer timeout
    // (Same as OnLeftDown).  Otherwise hide our preview pane.
    void OnChildFocus(wxChildFocusEvent & evt)
    {
        evt.Skip();
        if (m_previewPane == -1)
            return;

        wxWindow * focus = evt.GetWindow();
        // The user clicked on a wxAuiManager-drawn object
        // e.g. a caption or border.
        if (focus == m_mgr->GetManagedWindow())
        {
            // Is the mouse over our pane?
            if (IsMouseOnPane(GetPane(m_previewPane)))
            {
                m_isPreviewFocused = true;
                StopTimer();
                return;
            }
        }
        else
        {
            // Find the pane that the focused window belongs to.
            wxAuiPaneInfo & pane = m_mgr->FindPane(focus);
            if (pane.IsOk() && GetPane(m_previewPane).window == pane.window)
            {
                m_isPreviewFocused = true;
                StopTimer();
                return;
            }
        }
        // else the focus is on a different pane; hide our preview.
        HidePreviewPane();
        StopTimer();
        m_mgr->Update();
    }

    // Context Menu
    //-------------
    // This is the same context menu that is shown by MyAuiManager
    void OnContextMenu(wxContextMenuEvent & evt)
    {
        wxPoint pos = evt.GetPosition();
        if (pos == wxDefaultPosition)
            pos = wxGetMousePosition();
        pos = ScreenToClient(pos);
        int pane_index = HitTest(pos);
        if (pane_index != -1)
        {
            wxAuiPaneInfo & pane = GetPane(pane_index);
            m_mgr->ShowContextMenu(pane);
        }
    }

    // Timer events
    //-------------
    // When the user's focus is away from the preview pane, check to see
    // whether the mouse remains hovering over the preview pane.  If
    // it is away from the preview pane for more than a second, hide
    // the preview pane.

    void StopTimer()
    {
        m_timer.Stop();
    }

    void StartTimer()
    {
        m_timer.Start(1000);
    }

    void OnPaneTimeout(wxTimerEvent & evt)
    {
        if (m_previewPane == -1)
            return;

        wxAuiPaneInfo & pane = GetPane(m_previewPane);

        // Is there an AUI "action" happening, and is this pane
        // a part of it?
        // Without this check, we could crash.
        if (m_mgr->IsPaneActive(pane))
            StartTimer();
        // Is the mouse within the target window?
        else if (IsMouseOnPane(pane))
            StartTimer();
        // The user is away from our pane; hide it.
        else
        {
            HidePreviewPane();
            m_mgr->Update();
        }
    }
};



// ----------------------------------------------------------------
// MyAuiManager
// ----------------------------------------------------------------

MyAuiManager::MyAuiManager(wxWindow* managed_wnd, unsigned int flags)
        : wxAuiManager(managed_wnd, flags),
          m_top(NULL),
          m_bottom(NULL),
          m_left(NULL),
          m_right(NULL),
          m_menu(NULL),
          m_contextPane(NULL)
{
    Connect(wxEVT_AUI_PANE_BUTTON, wxAuiManagerEventHandler(MyAuiManager::OnPaneButton));
    Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(MyAuiManager::OnContextMenu));
    Connect(wxEVT_SIZE, wxSizeEventHandler(MyAuiManager::OnFrameSize));
    // We're going to assume that managed_wnd is not NULL
    m_frameSize = managed_wnd->GetClientSize();
    m_defaultFrameSize = m_frameSize;
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

// Omit "private" panes (such as the tabs), and save tabbed panes in
// the perspective string: "__tab(direction)=pane1;pane2;pane3|"
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
        if (! IsPrivatePane(pane)) // Don't save our tabs
            result += SavePaneInfo(pane)+wxT("|");
    }

    // Save the cached panes
    std::map<wxString, CachedPane>::iterator it;
    for (it = m_paneCache.begin(); it != m_paneCache.end(); ++it)
        result += it->second.perspective + wxT("|");

    int dock_i, dock_count = m_docks.GetCount();
    for (dock_i = 0; dock_i < dock_count; ++dock_i)
    {
        wxAuiDockInfo& dock = m_docks.Item(dock_i);

        result += wxString::Format(wxT("dock_size(%d,%d,%d)=%d|"),
                                   dock.dock_direction, dock.dock_layer,
                                   dock.dock_row, dock.size);
    }

    // Save our tabs
    // wxAUI_DOCK_TOP = 1; wxAUI_DOCK_LEFT = 4
    for (int tab_i = 1; tab_i < 5; ++ tab_i)
    {
        result += wxString::Format(_T("__tab(%d)="), tab_i);
        MyAuiManagerTabs * tab = GetTabs(tab_i);
        for (size_t tab_pane = 0; tab_pane < tab->m_panes.size(); ++tab_pane)
        {
            wxAuiPaneInfo & pane = tab->GetPane(tab_pane);
            wxString name = pane.name;
            name.Replace(_T(";"), _T("\\;"));
            name.Replace(_T("|"), _T("\\|"));
            result += name + _T(";");
        }
        result += _T("|");
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

    // Get rid of __tab sections and frame_size section
    wxString processed_layout;
    processed_layout.reserve(layout.size());
    {
        wxStringTokenizer tok(escaped_layout, _T("|"), wxTOKEN_RET_DELIMS);
        while (tok.HasMoreTokens())
        {
            wxString part = tok.GetNextToken();
            if (! part.StartsWith(_T("__tab")) &&
                ! part.StartsWith(_T("frame_size")))
            {
                processed_layout += part;
            }
        }
        // Unescape
        processed_layout.Replace(_T("\a"), _T("\\|"));
        processed_layout.Replace(_T("\b"), _T("\\;"));
    }

    wxSize frameSize = m_defaultFrameSize;

    if (! wxAuiManager::LoadPerspective(processed_layout, false))
        return false;

    // Cache panes that are not currently present for later, and process
    // __tab sections.
    m_paneCache.clear();
    wxStringTokenizer tok(escaped_layout, _T("|"), wxTOKEN_STRTOK);
    // Skip the first token, it's just the layout format "layout2"
    tok.GetNextToken();
    while (tok.HasMoreTokens())
    {
        wxString info_str = tok.GetNextToken();
        // __tab(direction)=paneName;paneName;paneName;...|
        if (info_str.StartsWith(_T("__tab")))
        {
            // Parse the tab string.
            wxString tab_part = info_str.BeforeFirst(wxT('='));
            tab_part = tab_part.AfterFirst(wxT('('));
            tab_part = tab_part.BeforeLast(wxT(')'));
            long direction;
            if (! tab_part.ToLong(&direction) || direction < 1 || direction > 4)
                continue;
            MyAuiManagerTabs * tab = GetTabs(direction);

            // We could have a corrupted tab string, so don't rely on GetTabs()
            // to return a valid window.
            if (! tab)
                continue;
            tab->ClearPanes();

            wxString tab_panes = info_str.AfterFirst(wxT('='));
            tab_panes.Trim();
            tab_panes.Trim(false);
            wxStringTokenizer pane_tok(tab_panes, _T(";"), wxTOKEN_STRTOK);
            while (pane_tok.HasMoreTokens())
            {
                wxString name = pane_tok.GetNextToken();
                name.Replace(wxT("\a"), wxT("|"));
                name.Replace(wxT("\b"), wxT(";"));
                wxAuiPaneInfo & pane = GetPane(name);
                if (pane.IsOk())
                {
                    AddToTabs(pane);
                }
                else // cache the tab
                {
                    CachedPane & cache = m_paneCache[name];
                    cache.tab = static_cast<wxAuiManagerDock>(direction);
                }
            }
        }
        else if (info_str.StartsWith(_T("dock_size")))
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
            {
                CachedPane & cache = m_paneCache[pane.name];
                cache.perspective = info_str;
            }
        }
    }

    ResizeDocks(frameSize, m_frameSize);

    UpdateMenu();
    if (update)
        Update();
    return true;
}


// Pane Cache

bool
MyAuiManager::GetCachedPane(const wxString & name, wxAuiPaneInfo & pane)
{
    // Look for our pane in m_paneCache
    std::map<wxString, CachedPane>::iterator it = m_paneCache.find(name);
    if (it != m_paneCache.end())
    {
        CachedPane & cache = it->second;
        // Load the cached pane
        if (! cache.perspective.IsEmpty())
        {
            LoadPaneInfo(cache.perspective, pane);
            return pane.name == name;
        }
    }
    return false;
}

bool
MyAuiManager::HasCachedPane(const wxString & name)
{
    // Look for our pane in m_paneCache
    std::map<wxString, CachedPane>::iterator it = m_paneCache.find(name);
    if (it == m_paneCache.end())
        return false;

    CachedPane & cache = it->second;
    // Are we in a tab?
    if (cache.tab != wxAUI_DOCK_NONE)
        return true;
    // Load the cached pane; are we shown?
    if (! cache.perspective.IsEmpty())
    {
        wxAuiPaneInfo pane;
        LoadPaneInfo(cache.perspective, pane);
        return pane.IsShown();
    }
    return false;
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
    // Look for our pane in m_paneCache
    std::map<wxString, CachedPane>::iterator it = m_paneCache.find(pane_info.name);
    // If the pane is cached, load the cached info string instead of the supplied pane.
    if (it != m_paneCache.end())
    {
        CachedPane & cache = it->second;
        // Load from a cached info string
        if (! cache.perspective.IsEmpty())
        {
            wxAuiPaneInfo cached_pane;
            LoadPaneInfo(cache.perspective, cached_pane);
            // Copy over the caption.
            if (! pane_info.caption.IsEmpty())
                cached_pane.caption = pane_info.caption;
            // Try the cached pane, then the supplied pane if that
            // doesn't work.
            if (! wxAuiManager::AddPane(window, cached_pane))
                if (! wxAuiManager::AddPane(window, pane_info))
                    return false;
        }
        else // Load from the supplied wxAuiPaneInfo
        {
            if (! wxAuiManager::AddPane(window, pane_info))
                return false;
        }
        // Add to a tab if the cache says so.
        if (cache.tab != wxAUI_DOCK_NONE)
        {
            MyAuiManagerTabs * tab = GetTabs(cache.tab);
            if (tab)
                AddToTabs(GetPane(window));
        }
        m_paneCache.erase(it);
    }
    else
    {
        if (! wxAuiManager::AddPane(window, pane_info))
            return false;
    }
    if (m_menu)
    {
        wxAuiPaneInfo & pane = GetPane(window);
        if (! IsPrivatePane(pane))
            AddToMenu(pane);
    }
    return true;
}



bool
MyAuiManager::DetachPane(wxWindow * window)
{
    wxAuiPaneInfo & pane = GetPane(window);
    if (! IsPrivatePane(pane))
    {
        RemoveContextWindow(pane);
        if (m_menu)
            RemoveFromMenu(pane);
        // Add this to the cache
        m_paneCache[pane.name].perspective = SavePaneInfo(pane);
    }
    if (! wxAuiManager::DetachPane(window))
        return false;
    return true;
}


// ----------------------------------------------------------------------------
// Update functions
// ----------------------------------------------------------------------------

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

void
MyAuiManager::Update()
{
    // Check for newly floating panes and remove them from the tabs.
    // Check for newly hidden panes and kill their focus.
    // Transfer pane size to pane best_size, so that the user's layout
    // changes are persistent.
    wxAuiPaneInfo & focused_pane = FindPane(wxWindow::FindFocus());
    for (size_t i = 0; i < m_panes.Count(); ++i)
    {
        wxAuiPaneInfo & pane = m_panes.Item(i);
        if (IsPrivatePane(pane))
            continue;
        SavePaneSize(pane);
        // Is this a newly floating pane that used to be part of
        // a tab?
        if (pane.IsFloating() && pane.frame == NULL)
            RemoveFromTabs(pane, false);
        // Is this a pane that has just been hidden?
        if (! pane.IsShown() && pane.window->IsShown())
        {
            // Did it have the focus?
            if (focused_pane.IsOk() && focused_pane.window == pane.window)
                // We can't have focus on a hidden window, so give it away
                GetManagedWindow()->SetFocus();
        }
    }

    // Force Clue panes to be the same size
    // This used to only work on Across and Down panes, which was much
    // simpler.
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

// ----------------------------------------------------------------------------
// Tabs
// ----------------------------------------------------------------------------

// Return the MyAuiManagerTabs associated with the given direction.
// Create the tabs window if it does not already exist.
MyAuiManagerTabs *
MyAuiManager::GetTabs(int direction)
{
    static wxAuiPaneInfo baseInfo;
    baseInfo.CaptionVisible(false)
            .PaneBorder(false)
            .CloseButton(false)
            .DockFixed()
            .Dockable(false)
            .Floatable(false)
            .Layer(MyAuiManagerTabs::dock_layer)
            .Movable(false)
            .Hide();

    switch (direction)
    {
        case wxAUI_DOCK_LEFT:
            if (! m_left)
            {
                m_left = new MyAuiManagerTabs(this, wxLEFT);
                AddPane(m_left,
                        wxAuiPaneInfo(baseInfo)
                            .Name(_T("__tabsLeft"))
                            .Left());
            }
            return m_left;
        case wxAUI_DOCK_RIGHT:
            if (! m_right)
            {
                m_right = new MyAuiManagerTabs(this, wxRIGHT);
                AddPane(m_right,
                        wxAuiPaneInfo(baseInfo)
                            .Name(_T("__tabsRight"))
                            .Right());
            }
            return m_right;
        case wxAUI_DOCK_TOP:
            if (! m_top)
            {
                m_top = new MyAuiManagerTabs(this, wxTOP);
                AddPane(m_top,
                        wxAuiPaneInfo(baseInfo)
                            .Name(_T("__tabsTop"))
                            .Top());
            }
            return m_top;
        case wxAUI_DOCK_BOTTOM:
            if (! m_bottom)
            {
                m_bottom = new MyAuiManagerTabs(this, wxBOTTOM);
                AddPane(m_bottom,
                        wxAuiPaneInfo(baseInfo)
                            .Name(_T("__tabsBottom"))
                            .Bottom());
            }
            return m_bottom;
    }
    wxFAIL_MSG(_T("Tabs do not exist"));
    return NULL;
}

void
MyAuiManager::AddToTabs(wxAuiPaneInfo & pane, bool hide)
{
    wxASSERT(pane.IsOk());
    MyAuiManagerTabs * tab = GetTabs(pane);
    if (! tab)
        return;
    if (hide)
        pane.Hide();
    tab->AddPane(pane);
}

void
MyAuiManager::RemoveFromTabs(wxAuiPaneInfo & pane, bool show)
{
    wxASSERT(pane.IsOk());
    MyAuiManagerTabs * tab = GetTabs(pane);
    if (! tab)
        return;
    if (tab->RemovePane(pane))
        if (show)
            pane.Show();
}

bool
MyAuiManager::IsInTabs(wxAuiPaneInfo & pane)
{
    wxASSERT(pane.IsOk());
    MyAuiManagerTabs * tab = GetTabs(pane);
    if (! tab)
        return false;
    return tab->HasPane(pane);
}

// Ensure that our tabs are always on the outer dock layer.
bool
MyAuiManager::ProcessDockResult(wxAuiPaneInfo & target,
                                const wxAuiPaneInfo & new_pos)
{
    if (new_pos.dock_layer >= MyAuiManagerTabs::dock_layer)
        return false;
    return wxAuiManager::ProcessDockResult(target, new_pos);
}


void
MyAuiManager::OnPaneButton(wxAuiManagerEvent & evt)
{
    wxAuiPaneInfo & pane = *evt.GetPane();
    switch (evt.GetButton()) {
        // The "pin" button is our "hide to tabs" button.
        case wxAUI_BUTTON_PIN:
        {
            MyAuiManagerTabs * tab = GetTabs(pane);
            if (tab->HasPane(pane))
                RemoveFromTabs(pane);
            else
                AddToTabs(pane);
            Update();
        }
            break;

        case wxAUI_BUTTON_CLOSE:
            if (FireCloseEvent(pane))
            {
                RemoveFromTabs(pane, false);
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
        if (IsPrivatePane(pane))
            continue;
        AddToMenu(pane);
    }
}

void
MyAuiManager::AddToMenu(wxAuiPaneInfo & pane)
{
    wxASSERT(! IsPrivatePane(pane));
    wxASSERT(m_menu);
    wxMenuItem * item = m_menu->AppendCheckItem(wxID_ANY, pane.caption);
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyAuiManager::OnMenu));
    Connect(item->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MyAuiManager::OnUpdateUI));
    item->Check(pane.IsShown());
}

void
MyAuiManager::RemoveFromMenu(wxAuiPaneInfo & pane)
{
    wxASSERT(! IsPrivatePane(pane));
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
    if (! evt.IsChecked())
        RemoveFromTabs(pane, false);
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
    evt.Check(! IsPaneClosed(pane));
}


// ----------------------------------------------------------------------------
// Context menu
// ----------------------------------------------------------------------------

// Set a window that recieves the context menu event.  Pane captions always
// get a context menu event.
void
MyAuiManager::SetContextWindow(wxAuiPaneInfo & info, wxWindow * window)
{
    wxASSERT(info.IsOk() && ! IsPrivatePane(info) && GetPane(info.window).IsOk());
    if (! window)
        RemoveContextWindow(info);
    else
        m_contextWindows[window] = &info;
}

void
MyAuiManager::RemoveContextWindow(wxAuiPaneInfo & info)
{
    wxASSERT(info.IsOk() && ! IsPrivatePane(info) && GetPane(info.window).IsOk());
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
//   * Hide ( = Pin / unpin)
//   -------
//     Close

enum MyAuiManagerContextId
{
    ID_AUI_CONTEXT_FLOATING = wxID_HIGHEST,
    ID_AUI_CONTEXT_DOCKED,
    ID_AUI_CONTEXT_HIDDEN,
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

    item = menu->AppendRadioItem(ID_AUI_CONTEXT_HIDDEN, _T("Hidden"));
    item->Enable(! (pane.dock_direction == wxAUI_DOCK_CENTER ||
                    pane.dock_direction == wxAUI_DOCK_NONE ||
                    pane.IsFloating()));
    item->Check(IsInTabs(pane));

    menu->AppendSeparator();

    item = menu->Append(ID_AUI_CONTEXT_CLOSE, _T("Close"));
    item->Enable(pane.HasCloseButton());

    return menu;
}

// Show a context menu for this pane, process the event, and delete the menu.
void
MyAuiManager::ShowContextMenu(wxAuiPaneInfo & pane)
{
    wxASSERT(pane.IsOk() && ! IsPrivatePane(pane));
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
            RemoveFromTabs(pane);
            Update();
            break;

        case ID_AUI_CONTEXT_DOCKED:
            pane.Dock();
            RemoveFromTabs(pane);
            Update();
            break;

        case ID_AUI_CONTEXT_HIDDEN:
            AddToTabs(pane);
            Update();
            break;

        case ID_AUI_CONTEXT_CLOSE:
            if (FireCloseEvent(pane))
            {
                // close the pane, but check that it
                // still exists in our pane array first
                // (the event handler above might have removed it)
                RemoveFromTabs(pane, false);
                if (GetPane(pane.window).IsOk())
                    ClosePane(pane);
                Update();
            }
            break;

        default:
            // The "title" button was clicked.
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

void
MyAuiManager::SavePaneSize(wxAuiPaneInfo & pane)
{
    if (pane.rect.IsEmpty())
    {
        if (pane.floating_size.IsFullySpecified())
            pane.best_size = pane.floating_size;
    }
    else
    {
        pane.best_size = pane.rect.GetSize();
        if (! pane.floating_size.IsFullySpecified())
            pane.floating_size = pane.best_size;
    }
}

wxAuiPaneInfo &
MyAuiManager::HitTestPane(int x, int y)
{
    wxAuiDockUIPart * part = HitTest(x, y);
    if (part && part->pane)
        return *(part->pane);
    return wxAuiNullPaneInfo;
}

bool
MyAuiManager::HasPane(wxAuiDockUIPart * part, wxAuiPaneInfo & pane)
{
    wxASSERT(part);
    if (part->pane)
        return part->pane == &pane;
    if (part->dock)
        return part->dock->panes.Index(&pane) != -1;
    return false;
}


// pane.IsClosed doesn't accurately reflect whether the pane
// is part of the layout or not, now that we have tabs
bool
MyAuiManager::IsPaneClosed(const wxAuiPaneInfo & pane)
{
    if (pane.IsShown())
        return false;
    MyAuiManagerTabs * tab = GetTabs(pane);
    return ! (tab && tab->HasPane(pane));
}

bool
MyAuiManager::IsPaneActive(wxAuiPaneInfo & pane)
{
    if (m_action == actionNone || ! m_action_part)
        return false;
    return HasPane(m_action_part, pane);
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
}

void
MyAuiManager::ResizeDocks(const wxSize & oldSize, const wxSize & newSize)
{
    // Calculate percent difference
    double x = double(newSize.x) / oldSize.x;
    double y = double(newSize.y) / oldSize.y;

    // Go through the docks and adjust their sizes
    for (size_t i = 0; i < m_docks.Count(); ++i)
    {
        wxAuiDockInfo & dock = m_docks.Item(i);
        if (dock.IsVertical())
            dock.size *= x;
        else
            dock.size *= y;
    }
}
