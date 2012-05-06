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

// Frame position
ConfigManager::Window_t::Window_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Window")),
      top(this, _T("top"), 20),
      left(this, _T("left"), 20),
      width(this, _T("width"), 500),
      height(this, _T("height"), 500),
      maximized(this, _T("maximized"), false)
{}
      
// Grid
ConfigManager::Grid_t::Grid_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Grid")),
      fit(this, _T("fit"), true),
      boxSize(this, _T("boxSize"), 20),
      style(this, _T("style"), DEFAULT_GRID_STYLE),
      letterFont(this, _T("letterFont"), *wxSWISS_FONT),
      numberFont(this, _T("numberFont"), *wxSWISS_FONT),
      lineThickness(this, _T("lineThickness"), 1),
      backgroundColor(this, _T("backgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)),
      focusedLetterColor(this, _T("focusedLetterColor"), *wxGREEN),
      focusedWordColor(this, _T("focusedWordColor"), wxColour(165, 218, 165)),
      whiteSquareColor(this, _T("whiteSquareColor"), *wxWHITE),
      blackSquareColor(this, _T("blackSquareColor"), *wxBLACK),
      selectionColor(this, _T("selectionColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) ),
      penColor(this, _T("penColor"), *wxBLACK),
      pencilColor(this, _T("pencilColor"), wxColor(200,200,200)),
      numberScale(this, _T("numberScale"), 42),
      letterScale(this, _T("letterScale"), 75)
{}

// Clue box
ConfigManager::Clue_t::Clue_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Clue")),
      font(this, _T("font"), wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)),
      spacing(this, _T("spacing"), wxPoint(5, 5)),

      listForegroundColor(this, _T("listForegroundColor"), *wxBLACK),
      listBackgroundColor(this, _T("listBackgroundColor"), *wxWHITE),
      selectedForegroundColor(this, _T("selectedForegroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) ),
      selectedBackgroundColor(this, _T("selectedBackgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) ),
      crossingForegroundColor(this, _T("crossingForegroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)),
      // Using the selected foreground color here can make the list look
      // really ugly.  If, for example, the default selected text color
      // is black, this would make the crossing clue's background color
      // to default to black, which draws too much attention to that clue.
      crossingBackgroundColor(this, _T("crossingBackgroundColor"), *wxWHITE),

      headingFont(this, _T("headingFont"), wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)),
      headingForegroundColor(this, _T("headingForegroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT ) ),
      headingBackgroundColor(this, _T("headingBackgroundColor"), wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE ) )
{}

// Clue prompt
ConfigManager::CluePrompt_t::CluePrompt_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Clue Prompt")),
      font(this, _T("font"), *wxSWISS_FONT),
      foregroundColor(this, _T("foregroundColor"), *wxBLACK),
      backgroundColor(this, _T("backgroundColor"), *wxWHITE),
      displayFormat(this, _T("displayFormat"), _T("<center>%N. %T</center>")),
      alignment(this, _T("alignment"), wxALIGN_CENTER_HORIZONTAL),
      useLua(this, _T("useLua"), false)
{}


// Printing
ConfigManager::Printing_t::Printing_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Printing")),
      Margins(this),
      Fonts(this),
      blackSquareBrightness(this, _T("blackSquareBrightness"), 0),
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
ConfigManager::Timer_t::Timer_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("Timer")),
      autoStart(this, _T("autoStart"), false)
{}

// FileHistory
ConfigManager::FileHistory_t::FileHistory_t(ConfigGroup * parent)
    : ConfigGroup(parent, _T("FileHistory")),
      saveFileHistory(this, _T("saveFileHistory"), true),
      reopenLastPuzzle(this, _T("reopenLastPuzzle"), false)
{}
