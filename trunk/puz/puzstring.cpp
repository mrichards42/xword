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


#include "puzstring.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <expat.h>
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
    string_t ret;
#else
std::wstring to_unicode(const std::string & str)
{
    std::wstring ret;
#endif // PUZ_UNICODE
    ret.reserve(str.size());
    std::string::const_iterator it;
    std::string::const_iterator begin = str.begin();
    std::string::const_iterator end = str.end();
    for (it = begin; it != end; ++it)
        ret.push_back(utf8_to_unicode(it, end));
    return ret;
}


#if PUZ_UNICODE
std::string encode_utf8(const string_t & str)
{
    std::string ret;
    ret.reserve(str.size());

    string_t::const_iterator it;
    string_t::const_iterator begin = str.begin();
    string_t::const_iterator end = str.end();
    for (it = begin; it != end; ++it)
        unicode_to_utf8(static_cast<unsigned int>(*it), ret);
    return ret;
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


//----------------------------------------------------------------------------
// Convert between formatted / unformatted
//----------------------------------------------------------------------------

void SetFormatted(string_t & str, const string_t & other)
{
    str = other;
    // Are there any XML characters, and is this valid XML?
    if (str.find_first_of(puzT("<>")) != string_t::npos)
    {
        if (! IsFormatted(encode_utf8(other)))
            EscapeXML(str);
        // Do nothing
    }
    // Other characters that are escaped using EscapeXML.
    else if (str.find_first_of(puzT("&\n")) != string_t::npos)
    {
        EscapeXML(str);
    }
}


string_t GetFormatted(const string_t & str)
{
    // Are there any XML characters
    if (str.find_first_of(puzT("<>")) != string_t::npos)
        return str;
    // Are there any escaped characters?
    else if (str.find_first_of(puzT("&")) != string_t::npos)
    {
        string_t ret = str;
        UnescapeXML(ret);
        return ret;
    }
    else
    {
        return str;
    }
}


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
std::string GetPuzText(const string_t & str)
{
    if (! IsOkForPuz(str))
    {
        // Check to see if our < > is only in a <br /> tag.
        throw ConversionError("Puz format does not support XHTML formatting.");
    }
    string_t ret = str;
    UnescapeXML(ret);
    return encode_puz(ret);
}

//----------------------------------------------------------------------------
// Escape / unescape XML
//----------------------------------------------------------------------------

void EscapeXML(string_t & str)
{
    // Replace & with &amp;
    // Replace < with &lt;
    // Replace > with &gt;
    // Replace \r\n | \r | \n with <br />
    size_t start = 0;
    size_t index = str.find_first_of(puzT("&<>\r\n"), start);
    if (index == string_t::npos)
        return; // Shortcut if we don't have anything to replace.
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
    str.assign(ret);
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


void UnescapeXML(string_t & str)
{
    // Replace &amp; with &
    // Replace &lt; with <
    // Replace &gt; with >
    // Replace <br /> with \n
    size_t start = 0;
    size_t index = str.find_first_of(puzT("<&"), start);
    if (index == string_t::npos)
        return; // Shortcut if we don't have anything to replace.
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
            // &amp;
            // &lt;
            // &gt;
            case puzT('&'):
            {
                size_t semi = str.find(puzT(';'), index);
                if (semi != string_t::npos)
                {
                    string_t substr = str.substr(index, semi - index);
                    if (substr == puzT("amp"))
                        ret.append(puzT("&"));
                    else if (substr == puzT("lt"))
                        ret.append(puzT("<"));
                    else if (substr == puzT("gt"))
                        ret.append(puzT(">"));
                    start = semi+1;
                }
                else
                {
                    start = index+1;
                }
            }
                break;
        }
        index = str.find_first_of(puzT("<&"), start);
    }
    ret.append(str.substr(start));
    str.assign(ret);
}


// ---------------------------------------------------------------------------
// IsFormatted: Checks XML validity, and checks tags against a list
// ---------------------------------------------------------------------------

// The comparison function (case-insensitive)
typedef bool (*allowedElements_key_cmp)(const std::string &, const std::string &);
static bool allowedElements_cmp(const std::string & a, const std::string & b)
{
    return stricmp(a.c_str(), b.c_str()) < 0;
}

// The elements we can handle
static std::string elems[] = {
    "a",
    "b",
    "br",
    "center",
    "code",
    "em",
    "font",
    "h1",
    "h2",
    "h3",
    "h4",
    "h5",
    "h6",
    "i",
    "p",
    "pre",
    "small",
    "strike",
    "strong",
    "sub",
    "sup",
    "tt",
    "u",
};

// This messy bit here sets the comparison function and initializes
// the set with the above array.
static std::set<std::string, allowedElements_key_cmp>
    allowedElements(elems + 0,
                    elems + sizeof(elems) / sizeof(std::string),
                    allowedElements_cmp);

static void XMLCALL CheckElement(void *, const XML_Char * name, const XML_Char **)
{
    // Check to see if this is an allowed element, throw an exception
    // if not
    if (allowedElements.find(name) == allowedElements.end())
        throw std::exception();
}


bool IsFormatted(const std::string & str)
{
    XML_Parser p = XML_ParserCreate("UTF-8");
    XML_SetStartElementHandler(p, CheckElement);
    try
    {
        // Feed expat a dummy root node, because XML requires
        // some kind of root element.
        if (! XML_Parse(p, "<p>", 3, false))
            throw std::exception();
        if (! XML_Parse(p, str.c_str(), str.size(), false))
            throw std::exception();
        if (! XML_Parse(p, "</p>", 4, true))
            throw std::exception();
    }
    catch (...)
    {
        XML_ParserFree(p);
        return false;
    }
    XML_ParserFree(p);
    return true;
}


} // namespace puz
