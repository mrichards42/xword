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


#include "Preferences.hpp"
#include "../CluePanel.hpp"
#include "../CluePrompt.hpp"
#include "../XGridCtrl.hpp"
#include "../widgets/SizedText.hpp"
#include "../App.hpp" // For the ConfigManager

PreferencesDialog::PreferencesDialog(MyFrame * frame)
    : wxFB_PreferencesDialog(frame, wxID_ANY),
      m_frame(frame)
{}


//------------------------------------------------------------------------------
// Grid Style
//------------------------------------------------------------------------------

// Helpers
void
PreferencesDialog::AddRemoveGridFlag(wxCommandEvent & evt, int flag)
{
    int style = m_frame->m_gridCtrl->GetGridStyle();
    if (evt.IsChecked())
        style = AddFlag(style, flag);
    else
        style = RemoveFlag(style, flag);
    m_frame->m_gridCtrl->SetGridStyle(style);
    evt.Skip();
}


void
PreferencesDialog::OnAfterLetter(wxCommandEvent & evt)
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

void
PreferencesDialog::OnBlankOnDirection(wxCommandEvent & evt)
{
    AddRemoveGridFlag(evt, BLANK_ON_DIRECTION);
}

void
PreferencesDialog::OnBlankOnNewWord(wxCommandEvent & evt)
{
    AddRemoveGridFlag(evt, BLANK_ON_NEW_WORD);
}

void
PreferencesDialog::OnPauseOnSwitch(wxCommandEvent & evt)
{
    AddRemoveGridFlag(evt, PAUSE_ON_SWITCH);
}

void
PreferencesDialog::OnMoveOnRightClick(wxCommandEvent & evt)
{
    AddRemoveGridFlag(evt, MOVE_ON_RIGHT_CLICK);
}

void
PreferencesDialog::OnCheckWhileTyping(wxCommandEvent & evt)
{
    AddRemoveGridFlag(evt, CHECK_WHILE_TYPING);
}


//------------------------------------------------------------------------------
// Colors
//------------------------------------------------------------------------------

// Grid
//-----
void
PreferencesDialog::OnSelectedLetterColor(wxColourPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetFocusedLetterColor(evt.GetColour());
    evt.Skip();
}

void
PreferencesDialog::OnSelectedWordColor(wxColourPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetFocusedWordColor(evt.GetColour());
    evt.Skip();
}

void
PreferencesDialog::OnPenColor(wxColourPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetPenColor(evt.GetColour());
    evt.Skip();
}

void
PreferencesDialog::OnPencilColor(wxColourPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetPencilColor(evt.GetColour());
    evt.Skip();
}


// Clue Prompt
//------------
void
PreferencesDialog::OnPromptBackgroundColor(wxColourPickerEvent & evt)
{
    m_frame->m_cluePrompt->SetBackgroundColour(evt.GetColour());
    m_frame->m_cluePrompt->Refresh();
    evt.Skip();
}

void
PreferencesDialog::OnPromptTextColor(wxColourPickerEvent & evt)
{
    m_frame->m_cluePrompt->SetForegroundColour(evt.GetColour());
    m_frame->m_cluePrompt->Refresh();
    evt.Skip();
}


// Clues List
//-----------
void
PreferencesDialog::OnClueBackgroundColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetBackgroundColour);
}

void
PreferencesDialog::OnClueTextColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetForegroundColour);
}

void
PreferencesDialog::OnSelectedClueBackgroundColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetSelectionBackground);
}

void
PreferencesDialog::OnSelectedClueTextColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetSelectionForeground);
}

void
PreferencesDialog::OnCrossingClueBackgroundColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetCrossingBackground);
}

void
PreferencesDialog::OnCrossingClueTextColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetCrossingForeground);
}

void
PreferencesDialog::OnClueHeadingTextColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetHeadingForeground);
}

void
PreferencesDialog::OnClueHeadingBackgroundColor(wxColourPickerEvent & evt)
{
    SetClueListColor(evt, &CluePanel::SetHeadingBackground);
}


//------------------------------------------------------------------------------
// Fonts
//------------------------------------------------------------------------------

void
PreferencesDialog::OnGridLetterFont(wxFontPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetLetterFont(evt.GetFont());
    m_frame->m_gridCtrl->Refresh();
    evt.Skip();
}

void
PreferencesDialog::OnGridNumberFont(wxFontPickerEvent & evt)
{
    m_frame->m_gridCtrl->SetNumberFont(evt.GetFont());
    m_frame->m_gridCtrl->Refresh();
    evt.Skip();
}

void
PreferencesDialog::OnCluePromptFont(wxFontPickerEvent & evt)
{
    m_frame->m_cluePrompt->SetFont(evt.GetFont());
    evt.Skip();
}

void
PreferencesDialog::OnClueFont(wxFontPickerEvent & evt)
{
    m_frame->m_across->SetFont(evt.GetFont());
    m_frame->m_down->SetFont(evt.GetFont());
    evt.Skip();
}

void
PreferencesDialog::OnClueHeadingFont(wxFontPickerEvent & evt)
{
    m_frame->m_across->SetHeadingFont(evt.GetFont());
    m_frame->m_down->SetHeadingFont(evt.GetFont());
    evt.Skip();
}


//------------------------------------------------------------------------------
// Misc
//------------------------------------------------------------------------------

void
PreferencesDialog::OnCluePromptFormat(wxCommandEvent & evt)
{
    m_frame->m_cluePrompt->SetDisplayFormat(m_cluePromptFormat->GetValue());

    // Refresh the text
    if (m_frame->m_gridCtrl->GetDirection() == DIR_ACROSS)
    {
        m_frame->m_cluePrompt->SetClue(
            m_frame->m_across->GetClueNumber(),
            DIR_ACROSS,
            m_frame->m_across->GetClueText()
        );
    }
    else
    {
        m_frame->m_cluePrompt->SetClue(
            m_frame->m_down->GetClueNumber(),
            DIR_DOWN,
            m_frame->m_down->GetClueText()
        );
    }

    evt.Skip();
}


