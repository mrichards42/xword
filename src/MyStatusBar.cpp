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


#include "App.hpp"
#include "MyStatusBar.hpp"
#include "utils/wrap.hpp"
#include <wx/tooltip.h>
#include "widgets/SizedText.hpp"
#include "messages.hpp"
#include "utils/color.hpp"

enum StatusIds
{
    STATUS_FIRST = 0,
    STATUS_GENERAL = STATUS_FIRST,
#ifdef XWORD_USE_LUA
    STATUS_LUA,
#endif
    STATUS_ALERT,
    STATUS_TIME,
    STATUS_LAST,

    STATUS_TOTAL = STATUS_LAST - STATUS_FIRST
};

BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
    EVT_SIZE        (        MyStatusBar::OnSize )
    EVT_LEFT_DCLICK ( MyStatusBar::OnDoubleClick )
END_EVENT_TABLE()

MyStatusBar::MyStatusBar(wxWindow * parent,
                         wxWindowID id,
                         long style,
                         const wxString & name)
    : wxStatusBar(parent, id, style, name)
{
#ifdef XWORD_USE_LUA
    int widths[] = {-2, 150, 200, 110};
#else
    int widths[] = {-2, 200, 110};
#endif // XWORD_USE_LUA

    SetFieldsCount(STATUS_TOTAL, widths);

    // We use wxStaticText for status and Lua errors even though we could just use SetStatusText in
    // order to ensure a consistent look with m_timer, which needs wxStaticText to be right-aligned.
    // The default text may have additional styling (e.g. a different color when inactive) which is
    // hard to emulate here without brittly copying per-platform private logic.
    m_status = new wxStaticText(this,
                                wxID_ANY,
                                wxEmptyString,
                                wxDefaultPosition,
                                wxDefaultSize,
                                wxST_NO_AUTORESIZE);
    m_status->SetFont(GetFont());
    m_status->SetToolTip("");

    m_alert = new SizedText(this, wxID_ANY);
    m_alert->SetAlign(wxALIGN_CENTER);
    m_alert->SetWrapMode(ST_TRUNCATE);
    m_alert->SetFont(GetFont());
    m_alert->SetToolTip(_T(""));

    m_alert->Connect(wxEVT_LEFT_DCLICK,
                 wxMouseEventHandler(MyStatusBar::OnAlertClick),
                 NULL, this);

#ifdef XWORD_USE_LUA
    m_luaErrors = new wxStaticText(this,
                                   wxID_ANY,
                                   wxEmptyString,
                                   wxDefaultPosition,
                                   wxDefaultSize,
                                   wxST_NO_AUTORESIZE);
    m_luaErrors->SetFont(GetFont());
    m_luaErrors->SetToolTip("");
#endif

    m_timer = new wxStaticText(this,
                               wxID_ANY,
                               wxEmptyString,
                               wxDefaultPosition,
                               wxDefaultSize,
                               wxST_NO_AUTORESIZE | wxALIGN_RIGHT);
    m_timer->SetFont(GetFont());
    m_timer->SetToolTip("");
}

void MyStatusBar::SetStatus(const wxString & text)
{
    m_status->SetLabel(text);
}

#ifdef XWORD_USE_LUA
void MyStatusBar::SetLuaErrors(int num)
{
    if (num < 1)
        m_luaErrors->SetLabel("");
    else if (num == 1)
        m_luaErrors->SetLabel("1 Error");
    else
        m_luaErrors->SetLabel(wxString::Format(("%d Errors"), num));
}
#endif // XWORD_USE_LUA


void MyStatusBar::SetAlert(const wxString & text, const wxColour & bgColor)
{
    m_alert->SetBackgroundColour(bgColor);
    long cutoff = wxGetApp().GetConfigManager().Status.brightnessCutoff();
    if (bgColor == wxNullColour || GetBrightness(bgColor) > cutoff)
        m_alert->SetForegroundColour(*wxBLACK);
    else
        m_alert->SetForegroundColour(*wxWHITE);
    m_alert->SetToolTip(text);
    m_alert->SetLabel(text);
    //WrapAlert();
}


void MyStatusBar::SetTime(int time, const wxString & msg)
{
    m_timer->SetLabel(wxTimeSpan::Seconds(time).Format() + msg);
}




void
MyStatusBar::OnSize(wxSizeEvent & WXUNUSED(evt))
{
    wxRect rect;
    if (GetFieldRect(STATUS_ALERT, rect)) {
        m_alert->SetSize(rect);
        //WrapAlert(rect.width);
    }
    // Center wxStaticTexts vertically within the field rectangle, since wxStaticText doesn't
    // support vertical alignment and the status bar might be taller (e.g. on Mac).
    if (GetFieldRect(STATUS_GENERAL, rect)) {
        rect.Offset(0, (rect.height - m_status->GetBestHeight(rect.width)) / 2);
        m_status->SetSize(rect);
    }
#ifdef XWORD_USE_LUA
    if (GetFieldRect(STATUS_LUA, rect)) {
        rect.Offset(0, (rect.height - m_luaErrors->GetBestHeight(rect.width)) / 2);
        m_luaErrors->SetSize(rect);
    }
#endif
    if (GetFieldRect(STATUS_TIME, rect)) {
        rect.Offset(0, (rect.height - m_timer->GetBestHeight(rect.width)) / 2);
        m_timer->SetSize(rect);
    }
}


void
MyStatusBar::OnAlertClick(wxMouseEvent & evt)
{
    wxWindow * alert = wxDynamicCast(evt.GetEventObject(), wxWindow);
    if (alert)
        XWordMessage(this, alert->GetToolTip()->GetTip());
}

void
MyStatusBar::WrapAlert(int width)
{
    if (width == -1) {
        wxRect rect;
        GetFieldRect(STATUS_ALERT, rect);
        width = rect.width;
    }

    wxString wrapped = Wrap(m_alert, m_alert->GetToolTip()->GetTip(), width, NULL);
    m_alert->SetLabel(wrapped.BeforeFirst(_T('\n')));
}


#ifdef XWORD_USE_LUA
#include "paths.hpp"

class LuaErrorsDlg : public wxDialog
{
public:
    LuaErrorsDlg(wxWindow * parent, const wxString & filename)
        : wxDialog(parent, wxID_ANY, _T("Errors"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    {
        m_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_READONLY | wxTE_MULTILINE);
        m_text->LoadFile(filename);

        wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_text, 1, wxALL | wxEXPAND, 10);
        sizer->Add(CreateButtonSizer(wxOK), 0, wxALL |wxEXPAND, 10);
        SetSizer(sizer);
    }

private:
    wxTextCtrl * m_text;
};

#endif // XWORD_USE_LUA

void MyStatusBar::OnDoubleClick(wxMouseEvent & evt)
{
#ifdef XWORD_USE_LUA
    wxRect rect;
    if (! GetFieldRect(STATUS_LUA, rect))
        return;

    if (rect.Contains(evt.GetPosition()) && wxGetApp().HasLuaLog())
    {
        wxWindow * errors = wxDynamicCast(evt.GetEventObject(), wxWindow);
        if (errors)
        {
            LuaErrorsDlg dlg(this, GetLuaLogFilename());
            dlg.ShowModal();
            // Reset messages if the user has already looked at them
            wxGetApp().SetLuaMessageCount(0);
        }
    }
#endif // XWORD_USE_LUA
}
