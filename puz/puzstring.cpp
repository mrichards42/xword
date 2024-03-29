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


#include "parse/pugixml/pugixml.hpp"
#include "puzstring.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <set>

namespace puz {

// ---------------------------------------------------------------------------
// UTF-8
// ---------------------------------------------------------------------------

static unsigned int get_continuation(std::string::const_iterator & it)
{
    unsigned char ch = static_cast<unsigned char>(*it);
    if (! (ch > 127 && ch < 192))
        throw InvalidEncoding();
    return ch;
}


static unsigned int utf8_to_unicode(std::string::const_iterator & it,
                                    std::string::const_iterator & end)
{
    // Translate the next code point
    const unsigned char b1 = static_cast<unsigned char>(*it);
    if (b1 < 128) // Ascii
    {
        return b1;
    }
    else if (b1 < 192) // Continuation character
    {
        throw InvalidEncoding();
    }
    else if (b1 < 224) // 2-byte sequence
    {
        if (++it == end) throw InvalidEncoding();
        unsigned int b2 = get_continuation(it);
        return (b1 - 192) * 64 + b2 - 128;
    }
    else if (b1 < 240) // 3-byte sequence
    {
        if (++it == end) throw InvalidEncoding();
        unsigned int b2 = get_continuation(it);
        if (++it == end) throw InvalidEncoding();
        unsigned int b3 = get_continuation(it);
        return ((b1 - 224) * 64 + (b2 - 128)) * 64 + b3 - 128;
    }
    else if (b1 < 244) // 4-byte sequence
    {
        if (++it == end) throw InvalidEncoding();
        unsigned int b2 = get_continuation(it);
        if (++it == end) throw InvalidEncoding();
        unsigned int b3 = get_continuation(it);
        if (++it == end) throw InvalidEncoding();
        unsigned int b4 = get_continuation(it);
        return (((b1 - 240) * 64 + (b2 - 128)) * 64 + (b3 - 128)) * 64 + b4 - 128;
    }

    throw InvalidEncoding();
}


static void unicode_to_utf8(unsigned int cp, std::string & str)
{
    // These conversions would be more elegant with bitshifts and
    // bit ands, but this works, so whatever.
    if (cp < 128)
    {
        str.push_back(static_cast<char>(cp));
    }
    else if (cp < 2048) // 2-byte sequence
    {
        int b1 = cp / 64;  cp -= b1 * 64;
        int b2 = cp;
        str.push_back(static_cast<char>(b1 + 192));
        str.push_back(static_cast<char>(b2 + 128));
    }
    else if (cp < 0xffff) // 3-byte sequence
    {
        int b1 = cp / 64 / 64;  cp -= b1 * 64 * 64;
        int b2 = cp / 64;       cp -= b2 * 64;
        int b3 = cp;
        str.push_back(static_cast<char>(b1 + 224));
        str.push_back(static_cast<char>(b2 + 128));
        str.push_back(static_cast<char>(b3 + 128));
    }
    else if (cp < 0xfffff) // 4-byte sequence
    {
        int b1 = cp / 64 / 64 / 64;  cp -= b1 * 64 * 64 * 64;
        int b2 = cp / 64 / 64;       cp -= b2 * 64 * 64;
        int b3 = cp / 64;            cp -= b3 * 64;
        int b4 = cp;
        str.push_back(static_cast<char>(b1 + 240));
        str.push_back(static_cast<char>(b2 + 128));
        str.push_back(static_cast<char>(b3 + 128));
        str.push_back(static_cast<char>(b4 + 128));
    }
    else
    {
        throw InvalidEncoding();
    }
}


#if PUZ_UNICODE
string_t decode_utf8(const std::string & str)
{
#else
std::wstring to_unicode(const std::string & str)
{
#endif // PUZ_UNICODE
    return pugi::as_wide(str);
}


#if PUZ_UNICODE
std::string encode_utf8(const string_t & str)
{
    return pugi::as_utf8(str);
}


#endif // PUZ_UNICODE



// ---------------------------------------------------------------------------
// Windows-1252 (.puz encoding)
// ---------------------------------------------------------------------------

// Conversion for 127 < ch < 160 (all others < 256 translate the same)
static unsigned int windowsTable [] = {
    0x20ac,      0, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152,      0, 0x017d,      0,
         0, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153,      0, 0x017e, 0x0178,
};

static char unicode_to_puz(unsigned int cp)
{
    if (cp < 128 || (cp >= 160 && cp <= 255))
        return static_cast<char>(cp);
    else
    {
        // Search for the code point in the replacement table
        unsigned int * result =
            std::find(windowsTable,
                      windowsTable + sizeof(windowsTable),
                      cp);
        if (result == windowsTable + sizeof(windowsTable))
            throw InvalidEncoding();
        return static_cast<char>(result - windowsTable) + 128;
    }
}

static bool can_encode_puz(unsigned int cp)
{
    if (cp < 128 || (cp >= 160 && cp <= 255))
        return true;
    else
    {
        // Search for the code point in the replacement table
        unsigned int * result =
            std::find(windowsTable,
                windowsTable + sizeof(windowsTable),
                cp);
        return result != windowsTable + sizeof(windowsTable);
    }
}

PUZ_API bool puz::can_encode_puz(const string_t & str) {
    string_t::const_iterator it;
    string_t::const_iterator begin = str.begin();
    string_t::const_iterator end = str.end();
    for (it = begin; it != end; ++it)
    {
#if PUZ_UNICODE
        if (!can_encode_puz(static_cast<unsigned int>(*it)))
#else
        if (!can_encode_puz(utf8_to_unicode(it, end)))
#endif // PUZ_UNICODE
            return false;
    }
    return true;
}

static unsigned int puz_to_unicode(unsigned char ch)
{
    if (ch < 128 || ch >= 160)
        return static_cast<unsigned int>(ch);
    else
        return windowsTable[ch-128];
}

std::string encode_puz(const string_t & str)
{
    std::string ret;
    ret.reserve(str.size());

    string_t::const_iterator it;
    string_t::const_iterator begin = str.begin();
    string_t::const_iterator end = str.end();
    for (it = begin; it != end; ++it)
    {
#if PUZ_UNICODE
        ret.push_back(unicode_to_puz(static_cast<unsigned int>(*it)));
#else
        ret.push_back(unicode_to_puz(utf8_to_unicode(it, end)));
#endif // PUZ_UNICODE
    }
    return ret;
}

string_t decode_puz(const std::string & str)
{
    string_t ret;
    ret.reserve(str.size());

    std::string::const_iterator it;
    std::string::const_iterator begin = str.begin();
    std::string::const_iterator end = str.end();
    for (it = begin; it != end; ++it)
    {
#if PUZ_UNICODE
        ret.push_back(puz_to_unicode(static_cast<unsigned char>(*it)));
#else
        unicode_to_utf8(puz_to_unicode(static_cast<unsigned char>(*it)), ret);
#endif // PUZ_UNICODE
    }
    return ret;
}



// ---------------------------------------------------------------------------
// Utility functions
// ---------------------------------------------------------------------------

string_t Trim(const string_t & str, const string_t & chars)
{
    size_t start = str.find_first_not_of(chars);
    if (start == string_t::npos)
        start = 0;
    size_t end = str.find_last_not_of(chars);
    if (end != string_t::npos)
        ++end;
    return str.substr(start, end - start);
}

// Turn a string in snake_case into title case
string_t TitleCase(const string_t & str)
{
    string_t ret(str);
    for (size_t i = 0; i < ret.size(); ++i)
    {
        if (i == 0)
        {
#ifdef PUZ_UNICODE
            ret[i] = towupper(str[i]);
#else
            ret[i] = toupper(str[i]);
#endif
        }
        else if (str[i] == '_' || str[i] == ' ')
        {
            ret[i] = ' ';
            ++i;
            if (i < ret.size())
            {
#ifdef PUZ_UNICODE
                ret[i] = towupper(str[i]);
#else
                ret[i] = toupper(str[i]);
#endif
            }
        }
    }
    return ret;
}

std::string GetExtension(const std::string & filename)
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
    // Return a lower-cased string.
    std::string ret = filename.substr(index+1);
    for (std::string::iterator it = ret.begin(); it != ret.end(); ++it)
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
    swprintf(buf, 6, L"%d", number);
#else
    sprintf(buf, 6, "%d", number);
#endif
    return string_t(buf);
}

int ToInt(const string_t & str)
{
    if (str.empty())
        return -1;
    // Return -1 if the number isn't a valid character
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


bool StartsWith(const string_t & str, const string_t & cmp)
{
    return str.size() >= cmp.size() &&
           str.compare(0, cmp.size(), cmp) == 0;
}

bool EndsWith(const string_t & str, const string_t & cmp)
{
    return str.size() >= cmp.size() &&
           str.compare(str.size() - cmp.size(), cmp.size(), cmp) == 0;
}

bool CaseInsensitiveEquals(const string_t& a, const string_t& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
#if PUZ_UNICODE
        if (towlower(a[i]) != towlower(b[i]))
#else
        if (tolower(a[i]) != tolower(b[i]))
#endif
            return false;
    return true;
}

//----------------------------------------------------------------------------
// Convert between formatted / unformatted
//----------------------------------------------------------------------------
static size_t GetBrTag(const string_t & str, size_t index);

// Puz can take anything that is unformatted, or only
// has <br /> tags.
static bool IsOkForPuz(const string_t & str)
{
    size_t index = str.find_first_of(puzT("<>"));
    while (index != string_t::npos)
    {
        if (str[index] == puzT('>'))
            return false;
        size_t end = GetBrTag(str, index);
        if (end == string_t::npos) // Not a <br /> tag.
            return false;
        index = str.find_first_of(puzT("<>"), end+1);
    }
    return true;
}

// Return unformatted text, and throw an exception if it is formatted.
std::string GetPuzText(const string_t & str, std::string(*encode_text)(const string_t&))
{
    if (! IsOkForPuz(str))
        throw ConversionError("Puz format does not support XHTML formatting.");
    return encode_text(unescape_xml(str, UNESCAPE_ALL));
}

//----------------------------------------------------------------------------
// Escape / unescape XML
//----------------------------------------------------------------------------

string_t escape_xml(const string_t & str)
{
    // Replace & with &amp;
    // Replace < with &lt;
    // Replace > with &gt;
    // Replace \r\n | \r | \n with <br />
    size_t start = 0;
    size_t index = str.find_first_of(puzT("&<>\r\n"), start);
    if (index == string_t::npos)
        return str; // Shortcut if we don't have anything to replace.
    string_t ret;
    ret.reserve(int(str.size() * 1.2));
    while (index != string_t::npos)
    {
        ret.append(str.substr(start, index-start));
        switch (str[index])
        {
            case puzT('&'):
                ret.append(puzT("&amp;"));
                break;
            case puzT('<'):
                ret.append(puzT("&lt;"));
                break;
            case puzT('>'):
                ret.append(puzT("&gt;"));
                break;
            // Line breaks
            case puzT('\r'):
                if (str.substr(index, 2) == puzT("\r\n"))
                    ++index;
            case puzT('\n'):
                ret.append(puzT("<br />"));
                break;
        }
        start = index+1;
        index = str.find_first_of(puzT("&<>\r\n"), start);
    }
    ret.append(str.substr(start));
    return ret;
}


// Return the end of the tag or string_t::npos
size_t GetBrTag(const string_t & str, size_t index)
{
    if (str.substr(index, 3) != puzT("<br"))
        return string_t::npos;
    index+=3;
    size_t end = str.find(puzT('>'), index);
    if (end == string_t::npos)
        return string_t::npos;
    string_t slash = TrimWhitespace(str.substr(index, end - index));
    if (slash.empty() || slash == puzT("/"))
        return end;
    return string_t::npos;
}


unsigned int get_entity_char(const string_t & entity, bool unescape_entities)
{
    size_t length = entity.size();
    if (length < 2)
        return 0;
    if (entity[0] == puzT('#')) // code point
    {
        unsigned int code = 0;
        // Hex
        if (entity[1] == puzT('x') || entity[1] == puzT('X'))
        {
            for (size_t i = 2; i < length; ++i)
            {
                char_t c = tolower(entity[i]);
                if (! isxdigit(c))
                    return 0;
                code = code * 16 + (isdigit(c) ? c - 48 : c - 87);
            }
        }
        // Decimal
        else
        {
            for (size_t i = 1; i < length; ++i)
            {
                char_t c = entity[i];
                if (! isdigit(c))
                    return 0;
                code = code * 10 + c - 48;
            }
        }
        return code;
    }
    // Quotes are always safe to unescape
    else if (entity == puzT("apos"))
        return '\'';
    else if (entity == puzT("quot"))
        return '"';
    // These entities could cause problems
    else if (unescape_entities)
    {
        if (entity == puzT("amp"))
            return '&';
        else if (entity == puzT("lt"))
            return '<';
        else if (entity == puzT("gt"))
            return '>';
    }
    return 0;
}

string_t unescape_xml(const string_t & str, int options)
{
    // Replace <br /> with \n
    // Replace character references
    string_t find_chars((options & UNESCAPE_BR) ? puzT("<&") : puzT("&"));
    const bool unescape_entities = (options & UNESCAPE_ENTITIES) != 0;

    size_t start = 0;
    size_t index = str.find_first_of(find_chars, start);
    if (index == string_t::npos)
        return str; // Shortcut if we don't have anything to replace.
    string_t ret;
    ret.reserve(int(str.size() * 1.2));
    while (index != string_t::npos)
    {
        ret.append(str.substr(start, index-start));
        switch (str[index])
        {
            // <br />
            case puzT('<'):
            {
                size_t end = GetBrTag(str, index);
                if (end != string_t::npos)
                {
                    // We're using windows line breaks, here's why:
                    // Across Lite only displays a line break
                    // on windows if it is \r\n.  I can only
                    // assume that Across Lite on Linux / Mac
                    // would display this as one or two line breaks.
                    // One or two line breaks is better than
                    // one or *no* line breaks.
                    ret.append(puzT("\r\n"));
                    start = end+1;
                }
                else
                {
                    start = index+1;
                }
            }
                break;
            // Character references
            case puzT('&'):
            {
                // Scan to the semicolon
                const char_t * c = str.c_str() + index + 1;
                for (; (*c >= puzT('a') && *c <= puzT('z')) ||
                       (*c >= puzT('A') && *c <= puzT('Z')) ||
                       (*c >= puzT('0') && *c <= puzT('9')) ||
                       *c == puzT('_') || *c == puzT('#'); c++) {}
                // Entity must end with a semicolon
                if (*c != puzT(';'))
                {
                    ret.append(1, puzT('&'));
                    start = index + 1;
                    break;
                }
                // Decode the entity and advance the scan pointer
                size_t nchars = c - (str.c_str() + index + 1) + 2;
                // Chop off the '&' and the ';'
                string_t entity = str.substr(index + 1, nchars - 2);
                unsigned int entity_char = get_entity_char(entity, unescape_entities);
                if (entity_char == 0)
                    ret.append(str.substr(index, nchars));
                else
                {
                    // Depending on the platform, entity_char may or may not be encoded as a single
                    // character. So we first encode it as UTF-8, and then decode it as a string_t.
                    std::string entity_str;
                    unicode_to_utf8(entity_char, entity_str);
#if PUZ_UNICODE
                    ret.append(decode_utf8(entity_str));
#else
                    ret.append(entity_str);
#endif
                }
                start = index + nchars;
            }
                break;
        }
        index = str.find_first_of(find_chars, start);
    }
    ret.append(str.substr(start));
    return ret;
}


string_t escape_character_references(const string_t & str);
string_t unescape_character_references(const string_t & str);


} // namespace puz
