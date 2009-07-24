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
// Set grid data
//------------------------------------------------------------------------------

void
HandlerBase::SetGridSolution(const ByteArray & data)
{
    wxASSERT(data.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    ByteArray::const_iterator it = data.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        // XSquare::Ascii() handles case conversion, etc.
        square->m_asciiSolution = square->Ascii(*it);
        if (wxIslower(*it))
            square->m_solution = static_cast<wxChar>(wxToupper(*it));
        else
            square->m_solution = static_cast<wxChar>(*it);
        ++it;
    }
}


void
HandlerBase::SetGridText(const ByteArray & data)
{
    wxASSERT(data.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    ByteArray::const_iterator it = data.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (islower(*it))
        {
            square->m_flag |= XFLAG_PENCIL;
            square->m_text = static_cast<wxChar>(toupper(*it));
        }
        else
            square->m_text = static_cast<wxChar>(*it);
        ++it;
    }
}




//------------------------------------------------------------------------------
// Clues
//------------------------------------------------------------------------------

void
HandlerBase::SetupClues()
{
    std::vector<wxString>::iterator clue_it = m_puz->m_clues.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if ( (square->m_clueFlag & ACROSS_CLUE) != 0 )
        {
            wxASSERT(clue_it != m_puz->m_clues.end());
            m_puz->m_across.push_back( XPuzzle::Clue(square->m_number,
                                                   *clue_it++) );
        }

        if ( (square->m_clueFlag & DOWN_CLUE) != 0 )
        {
            wxASSERT(clue_it != m_puz->m_clues.end());
            m_puz->m_down.push_back( XPuzzle::Clue(square->m_number,
                                                 *clue_it++) );
        }
    }
    wxASSERT(clue_it == m_puz->m_clues.end());
}




//------------------------------------------------------------------------------
// Set extra sections
//------------------------------------------------------------------------------

void
HandlerBase::SetGext(const ByteArray & data)
{
    wxASSERT(data.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    ByteArray::const_iterator it = data.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->m_flag = *it;
        ++it;
    }
}



void
HandlerBase::SetTextRebus(const std::vector<wxString> & data)
{
    wxASSERT(data.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    std::vector<wxString>::const_iterator it = data.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! it->empty())
        {
            // Deal with symbols
            if (it->at(0) == _T('['))
            {
                if (it->at(it->length() - 1) != _T(']'))
                    throw PuzLoadError(_T("Missing ']' in RUSR section"));

                wxChar num = it->at(1);
                if (num > 255)
                    throw PuzLoadError(_T("Invalid entry in RUSR section"));

                square->m_text = wxString(_T("[")) + num + _T("]");
            }
            else
                square->m_text = *it;
        }
        ++it;
    }
}



void
HandlerBase::SetSolutionRebus(const std::map<unsigned char, wxString> & table,
                              const ByteArray & grid)
{
    // Load the values from the grid rebus into the solution grid.
    // Each byte corresponds with a square, and is the index to a value in
    // rebusTable (or 0 for no rebus).

    ByteArray::const_iterator rebus_it = grid.begin();

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (*rebus_it > 0)
        {
            std::map<unsigned char, wxString>::const_iterator it;
            it = table.find(*rebus_it);
            if (it == table.end())
                throw PuzLoadError(_T("Invalid value in GRBS section"));
            square->m_solution = it->second;
        }
        ++rebus_it;
    }
}





//------------------------------------------------------------------------------
// Version
//------------------------------------------------------------------------------


void
HandlerBase::SetVersion(unsigned short major, unsigned short minor)
{
    wxASSERT(major < 10 && minor < 10);
    m_puz->m_version = major * 10 + minor;
}


wxString
HandlerBase::GetVersionString() const
{
    // A version with major or minor parts greater than 9 would put too many
    // bytes into the file
    wxASSERT(m_puz->m_version < 100);
    return wxString::Format(_T("%d.%d"),
                            m_puz->m_version / 10,
                            m_puz->m_version % 10);
}

