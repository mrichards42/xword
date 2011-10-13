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

#include "xml.hpp"
#include <sstream>
#include <fstream>

namespace puz {
namespace xml {

void Parser::LoadFromFilename(Puzzle * puz, const std::string & filename)
{
    std::ifstream stream(filename.c_str(),
                         std::ios_base::binary | std::ios_base::in);
    if (stream.fail())
        throw FileError(filename);
    LoadFromStream(puz, stream);
}

void Parser::LoadFromString(Puzzle * puz, const char * str)
{
    std::auto_ptr<pugi::xml_document> doc(new pugi::xml_document);
    pugi::xml_parse_result result = doc->load(str);

    if (! result)
        throw FileTypeError("xml");

    if (DoLoadPuzzle(puz, *doc))
        doc.release();
}

void Parser::LoadFromStream(Puzzle * puz, std::istream & stream)
{
    std::auto_ptr<pugi::xml_document> doc(new pugi::xml_document);
    pugi::xml_parse_result result = doc->load(stream);

    if (! result)
        throw FileTypeError("xml");

    if (DoLoadPuzzle(puz, *doc))
        doc.release();
}


// Utility functions
void TextToStream(node & n, std::ostringstream & stream)
{
    for (node child = n.first_child(); child; child = child.next_sibling())
    {
        pugi::xml_node_type type = child.type();
        if (type == pugi::node_pcdata || type == pugi::node_cdata)
            stream << child.value();
        else
            TextToStream(child, stream);
    }
}

string_t Parser::GetText(node & n)
{
    std::ostringstream stream;
    TextToStream(n, stream);
    return decode_utf8(stream.str());
}

string_t Parser::GetInnerXML(node & n)
{
    std::ostringstream stream;
    for (node child = n.first_child(); child; child = child.next_sibling())
        child.print(stream, "", pugi::format_raw);
    return decode_utf8(stream.str());
}

void SetInnerXML(node & node, const string_t & innerxml)
{
    // Parse the XML, and add it as a child.
    // We need to add a dummy xml wrapper element so that plain text still
    // parses.
    pugi::xml_document doc;
    std::string temp("<dummy>");
    temp.append(encode_utf8(innerxml)).append("</dummy>");
    pugi::xml_parse_result result = doc.load(temp.c_str());
    if (! result)
        SetText(node, innerxml);
    else
        node.append_copy(doc.first_child().first_child());
}

} // namespace xml
} // namespace puz
