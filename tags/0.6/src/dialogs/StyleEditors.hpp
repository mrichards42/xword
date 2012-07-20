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

#ifndef STYLE_EDITORS_H
#define STYLE_EDITORS_H

// The style editors for our StylePanel.

#include "wxFB_StylePanels.h"

//------------------------------------------------------------------------------
// Font Panels
//------------------------------------------------------------------------------

#include <wx/fontenum.h>

enum FontPanelStyles
{
    FP_FACENAME  = 1,
    FP_POINTSIZE = 1 << 1,
    FP_BOLD      = 1 << 2,
    FP_ITALIC    = 1 << 3,
    FP_UNDERLINE = 1 << 4,

    FP_STYLE = FP_BOLD | FP_ITALIC | FP_UNDERLINE,

    FP_DEFAULT = FP_FACENAME | FP_POINTSIZE | FP_STYLE
};

class FontPanel : public FontPanelBase
{
public:
    FontPanel(wxWindow * parent, wxWindowID id = wxID_ANY,
              const wxFont & font = wxNullFont, long style = FP_DEFAULT)
        : FontPanelBase(parent, id)
    {
        wxSizer * sizer = GetSizer();

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

        // Make the buttons square
        int btnSize = m_facename->GetSize().y;
        m_styles->GetItem(m_bold)->SetInitSize(btnSize, btnSize);
        m_styles->GetItem(m_italic)->SetInitSize(btnSize, btnSize);
        m_styles->GetItem(m_underline)->SetInitSize(btnSize, btnSize);

        // If we have styles, wrap the style sizer
        // when necessary
        if (style & FP_STYLE)
            Connect(wxEVT_SIZE, wxSizeEventHandler(FontPanel::OnSize));

        Layout();

        if (font.IsOk())
            SetSelectedFont(font);
        else
            SetSelectedFont(*wxSWISS_FONT);
    }

    wxFont GetSelectedFont()
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
    // Put the style parts on a second line if necessary.
    void OnSize(wxSizeEvent & evt)
    {
        // Figure out how much extra space we have
        wxSizer * container = GetContainingSizer();
        wxSizerItem * item = container->GetItem(this);
        const int maxWidth = container->GetSize().x - item->GetPosition().x;
        const int width = item->GetSize().x;

        // Check to see where the style sizer is now
        wxSizer * mainSizer = GetSizer();
        wxSizer * verticalSizer = m_facename->GetContainingSizer();
        if (verticalSizer->GetItem(m_styles) != NULL) // Two lines
        {
            // Can we put this on one line?
            if (maxWidth - width > m_styles->GetSize().x)
            {
                verticalSizer->Detach(m_styles);
                mainSizer->Add(m_styles);
                container->Layout();
            }
        }
        else // One line
        {
            // Must we put this one two lines?
            if (width > maxWidth)
            {
                mainSizer->Detach(m_styles);
                verticalSizer->Add(m_styles);
                container->Layout();
            }
        }
        evt.Skip();
    }
};


#endif // STYLE_EDITORS_H
