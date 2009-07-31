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
#include <wx/string.h>
#include <wx/defs.h> // for wxItemKind



// Define some macros to make calling ToolInfo functions from ToolManager
// cleaner.
// Templates are used so that GetTool() can be called with any type allowed.

#define TOOL_PROXY(ret, name, const_or_not)              \
    template <typename T>                                \
    ret name(T getter) const_or_not                      \
        { return GetTool(getter)->name(); }

#define TOOL_PROXY_1(ret, name, type1, var1, def1, const_or_not)       \
    template <typename T>                                              \
    ret name(T getter, type1 var1 def1) const_or_not                   \
        { return GetTool(getter)->name(var1); }

#define TOOL_PROXY_2(ret, name, type1, var1, def1,                     \
                                type2, var2, def2,                     \
                                const_or_not)                          \
    template <typename T>                                              \
    ret name(T getter, type1 var1 def1,                                \
                       type2 var2 def2) const_or_not                   \
        { return GetTool(getter)->name(var1, var2); }

#define TOOL_PROXY_3(ret, name, type1, var1, def1,                     \
                                type2, var2, def2,                     \
                                type3, var3, def3,                     \
                                const_or_not)                          \
    template <typename T>                                              \
    ret name(T getter, type1 var1 def1,                                \
                       type2 var2 def2,                                \
                       type3 var3 def3) const_or_not                   \
        { return GetTool(getter)->name(var1, var2, var3); }


#define TOOL_PROXY_VOID(name, const_or_not)              \
    template <typename T>                                \
    void name(T getter) const_or_not                     \
        { GetTool(getter)->name(); }

#define TOOL_PROXY_VOID_1(name, type1, var1, def1, const_or_not)       \
    template <typename T>                                              \
    void name(T getter, type1 var1 def1) const_or_not                   \
        { GetTool(getter)->name(var1); }

#define NOT_CONST
#define NO_DEFAULT


// Used with an array of tools for an initialized list to mark the end
const int TOOL_NONE = wxID_NONE;

// Any strings not specified will be wxEmptyString
struct ToolDesc
{
    int id;
    wxItemKind kind;
    const wxString label;
    const wxString icoName1;
    const wxString icoName2;
    const wxString helpStr;
};

class ToolManager
{
public:
    ToolManager();
    ~ToolManager();

    // Adding tools to the manager
    //----------------------------
    void AddTool(int id,
                 const wxString & label,
                 const wxString & icoName1,
                 const wxString & icoName2,
                 const wxString & helpString = wxEmptyString,
                 wxItemKind kind = wxITEM_NORMAL);

    void AddTool(int id,
                 const wxString & label,
                 const wxString & icoName1,
                 const wxString & helpString = wxEmptyString,
                 wxItemKind kind = wxITEM_NORMAL);

    void SetDesc(const ToolDesc tools []);



    // Tool access
    //------------

    // Return NULL if there is no tool
    const ToolInfo * GetTool (int id) const;
          ToolInfo * GetTool (int id);
    const ToolInfo * GetTool (const wxString & label) const;
          ToolInfo * GetTool (const wxString & label);

    typedef std::list<ToolInfo> toolList_t;
    const toolList_t & GetToolList() const { return m_toolList; }
          toolList_t & GetToolList()       { return m_toolList; }



    // IconSize functions
    //----------------------
    void SetIconSize_Menu       (int iconSize) { m_menuIconSize = iconSize; }
    void SetIconSize_ToolBar    (int iconSize) { m_tbIconSize = iconSize; }
    void SetIconSize_AuiToolBar (int iconSize) { m_auiIconSize = iconSize; }

    int  GetIconSize_Menu       () const       { return m_menuIconSize; }
    int  GetIconSize_ToolBar    () const       { return m_tbIconSize; }
    int  GetIconSize_AuiToolBar () const       { return m_auiIconSize; }



    // Eveything after here is used as a proxy for ToolInfo functions
    //---------------------------------------------------------------


    // Add to menus / toolbars
    //------------------------
    template <typename T>
    wxMenuItem *
    Add(wxMenu * menu, T getter, int width = -1, int height = -1);

    template <typename T>
    wxToolBarToolBase *
    Add(wxToolBar * tb, T getter, int width = -1, int height = -1);

    template <typename T>
    void
    Add(wxAuiToolBar * tb, T getter, int width = -1, int height = -1);



    // Insert into menus / toolbars
    //-----------------------------
    template <typename T>
    wxMenuItem *
    Insert(wxMenu * menu, T getter, size_t pos, int width = -1, int height = -1);

    template <typename T>
    wxToolBarToolBase *
    Insert(wxToolBar * tb, T getter, size_t pos, int width = -1, int height = -1);

    // wxAuitoolBar doesn't have an Insert method . . .



    // Remove from menus / toolbars
    //------------------------
    TOOL_PROXY_1(wxMenuItem *,        Remove, wxMenu *,    menu,
                 NO_DEFAULT, NOT_CONST);
    TOOL_PROXY_1(wxToolBarToolBase *, Remove, wxToolBar *, tb,
                 NO_DEFAULT, NOT_CONST);

    // wxAuitoolBar doesn't have a Remvoe method . . .


    // Delete from menus / toolbars
    //-----------------------------
    TOOL_PROXY_VOID_1 (Delete, wxMenu *,       menu, NO_DEFAULT, NOT_CONST);
    TOOL_PROXY_1(bool, Delete, wxToolBar *,    tb,   NO_DEFAULT, NOT_CONST);
    TOOL_PROXY_1(bool, Delete, wxAuiToolBar *, tb,   NO_DEFAULT, NOT_CONST);


