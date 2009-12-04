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


#ifndef PROPERTIES_DLG_H
#define PROPERTIES_DLG_H

#include "wxFB_Dialogs.h"
#include "../MyFrame.hpp"
#include "../CluePanel.hpp"
#include "../XGridCtrl.hpp"
#include "../widgets/SizedText.hpp"

class PropertiesDialog : public wxFB_PropertiesDialog
{
public:
    PropertiesDialog(MyFrame * frame)
        : wxFB_PropertiesDialog(frame, wxID_ANY),
          m_frame(frame)
    {}

    virtual bool Show(bool show = true)
    {
        if (show)
            LoadCurrentState();
        return wxFB_PropertiesDialog::Show(show);
    }

protected:
    MyFrame * m_frame;

    void LoadCurrentState();

    // Event Handlers
    //---------------


    // Grid style
    //-----------

    // Helpers
    static int AddFlag   (int flag, int add) { return flag | add; }
    static int RemoveFlag(int flag, int rem) { return flag & ~ rem; }
    static bool HasFlag (int flag, int test) { return (flag & test) != 0; }

    void AddRemoveGridFlag(wxCommandEvent & evt, int flag)
    {
        int style = m_frame->m_gridCtrl->GetGridStyle();
        if (evt.IsChecked())
            style = AddFlag(style, flag);
        else
            style = RemoveFlag(style, flag);
        m_frame->m_gridCtrl->SetGridStyle(style);
        evt.Skip();
    }


	virtual void OnAfterLetter( wxCommandEvent& evt )
    {
        const int selection = evt.GetInt();
        int style = m_frame->m_gridCtrl->GetGridStyle();
        if (selection == 0)
            style = RemoveFlag(style, MOVE_AFTER_LETTER | MOVE_TO_NEXT_BLANK);
        else if (selection == 1)
            style = AddFlag( RemoveFlag(style, MOVE_TO_NEXT_BLANK), MOVE_AFTER_LETTER);
        else
        {
            wxASSERT(selection == 2);
            style = AddFlag( RemoveFlag(style, MOVE_AFTER_LETTER), MOVE_TO_NEXT_BLANK);
        }
        m_frame->m_gridCtrl->SetGridStyle(style);
        evt.Skip();
    }

	virtual void OnBlankOnDirection( wxCommandEvent& evt )
        { AddRemoveGridFlag(evt, BLANK_ON_DIRECTION); }

	virtual void OnBlankOnNewWord( wxCommandEvent& evt )
        { AddRemoveGridFlag(evt, BLANK_ON_NEW_WORD); }

	virtual void OnPauseOnSwitch( wxCommandEvent& evt )
        { AddRemoveGridFlag(evt, PAUSE_ON_SWITCH); }

	virtual void OnMoveOnRightClick( wxCommandEvent& evt )
        { AddRemoveGridFlag(evt, MOVE_ON_RIGHT_CLICK); }

	virtual void OnCheckWhileTyping( wxCommandEvent& evt )
        { AddRemoveGridFlag(evt, CHECK_WHILE_TYPING); }


    // Colors
    //-------

    // Helpers
    template <typename T>
    void SetClueListColor(wxColourPickerEvent & evt, T func)
    {
        (m_frame->m_across->*func)(evt.GetColour());
        (m_frame->m_down->*func)(evt.GetColour());
        evt.Skip();
    }

    // Grid
	virtual void OnSelectedLetterColor( wxColourPickerEvent& evt )
        { m_frame->m_gridCtrl->SetFocusedLetterColor(evt.GetColour()); evt.Skip(); }

	virtual void OnSelectedWordColor( wxColourPickerEvent& evt )
        { m_frame->m_gridCtrl->SetFocusedWordColor(evt.GetColour()); evt.Skip(); }

	virtual void OnPenColor( wxColourPickerEvent& evt )
        { m_frame->m_gridCtrl->SetPenColor(evt.GetColour()); evt.Skip(); }

	virtual void OnPencilColor( wxColourPickerEvent& evt )
        { m_frame->m_gridCtrl->SetPencilColor(evt.GetColour()); evt.Skip(); }


    // Clue Prompt
	virtual void OnPromptBackgroundColor( wxColourPickerEvent& evt )
        { m_frame->m_cluePrompt->SetBackgroundColour(evt.GetColour()); evt.Skip(); }

	virtual void OnPromptTextColor( wxColourPickerEvent& evt )
        { m_frame->m_cluePrompt->SetForegroundColour(evt.GetColour()); evt.Skip(); }


