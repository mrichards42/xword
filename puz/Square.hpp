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


#ifndef PUZ_SQUARE_H
#define PUZ_SQUARE_H

#include <cassert>
#include <string>
#include "exceptions.hpp"
#include "util.hpp"

namespace puz {

const int REBUS_ENTRY_LENGTH = 8;

// Clue flags
enum ClueFlag
{
    NO_CLUE     = 0x00,
    ACROSS_CLUE = 0x01,
    DOWN_CLUE   = 0x02,
};


// GEXT flags
enum GextFlag
{
    FLAG_CLEAR  = 0x00,
    FLAG_PENCIL = 0x08,
    FLAG_BLACK  = 0x10,
    FLAG_X      = 0x20,
    FLAG_RED    = 0x40,
    FLAG_CIRCLE = 0x80
};


// Forward/backward
enum FindDirection
{
    PREV = 0,
    NEXT,
};


// Grid directions
enum GridDirection
{
    NONE =- 1,
    ACROSS = 0,
    DOWN
};

// Facilitate swapping across and down
inline GridDirection
SwapDirection(GridDirection dir)
{
    assert(dir != NONE);
    return dir == ACROSS ? DOWN : ACROSS;
}

// Used with Check()
enum CheckTest
{
    NO_CHECK_BLANK,
    CHECK_BLANK
};


// To be used as friends
class PUZ_API Grid;
class GridSquare;
class PUZ_API GridScrambler;

class PUZ_API Square
{
    friend class Grid;
    friend class GridSquare;
    friend class GridScrambler;

private:
    // Squares can only be created by a grid.
    explicit Square(Grid * grid = NULL);
    Square(const Square & other);
    Grid * m_grid;

public:
    Square & operator=(const Square & other);

    // Location information
    //---------------------
    short GetCol() const { return m_col; }
    short GetRow() const { return m_row; }


    // General information
    //--------------------
    bool IsOk()         const { return ! m_solution.empty(); }
    bool IsWhite()      const { return ! IsBlack(); }
    bool IsBlack()      const { return m_solution == "."; }
    bool IsBlank()      const { return m_text.empty(); }


    // Text
    //-----
    const std::string & GetText()     const { return m_text; }
    const std::string & GetSolution() const { return m_solution; }

    void SetText    (const std::string & text);
    void SetSolution(const std::string & solution);
    void SetSolution(const std::string & solution, char plain);
    void SetPlainSolution(char plain); // Leave solution rebus unchanged
    void SetSolutionRebus(const std::string & rebus); // Leave plain solution unchanged
    void SetSolutionSymbol(unsigned short symbol); // Must set plain solution separately

    bool Check(bool checkBlank = NO_CHECK_BLANK) const;

    char GetPlainText()     const { return ToAscii(m_text); }
    char GetPlainSolution() const { return m_asciiSolution; }

    bool HasTextRebus()      const;
    bool HasSolutionRebus()  const;
    bool HasTextSymbol()     const;
    bool HasSolutionSymbol() const;

    unsigned short GetTextSymbol()      const;
    unsigned short GetSolutionSymbol()  const;

    static bool IsValidChar(char ch);
    static bool IsValidString(const std::string & str);
    static bool IsSymbol(const std::string & str);

    // Clue
    //-----
    short GetNumber() const { return m_number; }
    bool HasClue(GridDirection dir) const
        { return GetWordStart(dir) == this; }
    bool HasClue() const { return HasClue(ACROSS) || HasClue(DOWN); }


    // Flag (GEXT)
    //------------
    void   SetFlag     (unsigned char flag)       { m_flag = flag; }
    void   AddFlag     (unsigned char flag)       { m_flag |=   flag; }
    void   RemoveFlag  (unsigned char flag)       { m_flag &= ~ flag; }
    void   ToggleFlag  (unsigned char flag)
    {
        if (HasFlag(flag))
            RemoveFlag(flag);
        else
            AddFlag(flag);
    }

    unsigned char GetFlag()          const { return m_flag; }
    bool HasFlag(unsigned char flag) const { return (m_flag & flag) != 0; }

    void ReplaceFlag (unsigned char flag1, unsigned char flag2)
    {
        RemoveFlag(flag1);
        AddFlag(flag2);
    }


    // Linked-list
    //------------
    Square * Next(GridDirection dir = ACROSS, FindDirection inc = NEXT)
        { return m_next[dir][inc]; }
    Square * Prev(GridDirection dir = ACROSS, FindDirection inc = NEXT)
        { return m_next[dir][1 - inc]; }
    Square * GetWordStart(GridDirection dir)
        { return m_wordStart[dir]; }
    Square * GetWordEnd(GridDirection dir)
        { return m_wordEnd[dir]; }

