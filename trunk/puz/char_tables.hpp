// This file is part of XWord
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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
// by a blank ('-') in the user grid.

// '.' indicates a black square.

// All other characters are invalid


const unsigned char upperCase [] = {
/*         0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f 
/* 0 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 1 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 2 */  ' ',    0,    0,  '#',  '$',  '%',  '&',    0,    0,    0,    0,  '+',    0,  '-',  '.',    0,
/* 3 */  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',    0,    0,    0,    0,    0,  '?',
/* 4 */  '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 5 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',    0,    0,    0,    0,
/* 6 */    0,  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 7 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',    0,    0,    0,    0,    0,
/* 8 */    0,    0,    0, 0x83,    0,    0,    0,    0,    0,    0, 0x8a,    0, 0x8c,    0, 0x8e,    0,
/* 9 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x8a,    0, 0x8c,    0, 0x8e, 0x9f,
/* a */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0xaa,    0,    0,    0,    0,    0,
/* b */    0,    0, 0xb2, 0xb3,    0, 0xb5,    0,    0,    0, 0xb9, 0xba,    0,    0,    0,    0,    0,
/* c */ 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
/* d */ 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,    0, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
/* e */ 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
/* f */ 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,    0, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0x9f,
};


const unsigned char ascii [] = {
/*         0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f 
/* 0 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 1 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
/* 2 */  '-',    0,    0,  'H',  'D',  'P',  'A',    0,    0,    0,    0,  'P',    0,  '-',  '.',    0,
/* 3 */  'Z',  'O',  'T',  'T',  'F',  'F',  'S',  'S',  'E',  'N',    0,    0,    0,    0,    0,  'Q',
/* 4 */  'A',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 5 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '-',    0,    0,    0,    0,
/* 6 */    0,  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/* 7 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',    0,    0,    0,    0,    0,
/* 8 */    0,    0,    0,  'F',    0,    0,    0,    0,    0,    0,  'S',    0,  'O',    0,  'Z',    0,
/* 9 */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  'S',    0,  'O',    0,  'Z',  'Y',
/* a */    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  'A',    0,    0,    0,    0,    0,
/* b */    0,    0,  'T',  'T',    0,  'U',    0,    0,    0,  'O',  'O',    0,    0,    0,    0,    0,
/* c */  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
/* d */  'D',  'N',  'O',  'O',  'O',  'O',  'O',    0,  'O',  'U',  'U',  'U',  'U',  'Y',  'B',  'B',
/* e */  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
/* f */  'D',  'N',  'O',  'O',  'O',  'O',  'O',    0,  'O',  'U',  'U',  'U',  'U',  'Y',  'B',  'Y',
};