    // Clues
	virtual void OnClueBackgroundColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetBackgroundColour); }

	virtual void OnClueTextColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetForegroundColour); }

	virtual void OnSelectedClueBackgroundColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetSelectionBackground); }

	virtual void OnSelectedClueTextColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetSelectionForeground); }

	virtual void OnCrossingClueBackgroundColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetCrossingBackground); }

	virtual void OnCrossingClueTextColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetCrossingForeground); }

	virtual void OnClueHeadingTextColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetHeadingForeground); }

	virtual void OnClueHeadingBackgroundColor( wxColourPickerEvent& evt )
        { SetClueListColor(evt, &CluePanel::SetHeadingBackground); }

    // Fonts
    //------
	virtual void OnGridFont( wxFontPickerEvent& evt )
        { m_frame->m_gridCtrl->SetFont(evt.GetFont()); evt.Skip(); }

	virtual void OnCluePromptFont( wxFontPickerEvent& evt )
        { m_frame->m_cluePrompt->SetFont(evt.GetFont()); evt.Skip(); }

	virtual void OnClueFont( wxFontPickerEvent& evt )
    {
        m_frame->m_across->SetFont(evt.GetFont());
        m_frame->m_down->SetFont(evt.GetFont());
        evt.Skip();
    }

	virtual void OnClueHeadingFont( wxFontPickerEvent& evt )
    {
        m_frame->m_across->SetHeadingFont(evt.GetFont());
        m_frame->m_down->SetHeadingFont(evt.GetFont());
        evt.Skip();
    }
};



void
PropertiesDialog::LoadCurrentState()
{
    // Grid Style
    const int gridStyle = m_frame->m_gridCtrl->GetGridStyle();

    if (HasFlag(gridStyle, MOVE_AFTER_LETTER))
    {
        if (HasFlag(gridStyle, MOVE_TO_NEXT_BLANK))
            m_afterLetter->SetSelection(2);
        else
            m_afterLetter->SetSelection(1);
    }
    else
        m_afterLetter->SetSelection(0);

    m_blankOnDirection->SetValue(HasFlag(gridStyle, BLANK_ON_DIRECTION));
    m_blankOnNewWord  ->SetValue(HasFlag(gridStyle, BLANK_ON_NEW_WORD));
    m_pauseOnSwitch   ->SetSelection(HasFlag(gridStyle, PAUSE_ON_SWITCH));
    m_moveOnRightClick->SetValue(HasFlag(gridStyle, MOVE_ON_RIGHT_CLICK));
    m_checkWhileTyping->SetValue(HasFlag(gridStyle, CHECK_WHILE_TYPING));


    // Colors
    m_selectedLetterColor->SetColour(m_frame->m_gridCtrl->GetFocusedLetterColor());
    m_selectedWordColor  ->SetColour(m_frame->m_gridCtrl->GetFocusedWordColor());
    m_penColor           ->SetColour(m_frame->m_gridCtrl->GetPenColor());
    m_pencilColor        ->SetColour(m_frame->m_gridCtrl->GetPencilColor());

    m_cluePromptBackground->SetColour(m_frame->m_cluePrompt->GetBackgroundColour());
    m_cluePromptText      ->SetColour(m_frame->m_cluePrompt->GetForegroundColour());

    m_clueBackground        ->SetColour(m_frame->m_across->GetBackgroundColour());
    m_clueText              ->SetColour(m_frame->m_across->GetForegroundColour());
    m_selectedClueBackground->SetColour(m_frame->m_across->GetSelectionBackground());
    m_selectedClueText      ->SetColour(m_frame->m_across->GetSelectionForeground());
    m_crossingClueBackground->SetColour(m_frame->m_across->GetCrossingBackground());
    m_crossingClueText      ->SetColour(m_frame->m_across->GetCrossingForeground());
    m_clueHeadingBackground  ->SetColour(m_frame->m_across->GetHeadingBackground());
    m_clueHeadingText       ->SetColour(m_frame->m_across->GetHeadingForeground());

    // Fonts
    m_gridFont      ->SetSelectedFont(m_frame->m_gridCtrl->GetFont());
    m_clueFont      ->SetSelectedFont(m_frame->m_across->GetFont());
    m_cluePromptFont->SetSelectedFont(m_frame->m_cluePrompt->GetFont());
}

#endif // PROPERTIES_DLG_H
