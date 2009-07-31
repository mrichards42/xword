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


#include "ToolInfo.hpp"
#include <wx/filename.h>


ToolInfo::ToolInfo(int id,
                   const wxString & label,
                   const wxString & icoName1,
                   const wxString & helpStr,
                   wxItemKind kind)
    : m_label(label),
      m_icoName1(icoName1),
      m_icoName2(_T("")),
      m_helpStr(helpStr),
      m_kind(kind),
      m_checked(false),
      m_enabled(true),
      m_id(id)
{
    if (id == wxID_ANY)
        m_id = wxNewId();
}



ToolInfo::ToolInfo(int id,
                   const wxString & label,
                   const wxString & icoName1, const wxString & icoName2,
                   const wxString & helpStr,
                   wxItemKind kind)
    : m_label(label),
      m_icoName1(icoName1),
      m_icoName2(icoName2),
      m_helpStr(helpStr),
      m_kind(kind),
      m_checked(false),
      m_enabled(true),
      m_id(id)
{
    if (id == wxID_ANY)
        m_id = wxNewId();
}



wxIcon
ToolInfo::GetIcon(int width, int height, bool checked) const
{
    wxString icon = (checked ? m_icoName1 : m_icoName2);

    if (icon.empty())
        return wxNullIcon;

    icon += wxString::Format(_T("_%d.png"), width);

    wxFileName iconPath(icon);
    iconPath.MakeAbsolute( wxPathOnly(wxTheApp->argv[0]) );
    iconPath.AppendDir(_T("images"));

    return wxIcon(iconPath.GetFullPath(), wxBITMAP_TYPE_PNG);

    //return wxIcon(icon, wxBITMAP_TYPE_ICO_RESOURCE, width, height);
}



wxBitmap
ToolInfo::GetBitmap(int width, int height, bool checked) const
 {
     const wxIcon icon = GetIcon(width, height, checked);
     if (! icon.IsOk())
        return wxNullBitmap;

     return wxBitmap(icon);
}


//---------------------------------------------------------------------------
// Basic Operations:
//   - Check
//   - Enable
//   - SetLabel
//   - SetIconName
//---------------------------------------------------------------------------
ToolInfo &
ToolInfo::Check(bool checked)
{
    wxASSERT_MSG(m_kind == wxITEM_CHECK, _T("This tool is not a check tool"));

    m_checked = checked;

    BOOST_FOREACH(wxAuiToolBar * tb, m_auiToolbars)
    {
        tb->ToggleTool(m_id, checked);
        tb->Refresh();
    }

    BOOST_FOREACH(wxToolBar * tb, m_toolbars)
    {
        tb->ToggleTool(m_id, checked);
    }

    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->Check(checked);

    return *this;
}


ToolInfo &
ToolInfo::Enable(bool enabled)
{
    m_enabled = enabled;

    BOOST_FOREACH(wxAuiToolBar * tb, m_auiToolbars)
    {
        tb->EnableTool(m_id, enabled);
        tb->Refresh();
    }

    BOOST_FOREACH(wxToolBar * tb, m_toolbars)
    {
        tb->EnableTool(m_id, enabled);
    }

    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->Enable(enabled);

    return *this;
}


ToolInfo &
ToolInfo::SetLabel(const wxString & label)
{
    m_label = label;

    // For the toolbars
    wxString stripped_label = wxMenuItem::GetLabelFromText(label);

    // Loop through all the toolbars/menus that this tool belongs to and set
    // the labels.
    BOOST_FOREACH(wxAuiToolBar * tb, m_auiToolbars)
    {
        tb->SetToolLabel(m_id, stripped_label);
        tb->Refresh();
    }

    BOOST_FOREACH(wxToolBar * tb, m_toolbars)
    {
        tb->FindById(m_id)->SetLabel(stripped_label);
        tb->Refresh();
    }

    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
        item->SetItemLabel(m_label);

    return *this;
}


ToolInfo &
ToolInfo::SetIconName(const wxString & icoName, bool checked)
{
    if (checked)
        m_icoName1 = icoName;
    else
        m_icoName2 = icoName;

    // Loop through all the toolbars/menus that this tool belongs to and set
    // the bitmaps.

    BOOST_FOREACH(wxAuiToolBar * tb, m_auiToolbars)
    {
        int width  = tb->GetToolBitmap(m_id).GetWidth();
        int height = tb->GetToolBitmap(m_id).GetHeight();
        tb->SetToolBitmap(m_id, GetBitmap(width, height));

        tb->Refresh();
    }

    BOOST_FOREACH(wxToolBar * tb, m_toolbars)
    {
        const int width  = tb->GetToolBitmapSize().GetWidth();
        const int height = tb->GetToolBitmapSize().GetWidth();
        tb->SetToolNormalBitmap(m_id, GetBitmap(width, height));
    }

    BOOST_FOREACH(wxMenuItem * item, m_menuItems)
    {
        // Only set a bitmap for normal items
        if (m_kind == wxITEM_NORMAL)
        {
            const int width  = item->GetBitmap().GetWidth();
            const int height = item->GetBitmap().GetHeight();
#ifdef __WXMSW__
            item->SetBitmaps( GetBitmap(width, height, true),
                              GetBitmap(width, height, false) );
#else // ! __WXMSW__
            item->SetBitmap( GetBitmap(width, height, true) );
#endif // __WXMSW__ / !
        }
    }

    return *this;
}





