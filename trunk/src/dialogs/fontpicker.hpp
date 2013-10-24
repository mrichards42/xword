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

// A Fontn Picker Panel

#include "wxFB_TreePanels.h"
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

class FontPickerPanel : public wxFB_FontPickerPanel
{
public:
    FontPickerPanel(wxWindow * parent, wxWindowID id = wxID_ANY,
              const wxFont & font = wxNullFont, long style = FP_DEFAULT)
        : wxFB_FontPickerPanel(parent, id)
    {
        // Hide unneeded ctrls
        wxASSERT(style != 0);
        if ((style & FP_FACENAME) == 0)
            m_sizer->Hide(m_facename, true);
        if ((style & FP_POINTSIZE) == 0)
        {
            // If the pointsize ctrl is hidden, place the whole picker on
            // one line
            m_sizer->Hide(m_pointsize, true);
            m_sizer->Detach(m_top);
            m_sizer->Detach(m_bottom);
            m_sizer->SetOrientation(wxHORIZONTAL);
            m_sizer->Add(m_top);
            m_sizer->Add(m_bottom);
        }
        if ((style & FP_BOLD) == 0)
            m_sizer->Hide(m_bold, true);
        if ((style & FP_ITALIC) == 0)
            m_sizer->Hide(m_italic, true);
        if ((style & FP_UNDERLINE) == 0)
            m_sizer->Hide(m_underline, true);

        // Make the buttons square
        int btnSize = m_facename->GetSize().y;
        m_sizer->GetItem(m_bold, true)->SetInitSize(btnSize, btnSize);
        m_sizer->GetItem(m_italic, true)->SetInitSize(btnSize, btnSize);
        m_sizer->GetItem(m_underline, true)->SetInitSize(btnSize, btnSize);

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
};


#endif // FONT_PICKER_H
