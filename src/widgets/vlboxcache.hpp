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


// Cached wxVListBox
//   Caches line heights
//   Caches a bitmap of each item
// See wxVlistBox documentation for use of overloaded functions

#ifndef CACHED_V_LIST_BOX_H
#define CACHED_V_LIST_BOX_H

#include <wx/vlbox.h>
#include <wx/dcmemory.h> // for use with OnMeasureItem
#include <vector>

extern const wxChar * wxCachedVListBoxNameStr;

class wxCachedVListBox
    : public wxVListBox
{
public:
    wxCachedVListBox() { Init(); }

    wxCachedVListBox(wxWindow *parent,
                     wxWindowID id,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = 0,
                     const wxString & name = wxCachedVListBoxNameStr)
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString & name = wxCachedVListBoxNameStr);

    virtual ~wxCachedVListBox() {}

    virtual void SetItemCount(size_t count)
    {
        m_heights.resize(count, -1);
        m_bmpCache.resize(count);
        wxVListBox::SetItemCount(count);
    }

    virtual bool IsCached(size_t index) const
        { return m_heights.at(index) != -1; }
    virtual void InvalidateCache()
        { m_heights.assign(GetItemCount(), -1); }

protected:
    void Init() { InvalidateCache(); }

    virtual wxCoord OnGetLineHeight(size_t line) const
    {
        if (! IsCached(line))
            return OnMeasureItem(line) + 2*GetMargins().y;
        return m_heights.at(line);
    }


    virtual wxCoord OnMeasureItem(size_t WXUNUSED(n)) const
        { return GetCharHeight(); }

    // Caching Stuff
    mutable std::vector<int> m_heights;
    mutable std::vector<wxBitmap> m_bmpCache;
    mutable wxSize m_oldSize;

    // Painting
    void DrawItem(wxDC & dc, wxRect & rect, size_t line);
    void OnPaint(wxPaintEvent & WXUNUSED(event));

    // Sizing -- Invalidates the cache
    void OnSize(wxSizeEvent & evt);

    DECLARE_EVENT_TABLE()
    DECLARE_ABSTRACT_CLASS(wxCachedVListBox)
};

#endif // CACHED_V_LIST_BOX_H