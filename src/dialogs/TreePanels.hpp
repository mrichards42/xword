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


#ifndef APPEARANCE_PANEL_H
#define APPEARANCE_PANEL_H

#include <wx/wxprec.h>
#include <list>
#ifndef WX_PRECOMP
#   include <wx/treectrl.h>
#endif

#include "../paths.hpp"
#include "../config.hpp"

#include "TreeCtrls.hpp"
#include "PreferencesPanel.hpp"
#include "MetadataFormat.hpp"

// The base class for all Appearance tree panels
class AppearanceBase
{
public:
    AppearanceBase() : m_disableChangedEvent(false) {}
    virtual ~AppearanceBase() {}
    // Load/Save Config
    void LoadConfig()
    {
        m_disableChangedEvent = true;
        DoLoadConfig();
        m_disableChangedEvent = false;
    }
    void SaveConfig() { DoSaveConfig(); }
    void ResetConfig()
    {
        m_disableChangedEvent = true;
        DoResetConfig();
        DoLoadConfig();
        m_disableChangedEvent = false;
    }

protected:
    virtual void DoLoadConfig()=0;
    virtual void DoSaveConfig()=0;
    virtual void DoResetConfig()=0;

    // Automatic save when a control value is changed
    void OnChanged(wxEvent & evt)
    {
        evt.Skip();
        if (! m_disableChangedEvent)
            SaveConfig();
    }

    template <typename T>
    void BindChangedEvent(T handler)
    {
        ::BindChangedEvent(handler, &AppearanceBase::OnChanged, this);
    }

    bool m_disableChangedEvent;
};


// The base class for Appearance wxPanels
class AppearancePanelBase : public AppearanceBase, public wxPanel
{
public:
    AppearancePanelBase(wxWindow * parent)
        : wxPanel(parent, wxID_ANY)
    {
        SetSizer(new wxFlexGridSizer(0, 2, 5, 5));
    }

    virtual ~AppearancePanelBase()
    {
        // Delete the pointers in m_cfgCtrls
        std::list<ConfigControlBase *>::iterator it;
        for (it = m_cfgCtrls.begin(); it != m_cfgCtrls.end(); ++it)
            delete *it;
    }

protected:
    // Add a managed and labeled control to the layout
    template <typename T>
    typename ConfigControl<T>::ctrlType * Add(ConfigValue<T> & cfg, const wxString & label)
    {
        return AddCtrl(AddConfigControl(new ConfigControl<T>(cfg)), label);
    }

    template <typename T, typename DESC>
    typename DESC::ctrlType * Add(ConfigValue<T> & cfg, const wxString & label,
                                  const DESC & desc)
    {
        return AddCtrl(AddConfigControl(new ConfigControl<T, DESC>(cfg, desc)), label);
    }

    // Add a managed control and bind the changed event (but don't add to the layout)
    template <typename T>
    typename T::ctrlType * AddConfigControl(T * info)
    {
        m_cfgCtrls.push_back(info);
        typename T::ctrlType * ctrl = static_cast<typename T::ctrlType *>(info->NewCtrl(this));
        info->LoadConfig();
        BindChangedEvent(ctrl);
        return ctrl;
    }

    // Add a control and label to the sizer
    template <typename CTRL>
    CTRL * AddCtrl(CTRL * ctrl, const wxString & label)
    {
        wxSizer * s = GetSizer();
        s->Add(new wxStaticText(this, wxID_ANY, label + ":"), 0, wxALIGN_CENTER_VERTICAL);
        s->Add(ctrl, 0, wxALIGN_CENTER_VERTICAL);
        return ctrl;
    }

    // Save/load Config
    virtual void DoLoadConfig()
    {
        std::list<ConfigControlBase *>::iterator it;
        for (it = m_cfgCtrls.begin(); it != m_cfgCtrls.end(); ++it)
            (*it)->LoadConfig();
    }

