/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "SizedText.hpp"

BEGIN_EVENT_TABLE(SizedText, wxStaticText)
    EVT_SIZE        (SizedText::OnSize)
    EVT_PAINT       (SizedText::OnPaint)
END_EVENT_TABLE()


SizedText::SizedText(wxWindow * parent, int id, const wxString & label, const wxPoint & position, const wxSize & size, int style, bool trunc)
    : wxControl(parent, id, position, size, style | wxBORDER_NONE),
      m_truncate(trunc)
{
    SetLabel(label);
}

SizedText::~SizedText()
{
}

void
SizedText::ResizeLabel()
{
    // This will break if the label is blank, so short cut that
    // For some reason, this function actually *doesn't* break in debug builds, 
    //    but does break in release builds (it enters an infinite loop trying to increase
    //    the font size from -infinity)
    if (m_fullLabel.empty()) {
        wxControl::SetLabel(_T(""));
        Refresh();
        return;
    }

    // Truncates the label (i.e. "this is some really lo...")
    if (m_truncate) {
        int maxWidth;
        GetClientSize(&maxWidth, NULL);

        int lineWidth;
        GetTextExtent(m_fullLabel, &lineWidth, NULL);
        if (lineWidth <= maxWidth)
            wxControl::SetLabel(m_fullLabel);
        else {
            wxString label;
            GetTextExtent(_T("..."), &lineWidth, NULL);

            wxString::iterator it = m_fullLabel.begin();
            for (it = m_fullLabel.begin(); it != m_fullLabel.end(); ++it) {
                int width;
                GetTextExtent(*it, &width, NULL);
                lineWidth += width;
                if (lineWidth <= maxWidth)
                    label.Append(*it);
                else
                    break;
            }
            label.Append(_T("..."));
            wxControl::SetLabel(label);
        }
    }
    // Wraps the label and resizes text to make the text as large as possible
    else {
        wxString label = m_fullLabel;

        int maxWidth, maxHeight;
        GetClientSize(&maxWidth, &maxHeight);
        int textWidth, textHeight;
        GetTextExtent(label, &textWidth, &textHeight);

        // Guess first (from my Python xword program
        double scaleX = double(maxWidth)  / textWidth;
        double scaleY = double(maxHeight) / textHeight;

        double clientRatio = double(maxHeight)  / maxWidth;
        double textRatio   = double(textHeight) / textWidth;

        int lines = wxRound(sqrt( clientRatio / textRatio ));
        if (lines < 1)
            lines = 1;

        // Guess font size
        wxFont font = GetFont();
        int fontPt = font.GetPointSize() * scaleY / lines;
        font.SetPointSize(fontPt);

        // Shrink to fit (maxHeight / lines)
        GetTextExtent(label, NULL, &textHeight, NULL, NULL, &font);
        while (fontPt >= 3 && textHeight * lines > maxHeight) {
            font.SetPointSize(--fontPt);
            GetTextExtent(label, NULL, &textHeight, NULL, NULL, &font);
        }

        // Wrap text
        if (lines > 1)
            label = ::WrapIntoLines(this, label, lines, &font);

        // A dummy dc for measuring text
        wxClientDC dc(this);

        // Grow and Shrink text to fit
        dc.GetMultiLineTextExtent(label, &textWidth, &textHeight, NULL, &font);
        while (fontPt < 100
               && (textHeight < maxHeight && textWidth < maxWidth)) // Only increase if both are too small
        {
            font.SetPointSize(++fontPt);
            dc.GetMultiLineTextExtent(label, &textWidth, &textHeight, NULL, &font);
        }

        while (fontPt >= 3
               && (textHeight > maxHeight || textWidth > maxWidth)) // Decrease if either are too large
        {
            font.SetPointSize(--fontPt);
            dc.GetMultiLineTextExtent(label, &textWidth, &textHeight, NULL, &font);
        }
        wxControl::SetFont(font);
        wxControl::SetLabel(label);
    }

    // An advantage of deriving from wxControl instead of wxStaticText
    //   is that there is no automatic repaint functionality.  We get
    //   to decide when that happens, which means no extra refreshing.
    Refresh();
    //Update();  // Update if you want the refresh to happen right away (can cause lagging)
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
    dc.DrawLabel(GetLabel(), wxRect(clientSize), GetWindowStyle() & wxALIGN_MASK);
}

