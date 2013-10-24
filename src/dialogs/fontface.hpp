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

#ifndef FONT_FACE_H
#define FONT_FACE_H

// A combo box that lists all of the fonts installed on the system.

#include <wx/combobox.h>

class FontEnumeratorThread;

class FontFaceCtrl : public wxComboBox
{
    friend class FontEnumeratorThread;
public:
    FontFaceCtrl(wxWindow * parent, wxWindowID id = wxID_ANY,
                 const wxString & faceName = wxEmptyString);

    void SetValue(const wxString & value);

    static void InitFacenames();
    static void ClearFacenames();
protected:
    wxString m_lastFaceName;

    // Initializing the facenames array takes some time, so we're going
    // to do it in a thread
    static wxArrayString s_facenames;
    static bool s_threadComplete;
    static FontEnumeratorThread * s_thread;

    void OnSetFocus(wxFocusEvent & evt);
    void OnCharHook(wxKeyEvent & evt);
    void OnKillFocus(wxFocusEvent & evt) { SetValue(GetValue()); evt.Skip(); }
};

#endif // FONT_FACE_H
