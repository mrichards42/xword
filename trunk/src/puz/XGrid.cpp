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


//------------------------------------------------------------------------------
//
// XGrid: a container class for a crossword grid
// Primary function is to hold a 2d array of XSquares
//
// Diagram of our organization of the grid (as in a python list)
// This is organized into a vector, m_grid.
// Access is obtained internally as m_grid[row][col], but externally as
//   m_grid.at(row, col), i.e. m_grid.at(x, y)
// This organization is important to remember for all functions, but 
//   especially for SetSize(width, height), which must first change the
//   height (cols), then loop through each line (rows) and resize each one
//
// [ [ELGAR.ABCTV.NOW],
//   [FOLIO.DELHI.OUR],
//   [FOURSISTERS.UZI],
//   [SKEWER..FEARNOT],
//   [...ARABS.AVE...],
//   [.FIVEQUESTIONS.],
//   [ARMED.MATES.APE],
//   [HOPS.APRON.PAIN],
//   [ALE.UNPIN.ARCED],
//   [.FIFTYONESTEPS.],
//   [...DEW.GROAN...],
//   [PIRANHA..ACTIVA],
//   [USA.SEVENPOINTS],
//   [TAJ.IRONY.SCREE],
//   [SKA.LENOX.TEENA]  ]
//
//------------------------------------------------------------------------------

#include "XGrid.hpp"
#include <map>
#include <wx/log.h>
#include "Scrambler.hpp"


XGrid::XGrid(size_t width, size_t height)
    : m_width(0), m_height(0)
{
    Clear();
    if (width > 0 || height > 0)
        SetSize(width, height);
}


XGrid::~XGrid()
{
}


void
XGrid::SetSize(size_t width, size_t height)
{
    m_grid.resize(height);

    // If the width changes, resize all of the nested vectors
    if (width != m_width)
    {
        Grid_t::iterator it;
        for (it = m_grid.begin(); it != m_grid.end(); ++it)
            it->resize(width);
    }
    // Otherwise, if the height has increased, resize all of the new vectors
    else if (height > m_height)
    {
        // resize the remaining rows
        Grid_t::iterator it;
        for (it = m_grid.begin() + m_height; it != m_grid.end(); ++it)
            it->resize(width);
    }

    m_height = height;
    m_width  = width;

    // Recalculate next and previous grid squares
    //----------------------------------------------
    for (size_t row = 0; row < GetHeight(); ++row)
    {
        for (size_t col = 0; col < GetHeight(); ++col)
        {
            XSquare & square = At(col, row);
            square.col = col;
            square.row = row;

            // Two special cases: top left and bottom right corners
            if (row == 0 && col == 0)
            {
                m_first = &square;
                square.m_next[DIR_ACROSS][FIND_PREV] = NULL;
                square.m_next[DIR_DOWN]  [FIND_PREV] = NULL;
                square.m_next[DIR_ACROSS][FIND_NEXT] = &At(col + 1, row);
                square.m_next[DIR_DOWN]  [FIND_NEXT] = &At(col,     row + 1);
            }
            else if (row == LastRow() && col == LastCol())
            {
                m_last = &square;
                square.m_next[DIR_DOWN]  [FIND_PREV] = &At(col,     row - 1);
                square.m_next[DIR_ACROSS][FIND_PREV] = &At(col - 1, row);
                square.m_next[DIR_DOWN]  [FIND_NEXT] = NULL;
                square.m_next[DIR_ACROSS][FIND_NEXT] = NULL;
            }

            // Otherwise respect the wrapping,
            // so we can iterate over all squares
            else
            {
                square.m_next[DIR_DOWN]  [FIND_PREV] = 
                    row > 0 ? 
                        &At(col,     row - 1)
                      : &At(col - 1, LastRow());

                square.m_next[DIR_DOWN]  [FIND_NEXT] =
                    row < LastRow() ? 
                        &At(col,     row + 1)
                      : &At(col + 1, 0);

                square.m_next[DIR_ACROSS][FIND_PREV] =
                    col > 0 ? 
                        &At(col - 1,   row)
                      : &At(LastCol(), row - 1);

                square.m_next[DIR_ACROSS][FIND_NEXT] =
                    col < LastCol() ?
                        &At(col + 1, row)
                      : &At(0,       row + 1);
            }

            // End of col/rows:
            square.m_isLast[DIR_ACROSS][FIND_NEXT] =  col == LastCol();
            square.m_isLast[DIR_DOWN]  [FIND_NEXT] =  row == LastRow();
            square.m_isLast[DIR_ACROSS][FIND_PREV] =  col == 0;
            square.m_isLast[DIR_DOWN]  [FIND_PREV] =  row == 0;
        }
    }
}



