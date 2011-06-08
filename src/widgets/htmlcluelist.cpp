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


// This is a modified version of wxHtmlListBox,
// from $(WXWIN)/src/generic/htmllbox.cpp


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/dcclient.h>
#endif

#include "htmlcluelist.hpp"
#include "../html/parse.hpp"

#include <wx/html/htmlcell.h>
#include <wx/html/winpars.h>

// this hack forces the linker to always link in m_* files
#include <wx/html/forcelnk.h>
FORCE_WXHTML_MODULES()

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

const wxChar * HtmlClueListBoxNameStr = wxT("HtmlClueListBox");

// ============================================================================
// private classes
// ============================================================================

// ----------------------------------------------------------------------------
// HtmlClueListBoxCache
// ----------------------------------------------------------------------------

// this class is used by HtmlClueListBox to cache the parsed representation of
// the items to avoid doing it anew each time an item must be drawn
class HtmlClueListBoxCache
{
private:
    // invalidate a single item, used by Clear() and InvalidateRange()
    void InvalidateItem(size_t n)
    {
        m_items[n] = (size_t)-1;
        delete m_cells[n];
        m_cells[n] = NULL;
    }

public:
    HtmlClueListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            m_items[n] = (size_t)-1;
            m_cells[n] = NULL;
        }

        m_next = 0;
    }

    ~HtmlClueListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            delete m_cells[n];
        }
    }

    // completely invalidate the cache
    void Clear()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            InvalidateItem(n);
        }
    }

    // return the cached cell for this index or NULL if none
    wxHtmlCell *Get(size_t item) const
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] == item )
                return m_cells[n];
        }

        return NULL;
    }

    // returns true if we already have this item cached
    bool Has(size_t item) const { return Get(item) != NULL; }

    // ensure that the item is cached
    void Store(size_t item, wxHtmlCell *cell)
    {
        delete m_cells[m_next];
        m_cells[m_next] = cell;
        m_items[m_next] = item;

        // advance to the next item wrapping around if there are no more
        if ( ++m_next == SIZE )
            m_next = 0;
    }

    // forget the cached value of the item(s) between the given ones (inclusive)
    void InvalidateRange(size_t from, size_t to)
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] >= from && m_items[n] <= to )
            {
                InvalidateItem(n);
            }
        }
    }

private:
    // the max number of the items we cache
    enum { SIZE = 100 };

    // the index of the LRU (oldest) cell
    size_t m_next;

    // the parsed representation of the cached item or NULL
    wxHtmlCell *m_cells[SIZE];

    // the index of the currently cached item (only valid if m_cells != NULL)
    size_t m_items[SIZE];
};

// ----------------------------------------------------------------------------
// HtmlClueListBoxStyle
// ----------------------------------------------------------------------------

// just forward wxDefaultHtmlRenderingStyle callbacks to the main class so that
// they could be overridden by the user code
class HtmlClueListBoxStyle : public wxDefaultHtmlRenderingStyle
{
public:
    HtmlClueListBoxStyle(const HtmlClueListBox& hlbox) : m_hlbox(hlbox) { }

    virtual wxColour GetSelectedTextColour(const wxColour& colFg)
    {
        return m_hlbox.GetSelectedTextColour(colFg);
    }

    virtual wxColour GetSelectedTextBgColour(const wxColour& colBg)
    {
        return m_hlbox.GetSelectedTextBgColour(colBg);
    }

private:
    const HtmlClueListBox& m_hlbox;

    DECLARE_NO_COPY_CLASS(HtmlClueListBoxStyle)
};

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(HtmlClueListBox, wxVListBox)
    EVT_SIZE(HtmlClueListBox::OnSize)
END_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_ABSTRACT_CLASS(HtmlClueListBox, wxVListBox)


// ----------------------------------------------------------------------------
// HtmlClueListBox creation
// ----------------------------------------------------------------------------

HtmlClueListBox::HtmlClueListBox()
{
    Init();
}

