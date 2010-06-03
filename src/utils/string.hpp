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

#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <wx/string.h>
#include <string>

// wxString and std::string conversions

static std::string wx2puz(const wxString & str)
{
    // Convert using windows encoding.
    return std::string(str.mb_str(wxCSConv(wxFONTENCODING_CP1252)));
}

static wxString puz2wx(const std::string & str)
{
    // Convert using windows encoding.
    return wxString(str.c_str(), wxCSConv(wxFONTENCODING_CP1252));
}


#endif // UTILS_STRING_H
