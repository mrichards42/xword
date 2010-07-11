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
#include "Grid.hpp"
#include <string>
#include <ctime>
#include "Checksummer.hpp"
#include <cassert>

namespace puz {

Scrambler::Scrambler(Grid & grid)
    : m_grid(grid)
{}



//------------------------------------------------------------------------------
// Scrambling functions
//------------------------------------------------------------------------------

bool
Scrambler::ScrambleSolution(unsigned short key_int)
{
    bool ok = m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0;
    ok = ok && (m_grid.m_flag & FLAG_NO_SOLUTION) == 0;

    assert(m_grid.First() != NULL);

    if (! ok)
        return false;

    if (key_int == 0)
        key_int = MakeKey();

    assert(1000 <= key_int && key_int <= 9999);

    // Read the key into an array of single digits
    unsigned char key[4];
    key[0] = int(key_int / 1000) % 10;
    key[1] = int(key_int / 100)  % 10;
    key[2] = int(key_int / 10)   % 10;
    key[3] = int(key_int / 1)    % 10;

    std::string solution = GetSolutionDown();

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
        std::string scramble_part;
        for (size_t j = 0; j < length; ++j)
        {
            char letter = solution[j] + key[j % 4];

            // Make sure the letter is capital
            if (letter > 90)
                letter -= 26;

            // Range for capital letters
            assert(isupper(letter));

            scramble_part.push_back(letter);
        }
        assert(scramble_part.length() == length);

        scramble_part = ShiftString(scramble_part, key[i]);
        assert(scramble_part.length() == length);

        scramble_part = ScrambleString(scramble_part);
        assert(scramble_part.length() == length);

        solution = scramble_part;
    }


    // Save the scrambled solution to the puzzle file
    std::string::iterator it = solution.begin();
    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next(DOWN))
    {
        if (square->IsBlack())
            continue;

        // Make sure we preserve any rebus in the solution
        if (! square->HasSolutionRebus())
            square->SetSolution(std::string(1, *it));
        else
            square->SetPlainSolution(*it);
        ++it;
    }
    assert(it == solution.end());

    m_grid.m_flag |= FLAG_SCRAMBLED;
    m_grid.m_cksum = cksum;
    m_grid.m_key = key_int;

    return true;
}


unsigned short
Scrambler::MakeKey()
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
std::string
Scrambler::ShiftString(const std::string & str, unsigned char keynum)
{
    assert(str.length() > 0);

    return str.substr(keynum) + str.substr(0, keynum);
}


// Split the string in two, then reassemble with successive characters from the
// split string
std::string
Scrambler::ScrambleString(const std::string & str)
{
    std::string ret;

    size_t mid = str.length() / 2;

    std::string front = str.substr(0, mid);
    std::string back  = str.substr(mid);

    size_t i;
    for (i = 0; i < mid; ++i)
    {
        ret.push_back(back [i]);
        ret.push_back(front[i]);
    }

    // This is the case if str.length() is an odd number
    if (i < back.length())
        ret.push_back(back[i]);

    return ret;
}







//-----------------------------------------------------------------------------
// Unscrambling functions
//
// These are almost exactly the scrambling functions run backward
//-----------------------------------------------------------------------------

bool
Scrambler::UnscrambleSolution(unsigned short key_int)
{
    bool ok = m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0;
    ok = ok && (m_grid.m_flag & FLAG_NO_SOLUTION) == 0;

    assert(1000 <= key_int && key_int <= 9999);
    assert(m_grid.First() != NULL);

    if (! ok)
        return false;

    // Read the key into an array of single digits
    unsigned char key[4];
    key[0] = int(key_int / 1000) % 10;
    key[1] = int(key_int / 100)  % 10;
    key[2] = int(key_int / 10)   % 10;
    key[3] = int(key_int / 1)    % 10;

    std::string solution = GetSolutionDown();

    size_t length = solution.length();

    // Don't unscramble really small puzzles
    if(length < 12)
        return 0;

    // Do the unscrambling
    for (int i = 3; i >= 0; --i)
    {
        solution = UnscrambleString(solution);
        assert(solution.length() == length);

        solution = UnshiftString(solution, key[i]);
        assert(solution.length() == length);

        std::string unscramble_part;

        for (size_t j = 0; j < length; ++j)
        {
            char letter = solution[j] - key[j % 4];

            // Make sure the letter is capital
            if (letter < 65)
                letter += 26;

            assert(isupper(letter));

            unscramble_part.push_back(letter);
        }
        assert(unscramble_part.length() == length);

        solution = unscramble_part;
    }

    unsigned short cksum = Checksummer::cksum_region(solution, 0);

    if (cksum != m_grid.m_cksum)
        return false;

    // Save the unscrambled solution to the grid
    std::string::iterator it = solution.begin();
    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next(DOWN))
    {
        if (square->IsBlack())
            continue;

        // Preserve any rebus in the solution
        if (! square->HasSolutionRebus())
            square->SetSolution(std::string(1, *it));
        else
            square->SetPlainSolution(*it);
        ++it;
    }
    assert(it == solution.end());

    // Reset the flags
    m_grid.m_flag &= ~FLAG_SCRAMBLED;
    m_grid.m_cksum = 0;
    m_grid.m_key = 0;

    return true;
}






// Static functions
//-----------------

// Shift the string by -keynum characters (wraps around)
std::string
Scrambler::UnshiftString(const std::string & str, unsigned char keynum)
{
    assert(str.length() > 0);

    return str.substr(str.length() - keynum)
         + str.substr(0, str.length() - keynum);
}



// Every other byte goes to the back or front of the string
std::string
Scrambler::UnscrambleString(const std::string & str)
{
    size_t length = str.length();

    std::string front;
    std::string back;

    size_t i;
    for (i = 0; i < length; ++i)
    {
        if (i % 2 == 0)
            back.push_back(str[i]);
        else
            front.push_back(str[i]);
    }

    return front + back;
}











//------------------------------------------------------------------------------
// General functions
//------------------------------------------------------------------------------

// Returns a character array with the solution to all white squares
// Note that this is arranged by moving DOWN instead of the normal ACROSS
// This is nul-terminated
std::string
Scrambler::GetSolutionDown()
{
    assert(m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0);

    std::string ret;

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next(DOWN))
    {
        if (square->IsWhite())
            ret.push_back(square->GetPlainSolution());
    }

    return ret;
}


// Same as GetSolutionDown, but for the user grid
std::string
Scrambler::GetUserGridDown()
{
    assert(m_grid.GetWidth() > 0 && m_grid.GetHeight() > 0);

    std::string ret;

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next(DOWN))
    {
        if (square->IsWhite())
            ret.push_back(square->GetPlainText());
    }

    return ret;
}



bool
Scrambler::CheckUserGrid(Grid & grid)
{
    Scrambler scrambler(grid);
    unsigned short cksum =
            Checksummer::cksum_region(scrambler.GetUserGridDown(), 0);
    return cksum == grid.GetCksum();
}


} // namespace puz
