// This file is part of XWord    
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
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


// FOR NOW WE DON'T NEED THIS



#ifndef MY_CONFIG_H
#define MY_CONFIG_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

// A struct for holding global config information
struct MyConfig
{
    struct {
        int x, y, width, height;
        bool maximized;

        wxRect GetRect() { return wxRect(x, y, width, height); }
    } window;

    struct {
        bool show_all;
        int  style;
    } grid;

};

extern MyConfig g_config;

#endif // MY_CONFIG_H