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

#ifndef PUZ_HANDLER_H
#define PUZ_HANDLER_H

#include <wx/stream.h>
#include <wx/log.h>
#include <wx/string.h>

#include "HandlerCommon.hpp"
#include "XPuzzle.hpp"
#include "PuzCk.hpp"

#include <string>

// The required Load/Save functions
void LoadPuz (XPuzzle * puz, wxInputStream & stream);
void SavePuz (XPuzzle * puz, wxOutputStream & stream);


void LoadSections(XPuzzle * puz, wxInputStream & stream);

//------------------------------------------------------------------------------
// Checksumming (basic function cksum_region defined in HandlerCommon.hpp)
//------------------------------------------------------------------------------

unsigned short cib_cksum      (XPuzzle * puz);

unsigned short primary_cksum  (XPuzzle * puz, unsigned short c_cib);

void           masked_cksum   (XPuzzle * puz,
                               unsigned short c_cib,
                               ByteArray & cksums);



void AddSection(XPuzzle * puz, const wxString & name, const ByteArray & data)
{
    puz->m_extraSections.push_back(XPuzzle::section(name, data));
}

void WriteSection(wxOutputStream & stream,
                  const wxString & name,
                  const ByteArray & data)             throw (...);

void WriteSection(wxOutputStream & stream,
                  const XPuzzle::section & section)   throw(...)
{
    WriteSection(stream, section.name, section.data);
}

void WriteSection(wxOutputStream & stream,
                  const wxString & name,
                  const wxString & data)              throw(...)
{
    ByteArray temp;
    StringToBytes(data, &temp);
    WriteSection(stream, name, temp);
}


// A helper function to keep all the messy bits in one place
void WriteSections(XPuzzle * puz, wxOutputStream & stream);


//------------------------------------------------------------------------------
// Saving and loading functions
//------------------------------------------------------------------------------
void
LoadPuz(XPuzzle * puz, wxInputStream & stream)
{
    PuzCk h;
    Read(stream, &h.c_primary, 2);
    Read(stream, h.formatstr, 12);

    Read(stream, &h.c_cib, 2);
    Read(stream, h.c_masked, 8);

    Read(stream, h.version, 4);
    Read(stream, &h.noise1c, 2);
    Read(stream, &h.c_grid,  2);
    Read(stream, &h.noise20, 2);
    Read(stream, &h.noise22, 2);
    Read(stream, &h.noise24, 2);
    Read(stream, &h.noise26, 2);
    Read(stream, &h.noise28, 2);
    Read(stream, &h.noise2a, 2);

    Read(stream, &h.width,  1);
    Read(stream, &h.height, 1);
    Read(stream, &h.num_clues, 2);
    Read(stream, &h.puz_type, 2);
    Read(stream, &h.puz_flag, 2);

    puz->m_grid.SetSize(h.width, h.height);

    puz->m_grid.m_cksum = h.c_grid;
    puz->m_grid.m_type = h.puz_type;
    puz->m_grid.m_flag = h.puz_flag;

    ByteArray data(h.width * h.height);

    Read(stream, &data[0], h.width * h.height);
    SetGridSolution(puz, data);

    Read(stream, &data[0], h.width * h.height);
    SetGridText    (puz, data);

    puz->m_title       = ReadString(stream);
    puz->m_author      = ReadString(stream);
    puz->m_copyright   = ReadString(stream);

    for (size_t i = 0; i < h.num_clues; ++i)
        puz->m_clues.push_back(ReadString(stream));

    puz->m_notes = ReadString(stream);

    // Find extra sections (i.e. GEXT, LTIM, etc)
    try
    {
        LoadSections(puz, stream);
    }
    // Note that this is a reference to an event, so we can set error.isProcessed
    catch (PuzLoadError & error)
    {
        if (error.isProcessed)
            throw;

        error.isProcessed = true;

        int ret = wxMessageBox(error.message
                      + _T("\n\n")
                        _T("Puzzle grid and clues are intact ")
                        _T("but other parts may be corrupted.")
                        _T("\n\n")
                        _T("Continue?"),
                        wxMessageBoxCaptionStr,
                        wxYES_NO | wxICON_EXCLAMATION);

        if (ret == wxNO)
            throw;
    }

    // Verify checksums
    unsigned short c_cib = cib_cksum(puz);
    unsigned short c_primary = primary_cksum(puz, c_cib);
    ByteArray c_masked;
    masked_cksum(puz, c_cib, c_masked);

    // Test the checksums
    const bool cksumsMatch = \
                   c_cib       == h.c_cib
                && c_primary   == h.c_primary
                && c_masked[0] == h.c_masked[0]
                && c_masked[1] == h.c_masked[1]
                && c_masked[2] == h.c_masked[2]
                && c_masked[3] == h.c_masked[3]
                && c_masked[4] == h.c_masked[4]
                && c_masked[5] == h.c_masked[5]
                && c_masked[6] == h.c_masked[6]
                && c_masked[7] == h.c_masked[7];


    // Print the checksums
    wxLogDebug(_T("Checksums      actual      calc'd"));
    wxLogDebug(_T("--------------------------------"));
    wxLogDebug(_T("CIB        %10d  %10d"), h.c_cib, c_cib);
    wxLogDebug(_T("Primary    %10d  %10d"), h.c_primary, c_primary);
    wxLogDebug(_T("Masked[0]  %10d  %10d"), h.c_masked[0], c_masked[0]);
    wxLogDebug(_T("Masked[1]  %10d  %10d"), h.c_masked[1], c_masked[1]);
    wxLogDebug(_T("Masked[2]  %10d  %10d"), h.c_masked[2], c_masked[2]);
    wxLogDebug(_T("Masked[3]  %10d  %10d"), h.c_masked[3], c_masked[3]);
    wxLogDebug(_T("Masked[4]  %10d  %10d"), h.c_masked[4], c_masked[4]);
    wxLogDebug(_T("Masked[5]  %10d  %10d"), h.c_masked[5], c_masked[5]);
    wxLogDebug(_T("Masked[6]  %10d  %10d"), h.c_masked[6], c_masked[6]);
    wxLogDebug(_T("Masked[7]  %10d  %10d"), h.c_masked[7], c_masked[7]);

    if (! cksumsMatch)
        throw PuzLoadError(_T("Checksums do not match"));

    SetupGrid(puz);
    SetupClues(puz);
}


