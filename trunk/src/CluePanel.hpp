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

    // Colors
    //-------
    enum textColor
    {
        TEXT,
        BACKGROUND
    };

    // Focused direction
    enum focusDirection
    {
        CROSSING,
        FOCUSED
    };

    // List colors
    void SetColor(focusDirection focus, textColor type, const wxColor & color)
        { m_colors[focus][type] = color; }
    const wxColor & GetColor(focusDirection focus, textColor type) const
        { return m_colors[focus][type]; }

    void SetSelectedForegroundColour(const wxColor & color)
        { SetColor(FOCUSED, TEXT, color); }
    void SetSelectedBackgroundColour(const wxColor & color)
        { SetColor(FOCUSED, BACKGROUND, color); }
    void SetCrossingForegroundColour(const wxColor & color)
        { SetColor(CROSSING, TEXT, color); }
    void SetCrossingBackgroundColour(const wxColor & color)
        { SetColor(CROSSING, BACKGROUND, color); }

    const wxColour & GetSelectedForegroundColour() const
        { return GetColor(FOCUSED, TEXT); }
    const wxColour & GetSelectedBackgroundColour() const
        { return GetColor(FOCUSED, BACKGROUND); }
    const wxColour & GetCrossingForegroundColour() const
        { return GetColor(CROSSING, TEXT); }
    const wxColour & GetCrossingBackgroundColour() const
        { return GetColor(CROSSING, BACKGROUND); }

    bool SetForegroundColour(const wxColor & color)
        { return m_clueList->SetForegroundColour(color); }
    wxColour GetForegroundColour() const
        { return m_clueList->GetForegroundColour(); }
    bool SetBackgroundColour(const wxColor & color)
        { return m_clueList->SetBackgroundColour(color); }
    wxColour GetBackgroundColour() const
        { return m_clueList->GetBackgroundColour(); }

    // Heading colors
    bool SetHeadingForeground(const wxColor & color)
        { return m_heading->SetForegroundColour(color); }
    wxColour GetHeadingForeground() const
        { return m_heading->GetForegroundColour(); }
    bool SetHeadingBackground(const wxColor & color)
        { return m_heading->SetBackgroundColour(color); }
    wxColour GetHeadingBackground() const
        { return m_heading->GetBackgroundColour(); }


    // Clue list font
    bool SetFont(const wxFont & font)
        { return m_clueList->SetFont(font); }
    wxFont GetFont() const
        { return m_clueList->GetFont(); }

    // Access to the ClueListBox
    //--------------------------
    void SetClueList(const ClueListBox::container_t & clues)
        { m_clueList->SetClueList(clues); }

    void SetClueNumber(unsigned int number, focusDirection focus)
    {
        m_clueList->SetSelectionForeground(
            GetColor(focus, TEXT) );

        m_clueList->SetSelectionBackground(
            GetColor(focus, BACKGROUND) );

        m_clueList->SetClueNumber(number);
    }

    const wxString & GetClueText()   const
        { return m_clueList->GetItem(m_clueList->GetSelection()).Text(); }

    unsigned int    GetClueNumber()  const
        { return m_clueList->GetItem(m_clueList->GetSelection()).Number(); }


private:
    void Init() { m_heading = NULL; m_clueList = NULL; m_direction = DIR_ACROSS; }

    wxStaticText * m_heading;
    ClueListBox * m_clueList;
    bool m_direction;

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