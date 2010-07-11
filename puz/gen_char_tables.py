# ctypes functions
from ctypes import windll
_toupper = windll.user32.CharUpperA
_isalnum = windll.user32.IsCharAlphaNumericA


# Make the keys and values numeric or strings
def fixdict(d):
    for k,v in d.items():
        sk, ik = k, k
        if isinstance(k, int):
            sk = chr(k)
        else:
            ik = ord(k)
        sv, iv = v, v
        if isinstance(v, int):
            sv = chr(v)
        else:
            iv = ord(v)
        d[sk] = sv
        d[ik] = iv

special_chars = {
    # Symbols
    '#':'H',
    '$':'D',
    '%':'P',
    '&':'A',
    '+':'P',
    '?':'Q',
    '@':'A',

    # Others
    '-':'-',
    ' ':'-',
    '.':'.',
    '[':'-',
}

fixdict(special_chars)

plain_chars = {
    # Numbers
    '0':'Z',
    '1':'O',
    '2':'T',
    '3':'T',
    '4':'F',
    '5':'F',
    '6':'S',
    '7':'S',
    '8':'E',
    '9':'N',

    # Windows encoded characters
    0x83:'F', # Small slanted f
    # 0x8a, 0x8c, 0x93, 0x9f are capitals
    0x9a:'S', # s w/ caron
    0x9c:'O', # Oe
    0x9e:'Z', # Z w/ caron
    0xaa:'A', # "feminine ordinal indicator"
    0xb2:'T', # superscript 2
    0xb3:'T', # superscript 3
    0xb5:'U', # micro
    0xb9:'O', # superscript 1
    0xba:'O', # "masculine ordinal indicator"
    # 0xc0 - 0xdc are capitals
    0xdf:'B', # "Latin Small Letter Sharp S"
    0xe0:'A', # e's with various decorations . . .
    0xe1:'A',
    0xe2:'A',
    0xe3:'A',
    0xe4:'A',
    0xe5:'A',
    0xe6:'A', # ae
    0xe7:'C', # c with Cedilla
    0xe8:'E', # e's with various decorations . . .
    0xe9:'E',
    0xea:'E',
    0xeb:'E',
    0xec:'I', # i's with various decorations . . .
    0xed:'I',
    0xee:'I',
    0xef:'I',
    0xf0:'D', # "Latin small letter Eth"
    0xf1:'N', # N + ~
    0xf2:'O', # o's with various decorations . . .
    0xf3:'O',
    0xf4:'O',
    0xf5:'O',
    0xf6:'O',
    0xf8:'O',
    0xf9:'U', # u's with various decorations . . .
    0xfa:'U',
    0xfb:'U',
    0xfc:'U',
    0xfd:'Y', # y with accent
    0xfe:'B', # "Latin small letter Thorn"
    0xff:'Y', # y with diaeresis
}
# Set the uppercase versions (not explicit for brevity)
for k,v in plain_chars.items():
    if isinstance(k, int):
        plain_chars[_toupper(k)] = v

fixdict(plain_chars)

plain_chars.update(special_chars)
    

# Other conversions
def _plain(n):
    if n in plain_chars:
        return plain_chars[n]
    else:
        return _toupper(n)

# Return a table representing hex codes and their corresponding translation
def maketable(translate = lambda x: x):
    r = [ [ None for i in xrange(16) ] for j in xrange(16) ]
    for n in xrange(256):
        ch = 0
        if n in special_chars or _isalnum(n):
            ch = translate(n)
            if 0 < ch < 128:
                ch = "'%c'" % ch
            elif ch > 127:
                ch = hex(ch)
        if ch == 0:
            ch = '0'
        r[n / 16][n % 16] = ch
    return r


# print a table for a C header file
import sys
def printtable(t, name, write = sys.stdout.write):
    write('const unsigned char %s [] = {\n' % name)
    hchars = '0123456789abcdef'
    # Write the heading /*    0   1 ... */
    write('/*     ')
    for c in hchars:
        write('%5s ' % c)
    write('\n')
    # Write the data
    for r, row in enumerate(t):
        write('/* %s */' % hchars[r]) # Row headings
        for c in row:
            write('%5s,' % c)
        write('\n')
    write('};\n')


# Write to "char_tables.hpp"
def main():
    f = open("char_tables.hpp", 'w')
    f.write("""// This file is part of XWord
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
""")

    f.write("\n\n")
    printtable(maketable(_toupper), 'upperCase', f.write)

    f.write("\n\n")
    printtable(maketable(_plain), 'ascii', f.write)
    f.close()

if __name__ == "__main__":
    main()