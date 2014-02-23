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

// HSP color space: http://alienryderflex.com/hsp.html

#include <wx/colour.h>
#include <cmath>

// Compute a word highlight color from a letter highlight color
extern wxColour GetWordHighlight(const wxColour & letter);

// Compute the theme square color from the letter highlight color
extern wxColour GetThemeColor(const wxColour & letter);

// Return percieved brightness from 0 to 255
static unsigned char GetBrightness(const wxColour & color)
{
    const unsigned char r = color.Red(),
                        g = color.Green(),
                        b = color.Blue();
    return static_cast<unsigned char>(sqrt(r*r*.299 + g*g*.587 + b*b*.114));
}

// Hue / Chroma / Luma (Percieved brightness)
// Hue: 0 - 360
// Chroma: 0 - 1
// Luma: 0 - 1
class HCLColor
{
public:
    HCLColor(double h_, double c_, double l_)
        : h(h_), c(c_), l(l_)
    {}

    HCLColor(const wxColour & color);

    void ToRGB(unsigned char * red, unsigned char * green, unsigned char * blue) const;
    wxColour ToColor() const
    {
        unsigned char r, g, b;
        ToRGB(&r, &g, &b);
        return wxColour(r,g,b);
    }

    void SetHue(double h_) { h = fmod(h_ + 360, 360); }
    void SetChroma(double c_) { c = std::max(std::min(c_, 1.), 0.); }
    void SetLuma(double l_) { l = std::max(std::min(l_, 1.), 0.); }

    double Hue() const { return h; }
    double Chroma() const { return c; }
    double Luma() const { return l; }

    bool operator<(const HCLColor & b) const;

protected:
    double h, c, l;
};