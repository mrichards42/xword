/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


// TODO (long term):
//   - Add an edit mode . . . 
//        Dynamically add and remove clues as the grid is edited
//        Make the clue text into editable wxTextCtrls


#ifndef CLUE_LIST_BOX_H
#define CLUE_LIST_BOX_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <odlistbox.hpp>
#include "puz/XPuzzle.hpp"
#include "PuzEvent.hpp"


class ClueListBox
    : public wxOwnerDrawnListBox<XPuzzle::Clue>
{
public:
    typedef wxOwnerDrawnListBox parent_t;

    explicit ClueListBox(wxWindow * parent, wxWindowID id = wxID_ANY) { SetMargins(5, 5); Create(parent, id); }
    virtual ~ClueListBox() {}

    void SetClueList(const container_t clues) { Freeze(); Clear(); Thaw(); Append(clues); }

    void SetClueNumber(int number);

    const wxString & GetClueText()   const { return GetItem(GetSelection()).Text(); }
    int              GetClueNumber() const { return GetItem(GetSelection()).Number(); }

    bool SetFont(const wxFont & font);

protected:
    // Drawing functions
    void OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const;
    void OnDrawItem(wxDC & dc, const wxRect & rect, size_t n) const;
    wxCoord OnMeasureItem(wxDC & dc, size_t n) const;

    void OnSelect(wxCommandEvent & evt);

    // Clue lists
    mutable std::vector<wxString> m_cachedClues;   // Cache to save us from wrapping every time
    mutable std::vector<int> m_numWidths;

    int FindClue(int number) const;

    void CalculateNumberWidth();
    int m_numWidth;


    // update the number widths and decide if the cache needs to be invalidated
    virtual void OnUpdateCount() { CalculateNumberWidth(); parent_t::OnUpdateCount(); }

    DECLARE_CACHED_ITEM_2(m_cachedClues, wxEmptyString, m_numWidths, -1, parent_t)
    DECLARE_EVENT_TABLE()
};


inline bool
ClueListBox::SetFont(const wxFont & font)
{
    bool ret = parent_t::SetFont(font);
    InvalidateCache();
    return ret;
}


#endif // CLUE_LIST_BOX_H