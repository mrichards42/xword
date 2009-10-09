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

    void SetAlert(const wxString & text, const wxColour & fgColor = *wxBLACK, const wxColour & bgColor = wxNullColour);
    void SetStatus(const wxString & text) { SetStatusText(text, 0); }
    void SetTime(int time) { SetStatusText( wxTimeSpan::Seconds(time).Format(), 2); }

private:
    void OnSize(wxSizeEvent & WXUNUSED(evt));
    void OnAlertClick(wxMouseEvent & evt);

    void WrapAlert(int width = -1);

    SizedText * m_alert;

    DECLARE_EVENT_TABLE()
};

#endif // MY_STATUS_BAR_H