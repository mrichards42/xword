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


#include "CluePrompt.hpp"
#include "puz/XSquare.hpp" // DIR_ACROSS / DIR_DOWN

IMPLEMENT_DYNAMIC_CLASS(CluePrompt, SizedText)

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
    if (! SizedText::Create(parent, id, label,
                            position, size,
                            style, name))
        return false;

    SetDisplayFormat(displayFormat);
    SetLabel(label);
    return true;
}


void
CluePrompt::SetClue(int number, bool direction, wxString text)
{
    wxString result = m_displayFormat;

    // Deal with literal "%" (not that anyone would really want a literal
    // percent sign in their clue prompt . . .)
    result.Replace(_T("%%"), _T("%Z"));
    // Just in case the text has any literal "%"s, make sure we don't
    // accidentally overwrite them.
    text.Replace(_T("%"), _T("%Z"));

    // Clue Number
    result.Replace(_T("%N"), wxString::Format(_T("%d"), number));

    // Clue Direction
    if (direction == DIR_ACROSS)
    {
        result.Replace(_T("%D"), _T("Across"));
        result.Replace(_T("%d"), _T("A"));
    }
    else
    {
        result.Replace(_T("%D"), _T("Down"));
        result.Replace(_T("%d"), _T("D"));
    }

    // Clue Text
    result.Replace(_T("%T"), text);

    // Literal "%"
    result.Replace(_T("%Z"), _T("%"));

    SetLabel(result);
}
