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


#include "ClueListBox.hpp"
//#include "PuzEvent.hpp"
#include "utils/wrap.hpp"
#include <wx/tokenzr.h>

const wxChar* ClueListBoxNameStr = _T("ClueListBox");

IMPLEMENT_DYNAMIC_CLASS(ClueListBox, wxOwnerDrawnListBox<XPuzzle::Clue>)


bool
ClueListBox::Create(wxWindow * parent, wxWindowID id)
{
    if (! parent_t::Create(parent, id))
        return false;

    // Connect the single event here so we don't need the event macros
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(ClueListBox::OnLeftDown));

    return true;
}


void
ClueListBox::SetClueNumber(unsigned int number)
{
    int index = FindClue(number);
    wxASSERT(index != wxNOT_FOUND || number == 0);
    SetSelection(index);
    if (index != wxNOT_FOUND)
    {
        RefreshLine(index);
        //Update(); // We shouldn't really need this
    }
}



int
ClueListBox::FindClue(unsigned int number) const
{
    unsigned int num = 0;
    for (container_t::const_iterator it = m_items.begin();
         it != m_items.end();
         ++it)
    {
        if (it->Number() == number)
            return num;
        ++num;
    }
    return wxNOT_FOUND;
}


//------------------------------------------------------------------------------
// Drawing functions
//------------------------------------------------------------------------------

void
ClueListBox::OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const
{
    if (IsSelected(n))
        dc.SetBackground(GetSelectionBackground());
    else
        dc.SetBackground(GetBackgroundColour());

    dc.Clear();
}


void
ClueListBox::OnDrawItem(wxDC & dc, const wxRect & rect, size_t n) const
{
    wxRect numRect(rect);
    numRect.SetWidth(m_numWidth);

    wxRect textRect(rect);
    textRect.SetX(numRect.GetRight() + GetMargins().x);
    textRect.SetWidth(rect.width - GetMargins().x);

    // Get fonts and colors
    dc.SetFont(GetFont());

    if (IsSelected(n))
        dc.SetTextForeground(GetSelectionForeground());
    else
        dc.SetTextForeground(GetForegroundColour());

    XPuzzle::Clue clue = GetItem(n);

    dc.DrawLabel(wxString::Format(_T("%d."), clue.Number()), numRect, wxALIGN_RIGHT|wxALIGN_TOP);

    wxASSERT(! m_cachedClues.at(n).empty());

    dc.DrawLabel(m_cachedClues.at(n), textRect);

    //dc.DrawLine(textRect.x, textRect.y, textRect.x + textRect.width, textRect.y + textRect.height);
}



wxCoord
ClueListBox::OnMeasureItem(size_t n) const
{
    XPuzzle::Clue clue = GetItem(n);

    // Cache the wrapped clue's text if it isn't already
    if (m_cachedClues.at(n).empty())
    {
        int maxWidth;
        GetClientSize(&maxWidth, NULL);
        m_cachedClues.at(n) = Wrap(this, clue.Text(),
                                   maxWidth - m_numWidth - GetMargins().x);
    }

    int height = 0;
    const wxArrayString lines = wxStringTokenize(m_cachedClues.at(n), _T("\n"));
    for (wxArrayString::const_iterator it = lines.begin();
         it != lines.end();
         ++it)
    {
        int lineHeight;
        GetTextExtent(*it, NULL, &lineHeight);
        height += lineHeight;
    }

    return height;
}


void
ClueListBox::CalculateNumberWidth()
{
    // Calculate the max width, filling in missing values
    int max_width = 0;
    container_t::iterator item_it = m_items.begin();
    std::vector<int>::iterator it, end;
    for (it = m_numWidths.begin(), end = m_numWidths.end(); it != end; ++it)
    {
        int & width = *it;
        // If there is no width for this item, calculate it
        if (width == -1)
            GetTextExtent(wxString::Format(_T("%d."), item_it->Number()), &width, NULL, NULL, NULL);
        if (width > max_width)
            max_width = width;
        ++item_it;
    }

    // Test to see if the cache needs to be invalidated
    if (max_width != m_numWidth)
    {
        m_numWidth = max_width;
        InvalidateCache();
    }
}




//------------------------------------------------------------------------------
// Selection handling
//------------------------------------------------------------------------------

// This is here in order to hijack wxVListBox's selection model.  Trick the 
// wxVListBox into thinking that there is no selection so that an event
// is fired every time an item is clicked.  This allows the color change to
// work correctly -- otherwise if the crossing clue were clicked, no event
// would be fired, since the selection didn't change.
void
ClueListBox::OnLeftDown(wxMouseEvent & evt)
{
    DoSetCurrent(wxNOT_FOUND);
    evt.Skip();
}