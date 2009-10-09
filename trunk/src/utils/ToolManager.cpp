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


#include "ToolManager.hpp"
#include <wx/filename.h>

// Macros to simplify looping

// This is a simplification of the BOOST_FOREACH macro
//   - Using a for loop to declare variables (the extra two for loops)
//     is a neat trick.
#define __SIMPLE_FOREACH(container_t, type, it_name, container)                       \
    for (container_t<type>::iterator _it = container.begin(), _end = container.end(); \
         _it != _end;                                                                 \
         ++_it)                                                                       \
         for (bool _continue = true; _continue;)                                      \
            for (type it_name = *_it; _continue; _continue = false)

#define FOR_AUI_TOOL_BARS(tool) \
    __SIMPLE_FOREACH(std::set, wxAuiToolBar *, tb,   tool->m_auiToolbars)

#define FOR_TOOL_BARS(tool) \
    __SIMPLE_FOREACH(std::set, wxToolBar *,    tb,   tool->m_toolbars)

#define FOR_MENU_ITEMS(tool) \
    __SIMPLE_FOREACH(std::set, wxMenuItem *,   item, tool->m_menuItems)




ToolManager::ToolManager()
    : m_menuIconSize(-1),
      m_tbIconSize(-1),
      m_auiIconSize(-1),
      m_managedWindow(NULL)
{
    m_iconLocation = wxPathOnly(wxTheApp->argv[0]);

    Connect( wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(ToolManager::OnToolSelected) );

}

ToolManager::~ToolManager()
{
}

//------------------------------------------------------------------------------
// Event capturing
//------------------------------------------------------------------------------
void
ToolManager::SetManagedWindow(wxWindow * window)
{
    m_managedWindow = window;
    window->PushEventHandler(this);
}

void
ToolManager::UnInit()
{
    if (m_managedWindow != NULL)
        m_managedWindow->RemoveEventHandler(this);
}


void
ToolManager::OnToolSelected(wxCommandEvent & evt)
{
    ToolInfo * tool = GetTool(evt.GetId());
    if (tool != NULL)
    {
        if (tool->GetKind() == wxITEM_CHECK)
            Toggle(tool);
    }
    evt.Skip();
}






void
ToolManager::SetDesc(const ToolDesc tools [])
{
    for ( ; tools->id != TOOL_NONE; ++tools)
        AddTool(tools->id,
                tools->label,
                tools->iconName,
                tools->helpStr,
                tools->kind);
}








wxIcon
ToolManager::GetIcon(const ToolInfo * tool, int iconSize) const
{
    if (! tool->HasIcon())
        return wxNullIcon;

    const wxString icon =
        wxString::Format(_T("%s_%d.png"), tool->GetIconName().c_str(), iconSize);

    wxFileName iconPath(icon);
    iconPath.MakeAbsolute(m_iconLocation);

    return wxIcon(iconPath.GetFullPath(), wxBITMAP_TYPE_PNG);
}



wxBitmap
ToolManager::GetBitmap(const ToolInfo * tool, int iconSize) const
 {
     if (! tool->HasIcon())
         return wxNullBitmap;

     wxASSERT(GetIcon(tool, iconSize).IsOk());

     return wxBitmap(GetIcon(tool, iconSize));
}



//---------------------------------------------------------------------------
// Basic Operations:
//   - Check
//   - Enable
//   - SetLabel
//   - SetIconName
//---------------------------------------------------------------------------
void
ToolManager::Check(ToolInfo * tool, bool checked)
{
    wxASSERT_MSG(tool->GetKind() == wxITEM_CHECK, _T("This tool is not a check tool"));

    tool->m_checked = checked;

    FOR_AUI_TOOL_BARS(tool)
    {
        tb->ToggleTool(tool->GetId(), checked);
        tb->Refresh();
    }

    FOR_TOOL_BARS(tool)
        tb->ToggleTool(tool->GetId(), checked);

    FOR_MENU_ITEMS(tool)
        item->Check(checked);
}


void
ToolManager::Enable(ToolInfo * tool, bool enabled)
{
    tool->m_enabled = enabled;

    FOR_AUI_TOOL_BARS(tool)
    {
        tb->EnableTool(tool->GetId(), enabled);
        tb->Refresh();
    }

    FOR_TOOL_BARS(tool)
        tb->EnableTool(tool->GetId(), enabled);

    FOR_MENU_ITEMS(tool)
        item->Enable(enabled);
}


