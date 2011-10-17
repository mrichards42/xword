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


#include "Preferences.hpp"
#include "../MyFrame.hpp"
#include "../CluePanel.hpp"
#include "../CluePrompt.hpp"
#include "../XGridCtrl.hpp"
#include "../widgets/SizedText.hpp"
#include "../App.hpp" // For the ConfigManager

PreferencesDialog::PreferencesDialog(wxWindow * parent)
    : wxFB_PreferencesDialog(parent, wxID_ANY)
{
}

//------------------------------------------------------------------------------
// Load config
//------------------------------------------------------------------------------

void
PreferencesDialog::LoadConfig()
{
    // Grid Style
    ConfigManager & config = wxGetApp().GetConfigManager();
    const int gridStyle = config.Grid.style();

    if (gridStyle & MOVE_AFTER_LETTER)
    {
        if (gridStyle & MOVE_TO_NEXT_BLANK)
            m_afterLetter->SetSelection(2);
        else
            m_afterLetter->SetSelection(1);
    }
    else
        m_afterLetter->SetSelection(0);

    m_blankOnDirection->SetValue((gridStyle & BLANK_ON_DIRECTION) != 0);
    m_blankOnNewWord  ->SetValue((gridStyle & BLANK_ON_NEW_WORD) != 0);
    m_pauseOnSwitch   ->SetSelection((gridStyle & PAUSE_ON_SWITCH) != 0);
    m_moveOnRightClick->SetValue((gridStyle & MOVE_ON_RIGHT_CLICK) != 0);
    m_checkWhileTyping->SetValue((gridStyle & CHECK_WHILE_TYPING) != 0);
    m_strictRebus->SetValue((gridStyle & STRICT_REBUS) != 0);

    // Colors
    m_selectedLetterColor->SetColour(config.Grid.focusedLetterColor());
    m_selectedWordColor  ->SetColour(config.Grid.focusedWordColor());
    m_penColor           ->SetColour(config.Grid.penColor());
    //m_pencilColor        ->SetColour(config.Grid.pcencilColor());
    m_gridBackgroundColor->SetColour(config.Grid.backgroundColor());
    m_whiteSquareColor   ->SetColour(config.Grid.whiteSquareColor());
    m_blackSquareColor   ->SetColour(config.Grid.blackSquareColor());
    m_gridSelectionColor ->SetColour(config.Grid.selectionColor());

    m_cluePromptBackground->SetColour(config.CluePrompt.backgroundColor());
    m_cluePromptText      ->SetColour(config.CluePrompt.foregroundColor());

    m_clueBackground        ->SetColour(config.Clue.listBackgroundColor());
    m_clueText              ->SetColour(config.Clue.listForegroundColor());
    m_selectedClueBackground->SetColour(config.Clue.selectedBackgroundColor());
    m_selectedClueText      ->SetColour(config.Clue.selectedForegroundColor());
    m_crossingClueBackground->SetColour(config.Clue.crossingBackgroundColor());
    m_crossingClueText      ->SetColour(config.Clue.crossingForegroundColor());
    m_clueHeadingBackground  ->SetColour(config.Clue.headingBackgroundColor());
    m_clueHeadingText       ->SetColour(config.Clue.headingForegroundColor());

    // Fonts
    m_gridLetterFont->SetSelectedFont(config.Grid.letterFont());
    m_gridNumberFont->SetSelectedFont(config.Grid.numberFont());
    m_clueFont      ->SetSelectedFont(config.Clue.font());
    m_clueHeadingFont->SetSelectedFont(config.Clue.headingFont());
    m_cluePromptFont->SetSelectedFont(config.CluePrompt.font());

    // Misc
    m_cluePromptFormat->ChangeValue(config.CluePrompt.displayFormat());
    m_letterScale->SetValue(config.Grid.letterScale());
    m_numberScale->SetValue(config.Grid.numberScale());
    m_lineThickness->SetValue(config.Grid.lineThickness());

    // Printing
    ConfigManager::Printing_t & printing = config.Printing;
    const long brightness = printing.blackSquareBrightness();
    m_printBlackSquareBrightness->SetValue(brightness);
    m_printBlackSquarePreview->SetBackgroundColour(
        wxColour(brightness, brightness, brightness));

    // Print grid alignment
    switch (printing.gridAlignment())
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
    m_printGridLetterFont->SetSelectedFont(printing.Fonts.gridLetterFont());
    m_printGridNumberFont->SetSelectedFont(printing.Fonts.gridNumberFont());
    m_printClueFont      ->SetSelectedFont(printing.Fonts.clueFont());

    const bool customFonts = printing.Fonts.useCustomFonts();
    m_printCustomFonts   ->SetValue(customFonts);
    m_printGridLetterFont->Enable(customFonts);
    m_printGridNumberFont->Enable(customFonts);
    m_printClueFont->Enable(customFonts);

    // Timer
    m_startTimer->SetValue(config.Timer.autoStart());

    // Auto Save
    m_autoSave->SetValue(config.autoSaveInterval());

    // File History
    m_saveFileHistory->SetValue(config.FileHistory.saveFileHistory());
    m_reopenLastPuzzle->SetValue(config.FileHistory.reopenLastPuzzle());
}


//------------------------------------------------------------------------------
// Save config
//------------------------------------------------------------------------------

