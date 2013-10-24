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

#include "../config.hpp" // For ConfigManager
#include <wx/propdlg.h>

#ifdef __WXOSX__
#   define XWORD_PREFERENCES_LIVE_PREVIEW 1
#   define XWORD_PREFERENCES_SHRINK 1
#else
#   define XWORD_PREFERENCES_LIVE_PREVIEW 1
#   define XWORD_PREFERENCES_SHRINK 0
#endif

class PreferencesDialog : public wxPropertySheetDialog
{
public:
    PreferencesDialog(wxWindow * parent);
    ~PreferencesDialog();
    ConfigManager & GetConfig();

protected:
#ifdef __WXOSX__
    virtual void DoMoveWindow(int x, int y, int width, int height);
#endif
    // Load the window / config states
    void LoadConfig();
    void SaveConfig();
    void ResetConfig();

    // Event Handlers
    void OnOK(wxCommandEvent & evt);
    void OnApply(wxCommandEvent & evt);
    void OnCancel(wxCommandEvent & evt);
    void OnClose(wxCloseEvent & evt) { Destroy(); }

    ConfigManager * m_config;
    ConfigManager m_oldConfig;
};

#endif // PREFERENCES_DLG_H
