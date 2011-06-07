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


#ifndef PUZ_CHECK_SUMMER_H
#define PUZ_CHECK_SUMMER_H

#include "Puzzle.hpp"
#include "puzstring.hpp"

namespace puz {

class PUZ_API Checksummer
{
    typedef std::vector<std::string> cluelist_t;
public:
    Checksummer() : m_version(13) {}
    explicit Checksummer(const Puzzle & puz, unsigned short version = 13);

    // Usually these are the only two functions you need to call
    void GetChecksums(unsigned short * cib,
                      unsigned short * primary,
                      unsigned char  masked []);

    bool TestChecksums(unsigned short cib,
                       unsigned short primary,
                       const unsigned char masked []);

    // Get / Set the members
    //----------------------

    // Use these functions to replace one of the items set by the constructor
    // taking a Puzzle, or after constructing the Checksummer without a Puzzle.

    void SetWidth     (unsigned char width)  { m_cib[0] = width; }
    void SetHeight    (unsigned char height) { m_cib[1] = height; }
    void SetClueLength(unsigned short len)   { m_cib[2] = LoByte(len);
                                               m_cib[3] = HiByte(len); }
    void SetGridType  (unsigned short type)  { m_cib[4] = LoByte(type);
                                               m_cib[5] = HiByte(type); }
    void SetGridFlag  (unsigned short flag)  { m_cib[6] = LoByte(flag);
                                               m_cib[7] = HiByte(flag); }

    void SetSolution  (const std::string & solution) { m_solution = solution; }
    void SetGridText  (const std::string & text)     { m_gridText = text; }
    const std::string & GetSolution() const { return m_solution; }
    const std::string & GetGridText() const { return m_gridText; }

    void SetTitle     (const std::string & title)     { m_title     = title; }
    void SetAuthor    (const std::string & author)    { m_author    = author; }
    void SetCopyright (const std::string & copyright) { m_copyright = copyright; }
    void SetNotes     (const std::string & notes)     { m_notes     = notes; }
    void SetClues     (const cluelist_t & clues)
    {
        m_clues.clear();
        for (cluelist_t::const_iterator it = clues.begin();
             it != clues.end();
             ++it)
        {
            m_clues.push_back(*it);
        }
        SetClueLength(m_clues.size());
    }

    const std::string & GetNotes() const { return m_notes; }
    const cluelist_t & GetClues() const { return m_clues; }

    void SetVersion   (unsigned short version)     { m_version   = version; }


    // Basic checksumming routine
    //---------------------------
    static
    unsigned short cksum_region(const unsigned char * base,
                                size_t length,
                                unsigned short cksum);

    static
    unsigned short cksum_region(const char * base,
                                size_t length,
                                unsigned short cksum)
    {
        return cksum_region(reinterpret_cast<const unsigned char *>(base),
                            length, cksum);
    }

    static
    unsigned short cksum_region(const std::string & data, unsigned short cksum)
        { return cksum_region(data.c_str(), data.size(), cksum); }

private:
    // These are used only with shorts (always 2 bytes in checksum routines).
    static unsigned char LoByte(unsigned short num)
        { return  num & 0x00ff; }
    static unsigned char HiByte(unsigned short num)
        { return (num & 0xff00) >> 8; }


private:
    // Checksummer works with 8-bit strings.
    unsigned char m_cib[8];
    std::string m_solution;
    std::string m_gridText;
    std::string m_title;
    std::string m_author;
    std::string m_copyright;
    std::string m_notes;
    cluelist_t m_clues;
    unsigned short m_version;
};

} // namespace puz

#endif // PUZ_CHECK_SUMMER_H
