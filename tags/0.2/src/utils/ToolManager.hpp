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


#ifndef MY_TOOL_MANAGER_H
#define MY_TOOL_MANAGER_H

#include "ToolInfo.hpp"
#include <list>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/string.h>
#    include <wx/event.h>
#    include <wx/defs.h> // for wxItemKind
#endif


// Define some macros to make calling ToolInfo functions from ToolManager
// cleaner.

#define TOOL_PROXY(ret, name, const_or_not)              \
    ret name(int id) const_or_not { return GetTool(id)->name(); }

#define TOOL_PROXY1(ret, name, type1, var1, const_or_not)              \
    ret name(int id, type1 var1) const_or_not { return GetTool(id)->name(var1); }

// Used with an array of tools for an initialized list to mark the end
const int TOOL_NONE = wxID_NONE;

// Any strings not specified will be wxEmptyString
struct ToolDesc
{
    int id;
    wxItemKind kind;
    const wxString label;
    const wxString iconName;
    const wxString helpStr;
};

class ToolManager : public wxEvtHandler
{
public:
    ToolManager();
    ~ToolManager();

    // Takes over logic for checking/unchecking items
    // These two functions follow the wxAuiManager model.
    void SetManagedWindow(wxWindow * window);
    void UnInit();

    void OnToolSelected(wxCommandEvent & evt);

    // Adding tools to the manager
    //----------------------------
    void AddTool(int id,
                 const wxString & label,
                 const wxString & iconName,
                 const wxString & helpString = wxEmptyString,
                 wxItemKind kind = wxITEM_NORMAL);

    void SetDesc(const ToolDesc tools []);



    // Tool access
    //------------

    // Return NULL if there is no tool
    const ToolInfo * GetTool (int id) const;
          ToolInfo * GetTool (int id);


    // Icon functions
    //---------------
    void SetIconSize_Menu       (int iconSize) { m_menuIconSize = iconSize; }
    void SetIconSize_ToolBar    (int iconSize) { m_tbIconSize = iconSize; }
    void SetIconSize_AuiToolBar (int iconSize) { m_auiIconSize = iconSize; }

    int  GetIconSize_Menu       () const       { return m_menuIconSize; }
    int  GetIconSize_ToolBar    () const       { return m_tbIconSize; }
    int  GetIconSize_AuiToolBar () const       { return m_auiIconSize; }

    void SetIconLocation(const wxString & path) { m_iconLocation = path; }
    const wxString & GetIconLocation() const    { return m_iconLocation; }

    wxIcon   GetIcon  (int id, int size) const { return GetIcon  (GetTool(id), size); }
    wxBitmap GetBitmap(int id, int size) const { return GetBitmap(GetTool(id), size); }

    wxIcon   GetIcon  (const ToolInfo * tool, int size) const;
    wxBitmap GetBitmap(const ToolInfo * tool, int size) const;


    // Add to menus / toolbars
    //------------------------
    wxMenuItem *        Add(wxMenu * menu,     int id, int iconSize = -1);
    wxToolBarToolBase * Add(wxToolBar * tb,    int id, int iconSize = -1);
    void                Add(wxAuiToolBar * tb, int id, int iconSize = -1);


    // Insert into menus / toolbars
    //-----------------------------
    wxMenuItem *
    Insert(wxMenu * menu, int id, size_t pos, int iconSize = -1);

    wxToolBarToolBase *
    Insert(wxToolBar * tb, int id, size_t pos, int iconSize = -1);

    // wxAuitoolBar doesn't have an Insert method . . .



    // Remove from menus / toolbars
    //-----------------------------
    wxMenuItem *        Remove(wxMenu * menu, int id);
    wxToolBarToolBase * Remove(wxToolBar * tb, int menu);
    // wxAuitoolBar doesn't have a Remvoe method . . .


    // Delete from menus / toolbars
    //-----------------------------
    void Delete(wxMenu * menu,     int id);
    bool Delete(wxToolBar * tb,    int id);
    bool Delete(wxAuiToolBar * tb, int id);


    // Information
    //------------
    TOOL_PROXY1(bool, IsAttached, wxMenu* ,       menu, const)
    TOOL_PROXY1(bool, IsAttached, wxToolBar* ,    menu, const)
    TOOL_PROXY1(bool, IsAttached, wxAuiToolBar* , menu, const)

    TOOL_PROXY(const wxString &, GetLabel,      const)
    TOOL_PROXY(      wxString,   GetLabelText,  const)
    TOOL_PROXY(const wxString &, GetHelpString, const)
    TOOL_PROXY(wxItemKind,       GetKind,       const)
    TOOL_PROXY(const wxString &, GetIconName,   const)
    TOOL_PROXY(bool,             HasIcon,       const)

    void SetLabel   (int id, const wxString & label);
    void SetIconName(int id, const wxString & iconName);

    // Tool States
    //------------

    // Check
    void Check    (int id, bool check = true) { Check(GetTool(id), check); }
    bool Toggle   (int id)                    { return Toggle(GetTool(id)); }
    bool IsChecked(int id) const { return GetTool(id)->IsChecked(); }

    void Check (ToolInfo * tool, bool check = true);
    bool Toggle(ToolInfo * tool)
        { Check(tool, ! tool->IsChecked()); return tool->IsChecked(); }

    // Enabling
    void Enable   (int id, bool enable = true) { Enable(GetTool(id), enable); }
    void Disable  (int id)                     { Enable(id, false); }
    bool IsEnabled(int id) const { return GetTool(id)->IsChecked(); }

    void Enable (ToolInfo * tool, bool enable = true);
    void Disable(ToolInfo * tool) { Enable(tool, false); }


private:
    std::list<ToolInfo> m_toolList;

    int m_menuIconSize;
    int m_tbIconSize;
    int m_auiIconSize;
    wxString m_iconLocation;
    wxWindow * m_managedWindow;
};



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------


// Adding tools
//-------------

inline void
ToolManager::AddTool(int id, const wxString & label, const wxString & iconName,
                     const wxString & helpString, wxItemKind kind)
{
    m_toolList.push_back(ToolInfo(id, label, iconName, helpString, kind));
}



// GetTool functions
//------------------

inline
const ToolInfo *
ToolManager::GetTool(int id) const
{
    std::list<ToolInfo>::const_iterator it, end;
    for (it = m_toolList.begin(), end = m_toolList.end();
         it != end;
         ++it)
    {
        if (it->GetId() == id)
            return &(*it);
    }
    return NULL;
}


inline
ToolInfo *
ToolManager::GetTool(int id)
{
    std::list<ToolInfo>::iterator it, end;
    for (it = m_toolList.begin(), end = m_toolList.end();
         it != end;
         ++it)
    {
        if (it->GetId() == id)
            return &(*it);
    }
    return NULL;
}




#undef TOOL_PROXY
#undef TOOL_PROXY_1



#endif // MY_TOOL_MANAGER_H