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

#ifndef PUZ_XML_H
#define PUZ_XML_H

#include "Puzzle.hpp"
#include "puzstring.hpp"
#include "pugixml/pugixml.hpp"

namespace puz {
namespace xml {

typedef pugi::xml_document document;
typedef pugi::xml_node node;
typedef pugi::xml_attribute attribute;

class Parser
{
public:
    Parser() {}
    virtual ~Parser() {}

    void LoadFromFilename(Puzzle * puz, const std::string & filename);
    void LoadFromString(Puzzle * puz, const char * str);
    void LoadFromStream(Puzzle * puz, std::istream & stream);

    // Override this to load the actual puzzle given an xml::document.
    // Any values that are used by the puzzle should be removed so that
    // the remaining values can be stored as extra data in the puzzle object.
    // Return true if your parser subclass will take responsibility for the
    // root pointer (e.g. the pointer is stored as Puzzle::FormatData)
    virtual bool DoLoadPuzzle(Puzzle * puz, document & doc) =0;

    // Utility functions
    //------------------

    // Throw an exception if the child node does not exist.  Return the node
    node RequireChild(node &, const char * name);

    // Text functions

    string_t GetText(node &);
    string_t GetAttribute(node &, const char * name);
    string_t GetInnerXML(node &);

    // Child text
    inline string_t GetText(node & n, const char * name)
        { return GetText(n.child(name)); }

    // Child InnerXML
    string_t GetInnerXML(const node & n, const char * name)
        { return GetInnerXML(n.child(name)); }
};

inline node
Parser::RequireChild(node & n, const char * name)
{
    node child = n.child(name);
    if (! child)
        throw FileError(std::string("Missing required element: \"") + name + "\"");
    return child;
}

inline string_t
Parser::GetAttribute(node & n, const char * name)
{
    return decode_utf8(n.attribute(name).value());
}

} // namespace xml
} // namespace puz

#endif // PUZ_XML_H