void
PreferencesDialog::SaveConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();
    config.AutoUpdate(true);

    long gridStyle = 0;
    switch (m_afterLetter->GetSelection())
    {
    case 2:
        gridStyle |= MOVE_TO_NEXT_BLANK;
        // Fallthrough
    case 1:
        gridStyle |= MOVE_AFTER_LETTER;
    }

    if (m_blankOnDirection->GetValue())
        gridStyle |= BLANK_ON_DIRECTION;
    if (m_blankOnNewWord->GetValue())
        gridStyle |= BLANK_ON_NEW_WORD;
    if(m_pauseOnSwitch->GetSelection() == 1)
        gridStyle |= PAUSE_ON_SWITCH;
    if (m_moveOnRightClick->GetValue())
        gridStyle |= MOVE_ON_RIGHT_CLICK;
    if (m_checkWhileTyping->GetValue())
        gridStyle |= CHECK_WHILE_TYPING;
    if (m_strictRebus->GetValue())
        gridStyle |= STRICT_REBUS;

    config.Grid.style = gridStyle;

    // Colors
    config.Grid.focusedLetterColor = m_selectedLetterColor->GetColour();
    config.Grid.focusedWordColor = m_selectedWordColor->GetColour();
    config.Grid.penColor = m_penColor->GetColour();
    //config.Grid.pencilColor = m_pencilColor->GetColour();
    config.Grid.backgroundColor = m_gridBackgroundColor->GetColour();
    config.Grid.whiteSquareColor = m_whiteSquareColor->GetColour();
    config.Grid.blackSquareColor = m_blackSquareColor->GetColour();
    config.Grid.selectionColor = m_gridSelectionColor->GetColour();

    config.CluePrompt.backgroundColor = m_cluePromptBackground->GetColour();
    config.CluePrompt.foregroundColor = m_cluePromptText->GetColour();

    config.Clue.listBackgroundColor = m_clueBackground->GetColour();
    config.Clue.listForegroundColor = m_clueText->GetColour();
    config.Clue.selectedBackgroundColor = m_selectedClueBackground->GetColour();
    config.Clue.selectedForegroundColor = m_selectedClueText->GetColour();
    config.Clue.crossingBackgroundColor = m_crossingClueBackground->GetColour();
    config.Clue.crossingForegroundColor = m_crossingClueText->GetColour();
    config.Clue.headingBackgroundColor = m_clueHeadingBackground->GetColour();
    config.Clue.headingForegroundColor = m_clueHeadingText->GetColour();

    // Fonts
    config.Grid.letterFont = m_gridLetterFont->GetSelectedFont();
    config.Grid.numberFont = m_gridNumberFont->GetSelectedFont();
    config.Clue.font = m_clueFont->GetSelectedFont();
    config.Clue.headingFont = m_clueHeadingFont->GetSelectedFont();
    config.CluePrompt.font = m_cluePromptFont->GetSelectedFont();

    // Misc
    config.CluePrompt.displayFormat = m_cluePromptFormat->GetValue();
    config.Grid.letterScale = m_letterScale->GetValue();
    config.Grid.numberScale = m_numberScale->GetValue();
    config.Grid.lineThickness = m_lineThickness->GetValue();

    // Printing settings
    //------------------
    ConfigManager::Printing_t & printing = config.Printing;
    printing.blackSquareBrightness = m_printBlackSquareBrightness->GetValue();

    // The alignment options
    long alignments[] = { wxALIGN_TOP | wxALIGN_LEFT,
                          wxALIGN_TOP | wxALIGN_RIGHT,
                          wxALIGN_BOTTOM | wxALIGN_LEFT,
                          wxALIGN_BOTTOM | wxALIGN_RIGHT };
    printing.gridAlignment = alignments[m_printGridAlignment->GetSelection()];

    // Fonts
    printing.Fonts.useCustomFonts = m_printCustomFonts->IsChecked();
    printing.Fonts.gridLetterFont = m_printGridLetterFont->GetSelectedFont();
    printing.Fonts.gridNumberFont = m_printGridNumberFont->GetSelectedFont();
    printing.Fonts.clueFont = m_printClueFont->GetSelectedFont();

    config.Timer.autoStart = m_startTimer->GetValue();
    config.autoSaveInterval = m_autoSave->GetValue();

    // File History
    config.FileHistory.saveFileHistory = m_saveFileHistory->IsChecked();
    config.FileHistory.reopenLastPuzzle = m_reopenLastPuzzle->IsChecked();
}

//------------------------------------------------------------------------------
// Events
//------------------------------------------------------------------------------

void
PreferencesDialog::OnPrintCustomFonts(wxCommandEvent & evt)
{
    const bool customFonts = evt.IsChecked();
    m_printGridLetterFont->Enable(customFonts);
    m_printGridNumberFont->Enable(customFonts);
    m_printClueFont->Enable(customFonts);
}

void
PreferencesDialog::OnBlackSquareBrightness(wxScrollEvent & evt)
{
    int value = evt.GetPosition();
    m_printBlackSquarePreview->SetBackgroundColour(wxColour(value, value, value));
    m_printBlackSquarePreview->Refresh();
}

void
PreferencesDialog::OnSaveFileHistory(wxCommandEvent & evt)
{
    m_reopenLastPuzzle->Enable(evt.IsChecked());
    if (! evt.IsChecked())
        m_reopenLastPuzzle->SetValue(false);
}
