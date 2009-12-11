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


#ifndef PREFERENCES_DLG_H
#define PREFERENCES_DLG_H

#include "wxFB_Dialogs.h"

#include "../MyFrame.hpp"

class PreferencesDialog : public wxFB_PreferencesDialog
{
public:
    PreferencesDialog(MyFrame * frame);

protected:
    MyFrame * m_frame;

    // Load the window / config states
    void LoadConfig();
    void SaveConfig();

    // Event Handlers
    //---------------

    virtual void OnInit(wxInitDialogEvent & evt) { LoadConfig(); evt.Skip(); }
    virtual void OnOK(wxCommandEvent & evt) { SaveConfig(); evt.Skip(); }

    // Grid style
    //-----------

    // Helpers
    static int AddFlag   (int flag, int add) { return flag | add; }
    static int RemoveFlag(int flag, int rem) { return flag & ~ rem; }
    static bool HasFlag (int flag, int test) { return (flag & test) != 0; }
    void AddRemoveGridFlag(wxCommandEvent & evt, int flag);

	virtual void OnAfterLetter(wxCommandEvent & evt);
	virtual void OnBlankOnDirection(wxCommandEvent & evt);
	virtual void OnBlankOnNewWord(wxCommandEvent & evt);
	virtual void OnPauseOnSwitch(wxCommandEvent & evt);
	virtual void OnMoveOnRightClick(wxCommandEvent & evt);
	virtual void OnCheckWhileTyping(wxCommandEvent & evt);

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
	virtual void OnSelectedLetterColor(wxColourPickerEvent & evt);
	virtual void OnSelectedWordColor(wxColourPickerEvent & evt);
	virtual void OnPenColor(wxColourPickerEvent & evt);
	virtual void OnPencilColor(wxColourPickerEvent & evt);

    // Clue Prompt
	virtual void OnPromptBackgroundColor(wxColourPickerEvent & evt);
	virtual void OnPromptTextColor(wxColourPickerEvent & evt);

    // Clue List
	virtual void OnClueBackgroundColor(wxColourPickerEvent & evt);
	virtual void OnClueTextColor(wxColourPickerEvent & evt);
	virtual void OnSelectedClueBackgroundColor(wxColourPickerEvent & evt);
	virtual void OnSelectedClueTextColor(wxColourPickerEvent & evt);
	virtual void OnCrossingClueBackgroundColor(wxColourPickerEvent & evt);
	virtual void OnCrossingClueTextColor(wxColourPickerEvent & evt);
	virtual void OnClueHeadingTextColor(wxColourPickerEvent & evt);
	virtual void OnClueHeadingBackgroundColor(wxColourPickerEvent & evt);

    // Fonts
    //------
	virtual void OnGridLetterFont(wxFontPickerEvent & evt);
    virtual void OnGridNumberFont(wxFontPickerEvent & evt);
	virtual void OnCluePromptFont(wxFontPickerEvent & evt);
	virtual void OnClueFont(wxFontPickerEvent & evt);
	virtual void OnClueHeadingFont(wxFontPickerEvent & evt);

    // Misc
    //-----
	virtual void OnCluePromptFormat(wxCommandEvent & evt);
    virtual void OnLetterScale(wxSpinEvent & evt);
    virtual void OnNumberScale(wxSpinEvent & evt);

    // Printing
    //---------
    virtual void OnPrintBlackSquareBrightness(wxScrollEvent & evt);
    virtual void OnPrintCustomFonts(wxCommandEvent & evt);
};


#endif // PREFERENCES_DLG_H
