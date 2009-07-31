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


#ifndef MY_CLUE_PANEL_H
#define MY_CLUE_PANEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/panel.h>
#    include <wx/stattext.h>
#endif

#include "ClueListBox.hpp"

// Focused direction
const bool CROSSING_CLUE = false;
const bool FOCUSED_CLUE  = true;

extern const wxChar* CluePanelNameStr;

class CluePanel
    : public wxPanel
{
public:
    CluePanel() { Init(); }

    CluePanel(wxWindow * parent,
              wxWindowID id,
              const wxString & heading,
              bool direction,
              const wxPoint & pos = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              long style = wxBORDER_NONE,
              const wxString & name = CluePanelNameStr)
    {
        Init();
        Create(parent, id, heading, direction, pos, size, style, name);
    }

    virtual ~CluePanel() {}


    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & heading,
                bool direction,
                const wxPoint & pos = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = wxBORDER_NONE,
                const wxString & name = CluePanelNameStr);


    bool GetDirection() const { return m_direction; }

    // Access to the ClueListBox
    //--------------------------
    void SetClueList(const ClueListBox::container_t & clues)
        { m_clueList->SetClueList(clues); }

    void SetClueNumber(unsigned int number, bool focused)
    {
        m_clueList->SetSelectionForeground(
            m_colors[focused][TEXT_COLOR] );

        m_clueList->SetSelectionBackground(
            m_colors[focused][BACKGROUND_COLOR] );

        m_clueList->SetClueNumber(number);
    }

    const wxString & GetClueText()   const
        { return m_clueList->GetItem(m_clueList->GetSelection()).Text(); }

    unsigned int    GetClueNumber()  const
        { return m_clueList->GetItem(m_clueList->GetSelection()).Number(); }


private:
    void Init() { m_heading = NULL; m_clueList = NULL; }

    wxStaticText * m_heading;
    ClueListBox * m_clueList;
    bool m_direction;

    enum
    {
        TEXT_COLOR = 0,
        BACKGROUND_COLOR
    };

    // [ Focused / unfocused ][ text / background ]
    wxColor m_colors[2][2];

    // Check to see if the panel is very small
    // If it is, display the clue box next to the heading
    void OnSize(wxSizeEvent & evt);

    void OnClueSelect(wxCommandEvent & evt);

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(CluePanel)
    DECLARE_DYNAMIC_CLASS(CluePanel)
};


#endif // MY_CLUE_PANEL_H
