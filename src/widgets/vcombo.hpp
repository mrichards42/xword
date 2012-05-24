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

#ifndef VIRTUAL_COMBO_H
#define VIRTUAL_COMBO_H

// A virtual combo box

#include <wx/combo.h>

class VirtualComboPopup;

class VirtualComboBox : public wxComboCtrl
{
    friend class VirtualComboPopup;
public:
    VirtualComboBox(wxWindow* parent, wxWindowID id,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0);

protected:
    // ------------------------------------------------------------------------
    // Overload these in the derived class
    // ------------------------------------------------------------------------
    virtual int FindItem(const wxString & s) const=0;
    virtual wxString GetItem(size_t i) const=0;
    virtual size_t GetCount() const=0;

    // VListBox methods
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
    virtual void OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const;
    virtual wxCoord OnMeasureItem(size_t n) const;

    // ComboPopup methods
    virtual void OnPopup() {}

    // Draw the combo box
    virtual void OnDrawCtrl(wxDC& dc, const wxRect& rect);

    static const int s_popupBorder;
};

#endif // VIRTUAL_COMBO_H
