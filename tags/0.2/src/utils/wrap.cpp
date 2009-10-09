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


#include "wrap.hpp"
#include <wx/window.h>
#include <wx/tokenzr.h>
#include <vector>
#include <wx/window.h>


wxString
Wrap(const wxWindow * window,
     const wxString & str,
     int maxWidth,
     const wxFont * font)
{
    wxString ret;

    // Remove existing line breaks
    wxStringTokenizer tok(str, _T(" -\n\r\f"), wxTOKEN_RET_EMPTY_ALL);
    int lineWidth = 0;

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
        word.Append(tok.GetLastDelimiter());

        window->GetTextExtent(word, &width, NULL, NULL, NULL, font);
        // Make sure the first line isn't blank
        if (lineWidth + width > maxWidth && ! ret.empty())
        {
            ret.Append(_T("\n"));
            lineWidth = 0;
        }
        lineWidth += width;
        ret.Append(word);
    }
    return ret;
}


// Splits text into a given number of lines with a relatively equal
// proportion of widths
wxString
WrapIntoLines(const wxWindow * window,
              const wxString & str,
              int lines,
              const wxFont * font)
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
        window->GetTextExtent(word, &width, NULL, NULL, NULL, font);
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