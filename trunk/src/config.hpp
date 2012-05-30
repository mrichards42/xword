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

#ifndef XWORD_CONFIG_H
#define XWORD_CONFIG_H

#include "utils/ConfigManager.hpp"

class ConfigManager : public ConfigManagerBase
{
public:
    ConfigManager()
        : Window(&m_group),
          Grid(&m_group),
          Clue(&m_group),
          CluePrompt(&m_group),
          Printing(&m_group),
          Timer(&m_group),
          FileHistory(&m_group),
          autoSaveInterval(&m_group, _T("autoSaveInterval"), 0),
          MetadataCtrls(&m_group, _T("Metadata")),
          useSimpleStyle(&m_group, _T("useSimpleStyle"), true)
    {}

    // Frame position
    class Window_t : public ConfigGroup {
    public:
        Window_t(ConfigGroup * parent);

        ConfigLong top;
        ConfigLong left;
        ConfigLong width;
        ConfigLong height;
        ConfigBool maximized;
    } Window;

    // Grid
    class Grid_t : public ConfigGroup {
    public:
        Grid_t(ConfigGroup * parent);

        ConfigBool fit;
        ConfigLong boxSize;
        ConfigLong style;
        ConfigFont letterFont;
        ConfigFont numberFont;
        ConfigLong lineThickness;
        ConfigColor backgroundColor;
        ConfigColor focusedLetterColor;
        ConfigColor focusedWordColor;
        ConfigColor whiteSquareColor;
        ConfigColor blackSquareColor;
        ConfigColor selectionColor;
        ConfigColor penColor;
        ConfigColor pencilColor;
        ConfigLong numberScale;
        ConfigLong letterScale;
    } Grid;

    // Clue box
    class Clue_t : public ConfigGroup {
    public:
        Clue_t(ConfigGroup * parent);

        ConfigFont font;
        ConfigPoint spacing;

        ConfigColor listForegroundColor;
        ConfigColor listBackgroundColor;
        ConfigColor selectedForegroundColor;
        ConfigColor selectedBackgroundColor;
        ConfigColor crossingForegroundColor;
        ConfigColor crossingBackgroundColor;

        ConfigFont headingFont;
        ConfigColor headingForegroundColor;
        ConfigColor headingBackgroundColor;
    } Clue;

    // Metadata
    class Metadata_t : public ConfigGroup {
    public:
        Metadata_t(ConfigGroup * parent,
            const wxString & name,
            const wxString & displayFormat_ = wxEmptyString,
            bool useLua_ = false,
            long alignment_ = wxALIGN_CENTER,
            const wxFont & font_ = *wxSWISS_FONT,
            const wxColour & foregroundColor_ = *wxBLACK,
            const wxColour & backgroundColor_ = *wxWHITE);

        ConfigString displayFormat;
        ConfigBool useLua;
        ConfigFont font;
        ConfigColor foregroundColor;
        ConfigColor backgroundColor;
        ConfigLong alignment;
    };

    typedef ConfigList<Metadata_t> MetadataCtrls_t;
    MetadataCtrls_t MetadataCtrls;

    // CluePrompt
    class CluePrompt_t : public Metadata_t {
    public:
        CluePrompt_t(ConfigGroup * parent);
    } CluePrompt;

    // Printing
    class Printing_t : public ConfigGroup {
    public:
        Printing_t(ConfigGroup * parent);

        ConfigLong blackSquareBrightness;
        ConfigLong gridAlignment;
        ConfigLong paperID;
        ConfigLong orientation;

        // Margins
        class Margins_t : public ConfigGroup {
        public:
            Margins_t(ConfigGroup * parent);

            ConfigLong left;
            ConfigLong right;
            ConfigLong top;
            ConfigLong bottom;
        } Margins;

        // Fonts
        class Fonts_t : public ConfigGroup {
        public:
            Fonts_t(ConfigGroup * parent);

            ConfigBool useCustomFonts;
            ConfigFont gridLetterFont;
            ConfigFont gridNumberFont;
            ConfigFont clueFont;
        } Fonts;

    } Printing;

    // Timer
    class Timer_t : public ConfigGroup {
    public:
        Timer_t(ConfigGroup * parent);

        ConfigBool autoStart;
    } Timer;

    // FileHistory
    class FileHistory_t : public ConfigGroup {
    public:
        FileHistory_t(ConfigGroup * parent);

        ConfigBool saveFileHistory;
        ConfigBool reopenLastPuzzle;
    } FileHistory;

    ConfigLong autoSaveInterval;
    ConfigBool useSimpleStyle;
};

#endif // XWORD_CONFIG_H
