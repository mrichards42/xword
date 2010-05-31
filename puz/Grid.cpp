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
// Grid: a container class for a crossword grid
// Primary function is to hold a 2d array of Squares
//
// Diagram of our organization of the grid (as in a python list)
// This is organized into a vector, m_vector.
// Access is obtained internally as m_vector[row][col], but externally as
//   Grid.at(col, row), i.e. Grid.at(x, y)
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

#include "Grid.hpp"
#include "Scrambler.hpp"
#include <cassert>

namespace puz {


Grid::Grid(size_t width, size_t height)
    : m_width(0),
      m_height(0),
      m_type(TYPE_NORMAL),
      m_flag(FLAG_NORMAL),
      m_key(0),
      m_cksum(0),
      m_gridState(GRID_NONE),
      m_first(NULL),
      m_last(NULL),
      m_firstWhite(NULL),
      m_lastWhite(NULL)
{
    if (width > 0 || height > 0)
        SetSize(width, height);
}


// This copy constructor needs to adjust all of the Square pointers to refer
// to the new Squares
Grid::Grid(const Grid & other)
    : m_vector(other.m_vector),
      m_width(other.m_width),
      m_height(other.m_height),
      m_type(other.m_type),
      m_flag(other.m_flag),
      m_key(other.m_key),
      m_cksum(other.m_cksum),
      m_gridState(GRID_SIZE),
      m_first(NULL),
      m_last(NULL),
      m_firstWhite(NULL),
      m_lastWhite(NULL)
{
    // Re-set up the grid.  This ensures that the Square pointers (e.g.
    // m_next, m_wordStart, etc.) refer to valid Squares.
    SetupGrid(other.m_gridState);
}


Grid::~Grid()
{
}


void
Grid::SetSize(size_t width, size_t height)
{
    m_vector.resize(height);

    // If the width changes, resize all of the nested vectors
    if (width != m_width)
    {
        Grid_t::iterator it;
        for (it = m_vector.begin(); it != m_vector.end(); ++it)
            it->resize(width, GridSquare(Square(this)));
    }
    // Otherwise, if the height has increased, resize all of the new vectors
    else if (height > m_height)
    {
        // resize the remaining rows
        Grid_t::iterator it;
        for (it = m_vector.begin() + m_height; it != m_vector.end(); ++it)
            it->resize(width, GridSquare(Square(this)));
    }

    m_height = height;
    m_width  = width;

    m_gridState = GRID_SIZE;
    SetupIteration();
}

void
Grid::SetupIteration()
{
    // Fill in Square members:
    //   - Row and Col
    //   - Next and Prev
    //   - IsLast (row/col next/prev)
    //-------------------------------------------------------------------
    for (size_t row = 0; row < GetHeight(); ++row)
    {
        for (size_t col = 0; col < GetWidth(); ++col)
        {
            Square & square = At(col, row);
            square.m_col = col;
            square.m_row = row;

            // Special cases for width or height == 1
            if (GetHeight() == 1 && GetWidth() == 1)
            {
                m_first = &square;
                m_last = &square;
                square.m_next[ACROSS][PREV] =
                square.m_next[DOWN]  [PREV] =
                square.m_next[ACROSS][NEXT] =
                square.m_next[DOWN]  [NEXT] = NULL;
            }
            else if (GetHeight() == 1)
            {
                if (col == 0)
                    m_first = &square;
                else if (col == LastCol())
                    m_last = &square;

                square.m_next[DOWN]  [PREV] =
                square.m_next[ACROSS][PREV] =
                    col == 0 ? NULL : &At(col - 1, row);

                square.m_next[DOWN]  [NEXT] =
                square.m_next[ACROSS][NEXT] =
                    col == LastCol() ? NULL : &At(col + 1, row);
            }
            else if (GetWidth() == 1)
            {
                if (row == 0)
                    m_first = &square;
                else if (row == LastRow())
                    m_last = &square;

                square.m_next[DOWN]  [PREV] =
                square.m_next[ACROSS][PREV] =
                    row == 0 ? NULL : &At(col, row - 1);

                square.m_next[DOWN]  [NEXT] =
                square.m_next[ACROSS][NEXT] =
                    row == LastRow() ? NULL : &At(col, row + 1);
            }
            // From here on out we have a normal grid (width and height > 1)

            // Two special cases: top left and bottom right corners
            else if (row == 0 && col == 0)
            {
                m_first = &square;
                square.m_next[ACROSS][PREV] = NULL;
                square.m_next[DOWN]  [PREV] = NULL;
                square.m_next[ACROSS][NEXT] = &At(col + 1, row);
                square.m_next[DOWN]  [NEXT] = &At(col,     row + 1);
            }
            else if (row == LastRow() && col == LastCol())
            {
                m_last = &square;
                square.m_next[DOWN]  [PREV] = &At(col,     row - 1);
                square.m_next[ACROSS][PREV] = &At(col - 1, row);
                square.m_next[DOWN]  [NEXT] = NULL;
                square.m_next[ACROSS][NEXT] = NULL;
            }

            // Otherwise next and previous squares will wrap rows and cols,
            // so we can iterate the entire grid.
            else
            {
                square.m_next[DOWN][PREV] = 
                    row > 0 ? 
                        &At(col,     row - 1)
                      : &At(col - 1, LastRow());

                square.m_next[DOWN][NEXT] =
                    row < LastRow() ? 
                        &At(col,     row + 1)
                      : &At(col + 1, 0);

                square.m_next[ACROSS][PREV] =
                    col > 0 ? 
                        &At(col - 1,   row)
                      : &At(LastCol(), row - 1);

                square.m_next[ACROSS][NEXT] =
                    col < LastCol() ?
                        &At(col + 1, row)
                      : &At(0,       row + 1);
            }

            // End of col/rows:
            square.m_isLast[ACROSS][NEXT] =  col == LastCol();
            square.m_isLast[DOWN]  [NEXT] =  row == LastRow();
            square.m_isLast[ACROSS][PREV] =  col == 0;
            square.m_isLast[DOWN]  [PREV] =  row == 0;
        }
    }
    // Update the grid state.
    m_gridState = GRID_ITERATION;
}



bool
Grid::ScrambleSolution(unsigned short key)
{
    Scrambler scrambler(*this);
    return scrambler.ScrambleSolution(key);
}


bool
Grid::UnscrambleSolution(unsigned short key)
{
    Scrambler scrambler(*this);
    return scrambler.UnscrambleSolution(key);
}


bool
Grid::CheckScrambledGrid()
{
    return Scrambler::CheckUserGrid(*this);
}


// Setup the grid after the black squares are arranged:
//  (1) Assign clue numbers to each square (0 if no clue).
//  (2) Assign the word start and end to each square.
//  (3) If the user text is empty, fill it with blanks (or "." for blacks)
void
Grid::SetupSolution()
{
    assert(SetupGrid(GRID_ITERATION));

    m_firstWhite = NULL;
    m_lastWhite  = NULL;

    m_first = &At(0,0);
    m_last = &At(LastCol(), LastRow());

    size_t clueNumber = 1;

    // Assign square clue numbers, word start, and word end
    //-----------------------------------------------------
    for (Square * square = First(); square != NULL; square = square->Next())
    {
        if (square->IsBlack())
        {
            // Make sure our square's text member knows that the square is
            // black too.
            square->m_text = ".";
            square->m_number = 0;

            square->m_wordStart[ACROSS] = NULL;
            square->m_wordStart[DOWN]   = NULL;
            square->m_wordEnd  [ACROSS] = NULL;
            square->m_wordEnd  [DOWN]   = NULL;
        }
        else
        {
            // Blank squares are truly blank
            if (square->m_text == "-")
                square->m_text = "";

            square->m_number = 0;

            // Run everything in across and down directions
            for (GridDirection direction = ACROSS; ; direction = DOWN)
            {
                // Clues are located in squares where a black square (or the
                // edge) preceeds and where a white square follows.

                const bool is_first_white = \
                                square->IsLast(direction, PREV) ||
                                square->Prev(direction)->IsBlack();

                const bool has_clue = is_first_white &&
                            (! square->IsLast(direction, NEXT) &&
                             square->Next(direction)->IsWhite() );

                if (has_clue)
                {
                    square->m_number = clueNumber;

                    // Bounce to the next black square and back, setting start
                    // and end squares for all in between.

                    // Word start
                    Square * start = square;
                    for (;; square = square->Next(direction))
                    {
                        assert(square != NULL);
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
                    assert(square != start);

                    // Word end
                    Square * end = square;
                    for (;; square = square->Prev(direction))
                    {
                        assert(square != NULL);
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

                    assert(square == start);
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
                if (direction == DOWN)
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
    for (Square * square = First(); square != NULL; square = square->Next())
    {
        if (square->IsWhite())
        {
            m_firstWhite = square;
            break;
        }
    }

    // Set last white square
    //-----------------------
    for (Square * square = Last(); square != NULL; square = square->Prev())
    {
        if (square->IsWhite())
        {
            m_lastWhite = square;
            break;
        }
    }

    // Update the grid state
    m_gridState = GRID_SOLUTION;
}


// Make sure the grid has gotten past the given setup stage.
// Call all setup functions up to the given setup stage.
// Return false if the grid cannot be setup (i.e. SetSize() was never called).
bool
Grid::SetupGrid(GridState state)
{
    assert(state > GRID_NONE);

    // The grid is already at or beyond this setup stage.
    if (state <= m_gridState)
        return true;

    // Make sure everything before this stage is setup.
    if (! SetupGrid(static_cast<GridState>(state - 1)))
        throw InvalidGrid();

    // Setup this stage
    switch(state)
    {
        case GRID_SOLUTION:
            SetupSolution();
        break;
        case GRID_ITERATION:
            SetupIteration();
        break;
        case GRID_SIZE:
            if (! (GetWidth() > 0 && GetHeight() > 0))
                throw InvalidGrid();
        break;
    }
    return true;
}



void
Grid::CountClues(size_t * across, size_t * down)
{
    if (! SetupGrid(GRID_SOLUTION))
        throw InvalidGrid();
    DoCountClues(across, down);
}

void
Grid::CountClues(size_t * across, size_t * down) const
{
    if (! SetupGrid(GRID_SOLUTION))
        throw InvalidGrid();
    DoCountClues(across, down);
}

void
Grid::DoCountClues(size_t * across, size_t * down) const
{
    *across = 0;
    *down = 0;
    // Count number of across and down clues
    for (const Square * square = First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(ACROSS))
            ++(*across);
        if (square->HasClue(DOWN))
            ++(*down);
    }
}




std::vector<Square *>
Grid::CheckGrid(bool checkBlank)
{
    std::vector<Square *> incorrect;

    Square * square;
    for (square = First(); square != NULL; square = square->Next())
        if (! square->Check(checkBlank))
            incorrect.push_back(square);

    return incorrect;
}


std::vector<Square *>
Grid::CheckWord(Square * start, Square * end, bool checkBlank)
{
    std::vector<Square *> incorrect;
    GridDirection direction;
    FindDirection increment;

    // Find the iteration direction and increment
    int row_dif = end->GetRow() - start->GetRow();
    int col_dif = end->GetCol() - start->GetCol();

    if (row_dif == 0)
    {
        if (col_dif == 0)
            throw NoWord();
        direction = ACROSS;
        increment = col_dif < 0 ? PREV : NEXT;
    }
    else if (col_dif == 0)
    {
        direction = DOWN;
        increment = row_dif < 0 ? PREV : NEXT;
    }
    else
        throw NoWord();

    end = end->Next(direction, increment);
    for (Square * square = start;
         square != end;
         square = square->Next(direction, increment))
    {
        if (! square->Check(checkBlank))
            incorrect.push_back(square);
    }

    return incorrect;
}



// FindSquare functions

bool FIND_WHITE_SQUARE (const Square * square)
{
    return square->IsWhite();
}

bool FIND_BLANK_SQUARE (const Square * square)
{
    return square->IsBlank();
}

bool FIND_BLACK_SQUARE (const Square * square)
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
FIND_WORD::FIND_WORD(GridDirection direction, const Square * square)
    : m_direction(direction), m_wordStart(square->GetWordStart(direction))
{}

bool FIND_WORD::operator () (const Square * square)
{
    return square->GetWordStart(m_direction) != NULL
        && square->GetWordStart(m_direction) != m_wordStart;
}


// Functor for CLUE
//----------------------
FIND_CLUE::FIND_CLUE(GridDirection direction, const Square * square)
    : m_direction(direction),
      m_number(square->GetNumber())
{
    assert(m_number != 0 && square->HasClue(m_direction));
}

bool FIND_CLUE::operator () (const Square * square)
{
    return square->GetNumber() != 0 &&
           square->HasClue(m_direction) &&
           square->GetNumber() != m_number;
}












//------------------------------------------------------------------------------
// Get grid data
//------------------------------------------------------------------------------

std::string
Grid::GetGridSolution(const std::string & delim) const
{
    std::string data;
    data.reserve(GetWidth() * GetHeight());

    for (const Square * square = First();
         square != NULL;
         square = square->Next())
    {
        data.push_back(square->GetPlainSolution());
        if (square->IsLast(ACROSS))
            data.append(delim);
    }
    return data;
}


std::string
Grid::GetGridText(const std::string & delim) const
{
    std::string data;
    data.reserve(GetWidth() * GetHeight());

    for (const Square * square = First();
         square != NULL;
         square = square->Next())
    {
        // Send text as lower case if the square is in pencil
        if (square->HasFlag(FLAG_PENCIL))
            data.push_back( tolower(square->GetPlainText()) );
        else
            data.push_back(square->GetPlainText());

        if (square->IsLast(ACROSS))
            data.append(delim);
    }
    return data;
}


std::string
Grid::GetGext(const std::string & delim) const
{
    std::string data;
    data.reserve(GetWidth() * GetHeight());

    for (const Square * square = First();
         square != NULL;
         square = square->Next())
    {
        data.push_back(square->m_flag);
        if (square->IsLast(ACROSS))
            data.append(delim);
    }
    return data;
}

} // namespace puz
