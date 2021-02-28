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


#ifndef PUZ_SQUARE_H
#define PUZ_SQUARE_H

#include <cassert>
#include <cmath>
#include <vector>
#include "puzstring.hpp"
#include "exceptions.hpp"

namespace puz {

const int REBUS_ENTRY_LENGTH = 8;

// GEXT flags
enum GextFlag
{
    // Across Lite flags
    FLAG_CLEAR     = 0x00, // Nothing
    FLAG_PENCIL    = 0x08, // Pencil entry
    FLAG_BLACK     = 0x10, // Checked/was incorrect
    FLAG_X         = 0x20, // Checked/currently incorrect
    FLAG_REVEALED  = 0x40, // Revealed
    FLAG_CIRCLE    = 0x80, // Has a circle

    ACROSS_LITE_MASK = FLAG_CLEAR |
                       FLAG_PENCIL |
                       FLAG_BLACK |
                       FLAG_X |
                       FLAG_REVEALED |
                       FLAG_CIRCLE,

    // Additional Flags
    FLAG_COLOR     = 0x100, // Has a color
    FLAG_MISSING   = 0x200, // No square
    FLAG_CORRECT   = 0x400, // Checked/correct
    FLAG_THEME     = 0x800, // A theme square
    FLAG_ANNOTATION      = 0x1000, // An annotation square (aka "clue" square in JPZ)

    // Check square options
    FLAG_CHECK_MASK = FLAG_X |
                      FLAG_REVEALED |
                      FLAG_CORRECT
};


// Forward/backward
enum FindDirection
{
    PREV = 0,
    NEXT,
};


// Grid directions, in angles
enum GridDirection
{
    ACROSS = 0,
    DOWN   = 270,

    UP     = 90,
    LEFT   = 180,
    RIGHT  = ACROSS,

    NORTH = UP,
    SOUTH = DOWN,
    EAST  = RIGHT,
    WEST  = LEFT,

    DIAGONAL_NE = 45,
    DIAGONAL_NW = 135,
    DIAGONAL_SW = 225,
    DIAGONAL_SE = 315
};

// GridDirection functions
inline GridDirection
ConstrainDirection(unsigned short dir)
{
    // Make the direction fit into the GridDirection enum.
    return GridDirection(((dir % 360) / 45) * 45);
}

inline unsigned short
InvertDirection(unsigned short dir)
{
    return (dir + 180) % 360;
}

inline bool
IsHorizontal(unsigned short dir)
{
    return dir == LEFT || dir == RIGHT;
}

inline bool
IsVertical(unsigned short dir)
{
    return dir == UP || dir == DOWN;
}

inline bool
IsDiagonal(unsigned short dir)
{
    return (dir % 90) != 0;
}

// This used to be called "IsSameDirection", which is
// misleading, but this name is just awkward
inline bool
AreInLine(unsigned short dir1, unsigned short dir2)
{
    return dir1 == dir2 || dir1 == InvertDirection(dir2);
}


// Marks
enum CornerMark
{
    MARK_TL = 0,
    MARK_TR,
    MARK_BL,
    MARK_BR
};

// Bars
enum Bar
{
    BAR_TOP = 0,
    BAR_LEFT,
    BAR_RIGHT,
    BAR_BOTTOM
};

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
    // These are defined in terms of text, not solution.
    // This makes certain things easier for diagramless puzzles.
    // SetSolution(Square::Black) also calls SetText(Square::Black) for
    // consistency, so if your puzzle is diagramless, you will have to
    // explicitly call SetText("") on black squares to make them empty.
    bool IsWhite()         const { return !IsBlack() && !IsMissing(); }
    bool IsBlack()         const { return m_text == Black || IsAnnotation(); }
    bool IsBlank()         const { return m_text == Blank; }
    // Corresponding functions for the solution
    bool IsSolutionWhite() const { return !IsSolutionBlack() && !IsMissing(); }
    bool IsSolutionBlack() const { return m_solution == Black || IsAnnotation(); }
    bool IsSolutionBlank() const { return m_solution == Blank; }

    // Text
    //-----

    // Text and Solution are guaranteed not to be empty.
    const string_t & GetText()     const { return m_text; }
    const string_t & GetSolution() const { return m_solution; }

    void SetText    (const string_t & text, bool propagate = true);
    void SetSolution(const string_t & solution);
    void SetSolution(const string_t & solution, char plain);
    void SetPlainSolution(char plain); // Leave solution rebus unchanged
    void SetSolutionRebus(const string_t & rebus); // Leave plain solution unchanged
    void SetSolutionSymbol(unsigned char symbol); // Must set plain solution separately

    void SetBlack() { SetSolution(Black); }

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
    static const char_t * Blank;
    static const char_t * Black;

    // Number
    //-----
    bool HasNumber() const                  { return ! m_number.empty(); }
    const string_t & GetNumber() const      { return m_number; }
    void SetNumber(const string_t & number) { m_number = number; }
    void SetNumber(int number)              { m_number = ToString(number); }

    // Corner marks
    string_t m_mark[4];

    // Bars
    bool m_bars[4];

    // Clue
    //-----
    // Is this square is a position that would usually have a clue ?
    bool WantsClue() const { return WantsClue(ACROSS) || WantsClue(DOWN); }
    bool WantsClue(GridDirection dir) const;
    bool SolutionWantsClue(GridDirection dir) const;
    bool SolutionWantsClue() const
        { return SolutionWantsClue(ACROSS) || SolutionWantsClue(DOWN); }

