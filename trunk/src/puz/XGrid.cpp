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
//   m_grid.at(col, row), i.e. m_grid.at(x, y)
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

    SetupIteration();
}

void
XGrid::SetupIteration()
{
    // Fill in XSquare members:
    //   - Row and Col
    //   - Next and Prev
    //   - IsLast (row/col next/prev)
    //-------------------------------------------------------------------
    for (size_t row = 0; row < GetHeight(); ++row)
    {
        for (size_t col = 0; col < GetWidth(); ++col)
        {
            XSquare & square = At(col, row);
            square.m_col = col;
            square.m_row = row;

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

unsigned short
XGrid::BruteForceUnscramble()
{
    XGridScrambler scrambler(*this);
    return scrambler.BruteForceUnscramble();
}

bool
XGrid::CheckScrambledGrid()
{
    return XGridScrambler::CheckUserGrid(*this);
}


#include <wx/stopwatch.h>
#include <wx/log.h>


// Setup the entire grid:
//  (1) Assigns clue numbers to each square (0 if no clue).
//  (2) Assigns the word start and end to each square.
//  (3) If the text is empty, fill it with blanks (or "." for black squares)
void
XGrid::SetupGrid()
{
    wxASSERT(GetWidth() > 0 && GetHeight() > 0);

    wxStopWatch sw;

    SetupIteration();

    m_firstWhite = NULL;
    m_lastWhite  = NULL;

    size_t clueNumber = 1;

    // Assign square clue numbers, word start, and word end
    //-----------------------------------------------------
    for (XSquare * square = First(); square != NULL; square = square->Next())
    {
        if (square->IsBlack())
        {
            // Make sure our square's text member knows that the square is
            // black too (not just the solution).
            square->m_text = _T(".");
            square->m_number   = 0;

            square->m_wordStart[DIR_ACROSS] = NULL;
            square->m_wordStart[DIR_DOWN]   = NULL;
            square->m_wordEnd  [DIR_ACROSS] = NULL;
            square->m_wordEnd  [DIR_DOWN]   = NULL;
        }
        else
        {
            // 12-3-09: I'm not sure why this assert is here.
            //wxASSERT(square->m_text != _T("-"));

            if (square->m_text == _T("-"))
                square->m_text = _T("");

            // Run everything in across and down directions
            for (bool direction = false; ; direction = !direction)
            {
                // Clues are located in squares where a black square (or the
                // edge) preceeds and where a white square follows.

                const bool is_first_white = \
                                square->IsLast(direction, FIND_PREV) ||
                                square->Prev(direction)->IsBlack();

                const bool has_clue = is_first_white &&
                            (! square->IsLast(direction, FIND_NEXT) &&
                             square->Next(direction)->IsWhite() );

                if (has_clue)
                {
                    square->m_number = clueNumber;

                    // Bounce to the next black square and back, setting start
                    // and end squares for all in between.

                    // Word start
                    XSquare * start = square;
                    for (;; square = square->Next(direction))
                    {
                        wxASSERT(square != NULL);
                        if (square->IsBlack())
                        {
                            // Back up one square so we end up with the
                            // last white square in this word.
                            square = square->Prev(direction);
                            break;
                        }
                        square->m_wordStart[direction] = start;
                        if (square->IsLast(direction))
                            break;
                    }
                    wxASSERT(square != start);

                    // Word end
                    XSquare * end = square;
                    for (;; square = square->Prev(direction))
                    {
                        wxASSERT(square != NULL);
                        if (square->IsBlack())
                        {
                            // Go forward a square so that we end up with the
                            // same square we started with
                            square = square->Next(direction);
                            break;
                        }
                        square->m_wordEnd[direction] = end;
                        if (square->IsFirst(direction))
                            break;
                    }

                    wxASSERT(square == start);
                }

                // If there is no clue number on this square but it is
                // preceded by a black square or the edge of the board, this
                // square is not part of a word in this direction (i.e. it
                // is an unchecked square).
                else if (is_first_white)
                {
                    square->m_wordStart[direction] = NULL;
                    square->m_wordEnd  [direction] = NULL;
                }

                // End the loop once we've finished both across and down
                if (direction)
                    break;
            }

            // Increment clue number if this square recieved a clue number.
            if (square->m_number != 0)
                ++clueNumber;

            // Off to the next square!
        }
    }


    // Set first white square
    //-----------------------
    for (XSquare * square = First(); square != NULL; square = square->Next())
    {
        if (square->IsWhite())
        {
            m_firstWhite = square;
            break;
        }
    }

    // Set last white square
    //-----------------------
    for (XSquare * square = Last(); square != NULL; square = square->Prev())
    {
        if (square->IsWhite())
        {
            m_lastWhite = square;
            break;
        }
    }

    wxLogDebug(_T("Time to set up grid: %d"), sw.Time());
}


void
XGrid::CountClues(size_t * across, size_t * down) const
{
    *across = 0;
    *down = 0;
    // Count number of across and down clues
    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(DIR_ACROSS))
            ++(*across);
        if (square->HasClue(DIR_DOWN))
            ++(*down);
     }
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
SetIterateParams(XSquare * start,
                 XSquare * end,
                 bool * direction,
                 bool * increment)
{
    int row_dif = end->GetRow() - start->GetRow();
    int col_dif = end->GetCol() - start->GetCol();

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
        for (square = start;
             square != end;
             square = square->Next(direction, increment))
        {
            if (! square->Check(checkBlank))
                incorrect.push_back(square);
        }
    }

    return incorrect;
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
    : m_direction(direction), m_wordStart(square->GetWordStart(direction))
{}

bool FIND_WORD::operator () (const XSquare * square)
{
    return square->GetWordStart(m_direction) != NULL
        && square->GetWordStart(m_direction) != m_wordStart;
}


// Functor for FIND_CLUE
//----------------------
FIND_CLUE::FIND_CLUE(bool direction, const XSquare * square)
    : m_direction(direction),
      m_number(square->GetNumber())
{
    wxASSERT(m_number != 0 && square->HasClue(m_direction));
}

bool FIND_CLUE::operator () (const XSquare * square)
{
    return square->GetNumber() != 0 &&
           square->HasClue(m_direction) &&
           square->GetNumber() != m_number;
}












//------------------------------------------------------------------------------
// Get grid data
//------------------------------------------------------------------------------

ByteArray
XGrid::GetGridSolution(const wxString & delim) const
{
    ByteArray data;
    data.reserve(GetWidth() * GetHeight());

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        data.push_back(square->GetPlainSolution());
        if (square->IsLast(DIR_ACROSS))
            data.push_string(delim);
    }
    return data;
}


ByteArray
XGrid::GetGridText(const wxString & delim) const
{
    ByteArray data;
    data.reserve(GetWidth() * GetHeight());

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        // Send text as lower case if the square is in pencil
        if (square->HasFlag(XFLAG_PENCIL))
            data.push_back( tolower(square->GetPlainText()) );
        else
            data.push_back(square->GetPlainText());

        if (square->IsLast(DIR_ACROSS))
            data.push_string(delim);
    }
    return data;
}


ByteArray
XGrid::GetGext(const wxString & delim) const
{
    ByteArray data;
    data.reserve(GetWidth() * GetHeight());

    for (const XSquare * square = First();
         square != NULL;
         square = square->Next())
    {
        data.push_back(square->m_flag);
        if (square->IsLast(DIR_ACROSS))
            data.push_string(delim);
    }
    return data;
}