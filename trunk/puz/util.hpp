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

#ifndef PUZ_UTIL_H
#define PUZ_UTIL_H

// Miscellaneous utility functions

#include <string>

namespace puz {

inline void MakeUpper(std::string & str)
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
        *it = toupper(*it);
}

inline void MakeLower(std::string & str)
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
        *it = tolower(*it);
}

inline std::string GetExtension(const std::string & filename)
{
    // Find the last dot
    const size_t index = filename.find_last_of('.');
    // No extenion if there was not a match
    if (index == std::string::npos)
        return "";
    // No extension if the match occurred before the last directory separator
    const size_t dirsep_index = filename.find_last_of("/\\");
    if (dirsep_index != std::string::npos && index < dirsep_index)
        return "";
    return filename.substr(index+1);
}


} // namespace puz

#endif // PUZ_UTIL_H
