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


#include "SelectionEvent.hpp"
#include "XGridCtrl.hpp"

DEFINE_EVENT_TYPE( wxEVT_Grid_SELECTION )

GridSelectionEvent::GridSelectionEvent(
    int id, wxEventType type,
    XGridCtrl * grid, puz::Square * start, puz::Square * end)
    : wxEvent(id, type),
      m_grid(grid),
      m_start(start),
      m_end(end)
{}

std::vector<puz::Square *>
GridSelectionEvent::GetSelection()
{
    std::vector<puz::Square *> selection;

    for (int col = m_start->GetCol(); col <= m_end->GetCol(); ++col)
        for (int row = m_start->GetRow(); row <= m_end->GetRow(); ++row)
            selection.push_back(&m_grid->At(col, row));

    return selection;
}
