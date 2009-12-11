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


#include "CluePanel.hpp"
#include "PuzEvent.hpp"

BEGIN_EVENT_TABLE(CluePanel, wxPanel)
    EVT_SIZE          (              CluePanel::OnSize)
    EVT_LISTBOX       (wxID_ANY,     CluePanel::OnClueSelect)
END_EVENT_TABLE()

const wxChar * CluePanelNameStr = _T("CluePanel");

IMPLEMENT_DYNAMIC_CLASS(CluePanel, wxPanel)


bool
CluePanel::Create(wxWindow* parent,
                  wxWindowID id,
                  const wxString & heading,
                  bool direction,
                  const wxPoint & pos,
                  const wxSize & size,
                  long style,
                  const wxString & name)
{
    if (! wxPanel::Create(parent, id, pos, size, style, name))
        return false;

    m_direction = direction;
    m_focusDirection = CROSSING;

    // Create windows
    m_heading = new wxStaticText(this,
                                 wxID_ANY,
                                 heading,
                                 wxDefaultPosition,
                                 wxDefaultSize,
                                 wxST_NO_AUTORESIZE | wxBORDER_DOUBLE);

    m_clueList = new ClueListBox(this, wxID_ANY);

    // Default fonts
    m_heading->SetFont( wxFont(13, wxFONTFAMILY_SWISS,
                                   wxFONTSTYLE_NORMAL,
                                   wxFONTWEIGHT_NORMAL) );

    // Default Colors
    SetSelectionForeground(
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );

    SetSelectionBackground(
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );

    SetCrossingForeground(
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );

    SetCrossingBackground(
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );

    // Layout
    wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_heading,   0, wxEXPAND);
        sizer->Add(m_clueList,  1, wxEXPAND);
    SetSizerAndFit(sizer);
    SetMinSize(m_heading->GetSize());

    return true;
}




void
CluePanel::OnClueSelect(wxCommandEvent & WXUNUSED(evt))
{
    if (m_focusDirection == CROSSING)
    {
        m_clueList->SetSelectionForeground(m_colors[FOCUSED][TEXT]);
        m_clueList->SetSelectionBackground(m_colors[FOCUSED][BACKGROUND]);
        m_focusDirection = FOCUSED;
    }

    wxPuzEvent puzEvt   (wxEVT_PUZ_CLUE_FOCUS, GetId());
    puzEvt.SetClueNumber(GetDirection(), GetClueNumber());
    puzEvt.SetDirection (GetDirection());
    puzEvt.SetClueText  (GetClueText());

    ::wxPostEvent(GetEventHandler(), puzEvt);
}



void
CluePanel::OnSize(wxSizeEvent & evt)
{
    // If this isn't true, the window hasn't finished being created
    if (m_heading != NULL && GetSizer() != NULL)
    {
        if (evt.GetSize().y <= 2* m_heading->GetSize().y)
            wxDynamicCast(GetSizer(), wxBoxSizer)->SetOrientation(wxHORIZONTAL);
        else
            wxDynamicCast(GetSizer(), wxBoxSizer)->SetOrientation(wxVERTICAL);
    }

    // Make sure to skip or the wxSizer::Layout() won't be called!
    evt.Skip();
}