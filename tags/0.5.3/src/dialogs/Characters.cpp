// This file is part of XWord    
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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

#include "Characters.hpp"
#include "../MyFrame.hpp"
#include "puz/Square.hpp"
#include "../widgets/SizedText.hpp"
#include "../utils/string.hpp"

CharactersPanel::CharactersPanel(MyFrame * frame)
    : wxFB_CharactersPanel(frame),
      m_frame(frame),
      m_ctrlCount(0)
{
    Hide();
    m_scroller->Connect(wxEVT_SIZE, wxSizeEventHandler(CharactersPanel::OnSize), NULL, this);
    wxFont font = m_frame->GetFont();
    font.SetPointSize(12);
    SetFont(font);

    for (int ch = 1; ch < 0xff; ++ch)
    {
        if (puz::Square::ToGrid(ch) == ch && puz::Square::ToPlain(ch) != ch)
            AddCharacter(ch);
    }
    const int cols = static_cast<int>(sqrt(static_cast<double>(m_ctrlCount * 2)));
    m_sizer->SetCols(cols);
    Fit();
}


void
CharactersPanel::AddCharacter(int ch)
{
    InvalidateBestSize();

    wxString str = static_cast<wxChar>(ch);
    SizedText * ctrl = NewStaticText(str, ch);
    ctrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(CharactersPanel::OnLeftDown), NULL, this);
    ctrl->Connect(wxEVT_ENTER_WINDOW, wxMouseEventHandler(CharactersPanel::OnMouseOver), NULL, this);
    ctrl->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(CharactersPanel::OnMouseOut), NULL, this);
    ++m_ctrlCount;

}

SizedText *
CharactersPanel::NewStaticText(const wxString & label, wxWindowID id)
{
    SizedText * ctrl = new SizedText(m_panel, id, label,
                                           wxDefaultPosition,
                                           wxDefaultSize,
                                           wxALIGN_CENTER | ST_WRAP);
    ctrl->SetBackgroundColour(*wxWHITE);
    ctrl->SetPadding(5);
    ctrl->SetFont(GetFont());
    ctrl->SetMinSize(ctrl->GetBestSize());
    m_sizer->Add(ctrl, 0, wxEXPAND | wxALIGN_CENTER);
    return ctrl;
}


void
CharactersPanel::OnLeftDown(wxMouseEvent & evt)
{
    puz::Square * square = m_frame->GetFocusedSquare();
    if (square)
        m_frame->SetSquareText(square, wxChar(evt.GetId()));
}


// Highlighting characters
void
CharactersPanel::OnMouseOver(wxMouseEvent & evt)
{
    SizedText * ctrl = wxDynamicCast(evt.GetEventObject(), SizedText);
    if (ctrl)
    {
        ctrl->SetForegroundColour(*wxGREEN);
        ctrl->Refresh();
    }
}

void
CharactersPanel::OnMouseOut(wxMouseEvent & evt)
{
    SizedText * ctrl = wxDynamicCast(evt.GetEventObject(), SizedText);
    if (ctrl)
    {
        ctrl->SetForegroundColour(*wxBLACK);
        ctrl->Refresh();
    }
}




// Flow the grid sized items around
void
CharactersPanel::OnSize(wxSizeEvent & evt)
{
    const int itemwidth = m_sizer->GetItem(size_t(0))->GetSize().GetWidth() + 1;
    wxSize size = m_scroller->GetClientSize();

    const int cols = std::max(size.GetWidth() / itemwidth, 1);
    if (cols != m_sizer->GetCols())
    {
        Freeze();

        // Remove extra ctrls
        wxSizerItemList & items = m_sizer->GetChildren();
        while (items.size() > m_ctrlCount)
            items.back()->GetWindow()->Destroy();

        m_sizer->SetCols(cols);

        // Pad out the rest of the grid with blank text
        for (size_t i = 0; i < cols - m_ctrlCount % cols; ++i)
            NewStaticText();

        // *Absolutely* make sure we Fit() our panel before we do the layout,
        // or the sizer will think it has the previous amount of space, which
        // is disastrous if the previous space was small and the current space
        // is large (i.e. it will shrink the panels a ton, and cause massive
        // repainting lag as all the panels try to shrink their text)
        m_panel->Fit();
        m_sizer->Layout();
        Thaw();
    }

    evt.Skip();
}
