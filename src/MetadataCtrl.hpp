// This file is part of XWord
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#ifndef METADATA_CTRL_H
#define METADATA_CTRL_H

#include "widgets/HtmlText.hpp"
#include "puz/Puzzle.hpp"
class MyFrame;

class MetadataCtrl
    : public HtmlText
{
public:
    MetadataCtrl() : m_frame(NULL) {}

    MetadataCtrl(wxWindow * parent,
               wxWindowID id,
               const wxString & displayFormat = wxEmptyString,
               const wxPoint & position = wxDefaultPosition,
               const wxSize & size = wxDefaultSize,
               long style = 0,
               const wxString & name = _T("MetadataCtrl"))
        : m_frame(NULL)
    {
        Create(parent, id, displayFormat, position, size, style, name);
    }

    ~MetadataCtrl();

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & displayFormat = wxEmptyString,
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = 0,
                const wxString & name = _T("MetadataCtrl"));

    const wxString & GetDisplayFormat() const { return m_displayFormat; }
    void SetDisplayFormat(const wxString & format)
    {
        m_displayFormat = format;
        UpdateLabel();
    }
    void UpdateLabel() { SetLabel(FormatLabel()); }

protected:
    wxString FormatLabel(); // Interpret m_displayFormat
    wxString m_displayFormat;
    MyFrame * m_frame;

    DECLARE_NO_COPY_CLASS(MetadataCtrl)
    DECLARE_DYNAMIC_CLASS(MetadataCtrl)
};

#endif // METADATA_CTRL_H