void
PreferencesDialog::OnLetterScale(wxSpinEvent & evt)
{
    m_frame->m_gridCtrl->SetLetterScale(evt.GetPosition() / 100.);
    m_frame->m_gridCtrl->Refresh();
    evt.Skip();
}

void
PreferencesDialog::OnNumberScale(wxSpinEvent & evt)
{
    m_frame->m_gridCtrl->SetNumberScale(evt.GetPosition() / 100.);
    m_frame->m_gridCtrl->Refresh();
    evt.Skip();
}


//------------------------------------------------------------------------------
// Printing
//------------------------------------------------------------------------------

void
PreferencesDialog::OnPrintBlackSquareBrightness(wxScrollEvent & evt)
{
    const int brightness = m_printBlackSquareBrightness->GetValue();
    m_printBlackSquarePreview->SetBackgroundColour(
        wxColour(brightness, brightness, brightness)
    );
    m_printBlackSquarePreview->Refresh();
    evt.Skip();
}

void
PreferencesDialog::OnPrintCustomFonts(wxCommandEvent & evt)
{
    m_printGridLetterFont->Enable(evt.IsChecked());
    m_printGridNumberFont->Enable(evt.IsChecked());
    m_printClueFont->Enable(evt.IsChecked());
    evt.Skip();
}


//------------------------------------------------------------------------------
// Load / save config
//------------------------------------------------------------------------------

void
PreferencesDialog::LoadConfig()
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
    m_gridLetterFont->SetSelectedFont(m_frame->m_gridCtrl->GetLetterFont());
    m_gridNumberFont->SetSelectedFont(m_frame->m_gridCtrl->GetNumberFont());
    m_clueFont      ->SetSelectedFont(m_frame->m_across->GetFont());
    m_cluePromptFont->SetSelectedFont(m_frame->m_cluePrompt->GetFont());
    m_clueHeadingFont->SetSelectedFont(m_frame->m_across->GetHeadingFont());

    // Misc
    m_cluePromptFormat->ChangeValue(m_frame->m_cluePrompt->GetDisplayFormat());
    m_letterScale->SetValue(m_frame->m_gridCtrl->GetLetterScale() * 100);
    m_numberScale->SetValue(m_frame->m_gridCtrl->GetNumberScale() * 100);

    // Printing
    ConfigManager & config = wxGetApp().GetConfigManager();
    config.SetPath(_T("/Printing"));
    const long brightness = config.ReadLong(_T("blackSquareBrightness"));
    m_printBlackSquareBrightness->SetValue(brightness);
    m_printBlackSquarePreview->SetBackgroundColour(
        wxColour(brightness, brightness, brightness));

    // Print grid alignment
    const long alignment = config.ReadLong(_T("gridAlignment"));
    switch (alignment)
    {
        case wxALIGN_TOP | wxALIGN_LEFT:
            m_printGridAlignment->SetSelection(0);
            break;
        case wxALIGN_TOP | wxALIGN_RIGHT:
            m_printGridAlignment->SetSelection(1);
            break;
        case wxALIGN_BOTTOM | wxALIGN_LEFT:
            m_printGridAlignment->SetSelection(2);
            break;
        case wxALIGN_BOTTOM | wxALIGN_RIGHT:
            m_printGridAlignment->SetSelection(3);
            break;
        default:
            m_printGridAlignment->SetSelection(1);
            break;
    }

    // Print fonts
    config.SetPath(_T("/Printing/Fonts"));
    m_printGridLetterFont->SetSelectedFont (config.ReadFont(_T("gridLetterFont")));
    m_printGridNumberFont->SetSelectedFont (config.ReadFont(_T("gridNumberFont")));
    m_printClueFont      ->SetSelectedFont (config.ReadFont(_T("clueFont")));

    const bool customFonts = config.ReadBool(_T("useCustomFonts"));
    m_printCustomFonts   ->SetValue(customFonts);
    m_printGridLetterFont->Enable(customFonts);
    m_printGridNumberFont->Enable(customFonts);
    m_printClueFont->Enable(customFonts);
}



// This should only write to the config for values that are not representable
// in the GUI.  Those that are representable in the GUI should have their states
// changed as soon as the controls were changed in the dialog.  It is the frame's
// responsibility to manage writing the GUI configuration.
void
PreferencesDialog::SaveConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();

    // Printing settings
    //------------------
    config.SetPath(_T("/Printing"));
    config.WriteLong(_T("blackSquareBrightness"),
                      m_printBlackSquareBrightness->GetValue());

    // The alignment options
    long alignments[] = { wxALIGN_TOP | wxALIGN_LEFT,
                          wxALIGN_TOP | wxALIGN_RIGHT,
                          wxALIGN_BOTTOM | wxALIGN_LEFT,
                          wxALIGN_BOTTOM | wxALIGN_RIGHT };
    config.WriteLong(_T("gridAlignment"),
                     alignments[m_printGridAlignment->GetSelection()]);

    config.SetPath(_T("/Printing/Fonts"));
    config.WriteBool(_T("useCustomFonts"), m_printCustomFonts->IsChecked());
    config.WriteFont(_T("gridLetterFont"),
                     m_printGridLetterFont->GetSelectedFont());
    config.WriteFont(_T("gridNumberFont"),
                     m_printGridNumberFont->GetSelectedFont());
    config.WriteFont(_T("clueFont"),
                     m_printClueFont->GetSelectedFont());
}
