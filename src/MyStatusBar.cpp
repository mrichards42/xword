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


#include "MyStatusBar.hpp"
#include "utils/wrap.hpp"
#include <wx/tooltip.h>
#include "widgets/SizedText.hpp"

BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
    EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()

MyStatusBar::MyStatusBar(wxWindow * parent,
                         wxWindowID id,
                         long style,
                         const wxString & name)
    : wxStatusBar(parent, id, style, name)
{
    int widths[] = {-2, -1, 200};
    SetFieldsCount(3, widths);
    m_alert = new SizedText(this, wxID_ANY);
    m_alert->SetAlign(wxALIGN_CENTER);
    m_alert->SetWrapMode(ST_TRUNCATE);
    m_alert->SetFont(GetFont());
    m_alert->SetToolTip(_T(""));

    m_alert->Connect(wxEVT_LEFT_DCLICK,
                 wxMouseEventHandler(MyStatusBar::OnAlertClick),
                 NULL, this);
}

void
MyStatusBar::OnSize(wxSizeEvent & WXUNUSED(evt))
{
    wxRect rect;
    if (GetFieldRect(1, rect)) {
        m_alert->SetSize(rect);
        //WrapAlert(rect.width);
    }
}


void
MyStatusBar::SetAlert(const wxString & text, const wxColour & fgColor, const wxColour & bgColor)
{
    m_alert->SetBackgroundColour(bgColor);
    m_alert->SetForegroundColour(fgColor);
    m_alert->SetToolTip(text);
    m_alert->SetLabel(text);
    //WrapAlert();
}

void
MyStatusBar::OnAlertClick(wxMouseEvent & evt)
{
    wxWindow * alert = wxDynamicCast(evt.GetEventObject(), wxWindow);
    if (alert)
        wxMessageBox(alert->GetToolTip()->GetTip());
}

void
MyStatusBar::WrapAlert(int width)
{
    if (width == -1) {
        wxRect rect;
        GetFieldRect(1, rect);
        width = rect.width;
    }

    wxString wrapped = Wrap(m_alert, m_alert->GetToolTip()->GetTip(), width, NULL);
    m_alert->SetLabel(wrapped.BeforeFirst(_T('\n')));
}