    // Flags
    //------
    void         SetFlag (unsigned int flag, bool propagate = true) {
        m_flag = flag;
        if (propagate && !m_partner.empty()) {
            for (std::vector<Square*>::iterator it = m_partner.begin(); it != m_partner.end(); ++it)
                (*it)->SetFlag(flag, false);
        }
    }
    unsigned int GetFlag() const             { return m_flag; }
    bool         HasFlag(unsigned int flag) const
        { return (m_flag & flag) != 0; }

    void   AddFlag     (unsigned int flag, bool doit = true)
    {
        SetFlag(doit ? m_flag | flag : m_flag & ~ flag);
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

    bool IsTheme() const { return HasFlag(FLAG_THEME); }
    void SetTheme(bool doit = true) { AddFlag(FLAG_THEME, doit); }

    bool IsAnnotation() const { return HasFlag(FLAG_ANNOTATION); }
    void SetAnnotation(bool doit = true) { AddFlag(FLAG_ANNOTATION, doit); }

    bool HasColor() const { return HasFlag(FLAG_COLOR); }
    void SetColor(unsigned char red, unsigned char green, unsigned char blue);
    void SetColor(const string_t & hexcolor); // XXX or XXXXXX (# is optional)
    string_t GetHtmlColor() const;
    void SetHighlight(bool doit = true);
    bool HasHighlight() const;
    void RemoveColor();

    // Color (public so we don't need a GetColor)
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;

    // Image
    bool HasImage() const { return ! m_imagedata.empty(); }
    std::string m_imageformat;
    std::string m_imagedata;

    // Linked-list
    //------------
    Square * Next(GridDirection dir = ACROSS) { return m_next[dir]; }
    Square * Prev(GridDirection dir = ACROSS) { return m_next[(GridDirection)InvertDirection(dir)]; }

    bool IsLast(GridDirection dir) const
    {
        const Square * next = m_next[dir];
        if (! next)
            return true;
        switch(dir)
        {
            case LEFT:
            case RIGHT:
                return next->GetRow() != GetRow();
            case UP:
            case DOWN:
                return next->GetCol() != GetCol();
            case DIAGONAL_SE:
                return next->GetCol() != GetCol() + 1 || next->GetRow() != GetRow() + 1;
            case DIAGONAL_SW:
                return next->GetCol() != GetCol() - 1 || next->GetRow() != GetRow() + 1;
            case DIAGONAL_NW:
                return next->GetCol() != GetCol() - 1 || next->GetRow() != GetRow() - 1;
            case DIAGONAL_NE:
                return next->GetCol() != GetCol() + 1 || next->GetRow() != GetRow() - 1;
            default:
                assert(false);
                return true;
        }
    }
    bool IsFirst(GridDirection dir) const { return IsLast((GridDirection)InvertDirection(dir)); }

    // Const overloads
    const Square * Next(GridDirection dir = ACROSS) const
        { return m_next[dir]; }
    const Square * Prev(GridDirection dir = ACROSS) const
        { return m_next[(GridDirection)InvertDirection(dir)]; }

    // These should not be used to actually find the word . . . they calculate
    // the word start and end using FindWordBoundary.
    const Square * GetWordStart(GridDirection dir) const;
    const Square * GetWordEnd(GridDirection dir) const;
    const Square * GetSolutionWordStart(GridDirection dir) const;
    const Square * GetSolutionWordEnd(GridDirection dir) const;
    Square * GetWordStart(GridDirection dir);
    Square * GetWordEnd(GridDirection dir);
    Square * GetSolutionWordStart(GridDirection dir);
    Square * GetSolutionWordEnd(GridDirection dir);
    static const Square * FindWordBoundary(const Square * square, GridDirection dir);
    static const Square * FindSolutionWordBoundary(const Square * square, GridDirection dir);
    static Square * FindWordBoundary(Square * square, GridDirection dir);
    static Square * FindSolutionWordBoundary(Square * square, GridDirection dir);
    bool HasWord(GridDirection dir) const;

    bool IsBetween(const Square * start, const Square * end) const;

    std::vector<Square*> GetPartnerSquares() const { return m_partner; }
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

    // Flag (GEXT)
    unsigned int m_flag;

    // Partner squares (for Acrostics and Coded puzzles)
    std::vector<Square*> m_partner;

    // Linked-list
    //------------

    // XGrid is responsible for setting the linked-list and spatial info

    // This makes searching the grid ~20x faster, because there are no calls
    // to Grid::At().

    class SquareDirectionMap
    {
    public:
        Square * m_map[8]; // [ E, NE, N, NW, W, SW, S, SE]
        Square * & operator[](GridDirection dir) { return m_map[dir / 45]; }
        Square * const & operator[](GridDirection dir) const { return m_map[dir / 45]; }
        void clear()
        {
            m_map[0] = m_map[1] = m_map[2] = m_map[3] =
            m_map[4] = m_map[5] = m_map[6] = m_map[7] = NULL;
        }
    } m_next;
};

// Return the direction (angle) between two squares
static const double PI = std::atan(1.0)*4;
inline unsigned short GetDirection(const Square & first, const Square & second)
{
    // Note that the y coords are reversed, because our grid coordinate system
    // is also reversed.
    double radians = std::atan2(
        double(first.GetRow() - second.GetRow()), // y1 - y2
        double(second.GetCol() - first.GetCol())  // x2 - x1
    );
    return (unsigned short)((radians * 180. / PI) + 360) % 360;
}

} // namespace puz

#endif // PUZ_SQUARE_H
