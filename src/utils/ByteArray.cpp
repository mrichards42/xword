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


#include "ByteArray.hpp"


wxString
ByteArray::to_string(size_t nItems) const
{
    wxString str;
    append_to(str, nItems);
    return str;
}



void
ByteArray::append_to(wxString & str, size_t nItems) const
{
    const_iterator it = ByteArray::begin();
    const_iterator end;

    if (nItems == wxString::npos)
    {
        end = ByteArray::end();
        str.reserve(str.length() + size());
    }
    else
    {
        end = it + nItems;
        str.reserve(str.length() + nItems);
    }

    for (; it != end; ++it)
        str.Append(static_cast<wxChar>(*it));
}


void
ByteArray::push_string(const wxString & str, size_t nItems)
{
    wxString::const_iterator it = str.begin();
    wxString::const_iterator end;

    if (nItems == wxString::npos)
    {
        end = str.end();
        reserve(size() + str.length());
    }
    else
    {
        end = it + nItems;
        reserve(size() + nItems);
    }

    for (; it != end; ++it)
        push_back(static_cast<unsigned char>(*it));
}