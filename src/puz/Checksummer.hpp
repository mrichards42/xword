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


#ifndef CHECK_SUMMER_H
#define CHECK_SUMMER_H

#include "XPuzzle.hpp"

class Checksummer
{
private:
    // These are used only with shorts (always 2 bytes in checksum routines).
    unsigned char LoByte(unsigned short num) { return  num & 0x00ff; }
    unsigned char HiByte(unsigned short num) { return (num & 0xff00) >> 8; }

public:
    explicit Checksummer(const XPuzzle & puz, unsigned short version = 13);
    explicit Checksummer(unsigned short version = 13);

    // Usually these are the only two functions you need to call
    void GetChecksums(unsigned short * cib,
                      unsigned short * primary,
                      ByteArray      * masked);

    bool TestChecksums(unsigned short cib,
                       unsigned short primary,
                       const unsigned char masked []);

    // Set the members
    //----------------

    // Use these functions if you want to replace one of the items set by
    // the constructor taking an XPuzzle, or if you constructed this Checksummer
    // without an XPuzzle.

    void SetWidth     (unsigned char width)  { m_cib[0] = width; }
    void SetHeight    (unsigned char height) { m_cib[1] = height; }
    void SetClueLength(unsigned short len)   { m_cib[2] = LoByte(len);
                                               m_cib[3] = HiByte(len); }
    void SetGridType  (unsigned short type)  { m_cib[4] = LoByte(type);
                                               m_cib[5] = HiByte(type); }
    void SetGridFlag  (unsigned short flag)  { m_cib[6] = LoByte(flag);
                                               m_cib[7] = HiByte(flag); }

    void SetSolution  (const ByteArray & solution) { m_solution = solution; }
    void SetGridText  (const ByteArray & text)     { m_gridText = text; }

    void SetTitle     (const wxString & title)     { m_title     = title; }
    void SetAuthor    (const wxString & author)    { m_author    = author; }
    void SetCopyright (const wxString & copyright) { m_copyright = copyright; }
    void SetNotes     (const wxString & notes)     { m_notes     = notes; }    
    void SetClues     (const std::vector<wxString> & clues) { m_clues = clues; }

    void SetVersion   (unsigned short version)     { m_version   = version; }


    // Basic checksumming routine
    //---------------------------
    static
    unsigned short cksum_region(const unsigned char * base,
                                size_t length,
                                unsigned short cksum);

    static
    unsigned short cksum_region(const ByteArray & data, unsigned short cksum)
        { return cksum_region(&data[0], data.size(), cksum); }


private:
    unsigned char m_cib[8];
    ByteArray m_solution;
    ByteArray m_gridText;
    wxString m_title;
    wxString m_author;
    wxString m_copyright;
    wxString m_notes;
    std::vector<wxString> m_clues;
    unsigned short m_version;
};

#endif // CHECK_SUMMER_H