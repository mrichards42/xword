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
#include "puzstring.hpp"
#include "exceptions.hpp"
#include "util.hpp"

namespace puz {

const int REBUS_ENTRY_LENGTH = 8;

// GEXT flags
enum GextFlag
{
    FLAG_CLEAR  = 0x00,
    FLAG_PENCIL = 0x08,
    FLAG_BLACK  = 0x10,
    FLAG_X      = 0x20,
    FLAG_RED    = 0x40,
    FLAG_CIRCLE = 0x80,

    ACROSS_LITE_MASK = FLAG_CLEAR |
                       FLAG_PENCIL |
                       FLAG_BLACK |
                       FLAG_X |
                       FLAG_RED |
                       FLAG_CIRCLE,

    FLAG_COLOR = 0x100,
    FLAG_MISSING = 0x200
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
    ACROSS = 0,
    DOWN
};

// Facilitate swapping across and down
inline GridDirection
SwapDirection(GridDirection dir)
{
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
    Square();
    Square(const Square & other);

public:
    Square & operator=(const Square & other);

    // Location information
    //---------------------
    short GetCol() const { return m_col; }
    short GetRow() const { return m_row; }


    // General information
    //--------------------
    bool IsSolutionBlank() const { return m_solution == Blank; }
    bool IsWhite()         const { return ! IsBlack() && ! IsMissing(); }
    bool IsBlack()         const { return m_solution == Black; }
    bool IsBlank()         const { return m_text == Blank; }

    // Text
    //-----

    // Text and Solution are guaranteed not to be empty.
    const string_t & GetText()     const { return m_text; }
    const string_t & GetSolution() const { return m_solution; }

    void SetText    (const string_t & text);
    void SetSolution(const string_t & solution);
    void SetSolution(const string_t & solution, char plain);
    void SetPlainSolution(char plain); // Leave solution rebus unchanged
    void SetSolutionRebus(const string_t & rebus); // Leave plain solution unchanged
    void SetSolutionSymbol(unsigned char symbol); // Must set plain solution separately

    bool Check(bool checkBlank = false, bool strictRebus = false) const;

    char GetPlainText()     const { return ToPlain(m_text); }
    char GetPlainSolution() const { return m_asciiSolution; }

    bool HasTextRebus()      const;
    bool HasSolutionRebus()  const;
    bool HasTextSymbol()     const;
    bool HasSolutionSymbol() const;

    char_t GetTextSymbol()      const;
    char_t GetSolutionSymbol()  const;

    // Character lookup functions
    static bool IsValidChar(int ch);
    static bool IsValidString(const string_t & str);
    static bool IsSymbol(const string_t & str);
    static char_t ToGrid(int ch);
    static string_t ToGrid(const string_t & str);
    static char ToPlain(int ch);
    static char ToPlain(const string_t & str);

    // Solutions with special meaning
    static char_t * Blank;
    static char_t * Black;

    // Clue
    //-----
    bool HasNumber() const                  { return ! m_number.empty(); }
    const string_t & GetNumber() const      { return m_number; }
    void SetNumber(const string_t & number) { m_number = number; }
    void SetNumber(int number)              { m_number = ToString(number); }

    // Clue
    //-----
    bool HasClue() const { return HasClue(ACROSS) || HasClue(DOWN); }
    bool HasClue(GridDirection dir) const        { return m_hasClue[dir]; }
    void SetClue(GridDirection dir, bool clue) { m_hasClue[dir] = clue; }
    // Is this square is a position that would usually have a clue ?
    bool WantsClue() const { return WantsClue(ACROSS) || WantsClue(DOWN); }
    bool WantsClue(GridDirection dir) const;

    // Flag (GEXT)
    //------------
    void         SetFlag (unsigned int flag) { m_flag = flag; }
    unsigned int GetFlag() const             { return m_flag; }
    bool         HasFlag(unsigned int flag) const
        { return (m_flag & flag) != 0; }

    void   AddFlag     (unsigned int flag, bool doit = true)
    {
        doit ? m_flag |= flag : m_flag &= ~ flag;
    }
    void   RemoveFlag  (unsigned int flag) { AddFlag(flag, false); }
    void   ToggleFlag  (unsigned int flag)
    {
        AddFlag(flag, ! HasFlag(flag));
    }

    bool HasCircle() const { return HasFlag(FLAG_CIRCLE); }
    void SetCircle(bool doit = true) { AddFlag(FLAG_CIRCLE, doit); }

    bool IsMissing() const { return HasFlag(FLAG_MISSING); }
    void SetMissing(bool doit = true) { AddFlag(FLAG_MISSING, doit); }

    bool HasColor() const { return HasFlag(FLAG_COLOR); }
    void SetColor(unsigned char red, unsigned char green, unsigned char blue);
    void RemoveColor();

    // Color (public so we don't need a GetColor)
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;

    // Linked-list
    //------------
    Square * Next(GridDirection dir = ACROSS, FindDirection inc = NEXT)
        { return m_next[dir][inc]; }
    Square * Prev(GridDirection dir = ACROSS, FindDirection inc = NEXT)
        { return m_next[dir][1 - inc]; }
    Square * GetWordStart(GridDirection dir);
    Square * GetWordEnd(GridDirection dir);

    bool HasWord(GridDirection dir) const;

    bool IsLast(GridDirection dir, FindDirection inc = NEXT) const
        { return m_isLast[dir][inc]; }
    bool IsFirst(GridDirection dir, FindDirection inc = NEXT) const
        { return IsLast(dir, inc == NEXT ? PREV : NEXT); }

    // Const overloads
    const Square * Next(GridDirection dir = ACROSS, FindDirection inc = NEXT) const
        { return m_next[dir][inc]; }
    const Square * Prev(GridDirection dir = ACROSS, FindDirection inc = NEXT) const
        { return m_next[dir][inc == NEXT ? PREV : NEXT]; }
    const Square * GetWordStart(GridDirection dir) const;
    const Square * GetWordEnd(GridDirection dir) const;


protected:
    // Location information
    int m_col;
    int m_row;

    // Text
    string_t m_solution;
    string_t m_text;
    char m_asciiSolution;

    // Clue
    string_t m_number;
    bool m_hasClue[2]; // [across / down]

    // Flag (GEXT)
    unsigned int m_flag;

    // Linked-list
    //------------

    // XGrid is responsible for setting the linked-list and spatial info

    // This makes searching the grid ~20x faster, because there are no calls
    // to Grid::At().

    Square * m_next[2][2];  // [ across / down ] [ prev / next ]

    bool m_isLast[2][2];   // [ across / down ] [ prev / next ]

    mutable Square * m_wordStart[2];  // [ across / down ]
    mutable Square * m_wordEnd  [2];  // [ across / down ]
};
} // namespace puz

#endif // PUZ_SQUARE_H
