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


#ifndef TOOL_INFO_H
#define TOOL_INFO_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wx/aui/auibar.h>
#include <set>


class ToolManager;

// Class to hold tool information (all inlined)

class ToolInfo
{
    friend class ToolManager;
public:
    ToolInfo(int id,
             const wxString & label,
             const wxString & iconName,
             const wxString & helpStr = wxEmptyString,
             wxItemKind kind = wxITEM_NORMAL);

    ~ToolInfo() {}


    // Get tool information
    //---------------------
    int              GetId()         const { return m_id; }
    const wxString & GetLabel()      const { return m_label; }
          wxString   GetLabelText()  const { return wxStripMenuCodes(m_label); }
    const wxString & GetHelpString() const { return m_helpStr; }
    wxItemKind       GetKind()       const { return m_kind; }

    const wxString & GetIconName() const { return m_iconName; }
    bool HasIcon() const { return ! m_iconName.empty(); }

    bool IsChecked() const { return m_checked; }
    bool IsEnabled() const { return m_enabled; }

    bool IsAttached(const wxMenu * menu)     const
        { return menu->FindItem(m_id) != NULL; }

    bool IsAttached(const wxToolBar * tb)    const
        { return tb->FindById(m_id) != NULL; }

    bool IsAttached(const wxAuiToolBar * tb) const
        { return tb->FindTool(m_id) != NULL; }


private:
    int m_id;
    wxString m_label;
    wxString m_iconName;
    wxString m_helpStr;
    wxItemKind m_kind;
    bool m_checked;
    bool m_enabled;

    // Keep lists of where this tool is attached
    // We need to keep pointers to the actual toolbar not the tools, because
    // of the way that updating tools/toolbars is handled
    std::set<wxAuiToolBar *>  m_auiToolbars;
    std::set<wxToolBar *>     m_toolbars;
    std::set<wxMenuItem *>    m_menuItems;
};

#endif // TOOL_INFO_H