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


#ifndef PUZ_EVENT_H
#define PUZ_EVENT_H

#include <wx/event.h>
#include <wx/gdicmn.h>  // wxPoint

class wxPuzEvent
    : public wxCommandEvent
{
public:
    wxPuzEvent(wxEventType commandType = wxEVT_NULL, int id = 0);

    // accessors
    int              GetAcrossClue()   const    { return m_acrossClue; }
    int              GetDownClue()     const    { return m_downClue; }
    const wxString & GetClueText()     const    { return m_clueText; }
    bool             GetDirection()    const    { return m_direction; }
    int      GetClueNumber(bool dir)   const    { return dir == false ? m_acrossClue : m_downClue; } // DIR_ACROSS = false;
    int      GetClueNumber()           const    { return GetClueNumber(m_direction); }

    void SetAcrossClue   (int num)               { m_acrossClue = num; }
    void SetDownClue     (int num)               { m_downClue   = num; }
    void SetClueText     (const wxString & text) { m_clueText   = text; }
    void SetDirection    (bool dir)              { m_direction  = dir; }
    void SetClueNumber   (bool dir, int num)     { dir == false ? m_acrossClue = num : m_downClue = num; }

    // required for sending with wxPostEvent()
    wxEvent *Clone(void) const { return new wxPuzEvent(*this); }


private:
    int m_acrossClue;
    int m_downClue;
    wxString m_clueText;
    bool m_direction;
};



DECLARE_EVENT_TYPE( wxEVT_PUZ_GRID_FOCUS, -1 )
DECLARE_EVENT_TYPE( wxEVT_PUZ_CLUE_FOCUS, -1 )
DECLARE_EVENT_TYPE( wxEVT_PUZ_LETTER,     -1 )



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