void
ToolManager::SetLabel(int id, const wxString & label)
{
    ToolInfo * tool = GetTool(id);

    tool->m_label = label;

    // For the toolbars
    wxString stripped_label = wxMenuItem::GetLabelFromText(label);


    FOR_AUI_TOOL_BARS(tool)
    {
        tb->SetToolLabel(tool->GetId(), stripped_label);
        tb->Refresh();
    }

    FOR_TOOL_BARS(tool)
        tb->FindById(tool->GetId())->SetLabel(stripped_label);

    FOR_MENU_ITEMS(tool)
        item->SetItemLabel(label);
}


void
ToolManager::SetIconName(int id, const wxString & iconName)
{
    ToolInfo * tool = GetTool(id);

    tool->m_iconName = iconName;


    FOR_AUI_TOOL_BARS(tool)
    {
        const int iconSize = tb->GetToolBitmap(tool->GetId()).GetWidth();
        wxASSERT( iconSize == tb->GetToolBitmap(tool->GetId()).GetHeight() );
        tb->SetToolBitmap(tool->GetId(), GetBitmap(tool, iconSize));

        tb->Refresh();
    }

    FOR_TOOL_BARS(tool)
    {
        const int iconSize = tb->GetToolBitmapSize().GetWidth();
        wxASSERT( iconSize == tb->GetToolBitmapSize().GetHeight() );
        tb->SetToolNormalBitmap(tool->GetId(), GetBitmap(tool, iconSize));
    }

    FOR_MENU_ITEMS(tool)
    {
        // Only set a bitmap for normal items
        if (tool->GetKind() == wxITEM_NORMAL)
        {
            const int iconSize = item->GetBitmap().GetWidth();
            wxASSERT( iconSize == item->GetBitmap().GetHeight() );
            item->SetBitmap( GetBitmap(tool, iconSize) );
        }
    }
}





