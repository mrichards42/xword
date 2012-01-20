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


#ifndef PREFERENCES_DLG_H
#define PREFERENCES_DLG_H

#include "wxFB_Dialogs.h"

class PreferencesDialog : public wxFB_PreferencesDialog
{
public:
    PreferencesDialog(wxWindow * parent);

protected:
    // Load the window / config states
    void LoadConfig();
    void SaveConfig();

    // Event Handlers
    virtual void OnInit(wxInitDialogEvent & evt)
        { LoadConfig(); evt.Skip(); }
    virtual void OnOK(wxCommandEvent & evt)
        { SaveConfig(); GetParent()->Refresh(); evt.Skip(); }
    virtual void OnApply(wxCommandEvent & evt)
        { SaveConfig(); GetParent()->Refresh(); evt.Skip(); }

    virtual void OnSaveFileHistory(wxCommandEvent & evt);
    virtual void OnPrintCustomFonts(wxCommandEvent & evt);
    virtual void OnBlackSquareBrightness(wxScrollEvent & evt);
};


#endif // PREFERENCES_DLG_H