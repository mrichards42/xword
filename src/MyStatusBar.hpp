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


#ifndef MY_STATUS_BAR_H
#define MY_STATUS_BAR_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

class SizedText;

class MyStatusBar
    : public wxStatusBar
{
public:
    MyStatusBar(wxWindow * parent, wxWindowID id = wxID_ANY, long style = wxST_SIZEGRIP, const wxString& name = _T("statusBar"));
    ~MyStatusBar() {}

    void SetAlert(const wxString & text, const wxColour & bgColor = wxNullColour);
    void SetStatus(const wxString & text);
#ifdef XWORD_USE_LUA
    void SetLuaErrors(int num);
#endif
    void SetTime(int time, const wxString & msg = wxEmptyString);

private:
    void OnSize(wxSizeEvent & WXUNUSED(evt));

    SizedText * m_alert;
    wxStaticText * m_timer;
    wxStaticText * m_status;
#ifdef XWORD_USE_LUA
    wxStaticText * m_luaErrors;
#endif
    void OnAlertClick(wxMouseEvent & evt);
    void WrapAlert(int width = -1);

    void OnDoubleClick(wxMouseEvent & evt);

    DECLARE_EVENT_TABLE()
};

#endif // MY_STATUS_BAR_H