    bool IsLast(GridDirection dir, FindDirection inc = NEXT) const
        { return m_isLast [dir][inc]; }

    bool IsFirst(GridDirection dir, FindDirection inc = NEXT) const
        { return IsLast(dir, inc == NEXT ? PREV : NEXT); }

    // Const overloads
    const Square * Next(GridDirection dir = ACROSS, FindDirection inc = NEXT) const
        { return m_next[dir][inc]; }
    const Square * Prev(GridDirection dir = ACROSS, FindDirection inc = NEXT) const
        { return m_next[dir][inc == NEXT ? PREV : NEXT]; }
    const Square * GetWordStart(GridDirection dir) const
        { return m_wordStart[dir]; }
    const Square * GetWordEnd(GridDirection dir) const
        { return m_wordEnd[dir]; }


protected:
    // Location information
    int m_col;
    int m_row;

    // Text
    std::string m_solution;
    std::string m_text;
    char m_asciiSolution;

    // Clue number
    short m_number;

    // Flag (GEXT)
    unsigned char m_flag;

    // Linked-list
    //------------

    // XGrid is responsible for setting the linked-list and spatial info

    // This makes searching the grid ~20x faster, because there are no calls
    // to Grid::At().

    Square * m_next[2][2];  // [ across / down ] [ prev / next ]

    bool m_isLast[2][2];   // [ across / down ] [ prev / next ]

    Square * m_wordStart[2];  // [ across / down ]
    Square * m_wordEnd  [2];  // [ across / down ]

    // Throw an exception if we get a bad string
    void CheckString(const std::string & str);

private:
    // ASCII lookup functions
    static char ToAscii(char ch);
    static char ToAscii(const std::string & str)
        { return str.empty() ? '-' : ToAscii(str.at(0)); }
};



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Square text and solution
//------------------------------------------------------------------------------

inline
void
Square::SetText(const std::string & text)
{
    if (IsSymbol(text))
        m_text = text;
    else
    {
        CheckString(text);
        m_text = text;
        MakeUpper(m_text);
    }
}


inline
void
Square::SetSolution(const std::string & solution, char plain)
{
    SetSolutionRebus(solution);
    SetPlainSolution(plain);
}

inline
void
Square::SetSolution(const std::string & solution)
{
    SetSolution(solution, ToAscii(solution));
}


inline
void
Square::SetSolutionRebus(const std::string & rebus)
{
    if (IsSymbol(rebus))
    {
        SetSolutionSymbol(static_cast<unsigned short>(rebus.at(1)));
    }
    else
    {
        CheckString(rebus);
        m_solution = rebus;
        MakeUpper(m_solution);
    }
}

inline
void
Square::SetSolutionSymbol(unsigned short symbol)
{
    m_solution = "[ ]";
    m_solution.at(1) = static_cast<char>(symbol);
}




inline
bool
Square::Check(bool checkBlank)  const
{
    assert(! m_solution.empty());
    if (IsBlack())
        return true;
    if (IsBlank())
        return ! checkBlank;

    if (HasTextRebus())
        return m_solution == m_text;
    else
        return GetPlainText() == GetPlainSolution();
}




// Information about text and solution
//------------------------------------

inline
bool
Square::HasTextRebus() const
{
    return m_text.length() > 1;
}


inline
bool
Square::HasSolutionRebus() const
{//fix
    return m_solution.length() > 1 || m_solution.at(0) != GetPlainSolution();
}


inline
bool
Square::HasTextSymbol() const
{
    return IsSymbol(m_text);
}


inline
bool
Square::HasSolutionSymbol() const
{
    assert(! m_solution.empty());
    return IsSymbol(m_solution);
}


inline
unsigned short
Square::GetTextSymbol() const
{
    if (! HasTextSymbol())
        throw NoSymbol();
    return static_cast<unsigned short>(m_text.at(1));
}


inline
unsigned short
Square::GetSolutionSymbol() const
{
    if (! HasSolutionSymbol())
        throw NoSymbol();
    return static_cast<unsigned short>(m_solution.at(1));
}






inline
bool
Square::IsValidString(const std::string & str)
{
    if (str == ".")
        return true;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
        if (! IsValidChar(*it) || *it == '.')
            return false;
    return true;
}

inline
void
Square::CheckString(const std::string & str)
{
    if (str.length() > REBUS_ENTRY_LENGTH)
        throw LongString("Blah");
    if (! IsValidString(str))
        throw InvalidString();
}

inline
bool
Square::IsSymbol(const std::string & str)
{
    return str.length() == 3
        && str[0] == '['
        && str[2] == ']';
}


} // namespace puz

#endif // PUZ_SQUARE_H
