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

#include "puz/Puzzle.hpp"

extern const wxChar* ClueListBoxNameStr;

class CluePanel;
#include "widgets/htmlcluelist.hpp"

class ClueListBox
    : public HtmlClueListBox
{
    friend class CluePanel;
public:
    typedef HtmlClueListBox parent_t;

    ClueListBox() { Init(); }

    ClueListBox(wxWindow * parent, wxWindowID id)
    {
        Init();
        Create(parent, id);
    }

    virtual ~ClueListBox() {}

    void SetClueList(puz::ClueList * clues);
    void ClearClueList();

    puz::Clue * GetClue()
    {
        int selection = GetSelection();
        try
        {
            if (m_clues && selection != wxNOT_FOUND)
                return &m_clues->at(selection);
        }
        catch(std::out_of_range &)
        {
            // Pass
        }
        return NULL;
    }

    bool SetClue(const puz::Clue * clue);

    bool SetFont(const wxFont & font);
    bool SetBackgroundColour(const wxColour & color);
    bool SetForegroundColour(const wxColour & color);
    void SetSelectionBackground(const wxColour & color);
    void SetSelectionForeground(const wxColour & color);

protected:
    puz::ClueList * m_clues;
    wxColour m_selectionForeground;
    virtual wxColour GetSelectedTextColour(const wxColour& colFg) const
    {
        return m_selectionForeground;
    }

    void Init()
    {
        m_clues = NULL;
        m_numberWidth = -1;
    }

    int GetNumberWidth() const;
    mutable int m_numberWidth;

    virtual wxString OnGetItem(size_t n) const;

    int FindClue(const puz::Clue * clue) const;

    // Reset the current selection on left clicks so that an select event is
    // always fired.
    void OnLeftDown(wxMouseEvent & evt);

    // Veto this event and refresh the ctrl if our list doesn't contain this item.
    void OnSelection(wxCommandEvent & evt);

    void InvalidateCache()
    {
        m_numberWidth = -1;
        SetItemCount(m_clues ? m_clues->size() : 0);
    }

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(ClueListBox)
    DECLARE_DYNAMIC_CLASS(ClueListBox)
};

// Inline functions

inline void ClueListBox::SetClueList(puz::ClueList * clues)
{
    m_clues = clues;
    InvalidateCache();
    SetItemCount(m_clues->size());
    RefreshAll();
}

inline void ClueListBox::ClearClueList()
{
    m_clues = NULL;
    InvalidateCache();
    SetItemCount(0);
    RefreshAll();
}

inline bool
ClueListBox::SetFont(const wxFont & font)
{
    const bool ret = parent_t::SetFont(font);
    InvalidateCache();
    RefreshAll();
    return ret;
}

inline bool
ClueListBox::SetBackgroundColour(const wxColour & color)
{
    const bool ret = parent_t::SetBackgroundColour(color);
    InvalidateCache();
    RefreshAll();
    return ret;
}

inline bool
ClueListBox::SetForegroundColour(const wxColour & color)
{
    const bool ret = parent_t::SetForegroundColour(color);
    InvalidateCache();
    RefreshAll();
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
    m_selectionForeground = color;
    const int selection = GetSelection();
    if (selection != wxNOT_FOUND)
        RefreshLine(selection);
}


#endif // CLUE_LIST_BOX_H
