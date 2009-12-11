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


#ifndef X_GRID_H
#define X_GRID_H

#include <vector>
#include <wx/string.h>
#include <wx/gdicmn.h> // wxPoint
#include "XSquare.hpp"
#include "../utils/ByteArray.hpp"

// Friends
class XGridScrambler;
class HandlerBase;
class Checksummer;
class MyFrame;

enum XGridFlag
{
    XFLAG_NORMAL      = 0x0000,
    XFLAG_NO_SOLUTION = 0x0002,
    XFLAG_SCRAMBLED   = 0x0004
};

enum XGridType
{
    XTYPE_NORMAL      = 0x0001,
    XTYPE_DIAGRAMLESS = 0x0401
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

class XGrid
{
    friend class XGridScrambler;
    friend class HandlerBase;
    friend class Checksummer;
    friend class MyFrame;
public:
    explicit XGrid(size_t width = 0, size_t height = 0);
    ~XGrid();

    // Setup
    //------
    // Fills in XSquare Next / Prev, etc.  Called automatically from SetSize.
    void SetupIteration();
    // Setup numbers, etc.  This must be called after the solution is filled.
    void SetupGrid();

    // Size
    //-----
    void SetSize(size_t width, size_t height);
    size_t GetWidth()  const { return m_width; }
    size_t GetHeight() const { return m_height; }
    int    LastRow()   const { return m_height - 1; }
    int    LastCol()   const { return m_width - 1; }

    bool IsEmpty() const { return m_width == 0 || m_height == 0; }
    void Clear();


    GET_FUNCTION( XSquare *, First,      m_first)
    GET_FUNCTION( XSquare *, Last,       m_last)
    GET_FUNCTION( XSquare *, FirstWhite, m_firstWhite)
    GET_FUNCTION( XSquare *, LastWhite,  m_lastWhite)

    // Access to squares provided as (x,y):
    // i.e. square "b4" is At(1, 3)
    const XSquare & At(size_t col, size_t row) const
        { return m_grid.at(row).at(col); }
    XSquare & At(size_t col, size_t row)
        { return m_grid.at(row).at(col); }

    const XSquare & At(wxPoint pt)       const { return At(pt.x, pt.y); }
          XSquare & At(wxPoint pt)             { return At(pt.x, pt.y); }


    // Searching
    //----------
    // Accepts functions (or function-objects) that subscribe to the template:
    //    bool Function(const XSquare *)
    template<typename T>
    XSquare * FindSquare(XSquare * start,
                         T findFunc,
                         bool direction,
                         bool increment = FIND_NEXT,
                         bool skipBlack = NO_SKIP_BLACK_SQUARES,
                         bool wrapLines = NO_WRAP_LINES);

    // Search starting from the next square
    template<typename T>
    XSquare * FindNextSquare(XSquare * start,
                             T findFunc,
                             bool direction,
                             bool increment = FIND_NEXT,
                             bool skipBlack = NO_SKIP_BLACK_SQUARES,
                             bool wrapLines = NO_WRAP_LINES);

    // Flags
    bool IsScrambled() const { return (m_flag & XFLAG_SCRAMBLED) != 0; }
    bool HasSolution() const { return (m_flag & XFLAG_NO_SOLUTION) == 0; }
    unsigned short GetFlag() const { return m_flag; }

    // Type
    bool IsDiagramless() const { return m_type == XTYPE_DIAGRAMLESS; }
    unsigned short GetType() const { return m_type; }

    // Scrambling
    bool ScrambleSolution  (unsigned short key = 0);
    bool UnscrambleSolution(unsigned short key);
    unsigned short BruteForceUnscramble();
    bool CheckScrambledGrid();

    unsigned short  GetKey()   const { return m_key; }
    unsigned short  GetCksum() const { return m_cksum; }

    bool IsBetween(const XSquare * square,
                   const XSquare * start,
                   const XSquare * end) const;

    void CountClues(size_t * across, size_t * down) const;


    // Check functions
    //----------------
    std::vector<XSquare *> CheckGrid(bool checkBlank = false);
    std::vector<XSquare *> CheckWord(XSquare * start,
                                     XSquare * end,
                                     bool checkBlank = false);
    bool CheckSquare(const XSquare & square, bool checkBlank = false) const
        { return square.Check(checkBlank); }


    // These all operate with ASCII
    // delim is a delimiter between lines
    ByteArray GetGridSolution(const wxString & delim = _T("")) const;
    ByteArray GetGridText    (const wxString & delim = _T("")) const;
    ByteArray GetGext        (const wxString & delim = _T("")) const;

protected:
    typedef std::vector< std::vector<XSquare> > Grid_t;

    Grid_t m_grid;

    size_t m_width, m_height;
    XSquare * m_first;
    XSquare * m_last;
    XSquare * m_firstWhite;
    XSquare * m_lastWhite;

    short m_type;
    short m_flag;

    // These are used for grid scrambling
    unsigned short m_key;
    unsigned short m_cksum;
};


#undef GET_FUNCTION


inline void
XGrid::Clear()
{
    m_type = XTYPE_NORMAL;
    m_flag = XFLAG_NORMAL;
    m_key = 0;
    m_cksum = 0;
    m_first = NULL;
    m_last = NULL;
    m_firstWhite = NULL;
    m_lastWhite = NULL;
    SetSize(0,0);
}


inline bool
XGrid::IsBetween(const XSquare * square,
                 const XSquare * start,
                 const XSquare * end) const
{
    return square->m_col >= start->m_col && square->m_col <= end->m_col
        && square->m_row >= start->m_row && square->m_row <= end->m_row;
}


template <typename T>
XSquare *
XGrid::FindNextSquare(XSquare * start,
                      T findFunc,
                      bool direction,
                      bool increment,
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


template <typename T>
XSquare *
XGrid::FindSquare(XSquare * start,
                  T findFunc,
                  bool direction,
                  bool increment,
                  bool skipBlack,
                  bool wrapLines)
{
    if (start == NULL)
        return NULL;

    for (XSquare * square = start;
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



// Functions / functors for FindSquare
bool FIND_WHITE_SQUARE (const XSquare * square);
bool FIND_BLACK_SQUARE (const XSquare * square);
bool FIND_BLANK_SQUARE (const XSquare * square);

struct FIND_WORD
{
    FIND_WORD(bool direction, const XSquare * square);
    bool operator() (const XSquare * square);
private:
    bool m_direction;
    const XSquare *  m_wordStart;
};

// This looks for square->number, so in order to use it we must give
//    this struct and FindSquare a square at the start of a word
struct FIND_CLUE
{
    FIND_CLUE(bool direction, const XSquare * square);
    bool operator() (const XSquare * square);
private:
    bool m_direction;
    unsigned int m_number;
};

#endif // X_GRID_H