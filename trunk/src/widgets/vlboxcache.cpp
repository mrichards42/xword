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


#include "vlboxcache.hpp"
#include <wx/dcbuffer.h>
#include <wx/log.h>

BEGIN_EVENT_TABLE(wxCachedVListBox, wxVListBox)
    EVT_PAINT(wxCachedVListBox::OnPaint)
    EVT_SIZE (wxCachedVListBox::OnSize)
END_EVENT_TABLE()

IMPLEMENT_ABSTRACT_CLASS(wxCachedVListBox, wxVListBox)

const wxChar * wxCachedVListBoxNameStr = _T("CachedVListBox");

bool
wxCachedVListBox::Create(wxWindow *parent,
                         wxWindowID id,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString & name)
{
    return wxVListBox::Create(parent, id, pos, size, style, name);
}

void
wxCachedVListBox::OnSize(wxSizeEvent & evt)
{
    if (evt.GetSize().x != m_oldSize.x)
        InvalidateCache();
    m_oldSize = evt.GetSize();
    evt.Skip();
}


void
wxCachedVListBox::DrawItem(wxDC & dc, wxRect & rect, size_t line)
{
    OnDrawBackground(dc, rect, line);

    OnDrawSeparator(dc, rect, line);

    rect.Deflate(GetMargins().x, GetMargins().y);
    OnDrawItem(dc, rect, line);
}


// Onpaint handler is different from wxVListBox to allow for cached items.
void
wxCachedVListBox::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxSize clientSize = GetClientSize();

    wxAutoBufferedPaintDC dc(this);

    // the update rectangle
    wxRect rectUpdate = GetUpdateClientRect();

    // fill it with background colour
    dc.SetBackground(GetBackgroundColour());
    dc.Clear();

    // the bounding rectangle of the current line
    wxRect rectLine;
    rectLine.width = clientSize.x;

    // iterate over all visible lines
    const size_t lineMax = GetVisibleEnd();
    for ( size_t line = GetFirstVisibleLine(); line < lineMax; line++ )
    {
        const wxCoord hLine = OnGetLineHeight(line);

        rectLine.height = hLine;

        // and draw the ones which intersect the update rect
        if ( rectLine.Intersects(rectUpdate) )
        {
            // don't allow drawing outside of the lines rectangle
            wxDCClipper clip(dc, rectLine);

            // Draw selected item every time
            if (IsSelected(line))
            {
                wxRect rect = rectLine;
                DrawItem(dc, rect, line);
            }
            else if (IsCached(line))
            {
                dc.DrawBitmap(m_bmpCache.at(line),
                              rectLine.x, rectLine.y,
                              false); // not transparent
            }
            // not selected or cached
            else
            {
                wxRect rect = rectLine;

                // Create a memory DC to draw the item
                m_bmpCache.at(line).Create(rect.width, rect.height);
                wxMemoryDC mdc(m_bmpCache.at(line));
                if (mdc.IsOk()) {
                    mdc.SetBackground(GetBackgroundColour());
                    mdc.Clear();

                    // We don't need a clipper here because the bmp size
                    // restricts drawing.
                    rect.y = 0;

                    DrawItem(mdc, rect, line);
                    mdc.SelectObjectAsSource(wxNullBitmap);

                    // Cache worked
                    dc.DrawBitmap(m_bmpCache.at(line),
                                  rectLine.x, rectLine.y,
                                  false); // not transparent

                    m_heights.at(line) = hLine;
                }
                // wxMemoryDC create failed
                else {
                    wxLogDebug(_T("Creation of wxMemoryDC for item %d failed!"),
                               line);
                    DrawItem(dc, rect, line);
                }
            }
        }
        // this item does not intersect the update rect
        else
        {
            if ( rectLine.GetTop() > rectUpdate.GetBottom() )
            {
                // we are already below the update rect, no need to continue
                // further
                break;
            }
            // else: the next line may intersect the update rect
        }

        rectLine.y += hLine;
    }
}