    TOOL_PROXY_1(bool, IsAttached, wxMenu *,       menu, NO_DEFAULT, const);
    TOOL_PROXY_1(bool, IsAttached, wxToolBar *,    tb,   NO_DEFAULT, const);
    TOOL_PROXY_1(bool, IsAttached, wxAuiToolBar *, tb,   NO_DEFAULT, const);

    TOOL_PROXY  (int,                GetId,          const);
    TOOL_PROXY  (const wxString &,   GetLabel,       const);
    TOOL_PROXY  (wxString,           GetLabelText,   const);
    TOOL_PROXY  (const wxString &,   GetHelpString,  const);
    TOOL_PROXY  (wxItemKind,         GetKind,        const);
    TOOL_PROXY_1(const wxString &,   GetIconName, bool, checked, = true, const);
    TOOL_PROXY_1(bool,               HasBitmap,   bool, checked, = true, const);

    TOOL_PROXY_3(wxIcon,   GetIcon,   int,  width,   NO_DEFAULT,
                                      int,  height,  NO_DEFAULT,
                                      bool, checked, = true,
                                      const);

    TOOL_PROXY_3(wxBitmap, GetBitmap, int,  width,   NO_DEFAULT,
                                      int,  height,  NO_DEFAULT,
                                      bool, checked, = true,
                                      const);

    // Tool States
    //------------

    // Checking
    TOOL_PROXY_1(ToolInfo &,  Check, bool, check, = true, NOT_CONST);
    TOOL_PROXY  (bool,        Toggle,    NOT_CONST);
    TOOL_PROXY  (bool,        IsChecked, const);

    // Enabling
    TOOL_PROXY_1(ToolInfo &,  Enable, bool, enable, = true, NOT_CONST);
    TOOL_PROXY  (ToolInfo &,  Disable,   NOT_CONST);
    TOOL_PROXY  (bool,        IsEnabled, const);



private:
    toolList_t m_toolList;

    toolList_t::const_iterator Find (int id) const;
    toolList_t::iterator       Find (int id);
    toolList_t::const_iterator Find (const wxString & label) const;
    toolList_t::iterator       Find (const wxString & label);

    int m_menuIconSize;
    int m_tbIconSize;
    int m_auiIconSize;
};

//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------


// Adding tools
//-------------

inline void
ToolManager::AddTool(int id, const wxString & label, const wxString & icoName1,
                     const wxString & icoName2, const wxString & helpString, wxItemKind kind)
{
    m_toolList.push_back( ToolInfo(id,
                                   label,
                                   icoName1,
                                   icoName2,
                                   helpString,
                                   kind) );
}


inline void
ToolManager::AddTool(int id, const wxString & label, const wxString & icoName1,
                     const wxString & helpString, wxItemKind kind)
{
    m_toolList.push_back(ToolInfo(id, label, icoName1, helpString, kind));
}



// GetTool functions
//------------------

inline
const ToolInfo *
ToolManager::GetTool(int id) const
{
    toolList_t::const_iterator it;
    it = Find(id);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}


inline
ToolInfo *
ToolManager::GetTool(int id)
{
    toolList_t::iterator it;
    it = Find(id);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}


inline
const ToolInfo *
ToolManager::GetTool(const wxString & label) const
{
    toolList_t::const_iterator it;
    it = Find(label);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}


inline
ToolInfo *
ToolManager::GetTool(const wxString & label)
{
    toolList_t::iterator it;
    it = Find(label);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}




// Add to menu / toolbar
//----------------------

template <typename T>
wxMenuItem *
ToolManager::Add(wxMenu * menu, T getter, int width, int height)
{
    if (width  == -1)   width  = GetIconSize_Menu();
    if (height == -1)   height = GetIconSize_Menu();

    return GetTool(getter)->Add(menu, width, height);
}


template <typename T>
wxToolBarToolBase *
ToolManager::Add(wxToolBar * tb, T getter, int width, int height)
{
    if (width  == -1)   width  = GetIconSize_ToolBar();
    if (height == -1)   height = GetIconSize_ToolBar();

    return GetTool(getter)->Add(tb, width, height);
}


template <typename T>
void
ToolManager::Add(wxAuiToolBar * tb, T getter, int width, int height)
{
    if (width  == -1)   width  = GetIconSize_AuiToolBar();
    if (height == -1)   height = GetIconSize_AuiToolBar();

    GetTool(getter)->Add(tb, width, height);
}




// Insert into menu / toolbar
//----------------------
template <typename T>
wxMenuItem *
ToolManager::Insert(wxMenu * menu, T getter, size_t pos, int width, int height)
{
    if (width  == -1)   width  = GetIconSize_Menu();
    if (height == -1)   height = GetIconSize_Menu();

    return GetTool(getter)->Insert(menu, pos, width, height);
}


template <typename T>
wxToolBarToolBase *
ToolManager::Insert(wxToolBar * tb, T getter, size_t pos, int width, int height)
{
    if (width  == -1)   width  = GetIconSize_ToolBar();
    if (height == -1)   height = GetIconSize_ToolBar();

    return GetTool(getter)->Insert(tb, pos, width, height);
}




#undef TOOL_PROXY
#undef TOOL_PROXY_1
#undef TOOL_PROXY_2
#undef TOOL_PROXY_3
#undef TOOL_PROXY_VOID
#undef TOOL_PROXY_VOID_1
#undef NO_DEFAULT
#undef NOT_CONST



#endif // MY_TOOL_MANAGER_H
