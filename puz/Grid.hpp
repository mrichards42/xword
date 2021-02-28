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


#ifndef PUZ_GRID_H
#define PUZ_GRID_H

#include <vector>
#include "Square.hpp"
#include "Word.hpp"

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
    // Across Lite flags
    TYPE_NORMAL      = 0x0001,
    TYPE_DIAGRAMLESS = 0x0401,
    // Additional flags
    TYPE_ACROSTIC    = 0x1000,
    TYPE_CODED       = 0x2000,
};

// Parameters for FindSquare
enum FindOptions
{
    WRAP          = 0,
    NO_WRAP       = 0x01,
    WHITE_SQUARES = 0x02,

    FIND_IN_GRID  = WRAP,
    FIND_IN_WORD  = NO_WRAP | WHITE_SQUARES,
};


// This class helps Square hide its constructors
class GridSquare
{
public:
    GridSquare()
        : m_square()
    {}
    GridSquare(const Square & square)
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

    // Algorithmically determine grid numbers
    void NumberGrid();

    // Find and fill in partner squares (for Acrostics)
    void FindPartnerSquares();

public:

    // Size
    //-----
    void SetSize(size_t width, size_t height);
    size_t GetWidth()  const { return m_width; }
    size_t GetHeight() const { return m_height; }
    int    LastRow()   const { return m_height - 1; }
    int    LastCol()   const { return m_width - 1; }

    bool IsEmpty() const { return m_width == 0 || m_height == 0; }
    void Clear();

    Square * First() { return m_first; }
    Square * Last() { return m_last; }
    const Square * First() const { return m_first; }
    const Square * Last() const { return m_last; }

    // Access to squares provided as (x,y):
    // i.e. square "b4" is At(1, 3)
    const Square & At(size_t col, size_t row) const
        { return m_vector.at(row).at(col).m_square; }
    Square & At(size_t col, size_t row)
        { return m_vector.at(row).at(col).m_square; }

    const Square * AtNULL(int col, int row) const
    {
        if (col < 0 || col > LastCol() || row < 0 || row > LastRow())
            return NULL;
        return &m_vector[row][col].m_square;
    }
    Square * AtNULL(int col, int row)
    {
        if (col < 0 || col > LastCol() || row < 0 || row > LastRow())
            return NULL;
        return &m_vector[row][col].m_square;
    }

    // Searching
    //----------
    // Accepts functions (or function-objects) that subscribe to the template:
    //    bool Function(const Square *)
    template<typename T>
    Square * FindSquare(Square * start,
                        T findFunc,
                        GridDirection direction = ACROSS,
                        unsigned int options = FIND_IN_GRID);

    // FindSquare starting from the first square
    template<typename T>
    Square * FindSquare(T findFunc,
                        GridDirection direction,
                        unsigned int options = FIND_IN_GRID);

    // Find any square
    template<typename T>
    Square * FindSquare(T findFunc, unsigned int options = FIND_IN_GRID);

    // Search starting from the next square
    template<typename T>
    Square * FindNextSquare(Square * start,
                            T findFunc,
                            GridDirection direction = ACROSS,
                            unsigned int options = FIND_IN_GRID);



    // Const overloads
    template<typename T>
    const Square * FindSquare(const Square * start,
                              T findFunc,
                              GridDirection direction = ACROSS,
                              unsigned int options = FIND_IN_GRID) const;

    template<typename T>
    const Square * FindSquare(T findFunc,
                              GridDirection direction,
                              unsigned int options = FIND_IN_GRID) const;

    // Find any square
    template<typename T>
    const Square * FindSquare(T findFunc,
                              unsigned int options = FIND_IN_GRID) const;

    // Search starting from the next square
    template<typename T>
    const Square * FindNextSquare(const Square * start,
                                  T findFunc,
                                  GridDirection direction = ACROSS,
                                  unsigned int options = FIND_IN_GRID) const;


    // Flags
    bool IsScrambled() const { return (m_flag & FLAG_SCRAMBLED) != 0; }
    bool HasSolution() const { return (m_flag & FLAG_NO_SOLUTION) == 0; }
    unsigned short GetFlag() const { return m_flag; }
    void SetFlag(unsigned short flag) { m_flag = flag; }

