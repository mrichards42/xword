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


#ifndef Grid_SELECTION_EVENT_H
#define Grid_SELECTION_EVENT_H

#include <wx/event.h>
#include <vector>

class XGridCtrl;
namespace puz {
    class Square;
}

class GridSelectionEvent
    : public wxEvent
{
public:
    GridSelectionEvent(int id, wxEventType type,
                        XGridCtrl * grid,
                        puz::Square * start = NULL, puz::Square * end = NULL);

    // accessors
    puz::Square * GetSelectionStart() { return m_start; }
    puz::Square * GetSelectionEnd()   { return m_end; }
    std::vector<puz::Square *> GetSelection();

    void SetSelectionStart(puz::Square * square) { m_start = square; }
    void SetSelectionEnd(puz::Square * square) { m_end = square; }
    bool HasSelection() { return m_start != NULL && m_end != NULL; }

    // required for sending with wxPostEvent()
    wxEvent *Clone(void) const { return new GridSelectionEvent(*this); }

private:
    XGridCtrl * m_grid;
    puz::Square * m_start;
    puz::Square * m_end;
};


// Use DECLARE_LOCAL_EVENT_TYPE instead of DECLARE_EVENT_TYPE because
// wxWidgets built as a DLL will try to import events declared with
// DECLARE_EVENT_TYPE from the wxWidgets DLL
DECLARE_LOCAL_EVENT_TYPE( wxEVT_Grid_SELECTION,     -1 )



// Event handler function definition
typedef void (wxEvtHandler::*GridSelectionEventFunction)(GridSelectionEvent&);

#define GridSelectionEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(GridSelectionEventFunction, &func)


// Event table macros
#define EVT_Grid_SELECTION(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_Grid_SELECTION, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( GridSelectionEventFunction, & fn ), (wxObject *) NULL ),

#endif // Grid_SELECTION_EVENT_H
