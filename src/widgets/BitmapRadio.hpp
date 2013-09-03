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

#ifndef BITMAP_RADIO_H
#define BITMAP_RADIO_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#   include <wx/control.h>
#   include <wx/bitmap.h>
#endif

#include <list>

class wxBitmapToggleButton;

class BitmapRadioBox
    : public wxControl
{
public:
    BitmapRadioBox() {}

    BitmapRadioBox(wxWindow * parent,
              wxWindowID id,
              const wxString & label = wxEmptyString,
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              int cols = 1,
              const wxString & name = _T("BitmapRadioBox"))
    {
        Create(parent, id, label, position, size, cols, name);
    }

    virtual ~BitmapRadioBox() {}

    bool Create(wxWindow * parent,
              wxWindowID id,
              const wxString & label = wxEmptyString,
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              int cols = 1,
              const wxString & name = _T("BitmapRadioBox"));

    wxBitmapToggleButton * AddButton(wxWindowID id, const wxBitmap & bmp,
                                     const wxString & tip = wxEmptyString);

    int GetSelection() const;
    wxWindowID GetSelectionID() const;
    void SetSelection(int n);
    void SetSelectionID(wxWindowID id);

    virtual bool HasTransparentBackground() { return true; }

protected:
    void OnButton(wxCommandEvent & evt);
    std::list<wxBitmapToggleButton *> m_buttons;
    DECLARE_EVENT_TABLE()
};

#endif // BITMAP_RADIO_H