    // Type
    bool IsDiagramless() const { return m_type == TYPE_DIAGRAMLESS; }
    bool IsAcrostic() const { return m_type == TYPE_ACROSTIC; }
    bool IsCoded() const { return m_type == TYPE_CODED; }
    unsigned short GetType() const { return m_type; }
    void SetType(unsigned short type) { m_type = type; }

    // Scrambling
    bool ScrambleSolution  (unsigned short key = 0);
    bool UnscrambleSolution(unsigned short key);
    bool CheckScrambledGrid() const;

    unsigned short  GetKey()   const { return m_key; }
    unsigned short  GetCksum() const { return m_cksum; }
    void SetKey(unsigned short key) { m_key = key; }
    void SetCksum(unsigned short cksum) { m_cksum = cksum; }

    // Check functions
    //----------------
    void CheckGrid(std::vector<Square *> * incorrect,
                   bool checkBlank = false,
                   bool strictRebus = false);
    void CheckWord(std::vector<Square *> * incorrect,
                   const Word * word,
                   bool checkBlank = false,
                   bool strictRebus = false);
    bool CheckSquare(const Square & square, bool checkBlank = false,
                                            bool strictRebus = false) const
        { return square.Check(checkBlank, strictRebus); }

protected:
    typedef std::vector< std::vector< GridSquare > > Grid_t;
    Grid_t m_vector;

    size_t m_width, m_height;
    Square * m_first;
    Square * m_last;

    short m_type;
    short m_flag;

    // These are used for grid scrambling
    unsigned short m_key;
    unsigned short m_cksum;
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
    SetSize(0,0);
}

// Functions/functors for FindSquare

static bool FIND_WHITE_SQUARE (const Square * square)
{
    return square->IsWhite();
}

static bool FIND_BLANK_SQUARE (const Square * square)
{
    return square->IsBlank();
}

static bool FIND_BLACK_SQUARE (const Square * square)
{
    return square->IsBlack();
}

struct FIND_CLUE_NUMBER
{
    FIND_CLUE_NUMBER(const string_t & number)
        : m_number(number)
    {
    }

    bool operator() (const puz::Square * square)
    {
        return square->GetNumber() == m_number;
    }

    string_t m_number;
};


template <typename T>
Square *
Grid::FindSquare(Square * start,
                 T findFunc,
                 GridDirection direction,
                 unsigned int options)
{
    const bool only_whites = (options & WHITE_SQUARES) != 0;
    const bool wrap = (options & NO_WRAP) == 0;

    for (Square * square = start;
         square != NULL;
         square = square->Next(direction))
    {
        if (only_whites && ! square->IsWhite())
            break;

        if (findFunc(square))
            return square;

        if (! wrap && square->IsLast(direction))
            break;
    }

    return NULL;
}

template<typename T>
Square *
Grid::FindSquare(T findFunc,
                 GridDirection direction,
                 unsigned int options)
{
    return FindSquare(First(), findFunc, direction, options);
}


template<typename T>
Square *
Grid::FindSquare(T findFunc, unsigned int options)                 
{
    return FindSquare(First(), findFunc, ACROSS, options);
}


template <typename T>
Square *
Grid::FindNextSquare(Square * start,
                      T findFunc,
                      GridDirection direction,
                      unsigned int options)
{
    if (start == NULL ||
        ((options & NO_WRAP)
          && start->IsLast(direction)) )
            return NULL;

    return FindSquare(start->Next(direction),
                      findFunc,
                      direction,
                      options);
}




// Const overloads
template <typename T>
const Square *
Grid::FindSquare(const Square * start,
                 T findFunc,
                 GridDirection direction,
                 unsigned int options) const
{
    return const_cast<Grid*>(this)->
        FindSquare(const_cast<Square*>(start), findFunc, direction, options);
}

template<typename T>
const Square *
Grid::FindSquare(T findFunc,
                 GridDirection direction,
                 unsigned int options) const
{
    return FindSquare(First(), findFunc, direction, options);
}


template<typename T>
const Square *
Grid::FindSquare(T findFunc, unsigned int options) const
{
    return FindSquare(First(), findFunc, ACROSS, options);
}


template <typename T>
const Square *
Grid::FindNextSquare(const Square * start,
                      T findFunc,
                      GridDirection direction,
                      unsigned int options) const
{
    return const_cast<Grid*>(this)->
        FindNextSquare(const_cast<Square*>(start), findFunc, direction, options);
}

} // namespace puz

#endif // PUZ_GRID_H
