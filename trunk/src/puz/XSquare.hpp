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


#ifndef X_SQUARE_H
#define X_SQUARE_H

#include <wx/string.h>
#include <map>



// GEXT flags
enum XFlag
{
    XFLAG_CLEAR  = 0x00,
    XFLAG_PENCIL = 0x08,
    XFLAG_BLACK  = 0x10,
    XFLAG_X      = 0x20,
    XFLAG_RED    = 0x40,
    XFLAG_CIRCLE = 0x80
};



// Clue flags
enum ClueFlag
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

// Used with Check()
enum CheckTest
{
    CHECK_BLANK,
    NO_CHECK_BLANK
};


// To be used as friends
class XGrid;
class HandlerBase;
class XGridScrambler;
class XSquareModule;

class XSquare
{
friend class XGrid;
friend class XGridScrambler;
friend class HandlerBase;
friend class XSquareModule;

public:
    XSquare();

    // Location information
    //---------------------
    short GetCol() const { return m_col; }
    short GetRow() const { return m_row; }


    // General information
    //--------------------
    bool IsWhite()      const { return ! IsBlack(); }
    bool IsBlack()      const { return m_solution == _T("."); }
    bool IsBlank()      const { return m_text == _T("-"); }


    // Text
    //-----
    const wxString & GetText()     const { return m_text; }
    const wxString & GetSolution() const { return m_solution; }

    char GetPlainText()     const { return Ascii(m_text); }
    char GetPlainSolution() const { return m_asciiSolution; }

    bool HasTextRebus()      const;
    bool HasSolutionRebus()  const;
    bool HasTextSymbol()     const;
    bool HasSolutionSymbol() const;

    int GetTextSymbol()      const;
    int GetSolutionSymbol()  const;

    void SetText(const wxString & text) { m_text = text; }

    bool Check(bool checkBlank = NO_CHECK_BLANK) const;


    // Clue
    //-----
    short GetNumber() const { return m_number; }
    short GetClueFlag() const { return m_clueFlag; }
    bool HasClue(bool direction) const
        { return (m_clueFlag & (direction + 1)) != 0; }


    // Flag (GEXT)
    //------------
    bool HasFlag     (wxByte flag) const { return (m_flag & flag) != 0; }
    void AddFlag     (wxByte flag)       { m_flag |=   flag; }
    void RemoveFlag  (wxByte flag)       { m_flag &= ~ flag; }
    void ReplaceFlag (wxByte flag1, wxByte flag2)
        { RemoveFlag(flag1); AddFlag(flag2); }


    // Linked-list
    //------------
    XSquare * Next(bool dir = DIR_ACROSS, bool inc = FIND_NEXT)
        { return m_next[dir][inc]; }
    XSquare * Prev(bool dir = DIR_ACROSS, bool inc = FIND_NEXT)
        { return m_next[dir][!inc]; }
    XSquare * GetWordStart(bool dir)
        { return m_wordStart[dir]; }
    XSquare * GetWordEnd(bool dir)
        { return m_wordEnd[dir]; }

    bool IsLast(bool direction, int increment = FIND_NEXT) const
        { return m_isLast [direction][increment]; }

    bool IsFirst(bool direction, int increment = FIND_NEXT) const
        { return IsLast(direction, ! increment); }

    // Const overloads
    const XSquare * Next(bool dir = DIR_ACROSS, bool inc = FIND_NEXT) const
        { return m_next[dir][inc]; }
    const XSquare * Prev(bool dir = DIR_ACROSS, bool inc = FIND_NEXT) const
        { return m_next[dir][!inc]; }
    const XSquare * GetWordStart(bool dir) const
        { return m_wordStart[dir]; }
    const XSquare * GetWordEnd(bool dir) const
        { return m_wordEnd[dir]; }


protected:
    // Location information
    int m_col;
    int m_row;

    // Text
    wxString m_solution;
    wxString m_text;
    char m_asciiSolution;

    // Clue number
    short m_number;
    short m_clueFlag;

    // Flag (GEXT)
    wxByte m_flag;

    // Linked-list
    //------------

    // XGrid is responsible for setting the linked-list and spatial info

    // This makes searching the grid ~20x faster, because there are no calls
    // to At().

    XSquare * m_next[2][2];  // [ across / down ] [ prev / next ]

    bool m_isLast[2][2];   // [ across / down ] [ prev / next ]

    XSquare * m_wordStart[2];  // [ across / down ]
    XSquare * m_wordEnd  [2];  // [ across / down ]


private:
    // Character replacement table (to ASCII)
    //---------------------------------------
    static std::map<wxChar, char> sm_charTable;
    static void InitCharTable();

    // ASCII lookup functions
    static char Ascii(wxChar chr);
    static char Ascii(const wxString & str) { return Ascii(str.at(0)); }
};



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------

inline
bool
XSquare::HasTextRebus() const
{
    wxASSERT(! m_text.empty());
    return ! m_text.IsSameAs( static_cast<wxChar>(GetPlainText()) );
}


inline
bool
XSquare::HasSolutionRebus() const
{
    wxASSERT(! m_solution.empty());
    return ! m_solution.IsSameAs( static_cast<wxChar>(GetPlainSolution()) );
}


inline
bool
XSquare::HasTextSymbol() const
{
    wxASSERT(! m_text.empty());
    return m_text.length() == 3
        && m_text.at(0) == _T('[') && m_text.at(2) == _T(']');
}


inline
bool
XSquare::HasSolutionSymbol() const
{
    wxASSERT(! m_solution.empty());
    return m_solution.length() == 3
        && m_solution.at(0) == _T('[') && m_solution.at(2) == _T(']');
}


inline
int
XSquare::GetTextSymbol() const
{
    wxASSERT(HasTextSymbol());
    return static_cast<int>(m_text.at(1));
}


inline
int
XSquare::GetSolutionSymbol() const
{
    wxASSERT(HasSolutionSymbol());
    return static_cast<int>(m_solution.at(1));
}




inline
bool
XSquare::Check(bool checkBlank)  const
{
    wxASSERT(! m_text.empty() && ! m_solution.empty());
    if (IsBlack())
        return true;
    if (IsBlank())
        return ! checkBlank;

    if (HasTextRebus())
        return m_solution == m_text;
    else
        return GetPlainText() == GetPlainSolution();
}




//------------------------------------------------------------------------------
// Character table functions
//------------------------------------------------------------------------------

inline
char
XSquare::Ascii(wxChar chr)
{
    wxASSERT(sm_charTable.find(chr) != sm_charTable.end());
    return sm_charTable[chr];
}

#endif // X_SQUARE_H
