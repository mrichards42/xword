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

#ifndef SCALED_SPIN_CTRL_H
#define SCALED_SPIN_CTRL_H

#include <wx/spinctrl.h>

// Wrapper for wxSpinCtrl which scales the size based on the display density.
class ScaledSpinCtrl
    : public wxSpinCtrl
{
public:
    ScaledSpinCtrl() {}

    ScaledSpinCtrl(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxSP_ARROW_KEYS,
        int min = 0,
        int max = 100,
        int initial = 0,
        const wxString& name = "ScaledSpinCtrl")
    {
        Create(parent, id, value, pos, size, style, min, max, initial, name);
    }

    virtual ~ScaledSpinCtrl() {}

    bool Create(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxSP_ARROW_KEYS,
        int min = 0,
        int max = 100,
        int initial = 0,
        const wxString& name = "ScaledSpinCtrl")
    {
        return wxSpinCtrl::Create(parent, id, value, pos, FromDIP(size), style, min, max, initial, name);
    }
};

#endif // SCALED_SPIN_CTRL_H
