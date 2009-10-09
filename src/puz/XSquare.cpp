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
// You should have received a copy of ther GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "XSquare.hpp"
#include <wx/module.h>


XSquare::XSquare()
    : m_col(-1),
      m_row(-1),
      m_asciiSolution('\0'),
      m_solution(wxEmptyString),
      m_text(wxEmptyString),
      m_flag(XFLAG_CLEAR),
      m_number(0),
      m_clueFlag(NO_CLUE)
{
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


std::map<wxChar, char> XSquare::sm_charTable;


void
XSquare::InitCharTable()
{
    // For all symbols, replace with the first letter of the symbol
    // We're storing chars (not wxChars), so we don't use _T()

    // Numbers
    sm_charTable[_T('0')] = 'Z';  // [Z]ero
    sm_charTable[_T('1')] = 'O';  // [O]ne
    sm_charTable[_T('2')] = 'T';  // [T]wo
    sm_charTable[_T('3')] = 'T';  // [T]hree
    sm_charTable[_T('4')] = 'F';  // [F]our
    sm_charTable[_T('5')] = 'F';  // [Five
    sm_charTable[_T('6')] = 'S';  // [S]ix
    sm_charTable[_T('7')] = 'S';  // [S]even
    sm_charTable[_T('8')] = 'E';  // [E]ight
    sm_charTable[_T('9')] = 'N';  // [N]ine

    // Symbols (only using those allowed by Across Lite)
    sm_charTable[_T('@')] = 'A';  // [A]t
    sm_charTable[_T('#')] = 'H';  // [H]ash
    sm_charTable[_T('$')] = 'D';  // [D]ollar
    sm_charTable[_T('%')] = 'P';  // [P]ercent
    sm_charTable[_T('&')] = 'A';  // [A]mpersand
    sm_charTable[_T('+')] = 'P';  // [P]lus
    sm_charTable[_T('?')] = 'Q';  // [Q]uestion mark

    // '[' number ']' is used to indicate a webdings value
    // Across Lite replaces these with a blank
    sm_charTable[_T('[')] = '-';

    // Add regular letters
    for (char ch = 'A'; ch <= 'Z'; ++ch)
    {
        // Upper case
        sm_charTable[static_cast<wxChar>(ch)] = ch;
        // Lower case
        sm_charTable[static_cast<wxChar>(wxTolower(ch))] = ch;
    }

    // Other possible values
    sm_charTable[_T('.')] = '.';
}


//------------------------------------------------------------------------------
// XSquareModule
//------------------------------------------------------------------------------

class XSquareModule : public wxModule
{
public:
    XSquareModule() {}
    bool OnInit() { XSquare::InitCharTable(); return true; }
    void OnExit() { ; }

DECLARE_DYNAMIC_CLASS(XSquareModule)
};

IMPLEMENT_DYNAMIC_CLASS(XSquareModule, wxModule)