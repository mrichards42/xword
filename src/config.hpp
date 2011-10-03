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
          autoSaveInterval(&m_group, _T("autoSaveInterval"), 0)
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

    // Clue prompt
    class CluePrompt_t : public ConfigGroup {
    public:
        CluePrompt_t(ConfigGroup * parent);

        ConfigFont font;
        ConfigColor foregroundColor;
        ConfigColor backgroundColor;
        ConfigString displayFormat;
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

    ConfigLong autoSaveInterval;
};

#endif // XWORD_CONFIG_H
