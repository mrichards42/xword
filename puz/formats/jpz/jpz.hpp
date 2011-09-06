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

#ifndef PUZ_FORMAT_JPZ_H
#define PUZ_FORMAT_JPZ_H

#include "Puzzle.hpp"
#include <string>
#include "parse/xml.hpp"

namespace puz {

class JpzData : public Puzzle::FormatData
{
public:
    JpzData(xml::document * doc_) : doc(doc_) {}
    ~JpzData() { delete doc; }
    xml::document * doc;
};

void LoadJpz(Puzzle * puz, const std::string & filename, void * /* dummy */);
void SaveJpz(Puzzle * puz, const std::string & filename, void * /* dummy */);

} // namespace puz

#endif // PUZ_FORMAT_JPZ_H
