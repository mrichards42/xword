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

#include "Checksummer.hpp"
#include <iostream>
#include <cassert>
#include "puzstring.hpp"

namespace puz {

// Return clues in order
static void GetClueList(const Puzzle & puz, std::vector<std::string> * clues)
{
    // Assemble the clues list from across and down
    ClueList::const_iterator across_it = puz.GetAcross().begin();
    ClueList::const_iterator down_it   = puz.GetDown().begin();

    clues->clear();
    for (const Square * square = puz.GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(ACROSS))
        {
            clues->push_back(GetPuzText(across_it->GetText()));
            ++across_it;
        }
        if (square->HasClue(DOWN))
        {
            clues->push_back(GetPuzText(down_it->GetText()));
            ++down_it;
        }
    }
    assert(across_it == puz.GetAcross().end() && down_it == puz.GetDown().end());
}

Checksummer::Checksummer(const Puzzle & puz, unsigned short version)
    : m_title    (encode_puz(puz.GetTitle())),
      m_author   (encode_puz(puz.GetAuthor())),
      m_copyright(encode_puz(puz.GetCopyright())),
      m_notes    (GetPuzText(puz.GetNotes())),
      m_version  (version)
{
    // Solution and Text
    m_solution.reserve(puz.m_grid.GetWidth() * puz.m_grid.GetHeight());
    m_gridText.reserve(puz.m_grid.GetWidth() * puz.m_grid.GetHeight());
    for (const Square * square = puz.m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->IsBlack())
        {
            if (puz.m_grid.IsDiagramless())
            {
                m_solution.push_back(':');
                m_gridText.push_back(':');
            }
            else
            {
                m_solution.push_back('.');
                m_gridText.push_back('.');
            }
        }
        else
        {
            m_solution.push_back(square->GetPlainSolution());
            char plain = square->GetPlainText();
            m_gridText.push_back(square->IsBlank() || plain == 0
                                    ? '-'
                                    : plain);
        }
    }

    cluelist_t clues;
    GetClueList(puz, &clues);
    SetClues(clues);

    // Setup CIB manually
    SetWidth     (puz.m_grid.GetWidth());
    SetHeight    (puz.m_grid.GetHeight());
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
    unsigned char  c_masked[8];
    GetChecksums(&c_cib, &c_primary, c_masked);

    const bool success =
           c_cib       == cib
        && c_primary   == primary
        && c_masked[0] == masked[0]
        && c_masked[1] == masked[1]
        && c_masked[2] == masked[2]
        && c_masked[3] == masked[3]
        && c_masked[4] == masked[4]
        && c_masked[5] == masked[5]
        && c_masked[6] == masked[6]
        && c_masked[7] == masked[7];

#ifndef NDEBUG
    if (! success)
    {
        // Print the checksums
        int stream_width = std::cerr.width();
        std::cerr.width(10);
        std::cerr << "Checksums      actual      calc'd" << "\n";
        std::cerr << "---------------------------------" << "\n";
        std::cerr << "CIB        " << cib       << "  " << c_cib       << "\n";
        std::cerr << "Primary    " << primary   << "  " << c_primary   << "\n";
        std::cerr << "Masked[0]  " << (unsigned short)masked[0] << "  " << (unsigned short)c_masked[0] << "\n";
        std::cerr << "Masked[1]  " << (unsigned short)masked[1] << "  " << (unsigned short)c_masked[1] << "\n";
        std::cerr << "Masked[2]  " << (unsigned short)masked[2] << "  " << (unsigned short)c_masked[2] << "\n";
        std::cerr << "Masked[3]  " << (unsigned short)masked[3] << "  " << (unsigned short)c_masked[3] << "\n";
        std::cerr << "Masked[4]  " << (unsigned short)masked[4] << "  " << (unsigned short)c_masked[4] << "\n";
        std::cerr << "Masked[5]  " << (unsigned short)masked[5] << "  " << (unsigned short)c_masked[5] << "\n";
        std::cerr << "Masked[6]  " << (unsigned short)masked[6] << "  " << (unsigned short)c_masked[6] << "\n";
        std::cerr << "Masked[7]  " << (unsigned short)masked[7] << "  " << (unsigned short)c_masked[7] << "\n";
        std::cerr.width(stream_width);
    }
#endif // NDEBUG

    return success;
}

void
Checksummer::GetChecksums(unsigned short * cib,
                          unsigned short * primary,
                          unsigned char masked [])
{
    // References make the syntax nicer
    unsigned short & c_cib       = *cib;
    unsigned short & c_primary   = *primary;
    unsigned char * c_masked     = masked;


    // CIB checksum
    c_cib = cksum_region(m_cib, 8, 0);


    // Primary (whole file) checksum
    c_primary = c_cib;
    c_primary = cksum_region(m_solution, c_primary);
    c_primary = cksum_region(m_gridText, c_primary);
    if (! m_title.empty())
        c_primary = cksum_region(m_title.c_str(),
                                 m_title.size() + 1,
                                 c_primary);
    if (! m_author.empty())
        c_primary = cksum_region(m_author.c_str(),
                                 m_author.size() + 1,
                                 c_primary);
    if (! m_copyright.empty())
        c_primary = cksum_region(m_copyright.c_str(),
                                 m_copyright.size() + 1,
                                 c_primary);

    for (cluelist_t::const_iterator it = m_clues.begin();
         it != m_clues.end();
         ++it)
    {
        c_primary = cksum_region(it->c_str(), it->size(), c_primary);
    }

    if (! m_notes.empty() && m_version >= 13)
        c_primary = cksum_region(m_notes.c_str(),
                                 m_notes.size() + 1,
                                 c_primary);


    // Masked checksums
    unsigned short c_sol  = cksum_region(m_solution, 0);
    unsigned short c_grid = cksum_region(m_gridText, 0);

    unsigned short c_part = 0;
    if (! m_title.empty())
        c_part = cksum_region(m_title.c_str(),
                              m_title.size() + 1,
                              c_part);
    if (! m_author.empty())
        c_part = cksum_region(m_author.c_str(),
                              m_author.size() + 1,
                              c_part);
    if (! m_copyright.empty())
        c_part = cksum_region(m_copyright.c_str(),
                              m_copyright.size() + 1,
                              c_part);

    for (cluelist_t::const_iterator it = m_clues.begin();
         it != m_clues.end();
         ++it)
    {
        c_part = cksum_region(it->c_str(), it->size(), c_part);
    }

    if (! m_notes.empty() && m_version >= 13)
        c_part = cksum_region(m_notes.c_str(),
                              m_notes.size() + 1,
                              c_part);

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

} // namespace puz
