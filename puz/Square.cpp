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


#include "Square.hpp"
#include "Grid.hpp"

namespace puz {

Square::Square(Grid * grid)
    : m_col(-1),
      m_row(-1),
      m_asciiSolution('\0'),
      m_solution(""),
      m_text(""),
      m_flag(FLAG_CLEAR),
      m_number(0),
      m_grid(grid)
{
    assert(grid != NULL);

    m_next[ACROSS][PREV] = NULL;
    m_next[ACROSS][NEXT] = NULL;
    m_next[DOWN]  [PREV] = NULL;
    m_next[DOWN]  [NEXT] = NULL;

    m_isLast[ACROSS][PREV] = false;
    m_isLast[ACROSS][NEXT] = false;
    m_isLast[DOWN]  [PREV] = false;
    m_isLast[DOWN]  [NEXT] = false;

    m_wordStart[ACROSS] = NULL;
    m_wordEnd  [ACROSS] = NULL;
    m_wordStart[DOWN]   = NULL;
    m_wordEnd  [DOWN]   = NULL;
}

Square::Square(const Square & other)
    : m_col(other.m_col),
      m_row(other.m_row),
      m_asciiSolution(other.m_asciiSolution),
      m_solution(other.m_solution),
      m_text(other.m_text),
      m_flag(other.m_flag),
      m_number(other.m_number),
      m_grid(other.m_grid)
{
    m_next[ACROSS][PREV] = other.m_next[ACROSS][PREV];
    m_next[ACROSS][NEXT] = other.m_next[ACROSS][NEXT];
    m_next[DOWN]  [PREV] = other.m_next[DOWN]  [PREV];
    m_next[DOWN]  [NEXT] = other.m_next[DOWN]  [NEXT];

    m_isLast[ACROSS][PREV] = other.m_isLast[ACROSS][PREV];
    m_isLast[ACROSS][NEXT] = other.m_isLast[ACROSS][NEXT];
    m_isLast[DOWN]  [PREV] = other.m_isLast[DOWN]  [PREV];
    m_isLast[DOWN]  [NEXT] = other.m_isLast[DOWN]  [NEXT];

    m_wordStart[ACROSS] = other.m_wordStart[ACROSS];
    m_wordEnd  [ACROSS] = other.m_wordEnd  [ACROSS];
    m_wordStart[DOWN]   = other.m_wordStart[DOWN];
    m_wordEnd  [DOWN]   = other.m_wordEnd  [DOWN];
}

// Since all constructors are private, only a grid can
// create squares.  Thus, this function will only be called
// as follows:
//     grid1.At(col, row) = grid2.At(col, row);
// In order to preserve iteration, we need to prevent
// certain values from being copied:
//    col
//    row
//    grid
//    next
//    isLast
// Word start and end and square number can be preserved if we
// are copying white square to white square or
// black square to black square.
// Otherwise we need to roll back the grid state flag one step.
Square & Square::operator=(const Square & other)
{
    // One square is white and the other is black.
    // Erase word start and end and clue numbers.
    if (! IsOk() || IsBlack() != other.IsBlack())
    {
#ifndef NDEBUG
        m_wordStart[ACROSS] = NULL;
        m_wordEnd  [ACROSS] = NULL;
        m_wordStart[DOWN]   = NULL;
        m_wordEnd  [DOWN]   = NULL;
        m_number = 0;
#endif
        // Roll back the grid state flag if it is already
        // past the iteration state.
        if (m_grid->m_gridState > Grid::GRID_ITERATION)
            m_grid->m_gridState = Grid::GRID_ITERATION;
    }

    m_asciiSolution = other.m_asciiSolution;
    m_solution = other.m_solution;
    m_text = other.m_text;
    m_flag = other.m_flag;

    return *this;
}




// Replacement characters:

// A-Z and a-z are converted to upper case

// Symbols and numbers are replaced with their first letter:
// # = H[ash],
// $ = D[ollar],
// % = P[ercent],
// & = A[mpersand],
// + = P[lus],
// ? = Q[uestion]

// '[' number ']' is used to indicate a webdings symbol; these are replaced
// by a blank ('-') in the user grid.

// '.' indicates a black square.

// All others are invalid in the grid and cannot be converted.

const char squareChars[] = {
/*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f 
/* 0 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* 1 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* 2 */  '-',   0,   0, 'H', 'D', 'P', 'A',   0,   0,   0,   0, 'P',   0,   0, '.',   0,
/* 3 */  'Z', 'O', 'T', 'T', 'F', 'F', 'S', 'S', 'E', 'N',   0,   0,   0,   0,   0, 'Q',
/* 4 */  'A', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
/* 5 */  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '-',   0,   0,   0,   0,
/* 6 */    0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
/* 7 */  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',   0,   0,   0,   0,   0,
/* 8 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* 9 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* a */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* b */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* c */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* d */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* e */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* f */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

//------------------------------------------------------------------------------
// Character table functions (static)
//------------------------------------------------------------------------------

char
Square::ToAscii(char ch)
{
    assert(IsValidChar(ch));
    return squareChars[static_cast<unsigned char>(ch)];
}


bool
Square::IsValidChar(char ch)
{
    return squareChars[static_cast<unsigned char>(ch)] != 0;
}



void
Square::SetPlainSolution(char solution)
{
    // Valid characters for the plain solution
    if (! (isupper(solution) || solution == '.'))
        throw InvalidString(std::string(solution, 1));

    // Check to see if we are changing from black to white
    // or vice-versa.
    if (! IsOk() || IsBlack() != (solution == '.'))
    {
        // Roll back the grid state flag if it is already
        // past the iteration state.
        if (m_grid->m_gridState > Grid::GRID_ITERATION)
            m_grid->m_gridState = Grid::GRID_ITERATION;
    }

    m_asciiSolution = solution;
}

} // namespace puz
