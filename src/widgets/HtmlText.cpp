// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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


#include "HtmlText.hpp"

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/dcclient.h>
#endif

#include "../html/parse.hpp"
#include <wx/html/winpars.h>
#include <wx/tooltip.h>

// this hack forces the linker to always link in m_* files
#include <wx/html/forcelnk.h>
FORCE_WXHTML_MODULES()


IMPLEMENT_DYNAMIC_CLASS(HtmlText, wxControl)

BEGIN_EVENT_TABLE(HtmlText, wxControl)
    EVT_PAINT(HtmlText::OnPaint)
    EVT_SIZE(HtmlText::OnSize)
END_EVENT_TABLE()

bool
HtmlText::Create(wxWindow * parent,
                 wxWindowID id,
                 const wxString & label,
                 const wxPoint & position,
                 const wxSize & size,
                 long style,
                 const wxString & name)
{
    if (! (style & wxBORDER_MASK))
        style |= wxBORDER_NONE;
    if (! wxControl::Create(parent, id, position, size, style, wxDefaultValidator, name))
        return false;
    SetLabel(label);
    return true;
}

HtmlText::~HtmlText()
{
    if (m_parser)
    {
        delete m_parser->GetDC();
        delete m_parser;
    }
    delete m_cell;
}


bool HtmlText::SetFont(const wxFont & font)
{
    if (! wxControl::SetFont(font))
        return false;
    if (m_parser)
        m_parser->SetStandardFonts(font.GetPointSize(), font.GetFaceName());
    LayoutCell();
    Refresh();
    return true;
}

void
HtmlText::OnPaint(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    wxHtmlRenderingInfo info;
    // Vertical align
    // Horizontal align is taken care of in LayoutCell()
    int y = m_padding;
    int align = GetAlignment();
    if (align & wxALIGN_CENTER_VERTICAL)
        y = (GetClientSize().y - m_cell->GetHeight()) / 2;
    else if (align & wxALIGN_BOTTOM)
        y = GetClientSize().y - m_padding - m_cell->GetHeight();
    // Draw the cell
    m_cell->Draw(dc, m_padding, y, 0, INT_MAX, info);
}

static wxString ToText(wxHtmlContainerCell * cell);

void HtmlText::Parse(const wxString & label, int pointSize, const wxString & faceName)
{
    // I can't seem to identify exactly *what* is causing this, but every
    // time I lay out the window with font size 70, I get back a height of
    // 15 px.
    if (pointSize == 70)
        pointSize = 69;
    // Set fonts
    m_parser->SetStandardFonts(pointSize, faceName);
    // Set colors
    m_parser->SetActualColor(GetForegroundColour());
    // Parse the cell
    delete m_cell;
    m_cell = (wxHtmlCell *)m_parser->Parse(label);
}

