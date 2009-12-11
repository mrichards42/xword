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


#ifndef MY_WRAP_H
#define MY_WRAP_H

#include <wx/string.h>
#include <wx/tokenzr.h>
#include <vector>


template <class C>
wxString
Wrap(C * measurer, const wxString & str, int maxWidth,
     wxFont * font = NULL)
{
    wxString ret;

    // Remove existing line breaks
    wxStringTokenizer tok(str, _T(" -\n\r\f"), wxTOKEN_RET_EMPTY_ALL);
    int lineWidth = 0;

    // The previous delimiter should be attached to the return string if it
    // is not white space.  If it is white space, a space character should
    // be attached to the return string only if there is not a line break.
    wxChar prevDelim = _T('\0');

    while (tok.HasMoreTokens())
    {
        int width;
        // There's something funky going on here with wxStringTokenizer . . .
        // This works:
        //    wxString word = tok.GetNextToken() + tok.GetLastDelimiter();
        // But it prevents
        //    ret.Append(word)
        // for some reason

        wxString word = tok.GetNextToken();

        measurer->GetTextExtent(word + prevDelim, &width, NULL, NULL, NULL, font);
        // Check to see if we need to break the line.
        // Make sure the first line isn't blank
        if (lineWidth + width > maxWidth && ! ret.empty())
        {
            if (prevDelim == _T('-'))
                ret.Append(prevDelim);
            ret.Append(_T("\n"));
            lineWidth = 0;
        }
        // We don't need to break the line.  Add the delimiter.
        else
        {
            if (prevDelim == _T('-'))
                ret.Append(prevDelim);
            else if (prevDelim != _T('\0'))
                ret.Append(_T(' '));
        }
        lineWidth += width;
        ret.Append(word);
        prevDelim = tok.GetLastDelimiter();
    }
    return ret;
}


// Splits text into a given number of lines with a relatively equal
// proportion of widths
template <class C>
wxString
WrapIntoLines(C * measurer, const wxString & str, int lines,
              wxFont * font = NULL)
{
    std::vector<int> widths;
    std::vector<wxString> words;
    double bestWidth = 0;

    // Split into words and calculate each width
    wxStringTokenizer tok(str, _T(" -\n\r\f"), wxTOKEN_RET_EMPTY_ALL);
    while(tok.HasMoreTokens())
    {
        wxString word = tok.GetNextToken();
        word.Append(tok.GetLastDelimiter());
        int width;
        measurer->GetTextExtent(word, &width, NULL, NULL, NULL, font);
        words.push_back(word);
        widths.push_back(width);
        bestWidth += width;
    }
    bestWidth /= lines;

    // Arrange the widths into the most equal proportions.
    // This is certainly not the best way to do this, but it's quick and easy.

    wxString ret;

    int lineWidth = 0;
    for (size_t i = 0; i < widths.size(); ++i)
    {
        lineWidth += widths.at(i);
        ret.Append( words.at(i) );
        if (lineWidth > (0.8 * bestWidth))
        {
            lineWidth = 0;
            ret.Append(_T("\n"));
        }
    }
    return ret;
}
#endif // MY_WRAP_H
