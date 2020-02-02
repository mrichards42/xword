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


#include "SizedText.hpp"

BEGIN_EVENT_TABLE(SizedText, wxStaticText)
    EVT_SIZE        (SizedText::OnSize)
    EVT_PAINT       (SizedText::OnPaint)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(SizedText, wxControl)

bool
SizedText::Create(wxWindow * parent,
                  wxWindowID id,
                  const wxString & label,
                  const wxPoint & position,
                  const wxSize & size,
                  long style,
                  const wxString & name)
{
    m_padding = 0;

    if (! wxControl::Create(parent, id,
                            position, size,
                            style,
                            wxDefaultValidator,
                            name))
        return false;

//    SetAlign(style & wxALIGN_MASK);
    SetLabel(label);
    return true;
}


void
SizedText::ResizeLabel()
{
    InvalidateBestSize();
    if (m_fullLabel.empty())
        wxControl::SetLabel(_T(""));
    else if (IsTruncated())
        TruncateLabel();
    else
        WrapLabel();

    Refresh();
}




void
SizedText::WrapLabel()
{
    // We'll go into an infinite loop otherwise
    wxASSERT(! m_fullLabel.empty());

    wxString label = m_fullLabel;

    int maxWidth, maxHeight;
    GetClientSize(&maxWidth, &maxHeight);
    maxWidth -= 2 * GetPadding();
    maxHeight -= 2 * GetPadding();

    int textWidth, textHeight;
    GetTextExtent(label, &textWidth, &textHeight);

    // Guess the number of lines we need
    double scaleY = double(maxHeight) / textHeight;

    double clientRatio = double(maxHeight)  / maxWidth;
    double textRatio   = double(textHeight) / textWidth;

    int lines = wxRound(sqrt( clientRatio / textRatio ));
    if (lines < 1)
        lines = 1;


    // Guess that the font size will be the current font size divided by
    // the number of lines of text we have.
    wxFont font = GetFont();
    int fontPt = font.GetPointSize() * scaleY / lines;
    font.SetPointSize(fontPt);

    // Shrink the font to fit maxHeight / lines
    GetTextExtent(label, NULL, &textHeight, NULL, NULL, &font);
    while (fontPt >= 3 && textHeight * lines > maxHeight)
    {
        font.SetPointSize(--fontPt);
        GetTextExtent(label, NULL, &textHeight, NULL, NULL, &font);
    }

    // Do the wrapping
    if (lines > 1)
        label = ::WrapIntoLines(this, label, lines, &font);


    // A dummy dc for measuring text
    wxClientDC dc(this);

    // Grow and shrink text to fit
    //-----------------------------
    dc.GetMultiLineTextExtent(label, &textWidth, &textHeight, NULL, &font);

    // Increase font point if height _and_ width are too small
    while (fontPt < 100
           && (textHeight < maxHeight && textWidth < maxWidth))
    {
        font.SetPointSize(++fontPt);
        dc.GetMultiLineTextExtent(label,
                                  &textWidth,
                                  &textHeight,
                                  NULL,
                                  &font);
    }

    // Decrease font point if height _or_ width are too large
    while (fontPt >= 3
           && (textHeight > maxHeight || textWidth > maxWidth))
    {
        font.SetPointSize(--fontPt);
        dc.GetMultiLineTextExtent(label,
                                  &textWidth,
                                  &textHeight,
                                  NULL,
                                  &font);
    }

    CacheBestSize(wxSize(textWidth, textHeight) + GetExtraSpace());

    m_displayFont = font;
    wxControl::SetLabel(label);
}



void
SizedText::TruncateLabel()
{
    int maxWidth;
    GetClientSize(&maxWidth, NULL);
    maxWidth -= 2 * GetPadding();

    int lineWidth;
    GetTextExtent(m_fullLabel, &lineWidth, NULL);

    CacheBestSize(wxSize(lineWidth, GetCharHeight()) + GetExtraSpace());

    if (lineWidth <= maxWidth)
    {
        wxControl::SetLabel(m_fullLabel);
        return;
    }

    wxString label;
    GetTextExtent(_T("..."), &lineWidth, NULL);

    for (wxString::iterator it = m_fullLabel.begin();
         it != m_fullLabel.end();
         ++it)
    {
        int width;
        GetTextExtent(*it, &width, NULL);
        lineWidth += width;

        if (lineWidth > maxWidth)
            break;

        label.Append(*it);
    }
    label.Append(_T("..."));

    wxControl::SetLabel(label);
}

// Return space that needs to be accounted for aside from the text.
wxSize
SizedText::GetExtraSpace() const
{
    // This is pretty much lifted from wxStaticText::DoGetBestSize (src/msw/stattext.cpp)
    // border takes extra space
    //
    // TODO: this is probably not wxStaticText-specific and should be moved
    wxCoord border;
    switch ( GetBorder() )
    {
        case wxBORDER_STATIC:
        case wxBORDER_SIMPLE:
            border = 1;
            break;

        case wxBORDER_SUNKEN:
            border = 2;
            break;

        case wxBORDER_RAISED:
        case wxBORDER_DOUBLE:
            border = 3;
            break;

        default:
            wxFAIL_MSG( _T("unknown border style") );
            // fall through

        case wxBORDER_NONE:
            border = 0;
    }
    // SizedText: Add space for padding
    border += GetPadding();

    return wxSize(2*border, 2*border);
}


wxSize
SizedText::DoGetBestSize() const
{
    // This is pretty much lifted from wxStaticText (src/msw/stattext.cpp)

    wxClientDC dc(wx_const_cast(SizedText *, this));
    wxFont font(GetFont());
    if (!font.Ok())
        font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    dc.SetFont(font);

    wxCoord widthTextMax, heightTextTotal;
    // SizedText: Use GetLabel() instead of GetLabelText()
    // i.e. don't strip menu codes
    dc.GetMultiLineTextExtent(GetLabel(), &widthTextMax, &heightTextTotal);

    wxSize best(widthTextMax, heightTextTotal);
    best += GetExtraSpace(); // SizedText-specific method
    CacheBestSize(best);
    return best;
}


void
SizedText::OnPaint(wxPaintEvent & WXUNUSED(evt))
{
    wxPaintDC dc(this);
    wxRect rect(GetClientSize());
    rect.Deflate(GetPadding());
    dc.SetFont(m_displayFont);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.SetTextForeground(GetForegroundColour());
    dc.DrawLabel(GetLabel(),
                 rect,
                 GetWindowStyle() & wxALIGN_MASK);
}
