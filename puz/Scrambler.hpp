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


#ifndef PUZ_SCRAMBLER_H
#define PUZ_SCRAMBLER_H

// Scrambler uses 8-bit strings
#include <string>

namespace puz {

class PUZ_API Grid;

class PUZ_API Scrambler
{
public:
    Scrambler(Grid & grid);
    ~Scrambler() {}

    // ScrambleSolution() will set m_key and m_cksum on the XGrid
    // If key != 0, use the provided key, otherwise create one
    bool ScrambleSolution(unsigned short key_int = 0);
    bool UnscrambleSolution(unsigned short key_int);

    // Check a scrambled grid to see if it is correct
    static bool CheckUserGrid(Grid & grid);

private:
    Grid & m_grid;

    std::string GetSolutionDown();
    std::string GetUserGridDown();

    // Scrambling functions
    unsigned short MakeKey();

    static std::string ShiftString(const std::string & str,
                                   unsigned char keynum);
    static std::string ScrambleString(const std::string & str);

    // Unscrambling functions
    static std::string UnshiftString(const std::string & str,
                                     unsigned char keynum);
    static std::string UnscrambleString(const std::string & str);
};

} // namespace puz

#endif // PUZ_SCRAMBLER_H
