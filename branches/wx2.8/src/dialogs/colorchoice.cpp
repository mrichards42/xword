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
#include "../App.hpp"
#include <algorithm>
#include <set>

// A marker string to identify wxNullColour
static const wxChar * custom_color = _T("Custom...");

// ----------------------------------------------------------------------------
// The ComboCtrl class
// ----------------------------------------------------------------------------

ColorChoice::ColorChoice(wxWindow * parent, wxWindowID id,
                         const wxColour & color)
    : VirtualComboBox(parent, id, wxEmptyString, wxDefaultPosition,
                      wxDefaultSize, wxCB_READONLY)
{
    // Make sure we have a list of colors
    if (s_colors.empty())
        InitColors();

    // Set the first value
    if (! color.IsOk())
        SetColor(*wxWHITE);
    else
        SetColor(color);
    // Connect the "Custom..." event handler
    Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
            wxCommandEventHandler(ColorChoice::OnListSelected));
}


// The color vector
//-----------------

ColorChoice::ColorArray ColorChoice::s_colors;

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
    s_colors.push_back(wxNullColour, custom_color);

    // Add colors from config
    if (cfg)
    {
        std::set<wxString> colors;
        AddColors(colors, &cfg->GetGroup());
        std::set<wxString>::iterator it;
        for (it = colors.begin(); it != colors.end(); ++it)
            s_colors.push_back(wxColour(*it));
    }
}

void ColorChoice::ClearColors()
{
    s_colors.clear();
}


void ColorChoice::SetValue(const wxString & value)
{
    wxColour color(value);
    if (color.IsOk())
        m_lastColor = color;
    // Add it to the list if we don't already have it.
    for (size_t i = 0; i < s_colors.size(); ++i)
    {
        if (s_colors[i].color == color)
        {
            VirtualComboBox::SetValue(value);
            return;
        }
    }
    // If we didn't find this color, add it to the list
    s_colors.push_back(color);
    VirtualComboBox::SetValue(value);
}


// Find items
//-----------
int ColorChoice::FindItem(const wxString & s) const
{
    if (s == custom_color)
        return 0;
    wxColour color(s);
    for (size_t i = 0; i < s_colors.size(); ++i)
    {
        if (color == s_colors[i].color)
            return i;
    }
    return wxNOT_FOUND;
}

wxString ColorChoice::GetItem(size_t n) const
{
    if (n == 0)
        return custom_color;
    return s_colors.at(n).color.GetAsString();
}

size_t ColorChoice::GetCount() const
{
    return s_colors.size();
}

wxString ColorChoice::GetLabel(const wxColour & color) const
{
    for (size_t i = 0; i < s_colors.size(); ++i)
    {
        if (color == s_colors[i].color)
            return s_colors[i].label;
    }
    return wxEmptyString;
}


// Drawing
//--------

void ColorChoice::Draw(wxDC& dc, const wxRect& rect,
                       const wxColour & color, const wxString & label) const
{
    wxRect smallRect(rect);
    smallRect.Deflate(s_popupBorder);
    // Make a box for the color
    int h = smallRect.GetHeight();
    wxRect colorRect = wxRect(smallRect.GetTopLeft(), wxSize(h * 1.5, h));
    smallRect.Offset(colorRect.width + s_popupBorder, 0);
    dc.SetBrush(wxBrush(color, wxSOLID));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(colorRect);
    dc.DrawLabel(label, smallRect, wxALIGN_CENTER_VERTICAL);
}

void ColorChoice::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    wxRect smallRect = rect;
    smallRect.Deflate(s_popupBorder);
    if (! s_colors[n].color.IsOk())
        dc.DrawLabel(custom_color, smallRect, wxALIGN_CENTER_VERTICAL);
    else
        Draw(dc, rect, s_colors[n].color, s_colors[n].label);
}

void ColorChoice::OnDrawCtrl(wxDC & dc, const wxRect & rect)
{
    wxColour color = GetColor();
    Draw(dc, rect, color, GetLabel(color));
}

// Events
//-------
#include <wx/colordlg.h>

void ColorChoice::OnListSelected(wxCommandEvent & evt)
{
    if (evt.GetInt() == 0) // Select custom color
    {
        wxColor color = wxGetColourFromUser(this, m_lastColor);
        if (color.IsOk())
            SetColor(color);
        else
            SetColor(m_lastColor);
    }
    else
        evt.Skip();
}