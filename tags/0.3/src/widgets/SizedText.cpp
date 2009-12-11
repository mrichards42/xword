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
    if (m_fullLabel.empty())
        wxControl::SetLabel(_T(""));
    else if (IsTruncated())
        TruncateLabel();
    else
        WrapLabel();

    // An advantage of deriving from wxControl instead of wxStaticText is that
    // there is no automatic repaint functionality.  We get to decide when that
    // happens, which means no extra refreshing.
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

    int textWidth, textHeight;
    GetTextExtent(label, &textWidth, &textHeight);

    // Guess the number of lines we need
    double scaleX = double(maxWidth)  / textWidth;
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

    wxControl::SetFont(font);
    wxControl::SetLabel(label);
}



void
SizedText::TruncateLabel()
{
    int maxWidth;
    GetClientSize(&maxWidth, NULL);

    int lineWidth;
    GetTextExtent(m_fullLabel, &lineWidth, NULL);

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


void
SizedText::OnPaint(wxPaintEvent & WXUNUSED(evt))
{
    wxPaintDC dc(this);
    wxSize clientSize = GetClientSize();
    dc.SetFont(GetFont());
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.SetTextForeground(GetForegroundColour());
    dc.Clear();
    dc.DrawLabel(GetLabel(),
                 wxRect(clientSize),
                 GetWindowStyle() & wxALIGN_MASK);
}