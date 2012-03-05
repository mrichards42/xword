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


#ifndef MY_GRID_REBUS_H
#define MY_GRID_REBUS_H

#include <wx/window.h>
#include <wx/textctrl.h>

class XGridCtrl;
class RebusTextCtrl;

// A text ctrl for entering multiple letters in a square.  The ctrl expands
// as the user types.

class XGridRebusCtrl : public wxWindow
{
public:
    XGridRebusCtrl(XGridCtrl * grid);
    ~XGridRebusCtrl();
    wxTextCtrl * GetTextCtrl();
    const wxTextCtrl * GetTextCtrl() const;
    void SetValue(const wxString & str) { GetTextCtrl()->SetValue(str); }
    wxString GetValue() const { return GetTextCtrl()->GetValue(); }
    void UpdateSize();
    virtual void SetFocus() { GetTextCtrl()->SetFocus(); }

protected:
    XGridCtrl * m_grid;
    RebusTextCtrl * m_text;

    void OnTextChanged(wxCommandEvent & evt);
};

#endif // MY_GRID_REBUS_H
