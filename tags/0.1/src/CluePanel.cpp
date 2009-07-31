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

    // Create windows
    m_heading = new wxStaticText(this,
                                 wxID_ANY,
                                 heading,
                                 wxDefaultPosition,
                                 wxSize(100,40),
                                 wxST_NO_AUTORESIZE);

    m_clueList = new ClueListBox(this, wxID_ANY);

    // Setup fonts and colors
    wxFont clueHeadingFont(15,
                           wxFONTFAMILY_SWISS,
                           wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL);

    wxFont clueListFont(12,
                        wxFONTFAMILY_SWISS,
                        wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);

    // Default colors and fonts
    m_heading->SetForegroundColour(*wxWHITE);
    m_heading->SetBackgroundColour(*wxBLUE);
    m_heading->SetFont(clueHeadingFont);

    m_clueList->SetFont(clueListFont);


    // Colors
    m_colors[FOCUSED_CLUE][TEXT_COLOR] =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    m_colors[FOCUSED_CLUE][BACKGROUND_COLOR] = 
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    m_colors[CROSSING_CLUE][TEXT_COLOR] =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    m_colors[CROSSING_CLUE][BACKGROUND_COLOR] = 
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);


    m_clueList->SetSelectionForeground(
        m_colors[FOCUSED_CLUE][TEXT_COLOR] );

    m_clueList->SetSelectionBackground(
        m_colors[FOCUSED_CLUE][BACKGROUND_COLOR] );

    // Layout
    wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_heading,   0, wxEXPAND);
        sizer->Add(m_clueList,  1, wxEXPAND);
    SetSizerAndFit(sizer);
    SetMinSize(m_heading->GetSize());

    return true;
}




void
CluePanel::OnClueSelect(wxCommandEvent & evt)
{
    const XPuzzle::Clue & clue = m_clueList->GetItem(evt.GetSelection());

    m_clueList->SetSelectionForeground(
        m_colors[FOCUSED_CLUE][TEXT_COLOR] );

    m_clueList->SetSelectionBackground(
        m_colors[FOCUSED_CLUE][BACKGROUND_COLOR] );


    wxPuzEvent puzEvt   (wxEVT_PUZ_CLUE_FOCUS, GetId());
    puzEvt.SetClueNumber(GetDirection(), clue.Number());
    puzEvt.SetDirection (GetDirection());
    puzEvt.SetClueText  (clue.Text());

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
