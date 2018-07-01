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

#include "PreferencesPanel.hpp"
#include "../App.hpp"

// Implementation of preferences panels

//------------------------------------------------------------------------------
// AppearancePanel
//------------------------------------------------------------------------------
#include "TreePanels.hpp"
#include <wx/treebook.h>

// A treebook that hacks up the constructor to allow a style for the wxTreeCtrl
class MyTreebook : public wxTreebook
{
public:
    MyTreebook(wxWindow * parent, long treectrlStyle = 0)
    {
        // Everything in this constructor is from wxTreebook::Create,
        // with the addition of treectrlStyle
        if ( !wxControl::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxBK_LEFT | wxBORDER_NONE, wxDefaultValidator, wxEmptyString) )
            return;
        m_bookctrl = new wxTreeCtrl
                     (
                        this,
                        wxID_ANY,
                        wxDefaultPosition,
                        wxDefaultSize,
                        wxBORDER_THEME |
                        wxTR_DEFAULT_STYLE |
                        wxTR_HIDE_ROOT |
                        wxTR_SINGLE |
                        treectrlStyle
                     );
        GetTreeCtrl()->SetQuickBestSize(false);
        GetTreeCtrl()->AddRoot(wxEmptyString);
    #ifdef __WXMSW__
        wxSizeEvent evt;
        GetEventHandler()->AddPendingEvent(evt);
    #endif
    }

    virtual bool HasTransparentBackground() { return true; }

protected:
    // Override from wxBookCtrlBase
    wxSize CalcSizeFromPage(const wxSize& sizePage) const
    {
        // Add the size of the controller and the border between if it's shown.
        if (!m_bookctrl || !m_bookctrl->IsShown())
            return sizePage;
        // Do one better than wxBookCtrlBase and use the actual best size here
        const wxSize sizeController = GetTreeCtrl()->GetBestSize();
        wxSize size = sizePage;
        size.IncTo(wxSize(size.x + sizeController.x + GetInternalBorder(),
                   sizeController.y));
        return size;
    }
    // Override from wxBookCtrlBase
    wxRect GetPageRect() const
    {
        // Don't add the border if the controller isn't shown
        if (!m_bookctrl || !m_bookctrl->IsShown())
            return wxRect(wxPoint(), GetClientSize());
        return wxBookCtrlBase::GetPageRect();
    }
};


void AppearancePanel::SetupTree()
{
    m_treebook = new MyTreebook(this, wxTR_TWIST_BUTTONS);

    Bind(wxEVT_TREEBOOK_PAGE_CHANGED, &AppearancePanel::OnPageChanged, this);

    m_treebook->AddPage(new GlobalAppearance(m_treebook, m_config), "Global Styles");
    m_treebook->AddPage(new GridBaseAppearance(m_treebook, m_config.Grid), "Grid");
        m_treebook->AddSubPage(new GridSelectionAppearance(m_treebook, m_config.Grid), "Cursor/Selection");
        m_treebook->AddSubPage(new GridTweaksAppearance(m_treebook, m_config.Grid), "DisplayTweaks");
    m_treebook->AddPage(new ClueListAppearance(m_treebook, m_config.Clue), "Clue List");
        m_treebook->AddSubPage(new ClueListSelectionAppearance(m_treebook, m_config.Clue), "Selected Clue");
        m_treebook->AddSubPage(new ClueListCrossingAppearance(m_treebook, m_config.Clue), "Crossing Clue");
        m_treebook->AddSubPage(new ClueListHeadingAppearance(m_treebook, m_config.Clue), "Heading");
    m_treebook->AddPage(new CluePromptAppearance(m_treebook, m_config.CluePrompt), "Clue Prompt");
    m_treebook->AddPage(new StatusAppearance(m_treebook, m_config.Status), "Status Bar");
    m_treebook->AddPage(new wxStaticText(m_treebook, wxID_ANY, "Select a sub-item"), "Metadata");
        m_treebook->AddSubPage(new NotesAppearance(m_treebook, m_config.Notes), "Notes");

    // Add the metadata ctrls
    ConfigManager::MetadataCtrls_t & metadata = m_config.MetadataCtrls;
    ConfigManager::MetadataCtrls_t::iterator meta;
    for (meta = metadata.begin(); meta != metadata.end(); ++meta)
    {
        // Chop the metadata part of the name off
        wxString name;
        if (! meta->m_name.StartsWith(_T("/Metadata/"), &name))
            name = meta->m_name;
        // Add the page
        m_treebook->AddSubPage(new MetadataAppearance(m_treebook, *meta), name);
    }
    // Set MinSize as BestSize for the TreeCtrl
    wxTreeCtrl * tree = m_treebook->GetTreeCtrl();
    tree->ExpandAll();
    wxTreeItemIdValue dummy; // Have to scroll to the first item to be accurate
    tree->ScrollTo(tree->GetFirstChild(tree->GetRootItem(), dummy));
    tree->InvalidateBestSize();
    tree->SetMinSize(tree->GetBestSize());

    GetSizer()->Insert(0, m_treebook, 1, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 10);
    UpdateLayout();
}

