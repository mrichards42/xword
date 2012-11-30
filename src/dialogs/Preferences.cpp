// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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
#include "StyleEditors.hpp"
#if XWORD_USE_LUA
#   include "../xwordlua.hpp"
#endif // XWORD_USE_LUA

PreferencesDialog::PreferencesDialog(wxWindow * parent)
    : PreferencesDialogBase(parent, wxID_ANY),
      m_config()
{
    // Set a dummy config so that we can use this variable.
    m_config.SetConfig(new wxFileConfig);
    // Enumerate the font faces
    FontFaceCtrl::InitFaceNames();
}


PreferencesDialog::~PreferencesDialog()
{
    delete m_config.GetConfig();
    // Clear the font faces
    FontFaceCtrl::ClearFaceNames();
}

extern int wxluatype_wxNotebook;

void PreferencesDialog::OnInit(wxInitDialogEvent & evt)
{
    LoadConfig();
    SetupStyleTree();
#if XWORD_USE_LUA
    // Add preferences pages from lua
    MyFrame * frame = wxDynamicCast(GetParent(), MyFrame);
    wxASSERT(frame);
    if (frame)
    {
        wxLuaState & lua = frame->GetwxLuaState();
        lua_State * L = lua.GetLuaState();
        // Find the function xword.OnInitPreferencesDialog
        lua_getglobal(L, "xword");
        lua_getfield(L, -1, "OnInitPreferencesDialog");
        if (lua_isfunction(L, -1))
        {
            // Call the function with our notebook argument
            if (wxluaT_pushuserdatatype(L, m_notebook, wxluatype_wxNotebook))
                lua_pcall(L, 1, 0, 0);
        }
        lua_pop(L, 1); // remove the xword table from the stack
    }
#endif
    evt.Skip();
}

//------------------------------------------------------------------------------
// Load config
//------------------------------------------------------------------------------

void
PreferencesDialog::LoadConfig()
{
    try {
        m_config.Copy(wxGetApp().GetConfigManager());
    }
    catch (ConfigManager::CopyError &) {
        wxFAIL_MSG(_T("Error copying config"));
        // Can't really do anything here . . .
    }

    // Grid Style
    const int gridStyle = m_config.Grid.style();

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

    // Printing
    ConfigManager::Printing_t & printing = m_config.Printing;
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
    m_startTimer->SetValue(m_config.Timer.autoStart());

    // Auto Save
    m_autoSave->SetValue(m_config.autoSaveInterval());

    // File History
    m_saveFileHistory->SetValue(m_config.FileHistory.saveFileHistory());
    m_reopenLastPuzzle->SetValue(m_config.FileHistory.reopenLastPuzzle());
    m_reopenLastPuzzle->Enable(m_saveFileHistory->GetValue());
}


//------------------------------------------------------------------------------
// Save config
//------------------------------------------------------------------------------

void
PreferencesDialog::SaveConfig()
{
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

    m_config.Grid.style = gridStyle;

    // Printing settings
    //------------------
    ConfigManager::Printing_t & printing = m_config.Printing;
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

    m_config.Timer.autoStart = m_startTimer->GetValue();
    m_config.autoSaveInterval = m_autoSave->GetValue();

    // File History
    m_config.FileHistory.saveFileHistory = m_saveFileHistory->IsChecked();
    m_config.FileHistory.reopenLastPuzzle = m_reopenLastPuzzle->IsChecked();

    // If we have a selected styleTree panel, save that config too
    SaveStyleTreeConfig();

    wxGetApp().GetConfigManager().Copy(m_config);

#if XWORD_USE_LUA
    // Save preferences from lua
    MyFrame * frame = wxDynamicCast(GetParent(), MyFrame);
    wxASSERT(frame);
    if (frame)
    {
        wxLuaState & lua = frame->GetwxLuaState();
        lua_State * L = lua.GetLuaState();
        // Find the function xword.OnSavePreferences
        lua_getglobal(L, "xword");
        lua_getfield(L, -1, "OnSavePreferences");
        if (lua_isfunction(L, -1))
        {
            // Call the function with our notebook argument
            lua_pcall(L, 0, 0, 0);
        }
        lua_pop(L, 1); // remove the xword table from the stack
    }
#endif
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
}


//------------------------------------------------------------------------------
// StyleTree
//------------------------------------------------------------------------------
#include "StylePanel.hpp"

