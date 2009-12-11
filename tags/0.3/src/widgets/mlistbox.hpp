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


// This is an example of a class derived from wxOwnerDrawnListBox<wxString>
// It is used to wrap the list of items
// Use is similar to wxListBox

#ifndef MULTI_LINE_LIST_BOX_H
#define MULTI_LINE_LIST_BOX_H

#include "odlistbox.hpp"
#include "wrap.hpp"


class wxMultilineListBox : public wxOwnerDrawnListBox<wxString>
{
public:
    typedef wxOwnerDrawnListBox<wxString> Parent_t;

    // wxListbox-compatible constructors
    // ---------------------------------

    wxMultilineListBox() { }

    wxMultilineListBox(wxWindow * parent,
                       wxWindowID id,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       int n = 0, const wxString choices[] = NULL,
                       long style = 0)
    {
        Create(parent, id, pos, size, n, choices, style);
    }

    wxMultilineListBox(wxWindow * parent,
                       wxWindowID id,
                       const wxPoint & pos,
                       const wxSize & size,
                       const std::vector<wxString> & choices,
                       long style = 0)
    {
        Create(parent, id, pos, size, choices, style);
    }

    bool Create(wxWindow * parent, wxWindowID id,
                const wxPoint & pos = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                int n = 0, const wxString choices[] = NULL,
                long style = 0)
    {
        return Parent_t::Create(parent, id, pos, size, n, choices, style);
    }

    bool Create(wxWindow * parent, wxWindowID id,
                const wxPoint & pos,
                const wxSize & size,
                const std::vector<wxString> & choices,
                long style = 0)
    {
        return Parent_t::Create(parent, id, pos, size, choices, style);
    }

    virtual ~wxMultilineListBox() {}


protected:
    // Drawing overloads
    void OnDrawItem(wxDC & dc, const wxRect & rect, size_t n) const
    {
        dc.SetFont(GetFont());
        if (IsSelected(n))
            dc.SetTextForeground(GetSelectionForeground());
        else
            dc.SetTextForeground(GetForegroundColour());
        dc.DrawLabel(m_wrappedItems.at(n), rect);
    }

    wxCoord OnMeasureItem(wxDC & dc, size_t n) const
    {
        wxFont font = GetFont();

        // Cache the wrapped clue's text if it isn't already
        if (m_wrappedItems.at(n).IsEmpty()) {
            int maxWidth;
            GetClientSize(&maxWidth, NULL);
            m_wrappedItems.at(n) = ::Wrap(this, GetItem(n),  maxWidth);
        }
        int letterHeight = 0;
        dc.GetMultiLineTextExtent(m_wrappedItems.at(n), NULL, &letterHeight, NULL, &font);
        return letterHeight;
    }

    mutable std::vector<wxString> m_wrappedItems;

    DECLARE_CACHED_ITEM_1(m_wrappedItems, wxEmptyString, Parent_t)
};

#endif // MULTI_LINE_LIST_BOX_H