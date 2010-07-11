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


#ifndef PUZ_GRID_H
#define PUZ_GRID_H

#include <vector>
#include <string>
#include "Square.hpp"

namespace puz {

// Friends
class PUZ_API Scrambler;
class PUZ_API Checksummer;

enum GridFlag
{
    FLAG_NORMAL      = 0x0000,
    FLAG_NO_SOLUTION = 0x0002,
    FLAG_SCRAMBLED   = 0x0004
};

enum GridType
{
    TYPE_NORMAL      = 0x0001,
    TYPE_DIAGRAMLESS = 0x0401
};


// Parameters for FindSquare
const bool NO_WRAP_LINES = false;
const bool WRAP_LINES    = true;

const bool SKIP_BLACK_SQUARES    = true;
const bool NO_SKIP_BLACK_SQUARES = false;

// Macro to simplify GetX() functions
#define GET_FUNCTION(type, name, member)         \
    const type name() const { return member; }   \
          type name()       { return member; }   \

class GridSquare
{
public:
    GridSquare()
        : m_square()
    {}
    GridSquare(Square & square)
        : m_square(square)
    {}
    Square m_square;
};

class PUZ_API Grid
{
    friend class Scrambler;
    friend class Checksummer;
    friend class Square;

public:
    explicit Grid(size_t width = 0, size_t height = 0);
    Grid(const Grid & other);
    ~Grid();

    // Setup
    //------
    // Fills in Square Next / Prev, etc.  Called automatically from SetSize.
    void SetupIteration();
    // Setup numbers, etc.  This must be called after the solution is filled.
    void SetupSolution();

    // Ensure that the grid is setup in this order:
    //    SetSize
    //    SetupIteration
    //    Solution
    enum GridState
    {
        GRID_NONE = 0,
        GRID_SIZE,
        GRID_ITERATION,
        GRID_SOLUTION,
        GRID_ALL = GRID_SOLUTION
    };
protected:
    GridState m_gridState;
public:
    // Return true on success (we can at least report success for
    // a const overload).
    bool SetupGrid(GridState state = GRID_ALL);
    bool IsGridSetup(GridState state = GRID_ALL) const
        { return state <= m_gridState; }
    bool SetupGrid(GridState state = GRID_ALL) const
        { return IsGridSetup(state); }

    // Size
    //-----
    void SetSize(size_t width, size_t height);
    size_t GetWidth()  const { return m_width; }
    size_t GetHeight() const { return m_height; }
    int    LastRow()   const { return m_height - 1; }
    int    LastCol()   const { return m_width - 1; }

    bool IsEmpty() const { return m_width == 0 || m_height == 0; }
    void Clear();


    GET_FUNCTION( Square *, First,      m_first)
    GET_FUNCTION( Square *, Last,       m_last)
    GET_FUNCTION( Square *, FirstWhite, m_firstWhite)
    GET_FUNCTION( Square *, LastWhite,  m_lastWhite)

    // Access to squares provided as (x,y):
    // i.e. square "b4" is At(1, 3)
    const Square & At(size_t col, size_t row) const
        { return m_vector.at(row).at(col).m_square; }
    Square & At(size_t col, size_t row)
        { return m_vector.at(row).at(col).m_square; }


    // Searching
    //----------
    // Accepts functions (or function-objects) that subscribe to the template:
    //    bool Function(const Square *)
    template<typename T>
    Square * FindSquare(Square * start,
                         T findFunc,
                         GridDirection direction,
                         FindDirection increment = NEXT,
                         bool skipBlack = NO_SKIP_BLACK_SQUARES,
                         bool wrapLines = NO_WRAP_LINES);

    // Search starting from the next square
    template<typename T>
    Square * FindNextSquare(Square * start,
                             T findFunc,
                             GridDirection direction,
                             FindDirection increment = NEXT,
                             bool skipBlack = NO_SKIP_BLACK_SQUARES,
                             bool wrapLines = NO_WRAP_LINES);

    // Flags
    bool IsScrambled() const { return (m_flag & FLAG_SCRAMBLED) != 0; }
    bool HasSolution() const { return (m_flag & FLAG_NO_SOLUTION) == 0; }
    unsigned short GetFlag() const { return m_flag; }
    void SetFlag(unsigned short flag) { m_flag = flag; }

    // Type
    bool IsDiagramless() const { return m_type == TYPE_DIAGRAMLESS; }
    unsigned short GetType() const { return m_type; }
    void SetType(unsigned short type) { m_type = type; }

