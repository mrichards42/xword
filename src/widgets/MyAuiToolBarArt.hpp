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


#ifndef MY_AUI_TOOL_BAR_ART_H
#define MY_AUI_TOOL_BAR_ART_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wx/aui/framemanager.h>
#include <wx/aui/auibar.h>

static const int ID_CUSTOMIZE_TOOLBAR = wxID_HIGHEST + 1000;


// My copy of this class from the same header file as the wxAuiDefaultToolBarArt implementation
class MyToolbarCommandCapture : public wxEvtHandler
{
public:

    MyToolbarCommandCapture() { m_last_id = 0; }
    int GetCommandId() const { return m_last_id; }

    bool ProcessEvent(wxEvent& evt)
    {
        // Don't direct the CUSTOMIZE_TOOLBAR button to the parent; keep it with the toolbar
        if (evt.GetEventType() == wxEVT_COMMAND_MENU_SELECTED && evt.GetId() != ID_CUSTOMIZE_TOOLBAR)
        {
            m_last_id = evt.GetId();
            return true;
        }

        if (GetNextHandler())
            return GetNextHandler()->ProcessEvent(evt);

        return false;
    }

private:
    int m_last_id;
};








class MyAuiToolBarArt : public wxAuiDefaultToolBarArt
{
public:
    MyAuiToolBarArt()
        : wxAuiDefaultToolBarArt()
    {}

    virtual ~MyAuiToolBarArt() { }

    int ShowDropDown(wxWindow * wnd, const wxAuiToolBarItemArray & items);
};


// This is stolen diresctly from $(WXWIN)\src\aui\auibar.cpp (line 722)
// Only minor alterations to the menu code here
inline int
MyAuiToolBarArt::ShowDropDown(wxWindow * wnd, const wxAuiToolBarItemArray & items)
{
    wxMenu menuPopup;

    size_t items_added = 0;

    size_t count = items.GetCount();
    for (size_t i = 0; i < count; ++i)
    {
        wxAuiToolBarItem & item = items.Item(i);

        if (item.GetKind() == wxITEM_SEPARATOR)
        {
            if (items_added > 0)
                    menuPopup.AppendSeparator();
        }
        else
        {
            wxString text = item.GetLabel();

            if (text.empty())
                text = wxT(" ");

            wxMenuItem* m = new wxMenuItem(&menuPopup, item.GetId(), text, item.GetShortHelp(), item.GetKind());

            if (item.GetKind() != wxITEM_CHECK)
                m->SetBitmap(item.GetBitmap());
            menuPopup.Append(m);
            if (item.GetKind() == wxITEM_CHECK && item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
                m->Check();
            if (item.GetState() & wxAUI_BUTTON_STATE_DISABLED)
                m->Enable(false);
            items_added++;
        }
    }

    // find out where to put the popup menu of window items
    wxPoint pt = ::wxGetMousePosition();
    pt = wnd->ScreenToClient(pt);

    // find out the screen coordinate at the bottom of the tab ctrl
    wxRect cli_rect = wnd->GetClientRect();
    pt.y = cli_rect.y + cli_rect.height;

    MyToolbarCommandCapture* cc = new MyToolbarCommandCapture;
    wnd->PushEventHandler(cc);
    wnd->PopupMenu(&menuPopup, pt);
    int command = cc->GetCommandId();
    wnd->PopEventHandler(true);

    return command;
}

#endif // MY_AUI_TOOL_BAR_ART_H