void
LoadSections(XPuzzle * puz, wxInputStream & stream)
{
    // Rebus stuff
    ByteArray rebusTable;
    ByteArray gridRebus;

    for (;;)
    {
        stream.Peek();
        if (stream.GetLastError() == wxSTREAM_EOF)
            break;

        // An extra section is defined as:
        // Title (4 chars)
        // Section length   (le-short)
        // Section checksum (le-short)

        wxString title = ReadString(stream, 4);

        unsigned short length;
        unsigned short ck_section;
        Read(stream, &length, 2);
        Read(stream, &ck_section, 2);

        ByteArray data(length);
        Read(stream, &data[0], length);

        if (ck_section != cksum_region(data, 0x00))
            throw PuzLoadError(_T("Checksum does not match for %s region"),
                               title);

        if (stream.GetC() != 0)
            throw PuzLoadError(_T("Missiong nul-terminator for %s region"),
                               title);

        // Section-specific stuff
        //-----------------------------
        bool isKnown = true;

        // GEXT section (grid-extension?)
        if      (title == _T("GEXT"))
            SetGext(puz, data);

        // LTIM section (lit-time?)
        else if (title == _T("LTIM"))
        {
            wxString str;
            ByteArray::iterator it = data.begin();
            while (it != data.end() && *it != ',')
            {
                str.Append((wxChar)*it);
                ++it;
            }
            long time;
            if (! str.ToLong(&time))
                throw PuzLoadError(_T("Incorrect time value"));
            puz->m_time = time;
        }

        // RUSR section (rebus-user)
        else if (title == _T("RUSR"))
            SetRebusUserGrid(puz, data);

        // RTBL section (rebus-table)
        else if (title == _T("RTBL"))
        {
            rebusTable = data;
            if (! gridRebus.empty())
            {
                SetRebusSolution(puz, rebusTable, gridRebus);
                rebusTable.clear();
                gridRebus.clear();
            }
        }

        // GRBS section (grid-rebus)
        else if (title == _T("GRBS"))
        {
            gridRebus = data;
            if (! rebusTable.empty())
            {
                SetRebusSolution(puz, rebusTable, gridRebus);
                rebusTable.clear();
                gridRebus.clear();
            }
        }

        // Otherwise we don't know what to do with the section,
        //  so add it to the list of extra sections.
        else
        {
            bool isKnown = false;
            wxLogDebug(_T("Unknown section %s"), title);

            AddSection(puz, title, data);
        }

        if (isKnown)
            wxLogDebug(_T("Section %s"), title);
    }

    // If both of these sections are not present, save the one that is
    if (! rebusTable.empty())
        AddSection(puz, _T("RTBL"), rebusTable);
    if (! gridRebus.empty())
        AddSection(puz, _T("GRBS"), gridRebus);
}








