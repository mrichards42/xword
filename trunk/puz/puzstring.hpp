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

// String definitions for the puz library.
// Allows the option of using wide strings or 8-bit strings.
// Provides functions to convert between encodings.

#ifndef PUZ_STRING_H
#define PUZ_STRING_H

#ifndef PUZ_UNICODE
    #define PUZ_UNICODE 1
#endif

#include <string>

namespace puz {

#if PUZ_UNICODE
    #include <cwchar>

    // Strings are stored as wchar_t
    typedef wchar_t char_t;
    typedef std::wstring string_t;

    PUZ_API std::string encode_utf8(const string_t & str);
    PUZ_API string_t decode_utf8(const std::string & str);

    PUZ_API inline const std::wstring & to_unicode(const string_t & str) { return str; }

    #ifndef puzT
        #define puzT(str) L ## str
    #endif

    // Insert a string into a std::ostream
    template <typename STREAM> STREAM &
    operator<<(STREAM & stream, const string_t & str)
    {
        return (stream << encode_utf8(str));
    }
#else
    // Strings are stored as utf-8
    typedef char char_t;
    typedef std::string string_t;

    PUZ_API inline const std::string & encode_utf8(const string_t & str) { return str; }
    PUZ_API inline const string_t & decode_utf8(const std::string & str) { return str; }

    PUZ_API std::wstring to_unicode(const string_t & str);


    #ifndef puzT
        #define puzT(str) str
    #endif
#endif

// puz uses windows-1252 encoding
PUZ_API std::string encode_puz(const string_t & str);
PUZ_API string_t decode_puz(const std::string & str);

// Utility functions
string_t Trim(const string_t & str, const string_t & chars);
inline string_t TrimWhitespace(const string_t & str) { return Trim(str,  puzT("\n\f\r \t")); }

std::string GetExtension(const std::string & filename);

string_t ToString(int number);
int ToInt(const string_t & str);

// XML stuff
enum {
    UNESCAPE_BR = 1,
    UNESCAPE_ENTITIES = 2,
    UNESCAPE_ALL = UNESCAPE_BR | UNESCAPE_ENTITIES
    // Character references will always be unescaped
};

string_t escape_xml(const string_t & str);
string_t unescape_xml(const string_t & str, int options = 0);

std::string GetPuzText(const string_t & str);
} // namespace puz

#endif // PUZ_STRING_H