void AppearancePanel::OnAdvancedChoice(wxCommandEvent & evt)
{
    bool isSimple = evt.GetInt() == 0;
    m_config.useSimpleStyle = isSimple;
    // Copy this to the real config manager right away.
    wxGetApp().GetConfigManager().useSimpleStyle = isSimple;
    // Update the window layout
    UpdateLayout();
#if XWORD_PREFERENCES_SHRINK
    ((wxPropertySheetDialog *)wxGetTopLevelParent(this))->LayoutDialog(0);
#endif
}


void AppearancePanel::UpdateLayout()
{
    bool isSimple = m_config.useSimpleStyle();
    // Show the global appearance panel, and set the wxChoice to 
    m_treebook->GetTreeCtrl()->Show(! isSimple);
    m_advancedChoice->SetSelection(isSimple ? 0 : 1);
    if (isSimple)
        m_treebook->SetSelection(0);

    // Update the layout
    m_treebook->InvalidateBestSize();
    // Adjust the GlobalAppearance font picker layout
#ifndef __WXOSX__
    GlobalAppearance * panel = dynamic_cast<GlobalAppearance *>(m_treebook->GetPage(0));
    if (panel)
        panel->m_fontPicker->SetOrientation(isSimple ? wxHORIZONTAL : wxVERTICAL);
#endif // __WXOSX__
#if XWORD_PREFERENCES_SHRINK
   m_treebook->SetFitToCurrentPage(isSimple);
#endif
    Layout();
    Fit();
    // Using the SHRINK flag means we have to size the current window,
    // whereas omitting it requires sizing the parent window.
#if XWORD_PREFERENCES_SHRINK
    SendSizeEvent();
#else
    SendSizeEventToParent();
#endif
}

void AppearancePanel::OnPageChanged(wxBookCtrlEvent & evt)
{
    AppearanceBase * panel = dynamic_cast<AppearanceBase *>(m_treebook->GetCurrentPage());
    if (panel)
        panel->LoadConfig();
}

void AppearancePanel::OnResetDefaults(wxCommandEvent & evt)
{
    AppearanceBase * panel = dynamic_cast<AppearanceBase *>(m_treebook->GetCurrentPage());
    if (panel)
        panel->ResetConfig();
}

void AppearancePanel::DoLoadConfig()
{
}

void AppearancePanel::DoSaveConfig()
{

}


