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

#ifndef COLOR_CHOICE_H
#define COLOR_CHOICE_H

// A combo box that lists colors.

#include "../widgets/vcombo.hpp"
#include <wx/colour.h>
#include <vector>

class ConfigManager;

class ColorChoice : public VirtualComboBox
{
public:
    ColorChoice(wxWindow * parent, wxWindowID id = wxID_ANY,
                const wxColour & color = wxNullColour);

    wxColour GetColor() const { return wxColour(GetValue()); }
    void SetColor(const wxColour & color) { SetValue(color.GetAsString()); }
    void SetValue(const wxString & value);

    static void InitColors(ConfigManager * cfg = NULL);
    static void ClearColors();
protected:
    virtual int FindItem(const wxString & s) const;
    virtual wxString GetItem(size_t n) const;
    virtual size_t GetCount() const;
    wxString GetLabel(const wxColour & color) const;

    void Draw(wxDC & dc, const wxRect & rect, const wxColour & color,
              const wxString & label) const;
    void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
    void OnDrawCtrl(wxDC & dc, const wxRect & rect);

    wxColour m_lastColor; // The last valid color
    void OnListSelected(wxCommandEvent & evt);

    // A color and label array
    struct colorlabel_t { wxColour color; wxString label; };
    class ColorArray : public std::vector<colorlabel_t>
    {
    public:
        void push_back(const wxColour & color, const wxString & label = _T(""))
        {
            colorlabel_t c;
            c.color = color;
            c.label = label.empty() ? color.GetAsString() : label;
            std::vector<colorlabel_t>::push_back(c);
        }
    };
    static ColorArray s_colors;
};

#endif // COLOR_CHOICE_H
