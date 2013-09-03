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

#include "alignctrl.hpp"
#include <wx/tglbtn.h>
#include "../paths.hpp" // LoadXWordBitmap

bool AlignmentControl::Create(wxWindow * parent,
    wxWindowID id,
    wxOrientation orientation,
    const wxPoint & position,
    const wxSize & size,
    const wxString & name)
{
    if (! BitmapRadioBox::Create(parent, id, wxEmptyString, position, size, 3,
            name))
        return false;
    m_orientation = orientation;
    m_topleft = NULL;
    // Load the align sprite bitmap
    wxBitmap sprite = LoadXWordBitmap(_T("align"), 48);
    // create images based on orientation
    switch (m_orientation)
    {
        case wxHORIZONTAL:
            AddButton(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                sprite.GetSubBitmap(wxRect(0, 16, 16, 16)),
                "Left");
            AddButton(wxALIGN_CENTER,
                sprite.GetSubBitmap(wxRect(16, 16, 16, 16)),
                "Center");
            AddButton(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                sprite.GetSubBitmap(wxRect(32, 16, 16, 16)),
                "Right");
            break;
        case wxVERTICAL:
            AddButton(wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP,
                sprite.GetSubBitmap(wxRect(16, 0, 16, 16)),
                "Top");
            AddButton(wxALIGN_CENTER,
                sprite.GetSubBitmap(wxRect(16, 16, 16, 16)),
                "Middle");
            AddButton(wxALIGN_CENTER_HORIZONTAL | wxALIGN_BOTTOM,
                sprite.GetSubBitmap(wxRect(16, 32, 16, 16)),
                "Bottom");
            break;
        default: // Both
            m_topleft = AddButton(wxID_ANY,
                sprite.GetSubBitmap(wxRect(0, 0, 16, 16)),
                "Top Left");
            AddButton(wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP,
                sprite.GetSubBitmap(wxRect(16, 0, 16, 16)),
                "Top Center");
            AddButton(wxALIGN_RIGHT | wxALIGN_TOP,
                sprite.GetSubBitmap(wxRect(32, 0, 16, 16)),
                "Top right");
            AddButton(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                sprite.GetSubBitmap(wxRect(0, 16, 16, 16)),
                "Middle Left");
            AddButton(wxALIGN_CENTER,
                sprite.GetSubBitmap(wxRect(16, 16, 16, 16)),
                "Center");
            AddButton(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                sprite.GetSubBitmap(wxRect(32, 16, 16, 16)),
                "Middle Right");
            AddButton(wxALIGN_LEFT | wxALIGN_BOTTOM,
                sprite.GetSubBitmap(wxRect(0, 32, 16, 16)),
                "Bottom Left");
            AddButton(wxALIGN_CENTER_HORIZONTAL | wxALIGN_BOTTOM,
                sprite.GetSubBitmap(wxRect(16, 32, 16, 16)),
                "Bottom Center");
            AddButton(wxALIGN_RIGHT | wxALIGN_BOTTOM,
                sprite.GetSubBitmap(wxRect(32, 32, 16, 16)),
                "Bottom Right");
            m_orientation = wxBOTH;
            break;
    }
    return true;
}


int AlignmentControl::GetValue() const
{
    int id = GetSelectionID();
    if (m_orientation == wxBOTH && id == m_topleft->GetId())
        return wxALIGN_TOP | wxALIGN_LEFT;
    return id;
}

void AlignmentControl::SetValue(int align)
{
    if (align == (wxALIGN_TOP | wxALIGN_LEFT))
    {
        if (m_orientation == wxBOTH)
            SetSelection(0);
        else
            SetSelection(-1);
    }
    else
    {
        SetSelectionID(align);
    }
}
