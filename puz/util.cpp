// This file is part of XWord
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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

#include "util.hpp"
#include "Puzzle.hpp"
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace puz {

string_t GetExtension(const string_t & filename)
{
    // Find the last dot
    const size_t index = filename.find_last_of('.');
    // No extenion if there was not a match
    if (index == std::string::npos)
        return puzT("");
    // No extension if the match occurred before the last directory separator
    const size_t dirsep_index = filename.find_last_of(puzT("/\\"));
    if (dirsep_index != string_t::npos && index < dirsep_index)
        return puzT("");
    // Return a lower-cased string.
    string_t ret = filename.substr(index+1);
    for (string_t::iterator it = ret.begin(); it != ret.end(); ++it)
        *it = tolower(*it);
    return ret;
}


string_t ToString(int number)
{
    char_t buf[6];
    // Truncate the number if it will not fit into buf
    if (number > 99999)
        number = 99999;
#if PUZ_UNICODE
    swprintf(buf, L"%d", number);
#else
    sprintf(buf, "%d", number);
#endif
    return string_t(buf);
}

int ToInt(const string_t & str)
{
    if (str.empty())
        return -1;
    // Convert _number to an unsigned int
    int n = 0;
    string_t::const_iterator begin = str.begin();
    string_t::const_iterator end   = str.end();
    for (string_t::const_iterator it = begin; it != end; ++it)
    {
        if (! isdigit(*it))
            return -1;
        n = n * 10 + *it - 48; // ASCII 48 is '0'
    }
    return n;
}


} // namespace puz
