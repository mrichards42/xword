// This file is part of XWord
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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

#include "ConfigManager.hpp"

// ----------------------------------------------------------------------------
// ConfigGroup
// ----------------------------------------------------------------------------
ConfigGroup::ConfigGroup(ConfigManagerBase * cfg)
    : m_name(_T("")),
      m_cfg(cfg)
{
}

ConfigGroup::ConfigGroup(ConfigGroup * parent, const wxString & name)
    : m_name(parent->m_name + _T("/") + name),
      m_cfg(parent->m_cfg)
{
    if (parent)
        parent->m_children.push_back(this);
}

void ConfigGroup::Update(wxEvtHandler * h)
{
    std::list<ConfigGroup *>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->Update(h);
}

void ConfigGroup::RemoveCallbacks(wxEvtHandler * h)
{
    std::list<ConfigGroup *>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->RemoveCallbacks(h);
}

ConfigGroup * ConfigGroup::FindChild(const wxString & name)
{
    wxString childName = m_name + _T("/") + name;
    std::list<ConfigGroup *>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        if ((*it)->m_name == childName)
            return *it;
    }
    return NULL;
}

void ConfigGroup::UpdateLists()
{
    std::list<ConfigGroup *>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->UpdateLists();
}

void ConfigGroup::Copy(const ConfigGroup & other)
{
    if (m_name != other.m_name ||
        m_children.size() != other.m_children.size())
    {
        throw ConfigManagerBase::CopyError();
    }
    // Iterate all the groups within and copy the values over
    std::list<ConfigGroup *>::iterator group;
    std::list<ConfigGroup *>::const_iterator otherGroup;
    group = m_children.begin();
    otherGroup = other.m_children.begin();
    for (;
        group != m_children.end();
        ++group, ++otherGroup)
    {
        (*group)->Copy(**otherGroup);
    }
}


// ----------------------------------------------------------------------------
// ConfigList
// ----------------------------------------------------------------------------