// normal constructor which calls Create() internally
HtmlClueListBox::HtmlClueListBox(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
{
    Init();

    (void)Create(parent, id, pos, size, style, name);
}

void HtmlClueListBox::Init()
{
    m_htmlParser = NULL;
    m_htmlRendStyle = new HtmlClueListBoxStyle(*this);
    m_cache = new HtmlClueListBoxCache;
}

bool HtmlClueListBox::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
{
    return wxVListBox::Create(parent, id, pos, size, style, name);
}

HtmlClueListBox::~HtmlClueListBox()
{
    delete m_cache;

    if ( m_htmlParser )
    {
        delete m_htmlParser->GetDC();
        delete m_htmlParser;
    }

    delete m_htmlRendStyle;
}

// ----------------------------------------------------------------------------
// HtmlClueListBox font and color
// ----------------------------------------------------------------------------

bool HtmlClueListBox::SetFont(const wxFont & font)
{
    if (! wxVListBox::SetFont(font))
        return false;
    if (m_htmlParser)
        m_htmlParser->SetStandardFonts(font.GetPointSize(), font.GetFaceName());
    m_cache->Clear();
    return true;
}

bool HtmlClueListBox::SetForegroundColour(const wxColour & color)
{
    if (! wxVListBox::SetForegroundColour(color))
        return false;
    if (m_htmlParser)
        m_htmlParser->SetActualColor(color);
    m_cache->Clear();
    return true;
}

// ----------------------------------------------------------------------------
// HtmlClueListBox appearance
// ----------------------------------------------------------------------------

wxColour HtmlClueListBox::GetSelectedTextColour(const wxColour& colFg) const
{
    return m_htmlRendStyle->
                wxDefaultHtmlRenderingStyle::GetSelectedTextColour(colFg);
}

wxColour
HtmlClueListBox::GetSelectedTextBgColour(const wxColour& WXUNUSED(colBg)) const
{
    return GetSelectionBackground();
}

// ----------------------------------------------------------------------------
// HtmlClueListBox items markup
// ----------------------------------------------------------------------------

wxString HtmlClueListBox::OnGetItemMarkup(size_t n) const
{
    // we don't even need to wrap the value returned by OnGetItem() inside
    // "<html><body>" and "</body></html>" because wxHTML can parse it even
    // without these tags
    return OnGetItem(n);
}

// ----------------------------------------------------------------------------
// HtmlClueListBox cache handling
// ----------------------------------------------------------------------------

void HtmlClueListBox::CacheItem(size_t n) const
{
    if ( !m_cache->Has(n) )
    {
        if ( !m_htmlParser )
        {
            HtmlClueListBox *self = wxConstCast(this, HtmlClueListBox);

            self->m_htmlParser = new MyHtmlParser(self);
            m_htmlParser->SetDC(new wxClientDC(self));
#if !wxUSE_UNICODE
            if (GetFont().Ok())
                m_htmlParser->SetInputEncoding(GetFont().GetEncoding());
#endif
            // Set the fonts
            m_htmlParser->SetStandardFonts(GetFont().GetPointSize(),
                                           GetFont().GetFaceName());
            m_htmlParser->SetActualColor(GetForegroundColour());
        }

        wxHtmlContainerCell *cell = (wxHtmlContainerCell *)m_htmlParser->
                Parse(OnGetItemMarkup(n));
        wxCHECK_RET( cell, _T("wxHtmlParser::Parse() returned NULL?") );

        cell->Layout(GetClientSize().x - 2*GetMargins().x);

        m_cache->Store(n, cell);
    }
}

void HtmlClueListBox::OnSize(wxSizeEvent& event)
{
    // we need to relayout all the cached cells
    m_cache->Clear();

    event.Skip();
}

void HtmlClueListBox::RefreshLine(size_t line)
{
    m_cache->InvalidateRange(line, line);

    wxVListBox::RefreshLine(line);
}

void HtmlClueListBox::RefreshLines(size_t from, size_t to)
{
    m_cache->InvalidateRange(from, to);

    wxVListBox::RefreshLines(from, to);
}

void HtmlClueListBox::RefreshAll()
{
    m_cache->Clear();

    wxVListBox::RefreshAll();
}

void HtmlClueListBox::SetItemCount(size_t count)
{
    // the items are going to change, forget the old ones
    m_cache->Clear();

    wxVListBox::SetItemCount(count);
}

// ----------------------------------------------------------------------------
// HtmlClueListBox implementation of wxVListBox pure virtuals
// ----------------------------------------------------------------------------

void HtmlClueListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    CacheItem(n);

    wxHtmlCell *cell = m_cache->Get(n);
    wxCHECK_RET( cell, _T("this cell should be cached!") );

    wxHtmlRenderingInfo htmlRendInfo;

    // draw the selected cell in selected state
    if ( IsSelected(n) )
    {
        wxHtmlSelection htmlSel;
        htmlSel.Set(wxPoint(0,0), cell, wxPoint(INT_MAX, INT_MAX), cell);
        htmlRendInfo.SetSelection(&htmlSel);
        if ( m_htmlRendStyle )
            htmlRendInfo.SetStyle(m_htmlRendStyle);
        htmlRendInfo.GetState().SetSelectionState(wxHTML_SEL_IN);
    }

    // note that we can't stop drawing exactly at the window boundary as then
    // even the visible cells part could be not drawn, so always draw the
    // entire cell
    cell->Draw(dc,
               rect.x, rect.y,
               0, INT_MAX, htmlRendInfo);
}