void
SavePuz(XPuzzle * puz, wxOutputStream & stream)
{

    // Get checksums
    unsigned short c_cib = cib_cksum(puz);
    unsigned short c_primary = primary_cksum(puz, c_cib);
    ByteArray c_masked;
    masked_cksum(puz, c_cib, c_masked);

    wxASSERT(c_masked.size() == 8);

    // Write File
    Write(stream, &c_primary, 2);
    Write(stream, "ACROSS&DOWN\0", 12);

    Write(stream, &c_cib, 2);
    Write(stream, &c_masked[0], 8);

    Write(stream, "1.2\0", 4);

    Write(stream, "\0\0", 2);
    Write(stream, &puz->m_grid.m_cksum, 2);
    Write(stream, "\0\0", 2);
    Write(stream, "\0\0", 2);
    Write(stream, "\0\0", 2);
    Write(stream, "\0\0", 2);
    Write(stream, "\0\0", 2);
    Write(stream, "\0\0", 2);

    unsigned short width  = puz->m_grid.GetWidth();
    unsigned short height = puz->m_grid.GetHeight();
    unsigned short nClues = puz->m_clues.size();

    Write(stream, &width,  1);
    Write(stream, &height, 1);

    Write(stream, &nClues, 2);
    Write(stream, &puz->m_grid.m_type, 2);
    Write(stream, &puz->m_grid.m_flag, 2);

    WriteString(stream, GetGridSolution(puz));
    WriteString(stream, GetGridText(puz));
    WriteString(stream, puz->m_title     + _T('\0'));
    WriteString(stream, puz->m_author    + _T('\0'));
    WriteString(stream, puz->m_copyright + _T('\0'));

    std::vector<wxString>::iterator it;
    for (it = puz->m_clues.begin(); it != puz->m_clues.end(); ++it)
        WriteString(stream, *it + _T('\0'));

    WriteString(stream, puz->m_notes + _T('\0'));

    // Write the known and unknown sections
    WriteSections(puz, stream);
}


void
WriteSections(XPuzzle * puz, wxOutputStream & stream)
{
    // GEXT
    //-----
    ByteArray gext;
    const bool hasGext = GetGext(puz, &gext);
    if (hasGext)
        WriteSection(stream, wxString(_T("GEXT")), gext);

    // LTIM
    //-----
    if (puz->m_time != 0)
        WriteSection(stream, wxString(_T("LTIM")),
                             wxString::Format(_T("%d,%d"), puz->m_time, 0) );

    // RUSR section (rebus-user)
    //--------------------------
    {
        bool isEmpty = true;
        ByteArray rebus;

        // Assemble the string
        for (XSquare * square = puz->m_grid.First();
             square != NULL;
             square = square->Next())
        {
            if      (! square->rebus.empty())
            {
                isEmpty = false;
                StringToBytes(square->rebus, &rebus);
            }
            else if (square->rebusSym != 0)
            {
                isEmpty = false;
                rebus.push_back('[');
                rebus.push_back(square->rebusSym);
                rebus.push_back(']');
            }
            rebus.push_back(0);
        }

        if (! isEmpty)
            WriteSection(stream, _T("RUSR"), rebus);
    } // Scope



    // RTBL and GRBS sections (rebus-table / grid-rebus)
    //--------------------------------------------------
    {
        std::map<wxString, wxByte> tableMap;
        ByteArray rebus;
        ByteArray table; // Create this as we go

        // Using a starting index like this works, but it's not quite accurate
        // I think Across lite loads the rebus-table into memory and uses it
        // like a symbol table, so that the index for a given string is 
        // preserved across saves.
        // I doubt that feature is essential, and it's a lot easier to just
        // remake the table starting from 1 when we need it.
        wxByte index = 1;

        bool isEmpty = true;

        std::map<wxString, wxByte>::iterator table_it;

        // Assemble the grid rebus string
        for (XSquare * square = puz->m_grid.First();
             square != NULL;
             square = square->Next())
        {
            wxString sol;
            if      (! square->rebusSol.empty())
            {
                sol = square->rebusSol;
            }
            else if (square->rebusSymSol != 0)
            {
                // Symbols are '[' symbol ']'
                sol = _T("[");
                sol.Append((wxChar)square->rebusSymSol);
                sol.Append(_T("]"));
            }

            if (sol.empty())
            {
                rebus.push_back(0);
            }
            else // We have a rebus
            {
                isEmpty = false;
                table_it = tableMap.find(sol);
                if (table_it == tableMap.end()) // This is a new entry
                {
                    // The grid-rebus section adds 1 for every index
                    tableMap[sol] = index + 1;
                    rebus.push_back(index + 1);

                    // Add to the rebusTable (uses the actual index)
                    table.push_back(' ');
                    StringToBytes(wxString::Format(_T("%d"), index), &table);
                    table.push_back(':');
                    StringToBytes(sol, &table);
                    table.push_back(';');

                    ++index;
                }
                else // This string is already in the map
                {
                    rebus.push_back(table_it->second);
                }
            }
        }


        if (! isEmpty)
        {
            WriteSection(stream, _T("GRBS"), rebus);
            WriteSection(stream, _T("RTBL"), table);
        }
    } // Scope



    // Write the unknown sections
    //---------------------------
    std::vector<XPuzzle::section>::iterator s_it;
    for (s_it  = puz->m_extraSections.begin();
         s_it != puz->m_extraSections.end();
         ++s_it)
    {
        WriteSection(stream, s_it->name, s_it->data);
    }

}







