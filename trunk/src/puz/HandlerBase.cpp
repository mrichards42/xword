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


#include "HandlerBase.hpp"

//------------------------------------------------------------------------------
// Clues
//------------------------------------------------------------------------------

void
HandlerBase::SetupClues()
{
    // If the clues don't setup correctly, it means there is a mismatch between
    // the grid and the clues.  We can't load the puzzle.
    std::vector<wxString>::iterator clue_it = m_puz->m_clues.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(DIR_ACROSS))
        {
            if (clue_it == m_puz->m_clues.end())
                throw FatalPuzError(_T("Clues and grid don't match."));
            m_puz->m_across.push_back( XPuzzle::Clue(square->m_number,
                                                   *clue_it++) );
        }

        if (square->HasClue(DIR_DOWN))
        {
            if (clue_it == m_puz->m_clues.end())
                throw FatalPuzError(_T("Clues and grid don't match."));
            wxASSERT(clue_it != m_puz->m_clues.end());
            m_puz->m_down.push_back( XPuzzle::Clue(square->m_number,
                                                 *clue_it++) );
        }
    }
}