//------------------------------------------------------------------------------
// SolvePanel
//------------------------------------------------------------------------------
#include "../XGridCtrl.hpp" // Grid styles
void SolvePanel::DoLoadConfig()
{
    // Grid Style
    const int gridStyle = m_config.Grid.style();
    bool move = gridStyle & MOVE_AFTER_LETTER;
    m_moveAfterLetter->SetValue(move);
    m_nextBlank->Enable(move);
    m_nextSquare->Enable(move);
    if (gridStyle & MOVE_TO_NEXT_BLANK)
        m_nextBlank->SetValue(1);
    else
        m_nextSquare->SetValue(1);
    m_blankOnDirection->SetValue((gridStyle & BLANK_ON_DIRECTION) != 0);
    m_blankOnNewWord  ->SetValue((gridStyle & BLANK_ON_NEW_WORD) != 0);
    m_pauseOnSwitch   ->SetValue((gridStyle & PAUSE_ON_SWITCH) != 0);
    m_moveOnRightClick->SetValue((gridStyle & MOVE_ON_RIGHT_CLICK) != 0);
    m_checkWhileTyping->SetValue((gridStyle & CHECK_WHILE_TYPING) != 0);
    m_strictRebus->SetValue((gridStyle & STRICT_REBUS) != 0);

    // Timer
    m_startTimer->SetValue(m_config.Timer.autoStart());

    // Status
    m_showCompletionStatus->SetValue(m_config.Status.showCompletionStatus());

    // Autosave
    m_useAutoSave->SetValue(m_config.autoSaveInterval() > 0);
    m_autoSave->SetValue(m_config.autoSaveInterval());
    m_stAfter->Enable(m_useAutoSave->GetValue());
    m_autoSave->Enable(m_useAutoSave->GetValue());
    m_stSeconds->Enable(m_useAutoSave->GetValue());

    // File history
    m_saveFileHistory->SetValue(m_config.FileHistory.saveFileHistory());
    m_reopenLastPuzzle->SetValue(m_config.FileHistory.reopenLastPuzzle());
    m_reopenLastPuzzle->Enable(m_saveFileHistory->GetValue());
}

void SolvePanel::DoSaveConfig()
{
    long gridStyle = 0;
    if (m_moveAfterLetter->GetValue())
    {
        gridStyle |= MOVE_AFTER_LETTER;
        if (m_nextBlank->GetValue())
            gridStyle |= MOVE_TO_NEXT_BLANK;
    }
    if (m_blankOnDirection->GetValue())
        gridStyle |= BLANK_ON_DIRECTION;
    if (m_blankOnNewWord->GetValue())
        gridStyle |= BLANK_ON_NEW_WORD;
    if(m_pauseOnSwitch->GetValue())
        gridStyle |= PAUSE_ON_SWITCH;
    if (m_moveOnRightClick->GetValue())
        gridStyle |= MOVE_ON_RIGHT_CLICK;
    if (m_checkWhileTyping->GetValue())
        gridStyle |= CHECK_WHILE_TYPING;
    if (m_strictRebus->GetValue())
        gridStyle |= STRICT_REBUS;

    m_config.Grid.style = gridStyle;

    m_config.Timer.autoStart = m_startTimer->GetValue();

    m_config.Status.showCompletionStatus = m_showCompletionStatus->GetValue();

    m_config.autoSaveInterval =
        m_useAutoSave->IsChecked() ? m_autoSave->GetValue() : 0;

    m_config.FileHistory.saveFileHistory = m_saveFileHistory->IsChecked();
    m_config.FileHistory.reopenLastPuzzle = m_reopenLastPuzzle->IsChecked();
}

void SolvePanel::ConnectChangedEvents()
{
    BindChangedEvent(m_moveAfterLetter);
    BindChangedEvent(m_nextSquare);
    BindChangedEvent(m_nextBlank);
    BindChangedEvent(m_blankOnDirection);
    BindChangedEvent(m_blankOnNewWord);
    BindChangedEvent(m_pauseOnSwitch);
    BindChangedEvent(m_moveOnRightClick);
    BindChangedEvent(m_checkWhileTyping);
    BindChangedEvent(m_strictRebus);
    BindChangedEvent(m_startTimer);
    BindChangedEvent(m_showCompletionStatus);
    BindChangedEvent(m_useAutoSave);
    BindChangedEvent(m_autoSave);
    BindChangedEvent(m_saveFileHistory);
    BindChangedEvent(m_reopenLastPuzzle);
}

