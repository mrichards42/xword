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


#ifndef MY_DEFS_H
#define MY_DEFS_H

// Scaling factors
static const double NUMBER_SCALE = 2./5.;
static const double LETTER_SCALE = 5./6.;

// Styles
enum GridStyle
{
    PAUSE_ON_SWITCH     = 0x0001,
    BLANK_ON_NEW_WORD   = 0x0002,
    MOVE_AFTER_LETTER   = 0x0004,
    MOVE_TO_NEXT_BLANK  = 0x0008,
    BLANK_ON_DIRECTION  = 0x0010,
    CONTEXT_MENU        = 0x0020,
    MOVE_ON_RIGHT_CLICK = 0x0040,
    CHECK_WHILE_TYPING  = 0x0080,

    DEFAULT_GRID_STYLE = PAUSE_ON_SWITCH
                       | MOVE_AFTER_LETTER
                       | BLANK_ON_NEW_WORD,

    GRID_STYLE_MASK = PAUSE_ON_SWITCH
                    | BLANK_ON_NEW_WORD
                    | MOVE_AFTER_LETTER
                    | MOVE_TO_NEXT_BLANK
                    | BLANK_ON_DIRECTION
                    | CONTEXT_MENU
                    | MOVE_ON_RIGHT_CLICK
                    | CHECK_WHILE_TYPING
};


// Colors
enum SquareColor
{
    WHITE_SQUARE  = 0,
    LETTER_SQUARE = 1,
    WORD_SQUARE   = 2
};


#define GET_BITMAP(name, sizeX, sizeY) (wxBitmap(wxIcon(_T(#name), wxBITMAP_TYPE_ICO_RESOURCE, sizeX, sizeY)))

//#define _F wxString::Format

#endif // MY_DEFS_H