//------------------------------------------------------------------------------
// Add to menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolManager::Add(wxMenu * menu, int id, int iconSize)
{
    if (iconSize == -1)
        iconSize = GetIconSize_Menu();
    wxASSERT(iconSize != -1);

    ToolInfo * tool = GetTool(id);

    wxMenuItem * item = new wxMenuItem(menu,
                                       tool->GetId(),
                                       tool->GetLabel(),
                                       tool->GetHelpString(),
                                       tool->GetKind());

    // Under Windows, if any item has a bitmap it screws up rendering of the
    // menu.  Until there is a fix for that, don't do bitmaps for menus in
    // Windows.
#ifndef __WXMSW__
    // Only set a bitmap for normal items
    if (tool->GetKind() == wxITEM_NORMAL && tool->HasIcon())
        item->SetBitmap( GetBitmap(tool, iconSize) );
#endif // ! __WXMSW__

    item = menu->Append(item);

    if (tool->GetKind() == wxITEM_CHECK)
        item->Check(tool->IsChecked());

    tool->m_menuItems.insert(item);

    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolManager::Add(wxToolBar * tb, int id, int iconSize)
{
    if (iconSize == -1)
        iconSize = GetIconSize_ToolBar();
    wxASSERT(iconSize != -1);

    ToolInfo * tool = GetTool(id);

    const wxBitmap toolBmp = GetBitmap(tool, iconSize);
    // We can't add a tool to a toolbar without a bitmap
    if (! toolBmp.IsOk())
    {
        wxLogError(_T("Image missing for tool \"%s\".  ")
                   _T("Tool will not be available on the toolbar"),
                   tool->GetLabelText().c_str());
        return NULL;
    }

    wxToolBarToolBase * item = tb->AddTool(tool->GetId(),
                                           tool->GetLabelText(),
                                           toolBmp,
                                           wxNullBitmap,
                                           tool->GetKind(),
                                           tool->GetLabelText(),
                                           wxEmptyString);


    if (tool->GetKind() == wxITEM_CHECK)
        item->Toggle(tool->IsChecked());

    tool->m_toolbars.insert(tb);

    return item;
}


// wxAuiToolBar
void
ToolManager::Add(wxAuiToolBar * tb, int id, int iconSize)
{
    if (iconSize == -1)
        iconSize = GetIconSize_AuiToolBar();
    wxASSERT(iconSize != -1);

    ToolInfo * tool = GetTool(id);

    const wxBitmap toolBmp = GetBitmap(tool, iconSize);
    // We can't add a tool to a toolbar without a bitmap
    if (! toolBmp.IsOk())
    {
        wxLogError(_T("Image missing for tool \"%s\".  ")
                   _T("Tool will not be available on the toolbar"),
                   tool->GetLabelText().c_str());
        return;
    }

    tb->AddTool(tool->GetId(),
                tool->GetLabelText(),
                toolBmp,
                tool->GetHelpString(),
                tool->GetKind());

    if (tool->GetKind() == wxITEM_CHECK)
        tb->ToggleTool(tool->GetId(), tool->IsChecked());

    tool->m_auiToolbars.insert(tb);
}



//------------------------------------------------------------------------------
// Insert into menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolManager::Insert(wxMenu * menu, int id, size_t pos, int iconSize)
{
    if (iconSize == -1)
        iconSize = GetIconSize_Menu();
    wxASSERT(iconSize != -1);

    ToolInfo * tool = GetTool(id);

    wxMenuItem * item = new wxMenuItem(menu,
                                       tool->GetId(),
                                       tool->GetLabel(),
                                       tool->GetHelpString(),
                                       tool->GetKind());

    // Under Windows, if any item has a bitmap it screws up rendering of the
    // menu.  Until there is a fix for that, don't do bitmaps for menus in
    // Windows.
#ifndef __WXMSW__
    // Only set a bitmap for normal items
    if (tool->GetKind() == wxITEM_NORMAL && tool->HasIcon())
        item->SetBitmap( GetBitmap(tool, iconSize) );
#endif // ! __WXMSW__

    item = menu->Insert(pos, item);

    if (tool->GetKind() == wxITEM_CHECK)
        item->Check(tool->IsChecked());

    tool->m_menuItems.insert(item);

    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolManager::Insert(wxToolBar * tb, int id, size_t pos, int iconSize)
{
    if (iconSize == -1)
        iconSize = GetIconSize_ToolBar();
    wxASSERT(iconSize != -1);

    ToolInfo * tool = GetTool(id);

    const wxBitmap toolBmp = GetBitmap(tool, iconSize);
    // We can't add a tool to a toolbar without a bitmap
    if (! toolBmp.IsOk())
    {
        wxLogError(_T("Image missing for tool \"%s\".  ")
                   _T("Tool will not be available on the toolbar"),
                   tool->GetLabelText().c_str());
        return NULL;
    }

    wxToolBarToolBase * item = tb->InsertTool(pos,
                                              tool->GetId(),
                                              tool->GetLabelText(),
                                              toolBmp,
                                              wxNullBitmap,
                                              tool->GetKind(),
                                              tool->GetHelpString());
    if (tool->GetKind() == wxITEM_CHECK)
        item->Toggle(tool->IsChecked());

    tool->m_toolbars.insert(tb);

    return item;
}



//------------------------------------------------------------------------------
// Remove from menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
wxMenuItem *
ToolManager::Remove(wxMenu * menu, int id)
{
    ToolInfo * tool = GetTool(id);

    wxASSERT_MSG( tool->IsAttached(menu),
                  _T("This item isn't on the menu!") );

    wxMenuItem * item = menu->Remove(tool->GetId());
    tool->m_menuItems.erase(item);
    return item;
}


// wxToolBar
wxToolBarToolBase *
ToolManager::Remove(wxToolBar * tb, int id)
{
    ToolInfo * tool = GetTool(id);

    wxASSERT_MSG( tool->IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    wxToolBarToolBase * item = tb->RemoveTool(tool->GetId());
    tool->m_toolbars.erase(tb);
    return item;
}



//------------------------------------------------------------------------------
// Delete from menus / toolbars
//------------------------------------------------------------------------------

// wxMenu
void
ToolManager::Delete(wxMenu * menu, int id)
{
    ToolInfo * tool = GetTool(id);

    wxASSERT_MSG( tool->IsAttached(menu),
                  _T("This item isn't on the menu!") );

    tool->m_menuItems.erase(menu->FindItem(tool->GetId()));
    menu->Delete(tool->GetId());
}


// wxToolBar
bool
ToolManager::Delete(wxToolBar * tb, int id)
{
    ToolInfo * tool = GetTool(id);

    wxASSERT_MSG( tool->IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    tool->m_toolbars.erase(tb);
    return tb->DeleteTool(tool->GetId());
}


// wxAuiToolBar
bool
ToolManager::Delete(wxAuiToolBar * tb, int id)
{
    ToolInfo * tool = GetTool(id);

    wxASSERT_MSG( tool->IsAttached(tb),
                  _T("This item isn't on the toolbar!") );

    tool->m_auiToolbars.erase(tb);
    return tb->DeleteTool(tool->GetId());
}


#undef __SIMPLE_FOREACH
#undef FOR_AUI_TOOL_BARS
#undef FOR_TOOL_BARS
#undef FOR_MENU_ITEMS