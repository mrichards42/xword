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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/dc.h>
#endif

#include "colorchoice.hpp"
#include "../utils/color.hpp"
#include "../App.hpp"
#include <algorithm>
#include <set>

// A marker string to identify wxNullColour
static const wxChar * other_color = _T("Other...");

// ----------------------------------------------------------------------------
// The ComboCtrl class
// ----------------------------------------------------------------------------

ColorChoice::ColorChoice(wxWindow * parent, wxWindowID id,
                         const wxColour & color)
#ifdef __WXOSX__
    : wxChoice(parent, id)
#else
    : wxOwnerDrawnComboBox(parent, id, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxArrayString(), wxCB_READONLY)
#endif
{
    // If the ctrl is too small the color box gets messed up in wxOSX
    SetMinSize(wxSize(GetCharWidth() * 22, -1));
    s_ctrls.push_back(this);
    // Make sure we have a list of colors
    if (s_colors.empty())
        InitColors();
    else
        UpdateCtrls();
    // Set the initial value
    SetColor(color.IsOk() ? color : *wxWHITE);
    // Connect the event handler
#ifndef __WXOSX__
    Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
#else
    Connect(wxEVT_COMMAND_CHOICE_SELECTED,
#endif
            wxCommandEventHandler(ColorChoice::OnSelection));
}

ColorChoice::~ColorChoice()
{
    s_ctrls.erase(std::find(s_ctrls.begin(), s_ctrls.end(), this));
}

// The color vector and color syncing
//-----------------------------------

ColorChoice::ColorArray ColorChoice::s_colors;
std::vector<ColorChoice *> ColorChoice::s_ctrls;

// Helper function to gather all the colors
void AddColors(std::set<wxString> & colors, const ConfigGroup * group)
{
    const ConfigColor * color = dynamic_cast<const ConfigColor *>(group);
    if (color)
        colors.insert(color->Get().GetAsString());
    std::list<ConfigGroup *>::const_iterator it;
    for (it = group->m_children.begin(); it != group->m_children.end(); ++it)
        AddColors(colors, *it);
}

void ColorChoice::InitColors(ConfigManager * cfg)
{
    s_colors.clear();
    // Add the "Other..." item
    s_colors.push_back(wxNullColour, other_color);
    // Add colors from config
    if (cfg)
    {
        std::set<wxString> colors;
        AddColors(colors, &cfg->GetGroup());
        std::set<wxString>::iterator it;
        for (it = colors.begin(); it != colors.end(); ++it)
            s_colors.push_back(wxColour(*it));
    }
    UpdateCtrls();
}

void ColorChoice::ClearColors()
{
    s_colors.clear();
}

// Sort roughly by hue
bool color_sort(ColorChoice::colorlabel_t a, ColorChoice::colorlabel_t b)
{
    // wxNullColour should be last.
    if (!a.color.IsOk()) {
        return false;
    }
    if (!b.color.IsOk()) {
        return true;
    }

    return HCLColor(a.color) < HCLColor(b.color);
}

void ColorChoice::UpdateCtrls()
{
    if (s_ctrls.size() == 0)
        return;
    std::sort(s_colors.begin(), s_colors.end(), color_sort);
    wxArrayString choices;
    ColorChoice::ColorArray::const_iterator it;
    for (it = s_colors.begin(); it != s_colors.end(); ++it)
        choices.push_back(it->label);
    // Set the choices for our managed ctrls
    std::vector<ColorChoice *>::iterator ctrl;
    for (ctrl = s_ctrls.begin(); ctrl != s_ctrls.end(); ++ctrl)
    {
        wxString selection = (*ctrl)->GetValue();
        (*ctrl)->Set(choices);
        (*ctrl)->SetStringSelection(selection);
#ifdef __WXOSX__
        (*ctrl)->OSXUpdateAttributedStrings();
#endif
    }
}


void ColorChoice::SetValue(const wxString & value)
{
    if (value.empty())
        return;
    wxColour color(value);
    if (! color.IsOk())
        return;
    m_lastColor = color;
    // Set the selection
    if (! SetStringSelection(color.GetAsString()))
    {
        // If this color doesn't already exist in the ctrl, add it.
        s_colors.push_back(color);
        UpdateCtrls();
        SetStringSelection(color.GetAsString());
    }
}

#ifndef __WXOSX__

// Drawing
//--------
const int popupPadding = 3;

void ColorChoice::Draw(wxDC& dc, const wxRect& rect,
                       const wxColour & color, const wxString & label) const
{
    wxRect smallRect(rect);
    smallRect.Deflate(popupPadding);
    // Make a box for the color
    int h = smallRect.GetHeight();
    wxRect colorRect = wxRect(smallRect.GetTopLeft(), wxSize(h * 1.5, h));
    smallRect.Offset(colorRect.width + popupPadding, 0);
    dc.SetBrush(wxBrush(color, wxSOLID));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(colorRect);
    dc.DrawLabel(label, smallRect, wxALIGN_CENTER_VERTICAL);
}

void ColorChoice::OnDrawItem(wxDC& dc, const wxRect& rect,
                             int item, int flags) const
{
    if (item == wxNOT_FOUND)
        return;
    wxRect smallRect = rect;
    smallRect.Deflate(popupPadding);
    if (! s_colors[item].color.IsOk())
        dc.DrawLabel(other_color, smallRect, wxALIGN_CENTER_VERTICAL);
    else
        Draw(dc, rect, s_colors[item].color, s_colors[item].label);
}

wxCoord ColorChoice::OnMeasureItem(size_t n) const
{
    return GetCharHeight() + popupPadding * 2;
}

#endif //! __WXOSX__

// Events
//-------
#include <wx/colordlg.h>

void ColorChoice::OnSelection(wxCommandEvent & evt)
{
    wxString selection = GetValue();
    if (selection == other_color) // Select custom color
    {
        wxColor color = wxGetColourFromUser(this, m_lastColor);
        if (color.IsOk())
            SetColor(color);
        else
            SetColor(m_lastColor);
    }
    else
    {
        m_lastColor = wxColour(selection);
    }
    SendEvent();
}
