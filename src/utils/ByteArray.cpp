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
ByteArray::to_string(const wxMBConv & conv) const
{
    return wxString(reinterpret_cast<const char*>(c_str()), conv);
}



void
ByteArray::append_to(wxString & str, const wxMBConv & conv) const
{
    str.Append(to_string(conv));
}


void
ByteArray::push_string(const wxString & str, const wxMBConv & conv)
{
    // Now that is some ugly casting.
    // wxString::mb_str returns const wxCharBuffer in unicode build, which is implicity converted
    // to const char*.
    // Since we need to _explicitly_ cast to const unsigned char*, we need
    // the extra static_cast to const char*
    append(reinterpret_cast<const unsigned char*>(
                static_cast<const char*>(str.mb_str(conv))));
}