    virtual void DoSaveConfig()
    {
        std::list<ConfigControlBase *>::iterator it;
        for (it = m_cfgCtrls.begin(); it != m_cfgCtrls.end(); ++it)
            (*it)->SaveConfig();
    }

    virtual void DoResetConfig()
    {
        std::list<ConfigControlBase *>::iterator it;
        for (it = m_cfgCtrls.begin(); it != m_cfgCtrls.end(); ++it)
            (*it)->ResetConfig();
    }


    // A list to hold configuration/control information
    std::list<ConfigControlBase *> m_cfgCtrls;
};


// ---------------------------------------------------------------------------
// Appearance Panels
// ---------------------------------------------------------------------------

// Notes
class NotesAppearance : public AppearancePanelBase
{
public:
    NotesAppearance(wxWindow * parent, ConfigManager::Notes_t & notes)
        : AppearancePanelBase(parent)
    {
        Add(notes.font, "Font", FontDesc(FP_DEFAULT & ~FP_POINTSIZE));
        Add(notes.foregroundColor, "Text Color");
        Add(notes.backgroundColor, "Background");
    }
};

// Status Bar
class StatusAppearance : public AppearancePanelBase
{
public:
    StatusAppearance(wxWindow * parent, ConfigManager::Status_t & status)
        : AppearancePanelBase(parent)
    {
        Add(status.completeColor, "Complete Puzzle");
        Add(status.incorrectColor, "Incorrect Puzzle");
        Add(status.uncheckableColor, "No Solution");
    }
};

// Clue List
class ClueListAppearance : public AppearancePanelBase
{
public:
    ClueListAppearance(wxWindow * parent, ConfigManager::Clue_t & clue)
        : AppearancePanelBase(parent)
    {
        Add(clue.font, "Font", FontDesc(FP_FACENAME | FP_POINTSIZE));
        Add(clue.listForegroundColor, "Text");
        Add(clue.listBackgroundColor, "Background");
    }
};

// Clue List current clue
class ClueListSelectionAppearance : public AppearancePanelBase
{
public:
    ClueListSelectionAppearance(wxWindow * parent, ConfigManager::Clue_t & clue)
        : AppearancePanelBase(parent)
    {
        Add(clue.selectedForegroundColor, "Text");
        Add(clue.selectedBackgroundColor, "Background");
    }
};

// Clue List crossing clue
class ClueListCrossingAppearance : public AppearancePanelBase
{
public:
    ClueListCrossingAppearance(wxWindow * parent, ConfigManager::Clue_t & clue)
        : AppearancePanelBase(parent)
    {
        Add(clue.crossingForegroundColor, "Text");
        Add(clue.crossingBackgroundColor, "Background");
    }
};

// Clue List Heading
class ClueListHeadingAppearance : public AppearancePanelBase
{
public:
    ClueListHeadingAppearance(wxWindow * parent, ConfigManager::Clue_t & clue)
        : AppearancePanelBase(parent)
    {
        Add(clue.headingFont, "Font", FontDesc(FP_DEFAULT));
        Add(clue.headingForegroundColor, "Text");
        Add(clue.headingBackgroundColor, "Background");
    }
};

// Grid
class GridBaseAppearance : public AppearancePanelBase
{
public:
    GridBaseAppearance(wxWindow * parent, ConfigManager::Grid_t & grid)
        : AppearancePanelBase(parent)
    {
        Add(grid.letterFont, "Font", FontDesc(FP_DEFAULT & ~FP_POINTSIZE));
        Add(grid.numberFont, "Number", FontDesc(FP_DEFAULT & ~FP_POINTSIZE));
        Add(grid.penColor, "Text");
        Add(grid.backgroundColor, "Background");
    }
};

// Grid Selection
class GridSelectionAppearance : public AppearancePanelBase
{
public:
    GridSelectionAppearance(wxWindow * parent, ConfigManager::Grid_t & grid)
        : AppearancePanelBase(parent)
    {
        Add(grid.focusedLetterColor, "Selected Letter");
        Add(grid.focusedWordColor, "Selected Word");
        Add(grid.selectionColor, "Multi-square\nSelection");
        Add(grid.themeSquareColor, "Theme Square Color");
    }
};


