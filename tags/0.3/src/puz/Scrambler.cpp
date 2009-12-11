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


#include "Scrambler.hpp"
#include "XGrid.hpp"
#include <wx/string.h>
#include <ctime>
#include "Checksummer.hpp"

#include <wx/log.h>

XGridScrambler::XGridScrambler(XGrid & grid)
    : m_grid(grid)
{}



//------------------------------------------------------------------------------
// Scrambling functions
//------------------------------------------------------------------------------

bool
XGridScrambler::ScrambleSolution(unsigned short key_int)
{
    bool ok = m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0;
    ok = ok && (m_grid.m_flag & XFLAG_NO_SOLUTION) == 0;

    wxASSERT(m_grid.First() != NULL);

    if (! ok)
        return false;

    if (key_int == 0)
        key_int = MakeKey();

    wxASSERT(1000 <= key_int && key_int <= 9999);

    // Read the key into an array of single digits
    unsigned char key[4];
    key[0] = int(key_int / 1000) % 10;
    key[1] = int(key_int / 100)  % 10;
    key[2] = int(key_int / 10)   % 10;
    key[3] = int(key_int / 1)    % 10;

    wxString solution = GetSolutionDown();

    unsigned short cksum = Checksummer::cksum_region(solution, 0);
    if (cksum == 0)
        return false;

    size_t length = solution.length();

    // Don't scramble really small puzzles
    if(length < 12)
        return 0;

    // Do the scrambling
    for (int i = 0; i < 4; ++i)
    {
        wxString scramble_part;
        for (size_t j = 0; j < length; ++j)
        {
            wxChar letter = solution[j] + key[j % 4];

            // Make sure the letter is capital
            if (letter > 90)
                letter -= 26;

            // Range for capital letters
            wxASSERT(65 <= letter && letter <= 90);

            scramble_part.Append(letter);
        }
        wxASSERT(scramble_part.length() == length);

        scramble_part = ShiftString(scramble_part, key[i]);
        wxASSERT(scramble_part.length() == length);

        scramble_part = ScrambleString(scramble_part);
        wxASSERT(scramble_part.length() == length);

        solution = scramble_part;
    }


    // Save the scrambled solution to the puzzle file
    wxString::iterator it = solution.begin();
    for (XSquare * square = m_grid.First();
         square != NULL;
         square = square->Next(DIR_DOWN))
    {
        if (square->IsBlack())
            continue;

        // Make sure we preserve any rebus in the solution
        if (! square->HasSolutionRebus())
            square->SetSolution(*it);
        else
            square->SetPlainSolution(*it);
        ++it;
    }
    wxASSERT(it == solution.end());

    m_grid.m_flag |= XFLAG_SCRAMBLED;
    m_grid.m_cksum = cksum;
    m_grid.m_key = key_int;

    return true;
}


unsigned short
XGridScrambler::MakeKey()
{
    // If we already have a key, use that one
    if (m_grid.m_key != 0)
        return m_grid.m_key;

    // Generate 4 random numbers for the key
    unsigned short key;
    key = rand() % 9 + 1;
    key = key * 10 + rand() % 10;
    key = key * 10 + rand() % 10;
    key = key * 10 + rand() % 10;

    return key;
}




// Static functions
//-----------------

// Shift the string by keynum characters (wraps around)
wxString
XGridScrambler::ShiftString(const wxString & str, unsigned char keynum)
{
    wxASSERT(str.length() > 0);

    return str.substr(keynum) + str.substr(0, keynum);
}


// Split the string in two, then reassemble with successive characters from the
// split string
wxString
XGridScrambler::ScrambleString(const wxString & str)
{
    wxString ret;

    size_t mid = str.length() / 2;

    wxString front = str.substr(0, mid);
    wxString back  = str.substr(mid);

    size_t i;
    for (i = 0; i < mid; ++i)
    {
        ret.Append(back [i]);
        ret.Append(front[i]);
    }

    // This is the case if str.length() is an odd number
    if (i < back.length())
        ret.Append(back[i]);

    return ret;
}







//------------------------------------------------------------------------------
// Unscrambling functions
//
// These are almost exactly the scrambling functions run backward
//------------------------------------------------------------------------------

