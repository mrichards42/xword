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

// TODO:
//   Better support for wxAuiToolBar

//#include <wx/aui/auibar.h>
#include "../widgets/MyAuiToolBar.hpp"
#include <boost/foreach.hpp> // don't even try to fix all of these, just get boost if it won't compile, please
#include <list>
#include <algorithm>


// Class to hold tool information (all inlined)
class ToolInfo
{
public:
    ToolInfo(int id,
             const wxString & label,
             const wxString & icoName1,
             const wxString & icoName2,
             const wxString & helpStr = wxEmptyString,
             wxItemKind kind = wxITEM_NORMAL);

    ToolInfo(int id,
             const wxString & label,
             const wxString & icoName1,
             const wxString & helpStr = wxEmptyString,
             wxItemKind kind = wxITEM_NORMAL);

    ~ToolInfo() {}

    // Do stuff with the tool
    wxMenuItem        * Add(wxMenu * menu, int width = -1, int height = -1)
        { return Insert(menu, menu->GetMenuItemCount(), width, height); }
    wxToolBarToolBase * Add(wxToolBar * tb, int width = -1, int height = -1)
        { return Insert(tb, tb->GetToolsCount(), width, height); }
    void                Add(MyAuiToolBar * tb, int width = -1, int height = -1)
        { Insert(tb, tb->GetToolCount(), width, height ); }
    wxMenuItem        * Insert(wxMenu * menu, size_t pos, int width = -1, int height = -1);
    wxToolBarToolBase * Insert(wxToolBar * tb, size_t pos, int width = -1, int height = -1);
    void                Insert(MyAuiToolBar * tb, size_t pos, int width = -1, int height = -1);
    void                Remove(wxMenu * menu);
    void                Remove(wxToolBar * tb);
    void                Remove(MyAuiToolBar * tb);

    bool IsAttached(const wxMenu * menu)     const { return menu->FindItem(GetId(), NULL) != NULL; }
    bool IsAttached(const wxToolBar * tb)    const { return tb->FindById(GetId()) != NULL; }
    bool IsAttached(const MyAuiToolBar * tb) const { return std::find(m_auiToolbars.begin(), m_auiToolbars.end(), tb) != m_auiToolbars.end(); }

    wxIcon GetIcon(int width, int height, bool checked = true) const;
    wxBitmap GetBitmap(int width, int height, bool checked = true) const;
    bool HasBitmap(bool checked = true) const { return ! (checked ? m_icoName1.empty() : m_icoName2.empty()); }

    int              GetId()         const { return m_id; }
    const wxString & GetLabel()      const { return m_label; }
          wxString   GetLabelText()  const { return wxStripMenuCodes(m_label); }
    const wxString & GetHelpString() const { return m_helpStr; }
    wxItemKind       GetKind()       const { return m_kind; }

    const wxString & GetIconName(bool checked = true) const
        { return checked ? m_icoName1 : m_icoName2; }

    // These allow chaining
    ToolInfo & SetId         (int id)
        { m_id = (id == wxID_ANY ? wxNewId(): id); return *this; }
    ToolInfo & SetLabel      (const wxString & label);
    ToolInfo & SetHelpString (const wxString & helpStr)
        { m_helpStr = helpStr; return *this; }
    ToolInfo & SetKind       (wxItemKind kind)
        { m_kind = kind; return *this; }

    ToolInfo &  SetIconName(const wxString & icoName, bool checked = true);

    ToolInfo & Check(bool checked = true);
    bool Toggle () { Check(! m_checked); return IsChecked(); }
    bool IsChecked() const { return m_checked; }

    ToolInfo & Enable(bool enabled = true);
    ToolInfo & Disable()   { return Enable(false); }
    bool IsEnabled() const { return m_enabled; }

private:
    int m_id;
    wxString m_label;
    wxString m_icoName1;
    wxString m_icoName2;
    wxString m_helpStr;
    wxItemKind m_kind;
    bool m_checked;
    bool m_enabled;

    std::list<MyAuiToolBar *> m_auiToolbars;
    std::list<wxToolBarToolBase *> m_toolbarItems;
    std::list<wxMenuItem *> m_menuItems;
};


inline
ToolInfo::ToolInfo(int id, const wxString & label, const wxString & icoName1,
                   const wxString & helpStr, wxItemKind kind)
    : m_label(label),
      m_icoName1(icoName1),
      m_icoName2(_T("")),
      m_helpStr(helpStr),
      m_kind(kind),
      m_checked(false),
      m_enabled(false)
{
    SetId(id); // Checks for wxID_ANY
}


inline
ToolInfo::ToolInfo(int id, const wxString & label, const wxString & icoName1,
                   const wxString & icoName2, const wxString & helpStr, wxItemKind kind)
    : m_label(label),
      m_icoName1(icoName1),
      m_icoName2(icoName2),
      m_helpStr(helpStr),
      m_kind(kind),
      m_checked(false),
      m_enabled(false)
{
    SetId(id); // Checks for wxID_ANY
}



inline wxIcon
ToolInfo::GetIcon(int width, int height, bool checked) const
{
    const wxString & icon = (checked ? m_icoName1 : m_icoName2);
    if (icon.empty())
        return wxIcon(0,0, -1);
    return wxIcon(icon, wxBITMAP_TYPE_ICO_RESOURCE, width, height);
}



