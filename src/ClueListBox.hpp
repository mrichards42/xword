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

#include "widgets/odlistbox.hpp"
#include "puz/XPuzzle.hpp"

extern const wxChar* ClueListBoxNameStr;

class ClueListBox
    : public wxOwnerDrawnListBox<XPuzzle::Clue>
{
public:
    typedef wxOwnerDrawnListBox<XPuzzle::Clue> parent_t;

    ClueListBox() { Init(); }

    ClueListBox(wxWindow * parent, wxWindowID id)
    {
        Init();
        Create(parent, id);
    }

    bool Create(wxWindow * parent, wxWindowID id);

    virtual ~ClueListBox() {}

    void SetClueList(const container_t clues)
    {
        Freeze();
        Clear();
        Thaw();
        Append(clues);
    }

    void SetClueNumber(unsigned int number);

    bool SetFont(const wxFont & font);
    bool SetBackgroundColour(const wxColour & color);
    bool SetForegroundColour(const wxColour & color);
    void SetSelectionBackground(const wxColour & color);
    void SetSelectionForeground(const wxColour & color);

protected:
    void Init() { SetMargins(5, 5); m_numWidth = -1; }

    // Drawing functions
    void    OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const;
    void    OnDrawItem      (wxDC & dc, const wxRect & rect, size_t n) const;
    wxCoord OnMeasureItem   (size_t n) const;

    // Cache to save us from wrapping every time
    mutable std::vector<wxString> m_cachedClues;
    mutable std::vector<int> m_numWidths;

    int FindClue(unsigned int number) const;

    void CalculateNumberWidth();
    int  m_numWidth;


    // Update the number widths and decide if the cache needs to be invalidated.
    virtual void OnUpdateCount()
    {
        CalculateNumberWidth();
        parent_t::OnUpdateCount();
    }

    // Reset the current selection on left clicks so that an select event is
    // always fired.
    void OnLeftDown(wxMouseEvent & evt);

    DECLARE_CACHED_ITEM_2(m_cachedClues, wxEmptyString,
                          m_numWidths, -1,
                          parent_t)

    DECLARE_NO_COPY_CLASS(ClueListBox)
    DECLARE_DYNAMIC_CLASS(ClueListBox)
};


inline bool
ClueListBox::SetFont(const wxFont & font)
{
    const bool ret = parent_t::SetFont(font);
    InvalidateCache();
    Refresh();
    return ret;
}

inline bool
ClueListBox::SetBackgroundColour(const wxColour & color)
{
    InvalidateCache();
    const bool ret = parent_t::SetBackgroundColour(color);
    Refresh();
    return ret;
}

inline bool
ClueListBox::SetForegroundColour(const wxColour & color)
{
    InvalidateCache();
    const bool ret = parent_t::SetForegroundColour(color);
    Refresh();
    return ret;
}

inline void
ClueListBox::SetSelectionBackground(const wxColour & color)
{
    parent_t::SetSelectionBackground(color);
    const int selection = GetSelection();
    if (selection != wxNOT_FOUND)
        RefreshLine(selection);
}

inline void
ClueListBox::SetSelectionForeground(const wxColour & color)
{
    parent_t::SetSelectionForeground(color);
    const int selection = GetSelection();
    if (selection != wxNOT_FOUND)
        RefreshLine(selection);
}



#endif // CLUE_LIST_BOX_H