// Grid Tweaks
#include "wxFB_TreePanels.h"
class GridTweaksAppearance : public AppearanceBase, public wxFB_GridTweaks
{
public:
    GridTweaksAppearance(wxWindow * parent, ConfigManager::Grid_t & grid)
        : wxFB_GridTweaks(parent),
          m_config(grid)
    {
        BindChangedEvent(m_lineThickness);
        BindChangedEvent(m_letterScale);
        BindChangedEvent(m_numberScale);
        LoadConfig();
    }

protected:
    void DoLoadConfig()
    {
        m_lineThickness->SetValue(m_config.lineThickness());
        m_letterScale->SetValue(m_config.letterScale());
        m_numberScale->SetValue(m_config.numberScale());
    }

    void DoSaveConfig()
    {
        m_config.lineThickness = m_lineThickness->GetValue();
        m_config.letterScale = m_letterScale->GetValue();
        m_config.numberScale = m_numberScale->GetValue();
    }

    void DoResetConfig()
    {
        m_config.lineThickness.Reset();
        m_config.letterScale.Reset();
        m_config.numberScale.Reset();
    }
    ConfigManager::Grid_t & m_config;
};

// ---------------------------------------------------------------------------
// Metadata panels
// ---------------------------------------------------------------------------
#include <wx/wrapsizer.h>
class MetadataAppearance : public AppearancePanelBase
{
public:
    MetadataAppearance(wxWindow * parent, ConfigManager::Metadata_t & meta,
                           long flag = FP_FACENAME | FP_POINTSIZE)
        : AppearancePanelBase(parent)
    {
        // Standard Appearance
        Add(meta.font, "Font", FontDesc(flag));
        Add(meta.alignment, "Alignment", HAlignDesc());
        Add(meta.alignment, "Vertical Align", VAlignDesc());
        Add(meta.foregroundColor, "Text");
        Add(meta.backgroundColor, "Background");

        // DisplayFormat
        m_format = AddConfigControl(new ConfigControl<wxString, MetadataFormatDesc>(meta.displayFormat));
        // Prevent the property dialog from sizing too tall if m_format has
        // a long string
        m_format->SetInitialSize(wxSize(250, m_format->GetCharHeight() * 7));

#ifdef __WXOSX__
        // Since this might be a script, don't let OS X substitute fancy quotes.
        m_format->OSXEnableAutomaticQuoteSubstitution(false);
#endif

        // Use Lua checkbox
        m_useLua = AddConfigControl(new ConfigControl<bool>(meta.useLua));
        m_useLua->SetLabel("Script");
        m_useLua->SetToolTip("Format as a lua script");

        // Help button
        m_helpBtn = new wxBitmapButton(
            this, wxID_ANY, wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, wxSize(16,16)), wxDefaultPosition,
            wxDefaultSize, wxBORDER_NONE);
        m_helpBtn->Bind(wxEVT_BUTTON, &MetadataAppearance::OnFormatHelp, this);

        // Layout
        wxBoxSizer * labelSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer * s1 = new wxBoxSizer(wxHORIZONTAL);
        s1->Add(new wxStaticText(this, wxID_ANY, "Display\nFormat:"));
        s1->AddStretchSpacer();
        s1->Add(m_helpBtn);
        labelSizer->Add(s1, 0, wxEXPAND);
        labelSizer->Add(m_useLua);

        wxFlexGridSizer * sizer = wxDynamicCast(GetSizer(), wxFlexGridSizer);
        sizer->Add(labelSizer, 0, wxEXPAND);
        sizer->Add(m_format, 1, wxEXPAND);
    #if ! XWORD_USE_LUA
        labelSizer->Hide(m_useLua);
    #endif // XWORD_USE_LUA
        sizer->AddGrowableRow(sizer->GetChildren().GetCount() / 2 - 1);
        sizer->AddGrowableCol(1);
        //Layout();
    }

    virtual ~MetadataAppearance() {}

