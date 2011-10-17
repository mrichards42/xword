// This file is part of XWord
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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

// NB: No include guard!
//     This is meant only as an auxillary file to Square.cpp
// The tables are not small-screen friendly.  They take up at least 105
// columns of text so that they are readable as a hex table.

//-----------------------------------------------------------------------------
// Character replacement tables
//-----------------------------------------------------------------------------

// Any letter in Windows-1252 encoding is valid.
// Letters are converted to upper case (if possible)

// Valid symbols and numbers are replaced with their first letter:
// # = H[ash],
// $ = D[ollar],
// % = P[ercent],
// & = A[mpersand],
// + = P[lus],
// ? = Q[uestion]
// @ = A[t]

// '[' number ']' is used to indicate a webdings symbol; these are replaced
// by a blank in the user grid.

// ' ' indicates a blank square.
// '.' indicates a black square.

// All other characters are invalid

char_t * Square::Blank   = puzT(" ");
char_t * Square::Black   = puzT(".");

// Across Lite representation
static unsigned char ascii [] = {
/*         0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f 
/* 0 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 1 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 2 */  ' ',    0,    0,  'H',  'D',  'P',  'A',    0,    0,    0,    0,  'P',    0,  ' ',  '.',    0,
/* 3 */  'Z',  'O',  'T',  'T',  'F',  'F',  'S',  'S',  'E',  'N',    0,    0,    0,    0,    0,  'Q',
/* 4 */  'A',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 5 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  ' ',    0,    0,    0,    0,
/* 6 */    0,  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 7 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',    0,    0,    0,    0,    0,
};