bool
XGridScrambler::UnscrambleSolution(unsigned short key_int)
{
    bool ok = m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0;
    ok = ok && (m_grid.m_flag & XFLAG_NO_SOLUTION) == 0;

    wxASSERT(1000 <= key_int && key_int <= 9999);
    wxASSERT(m_grid.First() != NULL);

    if (! ok)
        return false;

    // Read the key into an array of single digits
    unsigned char key[4];
    key[0] = int(key_int / 1000) % 10;
    key[1] = int(key_int / 100)  % 10;
    key[2] = int(key_int / 10)   % 10;
    key[3] = int(key_int / 1)    % 10;

    wxString solution = GetSolutionDown();

    size_t length = solution.length();

    // Don't unscramble really small puzzles
    if(length < 12)
        return 0;

    // Do the unscrambling
    for (int i = 3; i >= 0; --i)
    {
        solution = UnscrambleString(solution);
        wxASSERT(solution.length() == length);

        solution = UnshiftString(solution, key[i]);
        wxASSERT(solution.length() == length);

        wxString unscramble_part;

        for (size_t j = 0; j < length; ++j)
        {
            wxChar letter = solution[j] - key[j % 4];

            // Make sure the letter is capital
            if (letter < 65)
                letter += 26;

            // Range for capital letters
            wxASSERT(65 <= letter && letter <= 90);

            unscramble_part.Append(letter);
        }
        wxASSERT(unscramble_part.length() == length);

        solution = unscramble_part;
    }

    unsigned short cksum = Checksummer::cksum_region(solution, 0);

    if (cksum != m_grid.m_cksum)
        return false;

    // Save the unscrambled solution to the grid
    wxString::iterator it = solution.begin();
    for (XSquare * square = m_grid.First();
         square != NULL;
         square = square->Next(DIR_DOWN))
    {
        if (square->IsBlack())
            continue;

        // Preserve any rebus in the solution
        if (! square->HasSolutionRebus())
            square->SetSolution(*it);
        else
            square->SetPlainSolution(*it);
        ++it;
    }
    wxASSERT(it == solution.end());

    // Reset the flags
    m_grid.m_flag &= ~XFLAG_SCRAMBLED;
    m_grid.m_cksum = 0;
    m_grid.m_key = 0;

    return true;
}






// Static functions
//-----------------

// Shift the string by -keynum characters (wraps around)
wxString
XGridScrambler::UnshiftString(const wxString & str, unsigned char keynum)
{
    wxASSERT(str.length() > 0);

    return str.substr(str.length() - keynum)
         + str.substr(0, str.length() - keynum);
}



// Every other byte goes to the back or front of the string
wxString
XGridScrambler::UnscrambleString(const wxString & str)
{
    size_t length = str.length();

    wxString front;
    wxString back;

    size_t i;
    for (i = 0; i < length; ++i)
    {
        if (i % 2 == 0)
            back.Append(str[i]);
        else
            front.Append(str[i]);
    }

    return front + back;
}











//------------------------------------------------------------------------------
// General functions
//------------------------------------------------------------------------------

// Returns a character array with the solution to all white squares
// Note that this is arranged by moving DOWN instead of the normal ACROSS
// This is nul-terminated
wxString
XGridScrambler::GetSolutionDown()
{
    wxASSERT(m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0);

    wxString ret;

    for (XSquare * square = m_grid.First();
         square != NULL;
         square = square->Next(DIR_DOWN))
    {
        if (square->IsWhite())
            ret.append(static_cast<wxChar>(square->GetPlainSolution()));
    }

    return ret;
}


// Same as GetSolutionDown, but for the user grid
wxString
XGridScrambler::GetUserGridDown()
{
    wxASSERT(m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0);

    wxString ret;

    for (XSquare * square = m_grid.First();
         square != NULL;
         square = square->Next(DIR_DOWN))
    {
        if (square->IsWhite())
            ret.append(static_cast<wxChar>(square->GetPlainText()));
    }

    return ret;
}



bool
XGridScrambler::CheckUserGrid(XGrid & grid)
{
    XGridScrambler scrambler(grid);
    unsigned short cksum =
            Checksummer::cksum_region(scrambler.GetUserGridDown(), 0);
    return cksum == grid.GetCksum();
}



unsigned short
XGridScrambler::BruteForceUnscramble()
{
    for (unsigned short i = 1000; i <= 9999; ++i)
    {
        if (UnscrambleSolution(i))
            return i;
    }
    return 0;
}