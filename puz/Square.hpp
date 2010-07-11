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

//#define PUZ_CHECK_STRINGS
#ifdef PUZ_CHECK_STRINGS
#   define PuzCheckString(str) Square::CheckString(str)
#else
#   define PuzCheckString
#endif

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
    void SetSolutionSymbol(unsigned char symbol); // Must set plain solution separately

    bool Check(bool checkBlank = false, bool strictRebus = false) const;

    char GetPlainText()     const { return ToPlain(m_text); }
    char GetPlainSolution() const { return m_asciiSolution; }

    bool HasTextRebus()      const;
    bool HasSolutionRebus()  const;
    bool HasTextSymbol()     const;
    bool HasSolutionSymbol() const;

    unsigned char GetTextSymbol()      const;
    unsigned char GetSolutionSymbol()  const;

    // Character lookup functions
    static bool IsValidChar(char ch);
    static bool IsValidString(const std::string & str);
    static bool IsSymbol(const std::string & str);
    static unsigned char ToUpper(unsigned char ch);
    static std::string ToUpper(const std::string & str);
    static unsigned char ToPlain(unsigned char ch);
    static unsigned char ToPlain(const std::string & str)
        { return str.empty() ? '-' : ToPlain(str.at(0)); }

    // Clue
    //-----
    short GetNumber() const { return m_number; }
    bool HasClue(GridDirection dir = NONE) const
       { return dir == NONE
                    ? HasClue(ACROSS) || HasClue(DOWN)
                    : GetWordStart(dir) == this; }


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

#ifdef PUZ_CHECK_STRINGS
    // Throw an exception if we get a bad string
    void CheckString(const std::string & str);
#endif

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
        PuzCheckString(text);
        m_text = ToUpper(text);
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
    SetSolution(solution, ToPlain(solution));
}


inline
void
Square::SetSolutionRebus(const std::string & rebus)
{
    if (rebus.empty())
        throw InvalidString();
    if (IsSymbol(rebus))
    {
        SetSolutionSymbol(static_cast<unsigned char>(rebus.at(1)));
    }
    else
    {
        PuzCheckString(rebus);
        m_solution = ToUpper(rebus);
    }
}

inline
void
Square::SetSolutionSymbol(unsigned char symbol)
{
    m_solution = "[ ]";
    m_solution.at(1) = static_cast<char>(symbol);
}




inline
bool
Square::Check(bool checkBlank, bool strictRebus)  const
{
    assert(! m_solution.empty());
    if (IsBlack())
        return true;
    if (IsBlank())
        return ! checkBlank;

    if (strictRebus || HasTextRebus())
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
    const size_t len = m_text.length();
    return len > 1 || (len > 0 && m_text.at(0) != GetPlainText());
}


inline
bool
Square::HasSolutionRebus() const
{
    assert(! m_solution.empty());
    const size_t len = m_solution.length();
    return len > 1 || (len > 0 && m_solution.at(0) != GetPlainSolution());
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
unsigned char
Square::GetTextSymbol() const
{
    if (! HasTextSymbol())
        throw NoSymbol();
    return static_cast<unsigned char>(m_text.at(1));
}


inline
unsigned char
Square::GetSolutionSymbol() const
{
    if (! HasSolutionSymbol())
        throw NoSymbol();
    return static_cast<unsigned char>(m_solution.at(1));
}




inline
bool
Square::IsValidString(const std::string & str)
{
    if (str == "." || str.empty())
        return true;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
        if (! IsValidChar(*it) || *it == '.')
            return false;
    return true;
}

#ifdef PUZ_CHECK_STRINGS
inline
void
Square::CheckString(const std::string & str)
{
    if (str.length() > REBUS_ENTRY_LENGTH)
        throw LongString("Blah");
    if (! IsValidString(str))
        throw InvalidString();
}
#endif // PUZ_CHECK_STRINGS

inline
bool
Square::IsSymbol(const std::string & str)
{
    return str.length() == 3
        && str[0] == '['
        && str[2] == ']';
}

inline
std::string
Square::ToUpper(const std::string & str)
{
    std::string ret = str;
    std::string::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it)
        *it = ToUpper(*it);
    return ret;
}

} // namespace puz

#undef PuzCheckString

#endif // PUZ_SQUARE_H
