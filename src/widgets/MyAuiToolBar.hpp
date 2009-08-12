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


#ifndef MY_AUI_TOOL_BAR_H
#define MY_AUI_TOOL_BAR_H

// HUGE CAVEAT:
//   Make sure to make the destructor of wxAuiToolBar virtual
//   $(WXWIN)\include\wx\aui\auibar.h


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wx/aui/auibar.h>
#include "MyAuiToolBarArt.hpp"

// Forward declare because we include this file
class ToolManager;


class MyAuiToolBar : public wxAuiToolBar
{
public:
    MyAuiToolBar(wxWindow* parent,
                 wxWindowID id = -1,
                 const wxPoint& position = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxAUI_TB_DEFAULT_STYLE)
        : wxAuiToolBar(parent, id, position, size, style)
    {
        // Make an extra item for the overflow
        wxAuiToolBarItem item;
        item.SetLabel(_T("Customize . . . "));
        item.SetBitmap(wxBitmap(0,0));
        item.SetDisabledBitmap(wxBitmap(0,0));
        item.SetShortHelp(_T("Customize toolbar buttons"));
        item.SetHasDropDown(false);
        item.SetId(ID_CUSTOMIZE_TOOLBAR);
        item.SetKind(wxITEM_NORMAL);
        item.SetMinSize(wxDefaultSize);
        item.SetSticky(false);

        // Add it to the end of the overflow
        m_custom_overflow_append.Add(item);

        SetArtProvider(new MyAuiToolBarArt());

        // Connect events . . . because the base class isn't supposed to be virtual,
        //   we can't use event tables?
        Connect(ID_CUSTOMIZE_TOOLBAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyAuiToolBar::OnCustomize));
        Connect(wxID_ANY, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyAuiToolBar::OnCheckBox));
    }

    void SetToolManager(ToolManager * mgr) { m_toolMgr = mgr; }
    ToolManager * GetToolManager() { return m_toolMgr; }

    virtual ~MyAuiToolBar() {}

    void InsertTool(size_t pos,
                    int tool_id,
                    const wxString& label,
                    const wxBitmap& bitmap,
                    const wxString& short_help_string = wxEmptyString,
                    wxItemKind kind = wxITEM_NORMAL)
    {
        AddTool(tool_id, label, bitmap, short_help_string, kind);
        if (pos != m_items.GetCount()-1)
            m_items.Insert(m_items.Detach(m_items.GetCount()-1), pos);
    }

    void InsertTool(size_t pos,
                    int tool_id,
                    const wxString& label,
                    const wxBitmap& bitmap,
                    const wxBitmap& disabled_bitmap,
                    wxItemKind kind,
                    const wxString& short_help_string,
                    const wxString& long_help_string,
                    wxObject* client_data)
    {
        AddTool(tool_id, label, bitmap, disabled_bitmap, kind, short_help_string, long_help_string, client_data);
        if (pos != m_items.GetCount()-1)
            m_items.Insert(m_items.Detach(m_items.GetCount()-1), pos);
    }


protected:
    void OnCustomize(wxCommandEvent & evt);
    void OnCheckBox(wxCommandEvent & evt);
    ToolManager * m_toolMgr;
};


#endif // MY_AUI_TOOL_BAR_H