void HtmlClueListBox::OnDrawBackground(wxDC & dc,
                                       const wxRect & rect,
                                       size_t n) const
{
    if (IsSelected(n))
    {
        dc.SetBrush(wxBrush(GetSelectionBackground(), wxSOLID));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rect);
    }
}

wxCoord HtmlClueListBox::OnMeasureItem(size_t n) const
{
    CacheItem(n);

    wxHtmlCell *cell = m_cache->Get(n);
    wxCHECK_MSG( cell, 0, _T("this cell should be cached!") );

    return cell->GetHeight();
}

// ----------------------------------------------------------------------------
// HtmlClueListBox implementation of wxHtmlWindowInterface
// ----------------------------------------------------------------------------

void HtmlClueListBox::SetHTMLWindowTitle(const wxString& WXUNUSED(title))
{
    // nothing to do
}

void HtmlClueListBox::OnHTMLLinkClicked(const wxHtmlLinkInfo& link)
{
    // nothing to do
}
/*
void HtmlClueListBox::OnLinkClicked(size_t WXUNUSED(n),
                                  const wxHtmlLinkInfo& link)
{
    // nothing to do
}
*/
wxHtmlOpeningStatus
HtmlClueListBox::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type),
                                const wxString& WXUNUSED(url),
                                wxString *WXUNUSED(redirect)) const
{
    return wxHTML_BLOCK;
}

wxPoint HtmlClueListBox::HTMLCoordsToWindow(wxHtmlCell *cell,
                                          const wxPoint& pos) const
{
    return wxDefaultPosition;
}

wxWindow* HtmlClueListBox::GetHTMLWindow() { return this; }

wxColour HtmlClueListBox::GetHTMLBackgroundColour() const
{
    return GetBackgroundColour();
}

void HtmlClueListBox::SetHTMLBackgroundColour(const wxColour& WXUNUSED(clr))
{
    // nothing to do
}

void HtmlClueListBox::SetHTMLBackgroundImage(const wxBitmap& WXUNUSED(bmpBg))
{
    // nothing to do
}

void HtmlClueListBox::SetHTMLStatusText(const wxString& WXUNUSED(text))
{
    // nothing to do
}

wxCursor HtmlClueListBox::GetHTMLCursor(HTMLCursor type) const
{
    return wxHtmlWindow::GetDefaultHTMLCursor(HTMLCursor_Default);
}
