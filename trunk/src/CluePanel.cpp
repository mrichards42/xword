/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "CluePanel.hpp"

BEGIN_EVENT_TABLE(CluePanel, wxPanel)
    EVT_SIZE            (CluePanel::OnSize)
END_EVENT_TABLE()


CluePanel::CluePanel(wxWindow* parent, wxWindowID id, const wxString & heading, bool direction, long style)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, style)
{
    m_heading = new wxStaticText(this, wxID_ANY, heading, wxDefaultPosition, wxSize(100,40), wxST_NO_AUTORESIZE);
    m_clueList = new ClueListBox(this);
    //m_clueList->SetDirection(direction);

    wxFont clueHeadingFont(15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont clueListFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    // Default colors and fonts
    m_heading->SetForegroundColour(*wxWHITE);
    m_heading->SetBackgroundColour(*wxBLUE);
    m_heading->SetFont(clueHeadingFont);

    m_clueList->SetFont(clueListFont);

    wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_heading,   0, wxEXPAND);
        sizer->Add(m_clueList,  1, wxEXPAND);
    SetSizerAndFit(sizer);
    SetMinSize(m_heading->GetSize());
}

CluePanel::~CluePanel()
{
}


void
CluePanel::OnSize(wxSizeEvent & evt)
{
    if (evt.GetSize().y <= 2* m_heading->GetSize().y)
        wxDynamicCast(GetSizer(), wxBoxSizer)->SetOrientation(wxHORIZONTAL);
    else
        wxDynamicCast(GetSizer(), wxBoxSizer)->SetOrientation(wxVERTICAL);

    evt.Skip();  // Make sure to skip or the wxSizer::Layout() won't be called!
}
