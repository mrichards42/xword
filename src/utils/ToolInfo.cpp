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


ToolInfo::ToolInfo(int id,
                   const wxString & label,
                   const wxString & iconName,
                   const wxString & helpStr,
                   wxItemKind kind)
    : m_label(label),
      m_iconName(iconName),
      m_helpStr(helpStr),
      m_kind(kind),
      m_checked(false),
      m_enabled(true),
      m_id(id)
{
    if (id == wxID_ANY)
        m_id = wxNewId();
}