protected:
    void OnFormatHelp(wxCommandEvent & evt)
    {
        MetadataFormatHelpPopup * popup;
        popup = new MetadataFormatHelpPopup(this, GetMetadataFields());
        popup->Position(m_helpBtn->GetScreenPosition(), m_helpBtn->GetSize());
        popup->Popup();
    }

    // Override this to change FormatHelpDialog behavior
    virtual wxArrayString GetMetadataFields()
    {
        const wxChar * fields[] = {
            _T("title"),
            _T("author"),
            _T("copyright"),
            _T("editor"),
            _T("publisher"),
            _T("date")
        };
        return wxArrayString(sizeof(fields) / sizeof(wxChar*), fields);
    }


    wxTextCtrl * m_format;
    wxCheckBox * m_useLua;
    wxBitmapButton * m_helpBtn;
};


// Clue Prompt
// Same as MetadataAppearance but without font size and using different meta fields
class CluePromptAppearance : public MetadataAppearance
{
public:
    CluePromptAppearance(wxWindow * parent, ConfigManager::Metadata_t & cfg)
        : MetadataAppearance(parent, cfg, FP_FACENAME)
    {}
protected:
    virtual wxArrayString GetMetadataFields()
    {
        const wxChar * fields[] = {
            _T("cluenumber"),
            _T("clue")
        };
        return wxArrayString(sizeof(fields) / sizeof(wxChar*), fields);
    }
};

// ---------------------------------------------------------------------------
// Global/Simple
// ---------------------------------------------------------------------------

#include "../utils/color.hpp"

class GlobalAppearance : public AppearancePanelBase
{
public:
    GlobalAppearance(wxWindow * parent, ConfigManager & config)
        : AppearancePanelBase(parent),
          m_config(config)
    {
        m_fontPicker = Add(config.Appearance.font, "Font");
        Add(config.Appearance.foregroundColor, "Text Color");
        Add(config.Appearance.backgroundColor, "Background");
        Add(config.Appearance.listHighlightColor, "List Highlight");
        Add(config.Appearance.gridHighlightColor, "Grid Highlight");
        Add(config.Grid.highlightTheme, "Highlight Theme Squares");
        GetSizer()->AddSpacer(5);
    }

    void DoSaveConfig()
    {
        // Update the grid focus colors
        m_config.Grid.focusedLetterColor.Reset();
        m_config.Grid.focusedWordColor.Reset();
        // The user expects all fonts to change point size as the global font
        // is adjusted, but the ComfigManager mechanism for linking
        // ConfigValues isn't smart enough for that, so we'll do it manually.
        wxFont font = m_fontPicker->GetSelectedFont();
        int difference = font.GetPointSize() - m_config.Appearance.font().GetPointSize();
        // Grid fonts and clue prompt will auto-update, since pt size isn't
        // taken into account
        AdjustFont(m_config.Clue.font, font, difference);
        AdjustFont(m_config.Clue.headingFont, font, difference);
        AdjustFont(m_config.Notes.font, font, difference);
        AdjustFont(m_config.Clue.headingFont, font, difference);
        // MetadataCtrls
        ConfigManager::MetadataCtrls_t & metadata = m_config.MetadataCtrls;
        ConfigManager::MetadataCtrls_t::iterator meta;
        for (meta = metadata.begin(); meta != metadata.end(); ++meta)
            AdjustFont(meta->font, font, difference);

        // Save the rest of the config values
        AppearancePanelBase::DoSaveConfig();
    }

    // Set the ConfigFont to the specified font, adjusting the point size
    // from the original ConfigFont accordingly
    void AdjustFont(ConfigFont & cfg, wxFont & font, int ptAdjustment)
    {
        font.SetPointSize(cfg.Get().GetPointSize() + ptAdjustment);
        cfg.Set(font);
    }

    FontPickerPanel * m_fontPicker;

protected:
    ConfigManager & m_config;
};

#endif // APPEARANCE_PANEL_H
