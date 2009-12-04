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


#ifndef X_GRID_SCRAMBLER_H
#define X_GRID_SCRAMBLER_H

class XGrid;
class wxString;

class XGridScrambler
{
public:
    XGridScrambler(XGrid & grid);
    ~XGridScrambler() {}

    // ScrambleSolution() will set m_key and m_cksum on the XGrid
    // If key != 0, use the provided key, otherwise create one
    bool ScrambleSolution(unsigned short key_int = 0);
    bool UnscrambleSolution(unsigned short key_int);

    // Try all keys from 1000 - 9999.
    // Return the key if it works, otherwise return 0.
    unsigned short BruteForceUnscramble();

    // Check a scrambled grid to see if it is correct
    static bool CheckUserGrid(XGrid & grid);

private:
    XGrid & m_grid;

    wxString GetSolutionDown();
    wxString GetUserGridDown();

    // Scrambling functions
    unsigned short MakeKey();

    static wxString ShiftString(const wxString & str, unsigned char keynum);
    static wxString ScrambleString(const wxString & str);

    // Unscrambling functions
    static wxString UnshiftString(const wxString & str, unsigned char keynum);
    static wxString UnscrambleString(const wxString & str);
};


#endif // X_GRID_SCRAMBLER_H