inline wxBitmap
ToolInfo::GetBitmap(int width, int height, bool checked) const
 {
     const wxString & icon = (checked ? m_icoName1 : m_icoName2);
     if (icon.empty())
         return wxBitmap(0,0);
     return wxBitmap(wxIcon(icon, wxBITMAP_TYPE_ICO_RESOURCE, width, height));
}


//---------------------------------------------------------------------------
// Check or Enable all tools
//---------------------------------------------------------------------------
inline ToolInfo &
ToolInfo::Check(bool checked)
{
    wxASSERT_MSG(m_kind == wxITEM_CHECK, _T("This tool is not a check tool"));

    m_checked = checked;

    // Spin through all the toolbars/menus that this tool belongs to and set the check
    BOOST_FOREACH(MyAuiToolBar * tb, m_auiToolbars) {
        tb->ToggleTool(m_id, checked);
        tb->Refresh(); // have to manually update aui toolbars
    }
    BOOST_FOREACH(wxToolBarToolBase * tool, m_toolbarItems)
        tool->SetToggle(checked);
    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->Check(checked);

    return *this;
}


inline ToolInfo &
ToolInfo::Enable(bool enabled)
{
    m_enabled = enabled;

    // Spin through all the toolbars/menus that this tool belongs to and set the check
    BOOST_FOREACH(MyAuiToolBar * tb, m_auiToolbars) {
        tb->EnableTool(m_id, enabled);
        tb->Refresh(); // have to manually update aui toolbars
    }
    BOOST_FOREACH(wxToolBarToolBase * tool, m_toolbarItems)
        tool->Enable(enabled);
    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->Enable(enabled);

    return *this;
}



inline ToolInfo & 
ToolInfo::SetIconName(const wxString & icoName, bool checked) {
    checked ? m_icoName1 = icoName : m_icoName2 = icoName;

    // Spin through all the toolbars/menus that this tool belongs to and set the bitmaps
    BOOST_FOREACH(MyAuiToolBar * tb, m_auiToolbars) {
        int width  = tb->GetToolBitmap(m_id).GetWidth();
        int height = tb->GetToolBitmap(m_id).GetHeight();
        tb->SetToolBitmap(m_id, GetBitmap(width, height));
        tb->Refresh(); // have to manually update aui toolbars
    }
    BOOST_FOREACH(wxToolBarToolBase * tool, m_toolbarItems) {
        tool->SetNormalBitmap(GetBitmap(tool->GetBitmap().GetWidth(), tool->GetBitmap().GetHeight()));
    }
    BOOST_FOREACH(wxMenuItem * item, m_menuItems) {
        if (! m_kind & wxITEM_CHECK) { // only set the menu bmp if it is not a check item
            int width  = item->GetBitmap().GetWidth();
            int height = item->GetBitmap().GetHeight();
            item->SetBitmaps(GetBitmap(width, height, true), GetBitmap(width, height, false));
        }
    }

    return *this;
}




inline ToolInfo & 
ToolInfo::SetLabel(const wxString & label) {
    m_label = label;
    wxString stripped_label = wxMenuItem::GetLabelFromText(label); // For the toolbars

    // Spin through all the toolbars/menus that this tool belongs to and set the labels
    BOOST_FOREACH(MyAuiToolBar * tb, m_auiToolbars) {
        tb->SetToolLabel(m_id, stripped_label);
        tb->Refresh(); // have to manually update aui toolbars
    }
    BOOST_FOREACH(wxToolBarToolBase * tool, m_toolbarItems)
        tool->SetLabel(stripped_label);
    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->SetItemLabel(m_label);

    return *this;
}





//---------------------------------------------------------------------------
// Add to toolbars and menus
//---------------------------------------------------------------------------
inline wxMenuItem *
ToolInfo::Insert(wxMenu * menu, size_t pos, int width, int height)
{
    wxMenuItem * item = new wxMenuItem(menu, m_id, m_label, m_helpStr, m_kind);
    if (m_kind == wxITEM_NORMAL)
        item->SetBitmap( GetBitmap(width, height) );
    item = menu->Insert(pos, item);
    if (m_kind == wxITEM_CHECK)
        item->Check(IsChecked());
    m_menuItems.push_back(item);
    return item;
}


inline wxToolBarToolBase *
ToolInfo::Insert(wxToolBar * tb, size_t pos, int width, int height)
{
    wxToolBarToolBase * tool = tb->InsertTool(pos, m_id, GetLabelText(), GetBitmap(width, height), GetBitmap(width, height, false), m_kind, m_helpStr);
    if (m_kind == wxITEM_CHECK)
        tool->SetToggle(IsChecked());
    m_toolbarItems.push_back(tool);
    return tool;
}


inline void
ToolInfo::Insert(MyAuiToolBar * tb, size_t pos, int width, int height)
{
    wxLogDebug(_T("Adding Tool: %s"), GetLabelText());
    tb->InsertTool(pos, m_id, GetLabelText(), GetBitmap(width, height), m_helpStr, m_kind);
    if (m_kind == wxITEM_CHECK)
        tb->ToggleTool(m_id, IsChecked());
    m_auiToolbars.push_back(tb);
}

#endif TOOL_INFO_H