bool
XGrid::ScrambleSolution(unsigned short key)
{
    XGridScrambler scrambler(*this);
    return scrambler.ScrambleSolution(key);
}


bool
XGrid::UnscrambleSolution(unsigned short key)
{
    XGridScrambler scrambler(*this);
    return scrambler.UnscrambleSolution(key);
}



void
XGrid::SetSolution(const char * solution)
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->solution = *solution++;

    SetupGrid();
}

void
XGrid::SetGrid(const char * grid)
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->text = *grid++;
}

void
XGrid::SetGext(const unsigned char * gext)
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->flag = *gext++;
}


void
XGrid::ClearSolution()
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->solution = '.';
}


void
XGrid::ClearGrid()
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->text = square->IsBlack() ? '.' : '-';
}

void
XGrid::ClearGext()
{
    for (XSquare * square = First(); square != NULL; square = square->Next())
        square->flag = XFLAG_CLEAR;
}








wxString
XGrid::GetSolution() const
{
    wxString solution;

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        solution.Append(square->solution);
    }

    return solution;
}

wxString
XGrid::GetGrid() const
{
    wxString grid;

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        grid.Append(square->text);
    }

    return grid;
}


wxString
XGrid::GetGext() const
{
    wxString gext;

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        gext.Append(square->flag);
    }

    return gext;
}


#include <wx/stopwatch.h>
#include <wx/log.h>


// Setup the entire grid:
//  (1) assigns clue numbers and flags to each square (0 and NO_CLUE if none)
//  (2) create clue maps that hold the across and down clue numbers and their start and end squares
//  (3) assigns the word start and end to each square
void
XGrid::SetupGrid()
{
    wxASSERT(GetWidth() > 0 && GetHeight() > 0);

    wxStopWatch sw;

    std::map<size_t, XSquare *> wordStart[2];
    std::map<size_t, XSquare *> wordEnd[2];

    wordStart[DIR_ACROSS][0] = NULL;
    wordEnd  [DIR_ACROSS][0] = NULL;

    wordStart[DIR_DOWN][0] = NULL;
    wordEnd  [DIR_DOWN][0] = NULL;

    m_firstWhite = NULL;
    m_lastWhite  = NULL;

    size_t clueNumber = 1;

    for (XSquare * square = First(); square != NULL; square = square->Next())
    {
        if (square->IsBlack())
        {
            square->clueFlag = NO_CLUE;
            square->clue[DIR_ACROSS] = 0;
            square->clue[DIR_DOWN]   = 0;
            square->number           = 0;
        }
        else
        {
            // Clues are located in squares where a black square (or the edge)
            //   preceeds and where a white square follows.

            // Test across and down clues
            for (int direction = 0; direction < 2; ++direction)
            {
                if (   square->IsLast(direction, FIND_PREV)
                    || square->Prev(direction)->IsBlack())
                {
                    if (   ! square->IsLast(direction, FIND_NEXT)
                        && square->Next(direction)->IsWhite())
                    {
                        if (direction == DIR_ACROSS)
                            square->clueFlag |= ACROSS_CLUE;
                        else
                            square->clueFlag |= DOWN_CLUE;

                        square->clue[direction] = clueNumber;
                        wordStart[direction][clueNumber] = square;
                    }
                }
                else if (square->Prev(direction)->IsWhite())
                {
                    // Fill in the clue number from the previous square
                    square->clue[direction] =
                        square->Prev(direction)->clue[direction];
                }
                else
                    square->clue[direction] = 0;

                // If the edge of the board or a black square is next,
                // this is the last square in the word
                if (    (   square->IsLast(direction, FIND_NEXT)
                         || square->Next  (direction, FIND_NEXT)->IsBlack())
                     && square->clue[direction] != 0)
                {
                    wordEnd[direction][square->clue[direction]] = square;
                }
            }

            // Increment clue number
            if (square->clueFlag != NO_CLUE)
                square->number = clueNumber++;
        }
    }

    // Set first white square
    for (XSquare * square = First(); square != NULL; square = square->Next())
        if (square->IsWhite())
        {
            m_firstWhite = square;
            break;
        }

    // Set last white square
    for (XSquare * square = Last();
         square != NULL;
         square = square->Next(DIR_ACROSS, FIND_PREV))
    {
        if (square->IsWhite())
        {
            m_lastWhite = square;
            break;
        }
    }

    // Set word start/end
    for (XSquare * square = First(); square != NULL; square = square->Next())
    {
        square->m_wordStart[DIR_ACROSS] =
            wordStart[DIR_ACROSS][square->clue[DIR_ACROSS]];

        square->m_wordEnd  [DIR_ACROSS] =
            wordEnd  [DIR_ACROSS][square->clue[DIR_ACROSS]];

        square->m_wordStart[DIR_DOWN]   =
            wordStart[DIR_DOWN]  [square->clue[DIR_DOWN]];

        square->m_wordEnd  [DIR_DOWN]   =
            wordEnd  [DIR_DOWN]  [square->clue[DIR_DOWN]];
    }

    wxLogDebug(_T("Time to set up grid: %d"), sw.Time());
}

