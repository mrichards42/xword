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


#include "PuzEvent.hpp"

DEFINE_EVENT_TYPE( wxEVT_PUZ_GRID_FOCUS )
DEFINE_EVENT_TYPE( wxEVT_PUZ_CLUE_FOCUS )
DEFINE_EVENT_TYPE( wxEVT_PUZ_LETTER )

wxPuzEvent::wxPuzEvent(wxEventType evtType, int id)
    : wxCommandEvent(evtType, id)
{
    m_direction  = false; // == DIR_ACROSS
    m_acrossClue = 0;
    m_downClue   = 0;
    m_clueText   = _T("");
}