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

ToolManager::ToolManager()
{
}

ToolManager::~ToolManager()
{
}



void
ToolManager::SetDesc(const ToolDesc tools [])
{
    for ( ; ; ++tools)
    {
        if (tools->id == TOOL_NONE)
            break;
        AddTool(tools->id,
                tools->label,
                tools->icoName1,
                tools->icoName2,
                tools->helpStr,
                tools->kind);
    }
}


// Find functions (four of them):
//    Find by id and label, const and non-const

ToolManager::toolList_t::const_iterator
ToolManager::Find(int id) const
{
    toolList_t::const_iterator it;
    for (it = m_toolList.begin(); it != m_toolList.end(); ++it)
        if (it->GetId() == id)
            return it;
    return it;
}


ToolManager::toolList_t::const_iterator
ToolManager::Find(const wxString & label) const
{
    toolList_t::const_iterator it;
    for (it = m_toolList.begin(); it != m_toolList.end(); ++it)
        if (it->GetLabelText() == label)
            return it;
    return it;
}


ToolManager::toolList_t::iterator
ToolManager::Find(int id)
{
    toolList_t::iterator it;
    for (it = m_toolList.begin(); it != m_toolList.end(); ++it)
        if (it->GetId() == id)
            return it;
    return it;
}


ToolManager::toolList_t::iterator
ToolManager::Find(const wxString & label)
{
    toolList_t::iterator it;
    for (it = m_toolList.begin(); it != m_toolList.end(); ++it)
        if (it->GetLabelText() == label)
            return it;
    return it;
}



//------------------------------------------------------------------------------
// Add to menus / toolbars
//------------------------------------------------------------------------------

