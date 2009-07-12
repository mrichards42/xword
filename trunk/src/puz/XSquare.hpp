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


// TODO:
//   - Make the chars wxChars, so we can better support unicode
//   - Or perhaps have both standard and unicode m_solution

#ifndef X_SQUARE_H
#define X_SQUARE_H


// GEXT flags (just learned the one for X and for a circle!)
// Perhaps there are some flags that use 0x01 - 0x08?
const wxByte XFLAG_CLEAR  = 0x00;
const wxByte XFLAG_BLACK  = 0x10;
const wxByte XFLAG_X      = 0x20;
const wxByte XFLAG_RED    = 0x40;
const wxByte XFLAG_CIRCLE = 0x80;


// Clue types
enum ClueType
{
    NO_CLUE     = 0x00,
    ACROSS_CLUE = 0x01,
    DOWN_CLUE   = 0x02,
};

// Forward/backward
enum FindIncrement
{
    FIND_PREV,
    FIND_NEXT,
};


// Directions
enum GridDirection
{
    DIR_ACROSS,
    DIR_DOWN
};

// Macro to simplify GetX() functions
#define GET_FUNCTION(type, declaration, member)         \
    const type declaration const { return member; }   \
          type declaration       { return member; }   \



class XGrid;

class XSquare
{
friend class XGrid;

public:
    // Defaults to a black square
    explicit XSquare(int a_col = -1,
                     int a_row = -1,
                     wxChar a_solution = '.',
                     wxChar a_text = '.',
                     wxByte a_flag = XFLAG_CLEAR,
                     short a_number = 0,
                     short a_clueFlag = NO_CLUE,
                     const wxString & a_rebus = wxEmptyString,
                     const wxString & a_rebusSol = wxEmptyString,
                     unsigned short a_rebusSym = 0,
                     unsigned short a_rebusSymSol = 0)
        : col(a_col),
          row(a_row),
          solution(a_solution),
          text(a_text),
          flag(a_flag),
          number(a_number),
          clueFlag(a_clueFlag),
          rebus(a_rebus),
          rebusSol(a_rebusSol),
          rebusSym(a_rebusSym),
          rebusSymSol(a_rebusSymSol)
    {
        clue[DIR_ACROSS] = 0;
        clue[DIR_DOWN]   = 0;

        m_next[DIR_ACROSS][FIND_PREV] = NULL;
        m_next[DIR_ACROSS][FIND_NEXT] = NULL;
        m_next[DIR_DOWN]  [FIND_PREV] = NULL;
        m_next[DIR_DOWN]  [FIND_NEXT] = NULL;

        m_isLast[DIR_ACROSS][FIND_PREV] = false;
        m_isLast[DIR_ACROSS][FIND_NEXT] = false;
        m_isLast[DIR_DOWN]  [FIND_PREV] = false;
        m_isLast[DIR_DOWN]  [FIND_NEXT] = false;

        m_wordStart[DIR_ACROSS] = NULL;
        m_wordEnd  [DIR_ACROSS] = NULL;
        m_wordStart[DIR_DOWN]   = NULL;
        m_wordEnd  [DIR_DOWN]   = NULL;
    }

    ~XSquare() {}

    // Location information
    int col;
    int row;

    // Square information
    wxChar   solution;
    wxChar   text;
    wxByte flag;

    // Rebus stuff
    wxString rebus;
    wxString rebusSol;
    unsigned short rebusSym;
    unsigned short rebusSymSol;

    // Clue information
    short number;
    short clueFlag;
    short clue[2]; // clue this belongs to; across and down

    // Flag functions
    void ReplaceFlag (wxByte flag1, wxByte flag2)
        { RemoveFlag(flag1); AddFlag(flag2); }
    void AddFlag     (wxByte a_flag)       { flag |=   a_flag; }
    void RemoveFlag  (wxByte a_flag)       { flag &= ~ a_flag; }
    bool HasFlag     (wxByte a_flag) const
        { return (flag & a_flag) != 0; }

    // Information functions
    bool IsRebus()      const { return ! rebusSol.empty(); }
    bool HasRebusText() const { return ! rebus.empty(); }
    bool IsWhite()      const { return ! IsBlack(); }
    bool IsBlack()      const { return solution == '.'; }
    bool IsBlank()      const { return text == '-'; }

    bool Check(bool checkBlank = false)  const
    {
        if (IsBlack() || (IsBlank() && ! checkBlank))
            return true;

        if (IsRebus() || HasRebusText())
            return rebus == rebusSol;

        return solution == text;
    }

    // Linked-list
    //     - functions (rather than member access) needed for const-corectness
    GET_FUNCTION(XSquare *,
                Next(bool direction = DIR_ACROSS, bool increment = FIND_NEXT),
                m_next[direction][increment])

    GET_FUNCTION(XSquare *,
                Prev(bool direction = DIR_ACROSS, bool increment = FIND_NEXT),
                m_next[direction][1-increment])

    GET_FUNCTION(XSquare *,
                WordStart(bool direction),
                m_wordStart[direction])

    GET_FUNCTION(XSquare *,
                WordEnd(bool direction),
                m_wordEnd[direction])


    // Test to see if word is the last or first in its column/row
    bool IsLast(bool direction, int increment = FIND_NEXT) const
    {
        return m_isLast [direction][increment];
    }

    bool IsFirst(bool direction, int increment = FIND_NEXT) const
    {
        return IsLast(direction, ! increment);
    }

protected:
    // Note that these protected members will be set by XGrid
    // when the grid is resized

    // Linked-list functionality (across/down, previous/next)
    // This is incredibly important, as it makes searching the grid ~20x faster,
    //     because there are no calls to At() and no loop variables
    XSquare * m_next[2][2];

    // last or first in column row (based on direction and increment)
    bool m_isLast[2][2];

    // These two aren't separated by increment (start/end) because they will 
    // only be called when it is known whether start or end is required
    XSquare * m_wordStart[2];
    XSquare * m_wordEnd  [2];
};


#undef GET_FUNCTION

#endif // X_SQUARE_H
