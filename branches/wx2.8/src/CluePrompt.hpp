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

#ifndef CLUE_PROMPT_H
#define CLUE_PROMPT_H

#include "MetadataCtrl.hpp"

class CluePrompt
    : public MetadataCtrl
{
public:
    CluePrompt() {}

    CluePrompt(wxWindow * parent,
               wxWindowID id,
               const wxString & displayFormat = _T("%number%. %clue%"),
               const wxPoint & position = wxDefaultPosition,
               const wxSize & size = wxDefaultSize,
               long style = wxALIGN_CENTER,
               const wxString & name = _T("CluePrompt"))
    {
        Create(parent, id, displayFormat, position, size, style, name);
    }

    ~CluePrompt();

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & displayFormat = _T("%number%. %clue%"),
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = wxALIGN_CENTER,
                const wxString & name = _T("CluePrompt"));

protected:
    DECLARE_NO_COPY_CLASS(CluePrompt)
    DECLARE_DYNAMIC_CLASS(CluePrompt)
};

#endif // CLUE_PROMPT_H
