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

#ifndef ALIGN_CTRL_H
#define ALIGN_CTRL_H

#include "bitmapradio.hpp"

class AlignmentControl
    : public BitmapRadioBox
{
public:
    AlignmentControl() {}

    AlignmentControl(wxWindow * parent,
              wxWindowID id,
              wxOrientation orientation = wxBOTH,
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              const wxString & name = _T("AlignmentControl"))
    {
        Create(parent, id, orientation, position, size, name);
    }

    virtual ~AlignmentControl() {}

    bool Create(wxWindow * parent,
              wxWindowID id,
              wxOrientation orientation = wxBOTH,
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              const wxString & name = _T("AlignmentControl"));

    int GetValue() const;
    void SetValue(int align);

protected:
    wxOrientation m_orientation;
    wxWindow * m_topleft;
};

#endif // ALIGN_CTRL_H
