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

#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <wx/string.h>
#include <wx/strconv.h>
#include "puz/puzstring.hpp"

inline std::string wx2file(const wxString & str)
{
    return std::string(str.mb_str(*wxConvFileName));
}

// wxString and puz::string_t conversions

#if PUZ_UNICODE

inline puz::string_t wx2puz(const wxString & str)
{
    return str.c_str();
}

inline wxString puz2wx(const puz::string_t & str)
{
    return str.c_str();
}

#else

inline puz::string_t wx2puz(const wxString & str)
{
    return puz::string_t(str.mb_str(wxConvUTF8));
}

inline wxString puz2wx(const puz::string_t & str)
{
    return wxString(str.c_str(), wxConvUTF8);
}

#endif // PUZ_UNICODE

#endif // UTILS_STRING_H
