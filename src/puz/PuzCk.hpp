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


// unsigned short is used for 2-byte numbers,
// while char is used for 1-byte numbers

struct PuzCk
{
    unsigned short c_primary;         //
             char  formatstr[12];     // 'ACROSS&DOWN\0'
    unsigned short c_cib;             //
    unsigned char  c_masked[8];       //
    unsigned char  version[4];        // '1.2\0' or something like it
    unsigned short noise1c;           // 
    unsigned short c_grid;            // Scrambled grid checksum
    unsigned short noise20;           //  All of this noise and unknown stuff
    unsigned short noise22;           //     can safely be set to 00
    unsigned short noise24;           //  Across Lite defaults to not changing
    unsigned short noise26;           //     anything it doesn't need to, so
    unsigned short noise28;           //     these regions are preserved
    unsigned short noise2a;           //
    unsigned char  width;             // Everything past this point makes up
    unsigned char  height;            //     the section that initializes all
    unsigned short num_clues;         //     the checksums (referred to as "cib")
    unsigned short puz_type;          // Normal puzzles are 0x0001 
    unsigned short puz_flag;          // Normal puzzles are 0x0000
};


// Perhaps all the blank space could be used for recording
//    time, score, a flag to tell if the puzzle is complete,
//    or something like that