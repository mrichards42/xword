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

#ifndef PUZ_FORMAT_IPUZ_H
#define PUZ_FORMAT_IPUZ_H

#include "Puzzle.hpp"
#include <string>
#include "parse/json.hpp"

namespace puz {
    namespace json {
    class Value;
}

class ipuzData : public Puzzle::FormatData
{
public:
    ipuzData(json::Value * root = NULL) : root(root) {}
    ~ipuzData() { delete root; }
    json::Value * root;
};

void LoadIpuz(Puzzle * puz, const std::string & filename, void * /* dummy */);
void LoadIpuzString(Puzzle * puz, const std::string & data);

} // namespace puz

#endif // PUZ_FORMAT_IPUZ_H