void HtmlText::LayoutCell()
{
    if ( ! m_parser )
    {
        m_parser = new MyHtmlParser(this);
        m_parser->SetDC(new wxClientDC(this));
        m_lastFontSize = GetFont().GetPointSize();
    }

    wxString label = GetLabel();

    // Fonts
    const wxString & faceName = GetFont().GetFaceName();
    // Start with the last font size, clamped to min and max sizes
    int pointSize = std::min(std::max(m_lastFontSize, m_minFontSize), m_maxFontSize);
    const bool fixedFontSize = ! HasFlag(HT_FILL); // Check for fixed font size
    if (fixedFontSize)
        pointSize = GetFont().GetPointSize();

    // Sizing
    int width  = GetClientSize().x - 2 * GetPadding();
    int height = GetClientSize().y - 2 * GetPadding();

    // Parse the cell
    delete m_cell;
    m_cell = new wxHtmlCell();
    SetToolTip(wxEmptyString);
    if (label.empty() || (width < 10 && ! fixedFontSize))
        return;

    // Font style
    if (GetFont().GetStyle() == wxFONTSTYLE_ITALIC)
        label = _T("<I>") + label + _T("</I>");
    if (GetFont().GetWeight() == wxFONTWEIGHT_BOLD)
        label = _T("<B>") + label + _T("</B>");
    if (GetFont().GetUnderlined())
        label = _T("<U>") + label + _T("</U>");

    // Horizontal Alignment
    // Vertical Alignment is taken care of in the drawing stage.
    int align = GetAlignment();
    if (align & wxALIGN_CENTER_HORIZONTAL)
        label = _T("<DIV ALIGN=CENTER>") + label + _T("</DIV>");
    else if (align & wxALIGN_RIGHT)
        label = _T("<DIV ALIGN=RIGHT>") + label + _T("</DIV>");

    Parse(label, pointSize, faceName);

    // Layout the cell
    m_cell->Layout(width);
    if (m_cell->GetHeight() == 0) // Parsing didn't really work
        return;

    // Parse and layout the cell with different font sizes to get
    // the optimal size.
    if (fixedFontSize)
    {
        if (HasFlag(HT_NOWRAP))
        {
            // Layout with MaxTotalWidth so that we don't accidentally wrap
            // the text.
            m_cell->Layout(1);
            m_cell->Layout(m_cell->GetMaxTotalWidth());
        }
    }
    else // ! fixedFontSize
    {
        if (HasFlag(HT_NOWRAP))
        {
            // GetMaxTotalWidth seems to work only if we call Layout with a
            // very small width.  We can't use 0 because that shortcuts the
            // layout mechanism.
            m_cell->Layout(1);
            // Increase cell width
            while (pointSize < m_maxFontSize && m_cell->GetMaxTotalWidth() < width)
            {
                Parse(label, ++pointSize, faceName);
                m_cell->Layout(1);
            }
            // Decrease cell width
            while (pointSize > m_minFontSize && m_cell->GetMaxTotalWidth() > width)
            {
                Parse(label, --pointSize, faceName);
                m_cell->Layout(1);
            }
            // Decrease cell height (and layout the cell width the actual width
            m_cell->Layout(width);
            while (pointSize > m_minFontSize && m_cell->GetHeight() > height)
            {
                Parse(label, --pointSize, faceName);
                m_cell->Layout(width);
            }
        }
        else // ! HT_NOWRAP
        {
            // Binary search to find the highest point size where the text still fits.
            int lowerBound = m_minFontSize;
            int upperBound = m_maxFontSize;
            while (lowerBound != upperBound) {
                pointSize = (lowerBound + upperBound + 1) / 2;
                Parse(label, pointSize, faceName);
                m_cell->Layout(width);
                if (m_cell->GetHeight() < height) {
                    lowerBound = pointSize;
                }
                else {
                    upperBound = pointSize - 1;
                }
            }
            pointSize = lowerBound;
            Parse(label, pointSize, faceName);
            m_cell->Layout(width);
        }
    }
    // Set m_layoutWidth
    int lastWidth = m_cell->GetWidth();
    m_cell->Layout(1);
    m_layoutWidth = m_cell->GetMaxTotalWidth();
    m_cell->Layout(lastWidth);

    m_lastFontSize = pointSize;
    wxString tip(ToText((wxHtmlContainerCell*)m_cell));
    // Truncate to 1024 chars
    if (tip.size() > 1024)
        tip = tip.Left(1024) + "[...]";
    SetToolTip(tip);
}


wxString ToText(wxHtmlContainerCell * container)
{
    wxString text;
    wxHtmlCell * cell = container->GetFirstChild();
    while (cell)
    {
        wxHtmlContainerCell * _container = wxDynamicCast(cell, wxHtmlContainerCell);
        if (_container)
            text << ToText(_container);
        else
            text << cell->ConvertToText(NULL);
        cell = cell->GetNext();
    }
    return text;
}

wxString HtmlText::GetPlainLabel() const
{
    wxToolTip * tip = GetToolTip();
    return tip ? tip->GetTip() : wxString(wxEmptyString);
}


// ----------------------------------------------------------------------------
// Implementation of wxHtmlWindowInterface
// ----------------------------------------------------------------------------

void HtmlText::SetHTMLWindowTitle(const wxString& WXUNUSED(title))
{
    // nothing to do
}

void HtmlText::OnHTMLLinkClicked(const wxHtmlLinkInfo& link)
{
    // nothing to do
}

wxHtmlOpeningStatus
HtmlText::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type),
                             const wxString& WXUNUSED(url),
                             wxString *WXUNUSED(redirect)) const
{
    return wxHTML_BLOCK;
}

wxPoint HtmlText::HTMLCoordsToWindow(wxHtmlCell *cell,
                                       const wxPoint& pos) const
{
    return wxDefaultPosition;
}

wxWindow* HtmlText::GetHTMLWindow() { return this; }

wxColour HtmlText::GetHTMLBackgroundColour() const
{
    return GetBackgroundColour();
}

void HtmlText::SetHTMLBackgroundColour(const wxColour& WXUNUSED(clr))
{
    // nothing to do
}

void HtmlText::SetHTMLBackgroundImage(const wxBitmap& WXUNUSED(bmpBg))
{
    // nothing to do
}

void HtmlText::SetHTMLStatusText(const wxString& WXUNUSED(text))
{
    // nothing to do
}

wxCursor HtmlText::GetHTMLCursor(HTMLCursor type) const
{
    return wxHtmlWindow::GetDefaultHTMLCursor(HTMLCursor_Default);
}