void SolvePanel::OnMoveAfterLetter(wxCommandEvent & evt)
{
    bool move = m_moveAfterLetter->GetValue();
    m_nextBlank->Enable(move);
    m_nextSquare->Enable(move);
    evt.Skip();
}

void SolvePanel::OnUseAutoSave(wxCommandEvent & evt)
{
    m_stAfter->Enable(evt.IsChecked());
    m_autoSave->Enable(evt.IsChecked());
    m_stSeconds->Enable(evt.IsChecked());
}

void SolvePanel::OnSaveFileHistory(wxCommandEvent & evt)
{
    m_reopenLastPuzzle->Enable(evt.IsChecked());
}

//------------------------------------------------------------------------------
// PrintPanel
//------------------------------------------------------------------------------
PrintPanel::PrintPanel(wxWindow * parent, ConfigManager & cfg)
    : wxFB_PrintPanel(parent),
      PreferencesPanelBase(cfg)
{
#ifdef __WXOSX__
    // Make the font panels layout on two lines for dialog sizing
    m_printGridLetterFont->SetOrientation(wxVERTICAL);
    m_printGridNumberFont->SetOrientation(wxVERTICAL);
    m_printClueFont->SetOrientation(wxVERTICAL);
#endif // __WXOSX__
    ConnectChangedEvents();
}

void PrintPanel::DoLoadConfig()
{
    ConfigManager::Printing_t & printing = m_config.Printing;
    const long brightness = printing.blackSquareBrightness();
    m_printBlackSquareBrightness->SetValue(brightness);
    m_printBlackSquarePreview->SetBackgroundColour(
        wxColour(brightness, brightness, brightness));

    // Print grid alignment
    switch (printing.gridAlignment())
    {
        case wxALIGN_TOP | wxALIGN_LEFT:
            m_alignTL->SetValue(1);
            break;
        case wxALIGN_TOP | wxALIGN_RIGHT:
            m_alignTR->SetValue(1);
            break;
        case wxALIGN_BOTTOM | wxALIGN_LEFT:
            m_alignBL->SetValue(1);
            break;
        case wxALIGN_BOTTOM | wxALIGN_RIGHT:
            m_alignBR->SetValue(1);
            break;
        default:
            m_alignTR->SetValue(1);
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
}

void PrintPanel::DoSaveConfig()
{
    ConfigManager::Printing_t & printing = m_config.Printing;
    printing.blackSquareBrightness = m_printBlackSquareBrightness->GetValue();

    if (m_alignTL->GetValue())
        printing.gridAlignment = wxALIGN_TOP | wxALIGN_LEFT;
    else if (m_alignTR->GetValue())
        printing.gridAlignment = wxALIGN_TOP | wxALIGN_RIGHT;
    else if (m_alignBL->GetValue())
        printing.gridAlignment = wxALIGN_BOTTOM | wxALIGN_LEFT;
    else if (m_alignBR->GetValue())
        printing.gridAlignment = wxALIGN_BOTTOM | wxALIGN_RIGHT;

    // Fonts
    printing.Fonts.useCustomFonts = m_printCustomFonts->IsChecked();
    printing.Fonts.gridLetterFont = m_printGridLetterFont->GetSelectedFont();
    printing.Fonts.gridNumberFont = m_printGridNumberFont->GetSelectedFont();
    printing.Fonts.clueFont = m_printClueFont->GetSelectedFont();
}

void PrintPanel::ConnectChangedEvents()
{
    // None of the ctrls here trigger an update in the UI
}


void PrintPanel::OnPrintCustomFonts(wxCommandEvent & evt)
{
    const bool customFonts = evt.IsChecked();
    m_printGridLetterFont->Enable(customFonts);
    m_printGridNumberFont->Enable(customFonts);
    m_printClueFont->Enable(customFonts);
}

void PrintPanel::OnBlackSquareBrightness(wxScrollEvent & evt)
{
    int value = evt.GetPosition();
    m_printBlackSquarePreview->SetBackgroundColour(wxColour(value, value, value));
    m_printBlackSquarePreview->Refresh();
}
