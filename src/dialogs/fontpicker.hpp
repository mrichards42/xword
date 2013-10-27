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

#ifndef FONT_PICKER_H
#define FONT_PICKER_H

// A Font Picker Panel

#include <wx/panel.h>
#include "fontface.hpp"
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>
#include <wx/fontpicker.h> // wxFontPickerEvent

//------------------------------------------------------------------------------
// Font Panels
//------------------------------------------------------------------------------

#include <wx/fontenum.h>

enum FontPickerPanelStyles
{
    FP_FACENAME  = 1,
    FP_POINTSIZE = 1 << 1,
    FP_BOLD      = 1 << 2,
    FP_ITALIC    = 1 << 3,
    FP_UNDERLINE = 1 << 4,

    FP_STYLE = FP_BOLD | FP_ITALIC | FP_UNDERLINE,

    FP_DEFAULT = FP_FACENAME | FP_POINTSIZE | FP_STYLE
};

class FontPickerPanel : public wxPanel
{
public:
    FontPickerPanel(wxWindow * parent, wxWindowID id = wxID_ANY,
              const wxFont & font = wxNullFont, long style = FP_DEFAULT)
        : wxPanel(parent, id)
    {
        // Create controls
        // Place on two lines if all controls are present
        wxBoxSizer * sizer = new wxBoxSizer((style & FP_POINTSIZE) ? wxVERTICAL : wxHORIZONTAL);
        wxBoxSizer * top = new wxBoxSizer(wxHORIZONTAL);
        
        m_facename = new FontFaceCtrl(this, wxID_ANY);
        top->Add(m_facename, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
        
        m_pointsize = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, -1), wxSP_ARROW_KEYS, 5, 100, 5);
        top->Add(m_pointsize, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 2);
        
        sizer->Add(top, 0, wxEXPAND | wxRIGHT, 2);
        
        wxBoxSizer * bottom = new wxBoxSizer(wxHORIZONTAL);

        wxSize buttonSize(m_pointsize->GetSize().y, m_pointsize->GetSize().y);
    #ifdef __WXOSX__
        int buttonStyle = wxBORDER_SIMPLE;
        int fontSize = -1;
    #else
        int buttonStyle = 0;
        int fontSize = wxNORMAL_FONT->GetPointSize();
    #endif // __WXOSX__
        m_bold = new wxToggleButton(this, wxID_ANY, wxT("B"), wxDefaultPosition, buttonSize, buttonStyle);
        m_bold->SetFont(wxFont(wxFontInfo(fontSize).Family(wxFONTFAMILY_ROMAN).Bold()));
        bottom->Add(m_bold, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
        
        m_italic = new wxToggleButton(this, wxID_ANY, wxT("I"), wxDefaultPosition, buttonSize, buttonStyle);
        m_italic->SetFont(wxFont(wxFontInfo(fontSize).Family(wxFONTFAMILY_ROMAN).Italic()));
        bottom->Add(m_italic, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
        
        m_underline = new wxToggleButton(this, wxID_ANY, wxT("U"), wxDefaultPosition, buttonSize, buttonStyle);
        m_underline->SetFont(wxFont(wxFontInfo(fontSize).Family(wxFONTFAMILY_ROMAN).Underlined()));
        bottom->Add(m_underline, 0, wxALIGN_CENTER_VERTICAL);
        
        sizer->Add(bottom);
        SetSizer(sizer);

        // Hide unneeded ctrls
        wxASSERT(style != 0);
        if ((style & FP_FACENAME) == 0)
            sizer->Hide(m_facename, true);
        if ((style & FP_POINTSIZE) == 0)
            sizer->Hide(m_pointsize, true);
        if ((style & FP_BOLD) == 0)
            sizer->Hide(m_bold, true);
        if ((style & FP_ITALIC) == 0)
            sizer->Hide(m_italic, true);
        if ((style & FP_UNDERLINE) == 0)
            sizer->Hide(m_underline, true);

        // Connect changed events
        m_facename->Bind(wxEVT_COMBOBOX, &FontPickerPanel::OnChanged, this);
        m_pointsize->Bind(wxEVT_SPINCTRL, &FontPickerPanel::OnChanged, this);
        m_bold->Bind(wxEVT_TOGGLEBUTTON, &FontPickerPanel::OnChanged, this);
        m_italic->Bind(wxEVT_TOGGLEBUTTON, &FontPickerPanel::OnChanged, this);
        m_underline->Bind(wxEVT_TOGGLEBUTTON, &FontPickerPanel::OnChanged, this);

        Layout();

        if (font.IsOk())
            SetSelectedFont(font);
        else
            SetSelectedFont(*wxSWISS_FONT);
    }

    wxFont GetSelectedFont() const
    {
        return wxFont(
            m_pointsize->GetValue(), wxFONTFAMILY_DEFAULT,
            m_italic->GetValue() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
            m_bold->GetValue() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
            m_underline->GetValue(),
            m_facename->GetValue());
    }

    void SetSelectedFont(const wxFont & font)
    {
        // Copy the data from the font
        m_facename->SetValue(font.GetFaceName());
        m_pointsize->SetValue(font.GetPointSize());
        m_bold->SetValue(font.GetWeight() == wxFONTWEIGHT_BOLD);
        m_italic->SetValue(font.GetStyle() == wxFONTSTYLE_ITALIC);
        m_underline->SetValue(font.GetUnderlined());
    }

    void SetOrientation(int orient)
    {
        wxBoxSizer * sizer = wxDynamicCast(GetSizer(), wxBoxSizer);
        if (sizer->GetOrientation() == orient)
            return;
        wxSizer * top = sizer->GetItem((size_t)0)->GetSizer();
        wxSizer * bottom = sizer->GetItem((size_t)1)->GetSizer();
        sizer->Detach(top);
        sizer->Detach(bottom);
        sizer->SetOrientation(orient);
        sizer->Add(top, 0, wxEXPAND | wxRIGHT, 2);
        sizer->Add(bottom);
    }

protected:
    // This event is called when any of the elements have been changed
    void OnChanged(wxCommandEvent & evt)
    {
        evt.Skip();
        SendEvent();
    }

    void SendEvent()
    {
        wxFontPickerEvent evt(this, GetId(), GetSelectedFont());
        GetEventHandler()->ProcessEvent(evt);
    }

    FontFaceCtrl * m_facename;
    wxSpinCtrl * m_pointsize;
    wxToggleButton * m_bold;
    wxToggleButton * m_italic;
    wxToggleButton * m_underline;
};


#endif // FONT_PICKER_H