void
PreferencesDialog::SetupStyleTree()
{
    m_styleTree->Freeze();
    m_styleTree->DeleteAllItems();

    wxTreeItemId root = m_styleTree->AddRoot(_T("All Styles"));

    wxTreeItemId metaroot;

    const bool simple = m_config.useSimpleStyle();
    if (simple)
    {
        wxTreeItemId styles = m_styleTree->AppendItem(root, _T("Styles"));
        m_styleTree->SetItemData(styles, new SimpleStyle(m_config));
        metaroot = m_styleTree->AppendItem(root, _T("Metadata"));
        m_styleTree->SelectItem(styles);

        m_simpleStyleButton->SetLabel(_T("Show Advanced Options"));
    }
    else
    {
        wxTreeItemId grid = m_styleTree->AppendItem(root, _T("Grid"));
        m_styleTree->SetItemData(grid, new GridBaseStyle(m_config.Grid));

        wxTreeItemId gridSelection = m_styleTree->AppendItem(grid, _T("Cursor/Selection"));
        m_styleTree->SetItemData(gridSelection, new GridSelectionStyle(m_config.Grid));

        wxTreeItemId gridTweaks = m_styleTree->AppendItem(grid, _T("Display Tweaks"));
        m_styleTree->SetItemData(gridTweaks, new GridTweaksStyle(m_config.Grid));

        wxTreeItemId clueList = m_styleTree->AppendItem(root, _T("Clue List"));
        m_styleTree->SetItemData(clueList, new ClueListStyle(m_config.Clue));

        wxTreeItemId clueListSelection = m_styleTree->AppendItem(clueList, _T("Selected Clue"));
        m_styleTree->SetItemData(clueListSelection, new ClueListSelectionStyle(m_config.Clue));

        wxTreeItemId clueListCrossing = m_styleTree->AppendItem(clueList, _T("Crossing Clue"));
        m_styleTree->SetItemData(clueListCrossing, new ClueListCrossingStyle(m_config.Clue));

        wxTreeItemId clueListHeading = m_styleTree->AppendItem(clueList, _T("Heading"));
        m_styleTree->SetItemData(clueListHeading, new ClueListHeadingStyle(m_config.Clue));

        wxTreeItemId cluePrompt = m_styleTree->AppendItem(root, _T("Clue Prompt"));
        m_styleTree->SetItemData(cluePrompt, new CluePromptStyle(m_config.CluePrompt));

        metaroot = m_styleTree->AppendItem(root, _T("Metadata"));

        wxTreeItemId notes = m_styleTree->AppendItem(metaroot, _T("Notes"));
        m_styleTree->SetItemData(notes, new NotesStyle(m_config.Notes));

        m_styleTree->SelectItem(grid);

        m_simpleStyleButton->SetLabel(_T("Show Simple Options"));
    }

    // Metadata
    if (metaroot.IsOk())
    {
        // Add the metadata ctrls
        ConfigManager::MetadataCtrls_t & metadata = m_config.MetadataCtrls;
        ConfigManager::MetadataCtrls_t::iterator meta;
        for (meta = metadata.begin(); meta != metadata.end(); ++meta)
        {
            // Chop the metadata part of the name off
            wxString name;
            if (! meta->m_name.StartsWith(_T("/Metadata/"), &name))
                name = meta->m_name;
            wxTreeItemId item = m_styleTree->AppendItem(metaroot, name);
            if (simple)
                m_styleTree->SetItemData(item, new SimpleMetadataStyle(*meta));
            else
                m_styleTree->SetItemData(item, new MetadataStyle(*meta));
        }
    }

    m_styleTree->ExpandAll();
    m_styleTree->Thaw();
}

void
PreferencesDialog::OnSimpleStyleButton(wxCommandEvent & evt)
{
    bool simple = ! m_config.useSimpleStyle();
    m_config.useSimpleStyle = simple;
    // Copy this to the real config manager right away.
    wxGetApp().GetConfigManager().useSimpleStyle = simple;
    SetupStyleTree();
}


// Helper functions
StyleBase * GetStyleData(wxTreeCtrl * ctrl, const wxTreeItemId & id)
{
    if (! id.IsOk())
        return NULL;
    return dynamic_cast<StyleBase *>(ctrl->GetItemData(id));
}

StyleBase * GetStyleData(wxTreeCtrl * ctrl)
{
    return GetStyleData(ctrl, ctrl->GetSelection());
}

void
PreferencesDialog::OnStyleTreeSelection(wxTreeEvent & evt)
{
    m_stylePanel->Freeze();
    // Destroy the old panel
    if (! m_styleSizer->GetChildren().empty())
    {
        // If we had an old panel, save the config from it
        StyleBase * data = GetStyleData(m_styleTree, evt.GetOldItem());
        if (data)
            data->SaveConfig();
        m_styleSizer->Clear(true); // Clear and destroy windows
    }

    // Make a new one
    wxWindow * newPanel = NULL;
    // See if this item has a custom style panel associated with it.
    StyleBase * data = GetStyleData(m_styleTree, evt.GetItem());
    if (data)
    {
        // Update the colors
        ColorChoice::ClearColors();
        ColorChoice::InitColors(&m_config);
        newPanel = data->NewStylePanel(m_stylePanel);
        data->LoadConfig();
    }
    else
    {
        newPanel = new wxPanel(m_stylePanel, wxID_ANY);
        wxSizer * hsizer = new wxBoxSizer(wxHORIZONTAL);
        hsizer->Add(new wxStaticText(newPanel, wxID_ANY, _T("Select a window to configure")), 1, wxALIGN_CENTER);
        wxSizer * vsizer = new wxBoxSizer(wxVERTICAL);
        vsizer->Add(hsizer, 1, wxALIGN_CENTER);
        newPanel->SetSizer(vsizer);
    }

    m_styleSizer->Add(newPanel, 1, wxEXPAND);

    m_styleSizer->Layout();
    m_stylePanel->Thaw();
}

void
PreferencesDialog::SaveStyleTreeConfig()
{
    StyleBase * data = GetStyleData(m_styleTree);
    if (data)
        data->SaveConfig();
}