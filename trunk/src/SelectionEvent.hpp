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


#ifndef XGRID_SELECTION_EVENT_H
#define XGRID_SELECTION_EVENT_H

#include <wx/event.h>
#include <vector>

class XGridCtrl;
class XSquare;

class XGridSelectionEvent
    : public wxEvent
{
public:
    XGridSelectionEvent(int id, wxEventType type,
                        XGridCtrl * grid,
                        XSquare * start = NULL, XSquare * end = NULL);

    // accessors
    XSquare * GetSelectionStart() { return m_start; }
    XSquare * GetSelectionEnd()   { return m_end; }
    std::vector<XSquare *> GetSelection();

    void SetSelectionStart(XSquare * square) { m_start = square; }
    void SetSelectionEnd(XSquare * square) { m_end = square; }
    bool HasSelection() { return m_start != NULL && m_end != NULL; }

    // required for sending with wxPostEvent()
    wxEvent *Clone(void) const { return new XGridSelectionEvent(*this); }

private:
    XGridCtrl * m_grid;
    XSquare * m_start;
    XSquare * m_end;
};


// Use DECLARE_LOCAL_EVENT_TYPE instead of DECLARE_EVENT_TYPE because
// wxWidgets built as a DLL will try to import events declared with
// DECLARE_EVENT_TYPE from the wxWidgets DLL
DECLARE_LOCAL_EVENT_TYPE( wxEVT_XGRID_SELECTION,     -1 )



// Event handler function definition
typedef void (wxEvtHandler::*XGridSelectionEventFunction)(XGridSelectionEvent&);

#define XGridSelectionEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(XGridSelectionEventFunction, &func)


// Event table macros
#define EVT_XGRID_SELECTION(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_XGRID_SELECTION, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( XGridSelectionEventFunction, & fn ), (wxObject *) NULL ),

#endif // XGRID_SELECTION_EVENT_H