    // Scrambling
    bool ScrambleSolution  (unsigned short key = 0);
    bool UnscrambleSolution(unsigned short key);
    bool CheckScrambledGrid();

    unsigned short  GetKey()   const { return m_key; }
    unsigned short  GetCksum() const { return m_cksum; }
    void SetKey(unsigned short key) { m_key = key; }
    void SetCksum(unsigned short cksum) { m_cksum = cksum; }

    bool IsBetween(const Square * square,
                   const Square * start,
                   const Square * end) const;

    // These are necessary for the SetupGrid() const overload
    void CountClues(size_t * across, size_t * down);
    void CountClues(size_t * across, size_t * down) const;


    // Check functions
    //----------------
    void CheckGrid(std::vector<Square *> * incorrect,
                   bool checkBlank = false,
                   bool strictRebus = false);
    void CheckWord(std::vector<Square *> * incorrect,
                   Square * start, Square * end,
                   bool checkBlank = false,
                   bool strictRebus = false);
    bool CheckSquare(const Square & square, bool checkBlank = false,
                                            bool strictRebus = false) const
        { return square.Check(checkBlank, strictRebus); }


    // These all operate with ASCII
    // delim is a delimiter between lines
    std::string GetGridSolution(const std::string & delim = "") const;
    std::string GetGridText    (const std::string & delim = "") const;
    std::string GetGext        (const std::string & delim = "") const;

protected:
    typedef std::vector< std::vector< GridSquare > > Grid_t;
    Grid_t m_vector;

    size_t m_width, m_height;
    Square * m_first;
    Square * m_last;
    Square * m_firstWhite;
    Square * m_lastWhite;

    short m_type;
    short m_flag;

    // These are used for grid scrambling
    unsigned short m_key;
    unsigned short m_cksum;

    // Used with CountClues() const-overloading
    void DoCountClues(size_t * across, size_t * down) const;
};


#undef GET_FUNCTION


inline void
Grid::Clear()
{
    m_type = TYPE_NORMAL;
    m_flag = FLAG_NORMAL;
    m_key = 0;
    m_cksum = 0;
    m_first = NULL;
    m_last = NULL;
    m_firstWhite = NULL;
    m_lastWhite = NULL;
    SetSize(0,0);
    m_gridState = GRID_NONE;
}


inline bool
Grid::IsBetween(const Square * square,
                 const Square * start,
                 const Square * end) const
{
    return square->m_col >= start->m_col && square->m_col <= end->m_col
        && square->m_row >= start->m_row && square->m_row <= end->m_row;
}

template <typename T>
Square *
Grid::FindSquare(Square * start,
                  T findFunc,
                  GridDirection direction,
                  FindDirection increment,
                  bool skipBlack,
                  bool wrapLines)
{
    if (start == NULL)
        return NULL;

    for (Square * square = start;
         square != NULL;
         square = square->Next(direction, increment))
    {
        if (! skipBlack && square->IsBlack())
            break;

        if (findFunc(square))
            return square;

        if (! wrapLines && square->IsLast(direction, increment))
            break;
    }

    return NULL;
}



template <typename T>
Square *
Grid::FindNextSquare(Square * start,
                      T findFunc,
                      GridDirection direction,
                      FindDirection increment,
                      bool skipBlack,
                      bool wrapLines)
{
    if (start == NULL || (! wrapLines && start->IsLast(direction, increment)) )
        return NULL;

    return FindSquare(start->Next(direction, increment),
                      findFunc,
                      direction,
                      increment,
                      skipBlack,
                      wrapLines);
}



// Functions / functors for FindSquare
PUZ_API bool FIND_WHITE_SQUARE (const Square * square);
PUZ_API bool FIND_BLACK_SQUARE (const Square * square);
PUZ_API bool FIND_BLANK_SQUARE (const Square * square);

struct PUZ_API FIND_WORD
{
    FIND_WORD(GridDirection direction, const Square * square);
    bool operator() (const Square * square);
private:
    GridDirection m_direction;
    const Square *  m_wordStart;
};

// This looks for square->number, so in order to use it we must give
//    this struct and FindSquare a square at the start of a word
struct PUZ_API FIND_CLUE
{
    FIND_CLUE(GridDirection direction, const Square * square);
    bool operator() (const Square * square);
private:
    GridDirection m_direction;
    unsigned int m_number;
};

} // namespace puz

#endif // PUZ_GRID_H
