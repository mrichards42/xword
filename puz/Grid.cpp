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
#include "iterator.hpp"
#include "Scrambler.hpp"

#include <map>

namespace puz {


Grid::Grid(size_t width, size_t height)
    : m_width(0),
      m_height(0),
      m_type(TYPE_NORMAL),
      m_flag(FLAG_NORMAL),
      m_key(0),
      m_cksum(0),
      m_first(NULL),
      m_last(NULL)
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
      m_first(NULL),
      m_last(NULL)
{
    // Re-set up the grid.  This ensures that the Square linked
    // list pointers (m_next) refer to valid Squares.
    SetupIteration();
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
            it->resize(width, GridSquare(Square()));
    }
    // Otherwise, if the height has increased, resize all of the new vectors
    else if (height > m_height)
    {
        // resize the remaining rows
        Grid_t::iterator it;
        for (it = m_vector.begin() + m_height; it != m_vector.end(); ++it)
            it->resize(width, GridSquare(Square()));
    }

    m_height = height;
    m_width  = width;

    SetupIteration();
}

void
Grid::SetupIteration()
{
    // Fill in Square members:
    //   - Row and Col
    //   - Next
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
                square.m_next.clear();
            }
            else if (GetHeight() == 1)
            {
                if (col == 0)
                    m_first = &square;
                else if (col == LastCol())
                    m_last = &square;

                square.m_next[UP] =
                square.m_next[LEFT] =
                    col == 0 ? NULL : &At(col - 1, row);

                square.m_next[DOWN] =
                square.m_next[RIGHT] =
                    col == LastCol() ? NULL : &At(col + 1, row);

                square.m_next[DIAGONAL_SE] =
                square.m_next[DIAGONAL_SW] =
                square.m_next[DIAGONAL_NW] =
                square.m_next[DIAGONAL_NE] = NULL;
            }
            else if (GetWidth() == 1)
            {
                if (row == 0)
                    m_first = &square;
                else if (row == LastRow())
                    m_last = &square;

                square.m_next[UP] =
                square.m_next[LEFT] =
                    row == 0 ? NULL : &At(col, row - 1);

                square.m_next[DOWN] =
                square.m_next[RIGHT] =
                    row == LastRow() ? NULL : &At(col, row + 1);

                square.m_next[DIAGONAL_SE] =
                square.m_next[DIAGONAL_SW] =
                square.m_next[DIAGONAL_NW] =
                square.m_next[DIAGONAL_NE] = NULL;
            }
            // From here on out we have a normal grid (width and height > 1)

            // Two special cases: top left and bottom right corners
            else if (row == 0 && col == 0)
            {
                m_first = &square;
                square.m_next[UP]    = NULL;
                square.m_next[LEFT]  = NULL;
                square.m_next[DOWN]  = &At(col, row + 1);
                square.m_next[RIGHT] = &At(col + 1, row);
                square.m_next[DIAGONAL_SE] = &At(col + 1, row + 1);
                square.m_next[DIAGONAL_SW] =
                square.m_next[DIAGONAL_NW] =
                square.m_next[DIAGONAL_NE] = NULL;
            }
            else if (row == LastRow() && col == LastCol())
            {
                m_last = &square;
                square.m_next[UP]    = &At(col, row - 1);
                square.m_next[LEFT]  = &At(col - 1, row);
                square.m_next[DOWN]  = NULL;
                square.m_next[RIGHT] = NULL;
                square.m_next[DIAGONAL_NW] = &At(col - 1, row - 1);
                square.m_next[DIAGONAL_SE] =
                square.m_next[DIAGONAL_SW] =
                square.m_next[DIAGONAL_NE] = NULL;
            }

            // Otherwise next and previous squares will wrap rows and cols,
            // so we can iterate the entire grid.
            else
            {
                square.m_next[UP] = 
                    row > 0 ? 
                        &At(col,     row - 1)
                      : &At(col - 1, LastRow());

                square.m_next[DOWN] =
                    row < LastRow() ? 
                        &At(col,     row + 1)
                      : &At(col + 1, 0);

                square.m_next[LEFT] =
                    col > 0 ? 
                        &At(col - 1,   row)
                      : &At(LastCol(), row - 1);

                square.m_next[RIGHT] =
                    col < LastCol() ?
                        &At(col + 1, row)
                      : &At(0,       row + 1);

                // Diagonal next is always NULL on the edge of the grid.
                square.m_next[DIAGONAL_NE] = AtNULL(col + 1, row - 1);
                square.m_next[DIAGONAL_NW] = AtNULL(col - 1, row - 1);
                square.m_next[DIAGONAL_SW] = AtNULL(col - 1, row + 1);
                square.m_next[DIAGONAL_SE] = AtNULL(col + 1, row + 1);
            }
        }
    }
    if (GetWidth() == 0 || GetHeight() == 0)
    {
        m_first = NULL;
        m_last = NULL;
    }
    else
    {
        m_first = &At(0,0);
        m_last = &At(LastCol(), LastRow());
    }
}

void Grid::NumberGrid()
{
    int clueNumber = 1;

    for (Square * square = First();
         square != NULL;
         square = square->Next())
    {
        if (square->WantsClue())
            square->SetNumber(clueNumber++);
        else
            square->SetNumber(puzT(""));
    }
}

void
Grid::FindPartnerSquares()
{
    if (!IsAcrostic() && !IsCoded()) return;
    std::multimap<string_t, Square*> partner_map;
    for (size_t row = 0; row < GetHeight(); ++row)
    {
        for (size_t col = 0; col < GetWidth(); ++col)
        {
            Square& square = At(col, row);
            if (square.HasNumber()) {
                typedef std::multimap<string_t, Square*>::iterator PartnerIterator;
                std::pair<PartnerIterator, PartnerIterator> result = partner_map.equal_range(square.GetNumber());
                for (PartnerIterator it = result.first; it != result.second; ++it) {
                    (*it).second->m_partner.push_back(&square);
                    square.m_partner.push_back((*it).second);
                }
                partner_map.insert(std::make_pair(square.GetNumber(), &square));
            }
        }
    }
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
Grid::CheckScrambledGrid() const
{
    return Scrambler::CheckUserGrid(*this);
}



void
Grid::CheckGrid(std::vector<Square *> * incorrect, bool checkBlank, bool strictRebus)
{
    Square * square;
    for (square = First(); square != NULL; square = square->Next())
        if (! square->Check(checkBlank, strictRebus))
            incorrect->push_back(square);
}

void
Grid::CheckWord(std::vector<Square *> * incorrect,
                const Word * word, bool checkBlank, bool strictRebus)
{
    if (! word)
        throw NoWord();
    square_iterator it;
    for (it = word->begin(); it != word->end(); ++it)
    {
        if (! it->Check(checkBlank, strictRebus))
            incorrect->push_back(&At(it->GetCol(), it->GetRow()));
    }
}

} // namespace puz
