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

#include <algorithm>
#include "ClueListBox.hpp"
//#include "PuzEvent.hpp"
#include "utils/wrap.hpp"
#include <wx/tokenzr.h>
#include "utils/string.hpp"

BEGIN_EVENT_TABLE(ClueListBox, HtmlClueListBox)
    EVT_LEFT_DOWN         (          ClueListBox::OnLeftDown)
    EVT_LISTBOX           (wxID_ANY, ClueListBox::OnSelection)
    EVT_LISTBOX_DCLICK    (wxID_ANY, ClueListBox::OnSelection)
END_EVENT_TABLE()

const wxChar* ClueListBoxNameStr = _T("ClueListBox");

IMPLEMENT_DYNAMIC_CLASS(ClueListBox, HtmlClueListBox)


bool ClueListBox::SetClue(const puz::Clue * clue)
{
    int index = FindClue(clue);
    // This doesn't work for diagramless puzzles
    // wxASSERT(index != wxNOT_FOUND || number == _T(""));
    SetSelection(index);
    if (index != wxNOT_FOUND)
    {
        RefreshLine(index);
        return true;
    }
    else
        return false;
}


int ClueListBox::FindClue(const puz::Clue * clue) const
{
    if (! m_clues || ! clue)
        return wxNOT_FOUND;
    int num = 0;
    puz::ClueList::const_iterator it;
    for (it = m_clues->begin(); it != m_clues->end(); ++it)
    {
        if (&*it == clue)
            return num;
        ++num;
    }
    return wxNOT_FOUND;
}


//------------------------------------------------------------------------------
// OnGetItem
//------------------------------------------------------------------------------

int ClueListBox::GetNumberWidth() const
{
    if (m_numberWidth == -1)
    {
        wxClientDC dc(wxConstCast(this, ClueListBox));
        dc.SetFont(GetFont());
        m_numberWidth = 0;
        puz::ClueList::const_iterator it;
        for (it = m_clues->begin(); it != m_clues->end(); ++it)
            m_numberWidth = std::max(m_numberWidth,
                                     dc.GetTextExtent(puz2wx(it->GetNumber())+_T(".")).GetWidth());
        m_numberWidth = m_numberWidth * 1.2;
    }
    return m_numberWidth;
}

wxString ClueListBox::OnGetItem(size_t n) const
{
    if (! m_clues)
        return wxEmptyString;
    const size_t count = m_clues->size();
    if (count <= n)
        return wxEmptyString;
    return wxString::Format(
        _T("<table border=0 cellspacing=0 cellpadding=0><tr>")
            _T("<td align=right valign=top width=%d>%s.</td>")
            _T("<td width=5></td>")
            _T("<td align=left valign=top>%s</td>")
        _T("</tr></table>"),
        GetNumberWidth(),
        (const wxChar *)puz2wx((*m_clues)[n].GetNumber()).c_str(),
        (const wxChar *)puz2wx((*m_clues)[n].GetText()).c_str()
    );
}




//------------------------------------------------------------------------------
// Selection handling
//------------------------------------------------------------------------------

// This is here in order to hijack wxVListBox's selection model.  Trick the 
// wxVListBox into thinking that there is no selection so that an event
// is fired every time an item is clicked.  This allows the color change to
// work correctly -- otherwise if the crossing clue were clicked, no event
// would be fired, since the selection didn't change.
void ClueListBox::OnLeftDown(wxMouseEvent & evt)
{
    DoSetCurrent(wxNOT_FOUND);
    evt.Skip();
}


void ClueListBox::OnSelection(wxCommandEvent & evt)
{
    if (m_clues && m_clues->size() <= evt.GetSelection())
    {
        InvalidateCache();
        RefreshAll();
    }
    else
    {
        evt.Skip();
    }
}