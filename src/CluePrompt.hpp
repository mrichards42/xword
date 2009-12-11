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

#ifndef CLUE_PROMPT_H
#define CLUE_PROMPT_H

#include "widgets/SizedText.hpp"


class CluePrompt : public SizedText
{
public:
    CluePrompt() {}

    CluePrompt(wxWindow * parent,
              wxWindowID id,
              const wxString & label = wxEmptyString,
              const wxString & displayFormat = _T("%N. %T"),
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              long style = wxBORDER_NONE | wxALIGN_CENTER | ST_WRAP,
              const wxString & name = _T("CluePrompt"))
    {
        Create(parent, id, label, displayFormat, position, size, style, name);
    }

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & label = wxEmptyString,
                const wxString & displayFormat = _T("%N. %T"),
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = wxBORDER_NONE | wxALIGN_CENTER | ST_WRAP,
                const wxString & name = _T("CluePrompt"));

    const wxString & GetDisplayFormat() const { return m_displayFormat; }
    void SetDisplayFormat(const wxString & format)
        { m_displayFormat = format; }

    void Clear() { SetLabel(wxEmptyString); }
    void SetClue(int number, bool direction, wxString text);

protected:
    wxString m_displayFormat;

    DECLARE_NO_COPY_CLASS(CluePrompt)
    DECLARE_DYNAMIC_CLASS(CluePrompt)
};

#endif // CLUE_PROMPT_H
