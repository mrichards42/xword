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

#include <wx/html/winpars.h>

class MyHtmlParser : public wxHtmlWinParser
{
public:
    MyHtmlParser(wxHtmlWindowInterface *wndIface = NULL)
        : wxHtmlWinParser(wndIface)
    {
        SetActualColor(*wxBLACK);
    }

    virtual ~MyHtmlParser() {}

    virtual void InitParser(const wxString& source);
};
