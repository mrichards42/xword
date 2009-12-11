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

#include "Checksummer.hpp"
#include <wx/log.h>

Checksummer::Checksummer(const XPuzzle & puz, unsigned short version)
    : m_solution (puz.m_grid.GetGridSolution()),
      m_gridText (puz.m_grid.GetGridText()),
      m_title    (puz.m_title),
      m_author   (puz.m_author),
      m_copyright(puz.m_copyright),
      m_notes    (puz.m_notes),
      m_clues(puz.m_clues),
      m_version  (version)
{
    // Setup CIB manually
    SetWidth     (puz.m_grid.GetWidth());
    SetHeight    (puz.m_grid.GetHeight());
    SetClueLength(puz.m_clues.size());
    SetGridType  (puz.m_grid.m_type);
    SetGridFlag  (puz.m_grid.m_flag);
}


//------------------------------------------------------------------------------
// Checksumming routines
//------------------------------------------------------------------------------

unsigned short
Checksummer::cksum_region(const unsigned char * base,
                          size_t length,
                          unsigned short cksum)
{
    for (size_t i = 0; i < length; ++i)
    {
        if ((cksum & 1) != 0)
            cksum = (cksum >> 1) + 0x8000;
        else
            cksum = cksum >> 1;
        cksum += *(base+i);

        // Make sure this is within the range of a 16-bit number.
        // We can't rely on unsigned short to be 16 bits always.
        cksum &= 0xffff;
    }

    return cksum;
}


bool
Checksummer::TestChecksums(unsigned short cib,
                           unsigned short primary,
                           const unsigned char masked [])
{
    unsigned short c_cib;
    unsigned short c_primary;
    ByteArray c_masked;
    GetChecksums(&c_cib, &c_primary, &c_masked);

    // Print the checksums
    wxLogDebug(_T("Checksums      actual      calc'd"));
    wxLogDebug(_T("--------------------------------"));
    wxLogDebug(_T("CIB        %10d  %10d"), cib, c_cib);
    wxLogDebug(_T("Primary    %10d  %10d"), primary, c_primary);
    wxLogDebug(_T("Masked[0]  %10d  %10d"), masked[0], c_masked[0]);
    wxLogDebug(_T("Masked[1]  %10d  %10d"), masked[1], c_masked[1]);
    wxLogDebug(_T("Masked[2]  %10d  %10d"), masked[2], c_masked[2]);
    wxLogDebug(_T("Masked[3]  %10d  %10d"), masked[3], c_masked[3]);
    wxLogDebug(_T("Masked[4]  %10d  %10d"), masked[4], c_masked[4]);
    wxLogDebug(_T("Masked[5]  %10d  %10d"), masked[5], c_masked[5]);
    wxLogDebug(_T("Masked[6]  %10d  %10d"), masked[6], c_masked[6]);
    wxLogDebug(_T("Masked[7]  %10d  %10d"), masked[7], c_masked[7]);

    return c_cib       == cib
        && c_primary   == primary
        && c_masked[0] == masked[0]
        && c_masked[1] == masked[1]
        && c_masked[2] == masked[2]
        && c_masked[3] == masked[3]
        && c_masked[4] == masked[4]
        && c_masked[5] == masked[5]
        && c_masked[6] == masked[6]
        && c_masked[7] == masked[7];
}

void
Checksummer::GetChecksums(unsigned short * cib,
                          unsigned short * primary,
                          ByteArray * masked)
{
    // References make the syntax nicer
    unsigned short & c_cib     = *cib;
    unsigned short & c_primary = *primary;
    ByteArray &      c_masked  = *masked;


    // CIB checksum
    c_cib = cksum_region(m_cib, 8, 0);


    // Primary (whole file) checksum
    c_primary = c_cib;
    c_primary = cksum_region(m_solution,                 c_primary);
    c_primary = cksum_region(m_gridText,                 c_primary);
    if (! m_title.empty())
        c_primary = cksum_region(m_title      + _T('\0'), c_primary);
    if (! m_author.empty())
        c_primary = cksum_region(m_author     + _T('\0'), c_primary);
    if (! m_copyright.empty())
        c_primary = cksum_region(m_copyright  + _T('\0'), c_primary);

    for (std::vector<wxString>::const_iterator it = m_clues.begin();
         it != m_clues.end();
         ++it)
    {
        c_primary = cksum_region(*it, c_primary);
    }

    if (! m_notes.empty() && m_version >= 13)
        c_primary = cksum_region(m_notes + _T('\0'), c_primary);


    // Masked checksums
    unsigned short c_sol  = cksum_region(m_solution,        0);
    unsigned short c_grid = cksum_region(m_gridText,        0);

    unsigned short c_part = 0;
    if (! m_title.empty())
        c_part = cksum_region(m_title      + _T('\0'), c_part);
    if (! m_author.empty())
        c_part = cksum_region(m_author     + _T('\0'), c_part);
    if (! m_copyright.empty())
        c_part = cksum_region(m_copyright  + _T('\0'), c_part);

    for (std::vector<wxString>::const_iterator it = m_clues.begin();
         it != m_clues.end();
         ++it)
    {
        c_part = cksum_region(*it, c_part);
    }

    if (! m_notes.empty() && m_version >= 13)
        c_part = cksum_region(m_notes + _T('\0'), c_part);

    c_masked.resize(8);

    // le-low bits
    c_masked[0] = 'I' ^ LoByte(c_cib);
    c_masked[1] = 'C' ^ LoByte(c_sol);
    c_masked[2] = 'H' ^ LoByte(c_grid);
    c_masked[3] = 'E' ^ LoByte(c_part);
    // le-high bits
    c_masked[4] = 'A' ^ HiByte(c_cib);
    c_masked[5] = 'T' ^ HiByte(c_sol);
    c_masked[6] = 'E' ^ HiByte(c_grid);
    c_masked[7] = 'D' ^ HiByte(c_part);
}