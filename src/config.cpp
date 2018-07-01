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

#include "config.hpp"
#include "XGridCtrl.hpp" // DEFAULT_GRID_STYLE
#include <wx/settings.h> // wxSystemSettings
#include "utils/color.hpp" // GetWordHighlight, GetThemeColor

// Frame position
ConfigManager::Window_t::Window_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Window")),
      top(this, _T("top"), 20),
      left(this, _T("left"), 20),
      width(this, _T("width"), 500),
      height(this, _T("height"), 500),
      maximized(this, _T("maximized"), false)
{}

// Global Appearance
ConfigManager::Appearance_t::Appearance_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("GlobalAppearance")),
      font(this, _T("font"), wxSWISS_FONT->Larger()),
      foregroundColor(this, _T("foregroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)),
      backgroundColor(this, _T("backgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)),
      listHighlightColor(this, _T("listHighlightColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)),
      gridHighlightColor(this, _T("gridHighlightColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT))
{
}

// Grid
ConfigManager::Grid_t::Grid_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Grid")),
      fit(this, _T("fit"), true),
      boxSize(this, _T("boxSize"), 20),
      style(this, _T("style"), DEFAULT_GRID_STYLE),
      letterFont(this, _T("letterFont"), &cfg->Appearance.font),
      numberFont(this, _T("numberFont"), &cfg->Appearance.font),
      lineThickness(this, _T("lineThickness"), 1),
      backgroundColor(this, _T("backgroundColor"), &cfg->Appearance.backgroundColor),
      focusedLetterColor(this, _T("focusedLetterColor"), &cfg->Appearance.gridHighlightColor),
      focusedWordColor(this, _T("focusedWordColor"), &cfg->Appearance.gridHighlightColor, &GetWordHighlight),
      whiteSquareColor(this, _T("whiteSquareColor"), *wxWHITE),
      blackSquareColor(this, _T("blackSquareColor"), *wxBLACK),
      highlightTheme(this, _T("highlightTheme"), false),
      themeSquareColor(this, _T("themeSquareColor"), &cfg->Appearance.gridHighlightColor, &GetThemeColor),
      selectionColor(this, _T("selectionColor"), &cfg->Appearance.listHighlightColor),
      penColor(this, _T("penColor"), &cfg->Appearance.foregroundColor),
      pencilColor(this, _T("pencilColor"), wxColor(200,200,200)),
      numberScale(this, _T("numberScale"), 42),
      letterScale(this, _T("letterScale"), 75)
{}


// Transform function for headingFont
wxFont scale_heading_font(const wxFont & font)
{
    return font.Scaled(1.2);
}

// Clue box
ConfigManager::Clue_t::Clue_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Clue")),
      font(this, _T("font"), &cfg->Appearance.font),
      spacing(this, _T("spacing"), wxPoint(5, 5)),

      listForegroundColor(this, _T("listForegroundColor"), &cfg->Appearance.foregroundColor),
      listBackgroundColor(this, _T("listBackgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX), &cfg->Appearance.backgroundColor),
      selectedForegroundColor(this, _T("selectedForegroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)),
      selectedBackgroundColor(this, _T("selectedBackgroundColor"), &cfg->Appearance.listHighlightColor),
      crossingForegroundColor(this, _T("crossingForegroundColor"), &cfg->Appearance.listHighlightColor),
      // Using the selected foreground color here can make the list look
      // really ugly.  If, for example, the default selected text color
      // is black, this would make the crossing clue's background color
      // to default to black, which draws too much attention to that clue.
      crossingBackgroundColor(this, _T("crossingBackgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX), &cfg->Appearance.backgroundColor),

      headingFont(this, _T("headingFont"), &cfg->Appearance.font, &scale_heading_font),
      headingForegroundColor(this, _T("headingForegroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)),
      headingBackgroundColor(this, _T("headingBackgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE))
{}


// Transform function for metadata fonts
wxFont scale_metadata_font(const wxFont & font)
{
    return font.Scaled(.9);
}
// Metadata
ConfigManager::Metadata_t::Metadata_t(
    ConfigGroup * parent, const wxString & name, const wxString & fmt, bool lua)
    : ConfigGroup(parent, name),
      displayFormat(this, _T("displayFormat")),
      useLua(this, _T("useLua"), lua),
      alignment(this, _T("alignment"), wxALIGN_CENTER),
      font(this, _T("font")),
      foregroundColor(this, _T("foregroundColor")),
      backgroundColor(this, _T("backgroundColor"))
{
    // We can't get a ConfigManager pointer in this constructor,
    // so we'll have to cast our m_cfg pointer
    ConfigManager * cfg = static_cast<ConfigManager *>(m_cfg);
    font.LinkValue(&cfg->Appearance.font, &scale_metadata_font);
    foregroundColor.LinkValue(&cfg->Appearance.foregroundColor);
    backgroundColor.LinkValue(&cfg->Appearance.backgroundColor);
    if (! fmt.empty())
        displayFormat.SetDefault(fmt);
}

// Clue prompt
ConfigManager::CluePrompt_t::CluePrompt_t(ConfigGroup * parent)
    : Metadata_t(parent, _T("Clue Prompt"), _T("%cluenumber%. %clue%"), false)
{}

// Notes
ConfigManager::Notes_t::Notes_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Notes")),
      font(this, _T("font"), &cfg->Appearance.font),
      foregroundColor(this, _T("foregroundColor"), &cfg->Appearance.foregroundColor),
      backgroundColor(this, _T("backgroundColor"), &cfg->Appearance.backgroundColor)
{}

// Status bar
ConfigManager::Status_t::Status_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("StatusBar")),
      completeColor(this, _T("completeColor"), *wxGREEN),
      incorrectColor(this, _T("incorrectColor"), *wxRED),
      uncheckableColor(this, _T("uncheckableColor"), *wxCYAN),
      brightnessCutoff(this, _T("brightnessCutoff"), 200),
      showCompletionStatus(this, _T("showCompletionStatus"), true)
{}

// Printing
ConfigManager::Printing_t::Printing_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Printing")),
      Margins(this),
      Fonts(this),
      blackSquareBrightness(this, _T("blackSquareBrightness"), (long)0),
      gridAlignment(this, _T("gridAlignment"), wxALIGN_TOP | wxALIGN_RIGHT),
      paperID(this, _T("paperID"), wxPAPER_LETTER),
      orientation(this, _T("orientation"), wxPORTRAIT)
{}

ConfigManager::Printing_t::Margins_t::Margins_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Margins")),
      left(this, _T("left"), 15),
      right(this, _T("right"), 15),
      top(this, _T("top"), 15),
      bottom(this, _T("bottom"), 15)
{}

ConfigManager::Printing_t::Fonts_t::Fonts_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Fonts")),
      useCustomFonts(this, _T("useCustomFonts"), false),
      gridLetterFont(this, _T("gridLetterFont"), *wxSWISS_FONT),
      gridNumberFont(this, _T("gridNumberFont"), *wxSWISS_FONT),
      clueFont(this, _T("clueFont"), *wxSWISS_FONT)
{}

// Timer
ConfigManager::Timer_t::Timer_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("Timer")),
      autoStart(this, _T("autoStart"), false)
{}

// FileHistory
ConfigManager::FileHistory_t::FileHistory_t(ConfigManager * cfg)
    : ConfigGroup(&cfg->m_group, _T("FileHistory")),
      saveFileHistory(this, _T("saveFileHistory"), true),
      reopenLastPuzzle(this, _T("reopenLastPuzzle"), false)
{}
