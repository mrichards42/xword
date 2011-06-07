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


#include "CluePrompt.hpp"
#include "utils/string.hpp"
#include "App.hpp" // GetApp

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/dcclient.h>
#endif

#include "utils/htmlparse.hpp"
#include <wx/html/winpars.h>

// this hack forces the linker to always link in m_* files
#include <wx/html/forcelnk.h>
FORCE_WXHTML_MODULES()


IMPLEMENT_DYNAMIC_CLASS(CluePrompt, wxControl)

BEGIN_EVENT_TABLE(CluePrompt, wxControl)
    EVT_PAINT(CluePrompt::OnPaint)
    EVT_SIZE(CluePrompt::OnSize)
END_EVENT_TABLE()

bool
CluePrompt::Create(wxWindow * parent,
                   wxWindowID id,
                   const wxString & label,
                   const wxString & displayFormat,
                   const wxPoint & position,
                   const wxSize & size,
                   long style,
                   const wxString & name)
{
    if (! wxControl::Create(parent, id, position, size, style, wxDefaultValidator, name))
        return false;

    SetDisplayFormat(displayFormat);
    SetLabel(label);

    // Config
    ConfigManager::CluePrompt_t & prompt =
        wxGetApp().GetConfigManager().CluePrompt;
    prompt.font.AddCallback(this, &CluePrompt::SetFont);
    prompt.backgroundColor.AddCallback(this, &CluePrompt::SetBackgroundColour);
    prompt.foregroundColor.AddCallback(this, &CluePrompt::SetForegroundColour);
    prompt.displayFormat.AddCallback(this, &CluePrompt::SetDisplayFormat);
    return true;
}

CluePrompt::~CluePrompt()
{
    if (m_parser)
    {
        delete m_parser->GetDC();
        delete m_parser;
    }
    delete m_cell;
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}


bool CluePrompt::SetFont(const wxFont & font)
{
    if (! wxControl::SetFont(font))
        return false;
    if (m_parser)
        m_parser->SetStandardFonts(font.GetPointSize(), font.GetFaceName());
    LayoutCell();
    return true;
}

void
CluePrompt::OnPaint(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    wxHtmlRenderingInfo info;
    m_cell->Draw(dc,
                 m_padding, (GetClientSize().y - m_cell->GetHeight()) / 2,
                 0, INT_MAX,
                 info);
}



void
CluePrompt::SetClue(const puz::Clue * clue)
{
    wxString result;
    if (clue)
    {
        wxString number = puz2wx(clue->GetNumber());
        wxString text = puz2wx(clue->GetText());
        if (! number.empty())
        {
            result = m_displayFormat;

            // Deal with literal "%" (not that anyone would really want a literal
            // percent sign in their clue prompt . . .)
            result.Replace(_T("%%"), _T("%Z"));
            // Just in case the text has any literal "%"s, make sure we don't
            // accidentally overwrite them.
            text.Replace(_T("%"), _T("%Z"));

            // Clue Number
            result.Replace(_T("%N"), number);

            // Clue Direction
            /*
            if (direction == puz::ACROSS)
            {
                result.Replace(_T("%D"), _T("Across"));
                result.Replace(_T("%d"), _T("A"));
            }
            else
            {
                result.Replace(_T("%D"), _T("Down"));
                result.Replace(_T("%d"), _T("D"));
            }
            */

            // Clue Text
            result.Replace(_T("%T"), text);

            // Literal "%"
            result.Replace(_T("%Z"), _T("%"));
        }
    }

    SetLabel(result);
    LayoutCell();
    Refresh();
}


static wxString ToText(wxHtmlContainerCell * cell);


void CluePrompt::Parse(const wxString & label, int pointSize, const wxString & faceName)
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

void CluePrompt::LayoutCell()
{
    if ( ! m_parser )
    {
        m_parser = new MyHtmlParser(this);
        m_parser->SetDC(new wxClientDC(this));
    }

    const wxString & label = GetLabel();
    const wxString & faceName = GetFont().GetFaceName();
    int pointSize = GetFont().GetPointSize();

    int width  = GetClientSize().x - 2 * GetPadding();
    int height = GetClientSize().y - 2 * GetPadding();

    delete m_cell;
    m_cell = new wxHtmlCell();
    SetToolTip(wxEmptyString);

    if (label.empty() || width < 10)
        return;

    // Parse the cell
    Parse(label, pointSize, faceName);
    m_cell->Layout(width);
    if (m_cell->GetHeight() == 0) // Parsing didn't really work
        return;

    // Parse and layout the cell with different font sizes to get
    // the optimal wrapping.
    if (m_cell->GetHeight() < height)
    {
        // Cell is too small
        do
        {
            ++pointSize;
            Parse(label, pointSize, faceName);
            m_cell->Layout(width);
        }
        while (m_cell->GetHeight() < height && pointSize < 150);
    }
    if (m_cell->GetHeight() > height)
    {
        // Cell is too large
        do
        {
            --pointSize;
            Parse(label, pointSize, faceName);
            m_cell->Layout(width);
            
        }
        while (m_cell->GetHeight() > height && pointSize > 6);
    }

    wxFont font = GetFont();
    font.SetPointSize(pointSize);
    wxControl::SetFont(font);

    SetToolTip(ToText((wxHtmlContainerCell*)m_cell));
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


// ----------------------------------------------------------------------------
// Implementation of wxHtmlWindowInterface
// ----------------------------------------------------------------------------

void CluePrompt::SetHTMLWindowTitle(const wxString& WXUNUSED(title))
{
    // nothing to do
}

void CluePrompt::OnHTMLLinkClicked(const wxHtmlLinkInfo& link)
{
    // nothing to do
}

wxHtmlOpeningStatus
CluePrompt::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type),
                             const wxString& WXUNUSED(url),
                             wxString *WXUNUSED(redirect)) const
{
    return wxHTML_BLOCK;
}

wxPoint CluePrompt::HTMLCoordsToWindow(wxHtmlCell *cell,
                                       const wxPoint& pos) const
{
    return wxDefaultPosition;
}

wxWindow* CluePrompt::GetHTMLWindow() { return this; }

wxColour CluePrompt::GetHTMLBackgroundColour() const
{
    return GetBackgroundColour();
}

void CluePrompt::SetHTMLBackgroundColour(const wxColour& WXUNUSED(clr))
{
    // nothing to do
}

void CluePrompt::SetHTMLBackgroundImage(const wxBitmap& WXUNUSED(bmpBg))
{
    // nothing to do
}

void CluePrompt::SetHTMLStatusText(const wxString& WXUNUSED(text))
{
    // nothing to do
}

wxCursor CluePrompt::GetHTMLCursor(HTMLCursor type) const
{
    return wxHtmlWindow::GetDefaultHTMLCursor(HTMLCursor_Default);
}
