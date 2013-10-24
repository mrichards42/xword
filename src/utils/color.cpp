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

#include "color.hpp"
#include <cmath>

// Helper functions
double get_luma(double r, double g, double b)
{
    //return sqrt(r*r*.299 + g*g*.587 + b*b*.114);
    return r * .299 + g * .587 + b * .114;
}


// Convert from RGB
HCLColor::HCLColor(const wxColour & color)
{
    const double red = color.Red() / 255.0,
                 green = color.Green() / 255.0,
                 blue = color.Blue() / 255.0;

    // find the min and max intensity (and remember the max for later)
    double minimumRGB = red;
    if ( green < minimumRGB )
        minimumRGB = green;
    if ( blue < minimumRGB )
        minimumRGB = blue;

    enum { RED, GREEN, BLUE } chMax = RED;
    double maximumRGB = red;
    if ( green > maximumRGB )
    {
        chMax = GREEN;
        maximumRGB = green;
    }
    if ( blue > maximumRGB )
    {
        chMax = BLUE;
        maximumRGB = blue;
    }

    // Chroma
    c = maximumRGB - minimumRGB;

    // Hue
    if (c == 0) // Gray (no hue)
        h = 0;
    else
    {
        switch (chMax)
        {
            case RED:   h = (green - blue) / c;     break;
            case GREEN: h = (blue - red)   / c + 2; break;
            case BLUE:  h = (red - green)  / c + 4; break;
        }
        h *= 60;
    }
    if (h < 0)
        h += 360;

    // Luma
    l = get_luma(red, green, blue);
}


// Convert to RGB
void HCLColor::ToRGB(unsigned char * red, unsigned char * green, unsigned char * blue) const
{
    if (c == 0)
    {
        *red = *green = *blue = std::min(l * 255, 255.);
        return;
    }
    double r, g, b;
    double h1 = h / 60.;
    double x = c * (1 - abs(fmod(h1, 2) - 1));
    switch (int(h1))
    {
        case 0: r = c; g = x; b = 0; break;
        case 1: r = x; g = c; b = 0; break;
        case 2: r = 0; g = c; b = x; break;
        case 3: r = 0; g = x; b = c; break;
        case 4: r = x; g = 0; b = c; break;
        case 5: r = c; g = 0; b = x; break;
    }
    // Adjust based on luma
    double m = l - get_luma(r, g, b);
    *red = std::max(std::min((r + m) * 255, 255.), 0.);
    *green = std::max(std::min((g + m) * 255, 255.), 0.);
    *blue = std::max(std::min((b + m) * 255, 255.), 0.);
}


// Sorting
bool HCLColor::operator<(const HCLColor & b) const
{
    if (h == b.h)
    {
        if (c == b.c)
            return l < b.l;
        return c < b.c;
    }
    return h < b.h;
}


#include <wx/image.h>
wxColour GetWordHighlight(const wxColour & letter)
{
    // HSV isn't the best color model, but wxWidgets already has
    // conversion functions, and it's at least better than trying to
    // mess with RGB.
    wxImage::RGBValue rgb(letter.Red(), letter.Green(), letter.Blue());
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
    // Numbers here were established by trial and error.
    if (hsv.value < .3) // Dark colors
    {
        hsv.value += .2;
    }
    else if (hsv.saturation < .4) // Gray colors
    {
        if (hsv.saturation > .1) // If it's not too gray, saturate it
            hsv.saturation += .2;
        // Adjust value up or down
        if (hsv.value < .5)
            hsv.value += .2;
        else
            hsv.value -= .2;
    }
    else // "Colorful" colors (saturated and medium to high value)
    {
        // Adjust saturation up or down
        if (hsv.saturation > .5)
            hsv.saturation = std::max(0., hsv.saturation * .25);
        else
            hsv.saturation = std::min(1., hsv.saturation / .25);
        // Adjust value up or down
        if (hsv.value > .5)
            hsv.value = std::max(0., hsv.value * .9);
        else
            hsv.value = std::min(1., hsv.value / .9);
    }
    rgb = wxImage::HSVtoRGB(hsv);
    return wxColour(rgb.red, rgb.green, rgb.blue);
}
