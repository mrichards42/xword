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


#ifndef PUZ_EVENT_H
#define PUZ_EVENT_H

#include <wx/event.h>
#include <wx/gdicmn.h>  // wxPoint
#include "../puz/Clue.hpp"
#include "../puz/Word.hpp"
#include "../puz/Square.hpp"

class wxPuzEvent
    : public wxCommandEvent
{
public:
    wxPuzEvent(wxEventType commandType = wxEVT_NULL, int id = 0);

    puz::Clue * GetClue() { return m_clue; }
    const puz::Word * GetWord() { return m_word; }
    puz::Square * GetSquare() { return m_square; }

    void SetClue(puz::Clue * clue) { m_clue = clue; }
    void SetWord(const puz::Word * word) { m_word = word; }
    void SetSquare(puz::Square * square) { m_square = square; }

    // required for sending with wxPostEvent()
    wxEvent *Clone(void) const { return new wxPuzEvent(*this); }


private:
    puz::Clue * m_clue;
    const puz::Word * m_word;
    puz::Square * m_square;
};


// Use DECLARE_LOCAL_EVENT_TYPE instead of DECLARE_EVENT_TYPE because
// wxWidgets built as a DLL will try to import events declared with
// DECLARE_EVENT_TYPE from the wxWidgets DLL
DECLARE_LOCAL_EVENT_TYPE( wxEVT_PUZ_GRID_FOCUS, -1 )
DECLARE_LOCAL_EVENT_TYPE( wxEVT_PUZ_CLUE_FOCUS, -1 )
DECLARE_LOCAL_EVENT_TYPE( wxEVT_PUZ_LETTER,     -1 )



// Event handler function definition
typedef void (wxEvtHandler::*wxPuzEventFunction)(wxPuzEvent&);



// Event table macros
#define EVT_PUZ_GRID_FOCUS(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PUZ_GRID_FOCUS, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent( wxPuzEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_PUZ_CLUE_FOCUS(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PUZ_CLUE_FOCUS, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent( wxPuzEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_PUZ_LETTER(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PUZ_LETTER, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent( wxPuzEventFunction, & fn ), (wxObject *) NULL ),

#endif // PUZ_EVENT_H