// Writes an extra section (e.g. GEXT, LTIM)
void
WriteSection(wxOutputStream & stream,
             const wxString & name, const ByteArray & data)
{
    size_t length = data.size();
    unsigned short cksum  = cksum_region(data, 0);
    WriteString(stream, name);
    Write(stream, &length, 2);
    Write(stream, &cksum, 2);
    Write(stream, &data[0], length);
    Write(stream, "\0", 1);
}





//------------------------------------------------------------------------------
// Checksumming
//------------------------------------------------------------------------------

// These are for used with shorts
#define LO_BYTE(num)   ( (num) & 0x00ff )
#define HI_BYTE(num) ( ( (num) & 0xff00 ) >> 8)


unsigned short
cib_cksum(XPuzzle * puz)
{
    // Numbers to initialize the checksum
    ByteArray cib_region(8);
    cib_region[0] = puz->m_grid.GetWidth();
    cib_region[1] = puz->m_grid.GetHeight();

    // First byte
    cib_region[2] = LO_BYTE(puz->m_clues.size());
    // Second byte
    cib_region[3] = HI_BYTE(puz->m_clues.size());

    cib_region[4] = LO_BYTE(puz->m_grid.m_type);
    cib_region[5] = HI_BYTE(puz->m_grid.m_type);
    cib_region[6] = LO_BYTE(puz->m_grid.m_flag);
    cib_region[7] = HI_BYTE(puz->m_grid.m_flag);

    return cksum_region(cib_region, 0x0000);
}


unsigned short
primary_cksum(XPuzzle * puz, unsigned short c_cib)
{
    unsigned short c_primary = c_cib;
    c_primary = cksum_string(GetGridSolution(puz),           c_primary);
    c_primary = cksum_string(GetGridText(puz),               c_primary);
    c_primary = cksum_string(puz->m_title     + _T('\0'),    c_primary);
    c_primary = cksum_string(puz->m_author    + _T('\0'),    c_primary);
    c_primary = cksum_string(puz->m_copyright + _T('\0'),    c_primary);

    for (std::vector<wxString>::iterator it = puz->m_clues.begin();
         it != puz->m_clues.end();
         ++it)
    {
        c_primary = cksum_string(*it, c_primary);
    }

    return c_primary;
}


void
masked_cksum(XPuzzle * puz, unsigned short c_cib, ByteArray & cksums)
{
    // Masked sums
    unsigned short c_sol  = cksum_string(GetGridSolution(puz), 0x0000);
    unsigned short c_grid = cksum_string(GetGridText(puz),     0x0000);

    unsigned short c_part;
    c_part = cksum_string(puz->m_title     + _T('\0'), 0x0000);
    c_part = cksum_string(puz->m_author    + _T('\0'), c_part);
    c_part = cksum_string(puz->m_copyright + _T('\0'), c_part);

    for (std::vector<wxString>::iterator it = puz->m_clues.begin();
         it != puz->m_clues.end();
         ++it)
    {
        c_part = cksum_string(*it, c_part);
    }

    cksums.resize(8);

    // le-low bits
    cksums[0] = 'I' ^ LO_BYTE(c_cib);
    cksums[1] = 'C' ^ LO_BYTE(c_sol);
    cksums[2] = 'H' ^ LO_BYTE(c_grid);
    cksums[3] = 'E' ^ LO_BYTE(c_part);
    // le-high bits
    cksums[4] = 'A' ^ HI_BYTE(c_cib);
    cksums[5] = 'T' ^ HI_BYTE(c_sol);
    cksums[6] = 'E' ^ HI_BYTE(c_grid);
    cksums[7] = 'D' ^ HI_BYTE(c_part);
}

#undef HI_BYTE
#undef LO_BYTE

#endif // PUZ_HANDLER_H
