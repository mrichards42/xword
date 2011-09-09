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


// An HTML parser that respects text color (silly that the normal one
// doesn't)

#include "parse.hpp"
#include "wx/html/htmlwin.h"
#include <wx/settings.h>

void MyHtmlParser::InitParser(const wxString& source)
{
    // wxHtmlWinParser resets ActualColor
    wxColour actualColor = GetActualColor();
    wxHtmlWinParser::InitParser(source);
    SetActualColor(actualColor);

    // wxHtmlWinParser Creates two containers,
    // then adds three cells to the inside container.
    // We'll close the second container, leaving it with no
    // actual data, then recreate that container with the
    // correct color *next* to it, and leave that container as
    // the one that the parser puts stuff into.

    CloseContainer();

    wxHtmlContainerCell * container = OpenContainer();

    container->InsertCell(new wxHtmlColourCell(GetActualColor()));
    wxColour windowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW) ;

    container->InsertCell
                 (
                   new wxHtmlColourCell
                       (
                         GetWindowInterface()
                            ? GetWindowInterface()->GetHTMLBackgroundColour()
                            : windowColour,
                         wxHTML_CLR_BACKGROUND
                       )
                  );

    container->InsertCell(new wxHtmlFontCell(CreateCurrentFont()));
}