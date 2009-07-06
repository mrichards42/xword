/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef MY_TOOL_MANAGER_H
#define MY_TOOL_MANAGER_H

#include "ToolInfo.hpp"
#include <wx/string.h>
#include <list>
#include <wx/defs.h> // for wxItemKind

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

    // Note that these functions return NULL if there is 
    //   no tool with the specified id or label, hence the pointers.
    const ToolInfo * GetTool (int id) const;
          ToolInfo * GetTool (int id);
    const ToolInfo * GetTool (const wxString & label) const;
          ToolInfo * GetTool (const wxString & label);

    typedef std::list<ToolInfo> toolList_t;
    const toolList_t & GetTools() const { return m_toolList; }
          toolList_t & GetTools()       { return m_toolList; }

    // Tool States
    // For now just provide a compatibility layer . . .  FIX THIS
    void Check(int id, bool check = true) { GetTool(id)->Check(check); }
    bool Toggle(int id)          { return GetTool(id)->Toggle(); }
    bool IsChecked(int id) const { return GetTool(id)->IsChecked(); }

    void Enable(int id, bool enable = true) { GetTool(id)->Enable(enable); }
    void Disable(int id)             { Enable(id, false); }
    bool IsEnabled(int id) const     { return GetTool(id)->IsEnabled(); }

protected:
    toolList_t m_toolList;

    toolList_t::const_iterator Find (int id) const;
    toolList_t::iterator       Find (int id);
    toolList_t::const_iterator Find (const wxString & label) const;
    toolList_t::iterator       Find (const wxString & label);
};


inline void
ToolManager::AddTool(int id, const wxString & label, const wxString & icoName1,
                     const wxString & icoName2, const wxString & helpString, wxItemKind kind)
{
    m_toolList.push_back(ToolInfo(id, label, icoName1, icoName2, helpString, kind));
}


inline void
ToolManager::AddTool(int id, const wxString & label, const wxString & icoName1,
                     const wxString & helpString, wxItemKind kind)
{
    m_toolList.push_back(ToolInfo(id, label, icoName1, helpString, kind));
}



// GetTool functions (four of them):
//    GetTool by id and label, const and non-const

inline const ToolInfo *
ToolManager::GetTool(int id) const
{
    toolList_t::const_iterator it;
    it = Find(id);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}

inline ToolInfo *
ToolManager::GetTool(int id)
{
    toolList_t::iterator it;
    it = Find(id);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}



inline const ToolInfo *
ToolManager::GetTool(const wxString & label) const
{
    toolList_t::const_iterator it;
    it = Find(label);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}

inline ToolInfo *
ToolManager::GetTool(const wxString & label)
{
    toolList_t::iterator it;
    it = Find(label);
    if (it == m_toolList.end())
        return NULL;
    return &(*it);
}




#endif // MY_TOOL_MANAGER_H
