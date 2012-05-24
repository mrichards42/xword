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

#include "../widgets/vcombo.hpp"

class FontEnumeratorThread;

class FontFaceCtrl : public VirtualComboBox
{
    friend class FontEnumeratorThread;
public:
    FontFaceCtrl(wxWindow * parent, wxWindowID id = wxID_ANY,
                 const wxString & faceName = wxEmptyString);

    virtual void SetValue(const wxString & value);

    static void InitFaceNames();
    static void ClearFaceNames();
protected:
    virtual int FindItem(const wxString & s) const;
    virtual wxString GetItem(size_t n) const;
    virtual size_t GetCount() const;

    wxString m_lastFaceName;

    static wxArrayString s_facenames;
    static bool s_facenamesComplete;
    static FontEnumeratorThread * s_thread;

    // Initializing the facenames array takes a lot of time, so we're
    // going to do it on idle.
    void OnIdle(wxIdleEvent & evt);
    
    void OnKillFocus(wxFocusEvent & evt)
        { SetValue(GetValue()); evt.Skip(); }
};

#endif // FONT_FACE_H
