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

#include "bitmapradio.hpp"
#include <wx/tglbtn.h>
#include <wx/sizer.h>
#include <wx/settings.h>

BEGIN_EVENT_TABLE(BitmapRadioBox, wxControl)
    EVT_TOGGLEBUTTON     (wxID_ANY, BitmapRadioBox::OnButton)
END_EVENT_TABLE()

bool BitmapRadioBox::Create(wxWindow * parent,
    wxWindowID id,
    const wxString & label,
    const wxPoint & position,
    const wxSize & size,
    int cols,
    const wxString & name)
{
    if (! wxControl::Create(parent, id,
                            position, size,
                            wxBORDER_NONE,
                            wxDefaultValidator,
                            name))
        return false;
    SetSizer(new wxGridSizer(cols, 5, 5));
    return true;
}


wxBitmapToggleButton *
BitmapRadioBox::AddButton(wxWindowID id, const wxBitmap & bmp,
                          const wxString & tip)
{
    // Add some padding to the buttons
    int x = 2 + wxSystemSettings::GetMetric(wxSYS_EDGE_X);
    int y = 2 + wxSystemSettings::GetMetric(wxSYS_EDGE_Y);
    wxBitmapToggleButton * btn = new wxBitmapToggleButton(this, id, bmp,
        wxDefaultPosition, bmp.GetSize() + wxSize(2*x, 2*y));
    if (! tip.empty())
        btn->SetToolTip(tip);
    GetSizer()->Add(btn);
    m_buttons.push_back(btn);
    return btn;
}

int BitmapRadioBox::GetSelection() const
{
    int i = 0;
    std::list<wxBitmapToggleButton *>::const_iterator it;
    for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
    {
        if ((*it)->GetValue())
            return i;
        ++i;
    }
    return wxNOT_FOUND;
}

wxWindowID BitmapRadioBox::GetSelectionID() const
{
    std::list<wxBitmapToggleButton *>::const_iterator it;
    for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
        if ((*it)->GetValue())
            return (*it)->GetId();
    return wxNOT_FOUND;
}


void BitmapRadioBox::SetSelection(int n)
{
    int i = 0;
    std::list<wxBitmapToggleButton *>::iterator it;
    for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
        (*it)->SetValue(i == n);
}

void BitmapRadioBox::SetSelectionID(wxWindowID id)
{
    std::list<wxBitmapToggleButton *>::iterator it;
    for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
        (*it)->SetValue((*it)->GetId() == id);
}


void BitmapRadioBox::OnButton(wxCommandEvent & evt)
{
    SetSelectionID(evt.GetId());
}