void
XGrid::CountClues(size_t * across, size_t * down) const
{
    // Count number of across and down clues
    int acrossClues = 0;
    int downClues = 0;
    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        if ( (square->clueFlag & ACROSS_CLUE) != 0 )
            ++acrossClues;
        if ( (square->clueFlag & DOWN_CLUE) != 0 )
            ++downClues;
     }
    *across = acrossClues;
    *down = acrossClues;
}




std::vector<XSquare *>
XGrid::CheckGrid(bool checkBlank)
{
    std::vector<XSquare *> incorrect;

    XSquare * square;
    for (square = First(); square != NULL; square = square->Next())
            if (! square->Check(checkBlank))
                incorrect.push_back(square);

    return incorrect;
}


// return false if the squares are not in line
bool
SetIterateParams(XSquare * start, XSquare * end, bool * direction, bool * increment)
{
    int row_dif = end->row - start->row;
    int col_dif = end->col - start->col;

    if (row_dif == 0) {
        if (col_dif == 0)
            return false;
        *direction = DIR_ACROSS;
        *increment = col_dif < 0 ? FIND_PREV : FIND_NEXT;
    }
    else if (col_dif == 0) {
        *direction = DIR_DOWN;
        *increment = row_dif < 0 ? FIND_PREV : FIND_NEXT;
    }
    else
        return false;

    return true;
}


std::vector<XSquare *>
XGrid::CheckWord(XSquare * start, XSquare * end, bool checkBlank)
{
    std::vector<XSquare *> incorrect;
    bool direction;
    bool increment;
    if (SetIterateParams(start, end, &direction, &increment)) {
        end = end->Next(direction, increment);
        XSquare * square;
        for (square = start; square != end; square = square->Next(direction, increment))
            if (! square->Check(checkBlank))
                incorrect.push_back(square);
    }

    return incorrect;
}



// Find functions
bool FIND_ACROSS_CLUE  (const XSquare * square)
{
    return (square->clueFlag & ACROSS_CLUE) != 0;
}

bool FIND_DOWN_CLUE    (const XSquare * square)
{
    return (square->clueFlag & DOWN_CLUE) != 0;
}

bool FIND_WHITE_SQUARE (const XSquare * square)
{
    return square->IsWhite();
}

bool FIND_BLANK_SQUARE (const XSquare * square)
{
    return square->IsBlank();
}

bool FIND_BLACK_SQUARE (const XSquare * square)
{
    return square->IsBlack();
}

bool FIND_CLUE_GREATER_THAN (int num1, int num2)
{
    return num1 > num2;
}

bool FIND_CLUE_LESS_THAN    (int num1, int num2)
{
    return num1 != 0 && num1 < num2;
}

bool FIND_CLUE_DIFFERENT    (int num1, int num2)
{ 
    return num1 != 0 && num1 != num2;
}

// Functor for FIND_WORD
//----------------------
FIND_WORD::FIND_WORD(bool direction, const XSquare * square)
    : m_direction(direction), m_number(square->clue[direction])
{}

bool FIND_WORD::operator () (const XSquare * square)
{
    return square->clue[m_direction] != 0
        && square->clue[m_direction] != m_number;
}


// Functor for FIND_CLUE
//----------------------
FIND_CLUE::FIND_CLUE(bool direction, const XSquare * square)
    : m_clueType(direction == DIR_ACROSS ? ACROSS_CLUE : DOWN_CLUE),
      m_number(square->number)
{
    wxASSERT(m_number != 0 && square->clueFlag & m_clueType);
}

bool FIND_CLUE::operator () (const XSquare * square)
{
    return square->number != 0
        && (square->clueFlag & m_clueType) != 0
        && square->number != m_number;
}