//------------------------------------------------------------------------------
// Add to menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolInfo::Add(wxMenu * menu, int width, int height)
{
    wxMenuItem * item = new wxMenuItem(menu,
                                       m_id,
                                       m_label,
                                       m_helpStr,
                                       m_kind);

    // Only set a bitmap for normal items
    if (m_kind == wxITEM_NORMAL)
        item->SetBitmap( GetBitmap(width, height) );

    item = menu->Append(item);

    if (m_kind == wxITEM_CHECK)
        item->Check(IsChecked());

    m_menuItems.insert(item);

    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolInfo::Add(wxToolBar * tb, int width, int height)
{
    wxToolBarToolBase * tool = tb->AddTool(m_id,
                                           GetLabelText(),
                                           GetBitmap(width, height),
                                           GetBitmap(width, height, false),
                                           m_kind,
                                           GetLabelText(),
                                           wxEmptyString);

/*
    wxToolBarToolBase * tool = tb->AddTool(m_id,
                                           GetLabelText(),
                                           GetBitmap(width, height),
                                           m_helpStr,
                                           m_kind);
*/
    if (m_kind == wxITEM_CHECK)
        tool->Toggle(IsChecked());

    m_toolbars.insert(tb);

    return tool;
}


// wxAuiToolBar
void
ToolInfo::Add(wxAuiToolBar * tb, int width, int height)
{
    tb->AddTool(m_id,
                GetLabelText(),
                GetBitmap(width, height),
                m_helpStr,
                m_kind);

    if (m_kind == wxITEM_CHECK)
        tb->ToggleTool(m_id, IsChecked());


    m_auiToolbars.insert(tb);
}



//------------------------------------------------------------------------------
// Insert into menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolInfo::Insert(wxMenu * menu, size_t pos, int width, int height)
{
    wxMenuItem * item = new wxMenuItem(menu,
                                       m_id,
                                       m_label,
                                       m_helpStr,
                                       m_kind);

    // Only set a bitmap for normal items
    if (m_kind == wxITEM_NORMAL)
        item->SetBitmap( GetBitmap(width, height) );

    item = menu->Insert(pos, item);

    if (m_kind == wxITEM_CHECK)
        item->Check(IsChecked());

    m_menuItems.insert(item);

    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolInfo::Insert(wxToolBar * tb, size_t pos, int width, int height)
{
    wxToolBarToolBase * tool = tb->InsertTool(pos,
                                              m_id,
                                              GetLabelText(),
                                              GetBitmap(width, height),
                                              GetBitmap(width, height, false),
                                              m_kind,
                                              m_helpStr);
    if (m_kind == wxITEM_CHECK)
        tool->Toggle(IsChecked());

    m_toolbars.insert(tb);

    return tool;
}



//------------------------------------------------------------------------------
// Remove from menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolInfo::Remove(wxMenu * menu)
{
    wxASSERT_MSG( IsAttached(menu),
                  _T("This item isn't on the menu!") );

    wxMenuItem * item = menu->Remove(m_id);
    m_menuItems.erase(item);
    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolInfo::Remove(wxToolBar * tb)
{
    wxASSERT_MSG( IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    wxToolBarToolBase * tool = tb->RemoveTool(m_id);
    m_toolbars.erase(tb);
    return tool;
}



//------------------------------------------------------------------------------
// Delete from menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
void
ToolInfo::Delete(wxMenu * menu)
{
    wxASSERT_MSG( IsAttached(menu),
                  _T("This item isn't on the menu!") );

    m_menuItems.erase(menu->FindItem(m_id));
    menu->Delete(m_id);
}


// wxToolBar
bool
ToolInfo::Delete(wxToolBar * tb)
{
    wxASSERT_MSG( IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    m_toolbars.erase(tb);
    return tb->DeleteTool(m_id);
}


// wxAuiToolBar
bool
ToolInfo::Delete(wxAuiToolBar * tb)
{
    wxASSERT_MSG( IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    m_auiToolbars.erase(tb);
    return tb->DeleteTool(m_id);
}
