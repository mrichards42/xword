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
#include <wx/colour.h>
#include <vector>

class ConfigManager;

#ifdef __WXOSX__
#include <wx/choice.h>
class ColorChoice : public wxChoice
#else
#include <wx/odcombo.h>
class ColorChoice : public wxOwnerDrawnComboBox
#endif
{
public:
    ColorChoice(wxWindow * parent, wxWindowID id = wxID_ANY,
                const wxColour & color = wxNullColour);
    ~ColorChoice();

    // Access
#ifdef __WXOSX__
    wxString GetValue() const { return GetStringSelection(); }
#endif
    wxColour GetColor() const { return wxColour(GetValue()); }
    void SetColor(const wxColour & color) { SetValue(color.GetAsString()); }
    void SetValue(const wxString & value);

    // Color syncing
    static void InitColors(ConfigManager * cfg = NULL);
    static void ClearColors();

    struct colorlabel_t { wxColour color; wxString label; };

protected:
#ifndef __WXOSX__
    // Drawing
    wxCoord OnMeasureItem(size_t n) const;
    void Draw(wxDC & dc, const wxRect & rect, const wxColour & color,
              const wxString & label) const;
    void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
#else
    void OSXUpdateAttributedStrings();
#endif //__WXOSX__

    wxColour m_lastColor; // The last valid color
    void OnSelection(wxCommandEvent & evt);

    // A static vector of all colors currently in use
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

    // A static vector of ColorChoices for syncing colors
    static void UpdateCtrls();
    static std::vector<ColorChoice *> s_ctrls;
};

#else  // __WXOSX__

#include <wx/choice.h>

class ColorChoice : public wxChoice
{
public:
    ColorChoice(wxWindow * parent, wxWindowID id = wxID_ANY,
                const wxColour & color = wxNullColour);
    ~ColorChoice();

    wxColour GetColor() const { return wxColour(GetStringSelection()); }
    void SetColor(const wxColour & color) { SetValue(color.GetAsString()); }
    void SetValue(const wxString & value);
    
    static void InitColors(ConfigManager * cfg = NULL);
    static void ClearColors();
protected:
    
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
    static void UpdateCtrls();
    static ColorArray s_colors;
    static std::vector<ColorChoice *> s_ctrls;
};

#endif // COLOR_CHOICE_H
