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


#include "PuzHandler.hpp"
#include "PuzCk.hpp"
#include <wx/msgdlg.h>
#include <wx/log.h>
#include "Checksummer.hpp"

//------------------------------------------------------------------------------
// DoLoad implementation
//------------------------------------------------------------------------------
void
PuzHandler::DoLoad()
{
    PuzCk h;
    Read(&h.c_primary, 2);
    Read(h.formatstr, 12);
    if (strcmp(h.formatstr, "ACROSS&DOWN") != 0)
        throw PuzLoadError(_T("This is not a valid Across puzzle file."));

    Read(&h.c_cib, 2);
    Read(h.c_masked, 8);

    Read(h.version, 4);

    if (h.version[0] != '1')
        throw PuzLoadError(_T("This puzzle uses a later version that this ")
                           _T("version of XWord can't read."));
    if (h.version[2] < '1' || h.version[2] > '9')
         throw PuzLoadError(_T("Bad puzzle version"));

    // We're using the assumption that both are valid numbers, as checked above
    // ASCII '1' is 0x31
    unsigned short version = (10 + (h.version[2] - 0x30));

    Read(&h.noise1c, 2);
    Read(&h.c_grid,  2);
    Read(&h.noise20, 2);
    Read(&h.noise22, 2);
    Read(&h.noise24, 2);
    Read(&h.noise26, 2);
    Read(&h.noise28, 2);
    Read(&h.noise2a, 2);

    Read(&h.width,  1);
    Read(&h.height, 1);
    Read(&h.num_clues, 2);
    Read(&h.puz_type, 2);
    Read(&h.puz_flag, 2);

    m_puz->m_grid.SetSize(h.width, h.height);

    SetGridChecksum(h.c_grid);
    SetGridType(h.puz_type);
    SetGridFlag(h.puz_flag);

    ByteArray solution(h.width * h.height);
    Read(&solution[0], h.width * h.height);
    SetGridSolution(solution);

    ByteArray gridText(h.width * h.height);
    Read(&gridText[0], h.width * h.height);
    SetGridText(gridText);

    m_puz->m_title       = ReadString();
    m_puz->m_author      = ReadString();
    m_puz->m_copyright   = ReadString();

    for (size_t i = 0; i < h.num_clues; ++i)
        m_puz->m_clues.push_back(ReadString());

    m_puz->m_notes = ReadString();

    // Find extra sections (i.e. GEXT, LTIM, etc)
    try
    {
        LoadSections();
    }
    // This is a reference to an event, so we can set error.isProcessed
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

    Checksummer cksum(*m_puz, version);

    // Use the grid and solution from the actual file, because Across Lite
    // doesn't save rebus solutions correctly all the time.
    cksum.SetSolution(solution);
    cksum.SetGridText(gridText);

    if (! cksum.TestChecksums(h.c_cib, h.c_primary, h.c_masked))
        throw PuzLoadError(_T("Checksums do not match"));

    SetupGrid();
    SetupClues();
}



//------------------------------------------------------------------------------
// DoSave implementation
//------------------------------------------------------------------------------

void
PuzHandler::DoSave()
{
    // Get checksums
    unsigned short c_cib;
    unsigned short c_primary;
    ByteArray      c_masked;

    Checksummer cksum(*m_puz, 13);
    cksum.GetChecksums(&c_cib, &c_primary, &c_masked);

    wxASSERT(c_masked.size() == 8);

    // Write the header
    Write(&c_primary, 2);
    Write("ACROSS&DOWN\0", 12);

    Write(&c_cib, 2);
    Write(&c_masked[0], 8);

    Write( GetVersionString() + _T('\0') );

    Write("\0\0", 2);
    unsigned short temp = GetGridChecksum();
    Write(&temp, 2);
    Write("\0\0", 2);
    Write("\0\0", 2);
    Write("\0\0", 2);
    Write("\0\0", 2);
    Write("\0\0", 2);
    Write("\0\0", 2);

    unsigned short width  = m_puz->m_grid.GetWidth();
    unsigned short height = m_puz->m_grid.GetHeight();
    unsigned short nClues = m_puz->m_clues.size();

    Write(&width,  1);
    Write(&height, 1);

    Write(&nClues, 2);
    temp = GetGridType();
    Write(&temp, 2);
    temp = GetGridFlag();
    Write(&temp, 2);


    // Write the puzzle's contents
    Write( m_puz->m_grid.GetGridSolution() );
    Write( m_puz->m_grid.GetGridText() );
    Write(m_puz->m_title     + _T('\0'));
    Write(m_puz->m_author    + _T('\0'));
    Write(m_puz->m_copyright + _T('\0'));

    std::vector<wxString>::iterator it;
    for (it = m_puz->m_clues.begin(); it != m_puz->m_clues.end(); ++it)
        Write(*it + _T('\0'));

    Write(m_puz->m_notes + _T('\0'));

    WriteSections();
}



//------------------------------------------------------------------------------
// Section loading
//------------------------------------------------------------------------------

void
PuzHandler::LoadSections()
{
    std::map<wxString, ByteArray> sections;

    // Read all the sections
    for (;;)
    {
        m_inStream->Peek();
        if (m_inStream->GetLastError() == wxSTREAM_EOF)
            break;

        // An extra section is defined as:
        // Title (4 chars)
        // Section length   (le-short)
        // Section checksum (le-short)

        ByteArray title(4);
        Read(&title[0], 4);

        unsigned short length;
        unsigned short ck_section;
        Read(&length, 2);
        Read(&ck_section, 2);

        ByteArray data(length);
        Read(&data[0], length);

        if (ck_section != Checksummer::cksum_region(data, 0))
            throw PuzLoadError(_T("Checksum does not match for %s region"),
                               title);

        if (m_inStream->GetC() != '\0')
            throw PuzLoadError(_T("Missiong nul-terminator for %s region"),
                               title);

        sections[title.to_string()] = data;
    }

    // Write the sections
    ByteArray data;

    data = sections[_T("GEXT")];
    if (data.has_data())
        SetGext(data);
    sections.erase(_T("GEXT"));

    data = sections[_T("LTIM")];
    if (data.has_data())
        SetLTIM(data);
    sections.erase(_T("LTIM"));


    data = sections[_T("RUSR")];
    if (data.has_data())
        SetRUSR(data);
    sections.erase(_T("RUSR"));

    ByteArray table = sections[_T("RTBL")];
    data = sections[_T("GRBS")];
    if (data.has_data() && table.has_data())
    {
        SetSolutionRebus(table, data);
        sections.erase(_T("RTBL"));
        sections.erase(_T("GRBS"));
    }
    if (! data.has_data())
        sections.erase(_T("GRBS"));
    if (! table.has_data())
        sections.erase(_T("RTBL"));

    // Add the remaining unknown sections
    for (std::map<wxString, ByteArray>::iterator it = sections.begin();
         it != sections.end();
         ++it)
    {
        AddSection(it->first, it->second);
    }
}



// LTIM (lit-time?)
//-----------------
void
PuzHandler::SetLTIM(const ByteArray & data)
{
    wxString str = data.to_string();

    // Split the string at the ','
    size_t index = str.find(_T(","));
    if (index == wxString::npos || index == 0)
        throw PuzLoadError(_T("Missing ',' in LTIM section"));

    long time;
    if (! str.Left(index).ToLong(&time))
        throw PuzLoadError(_T("Incorrect time value"));
    SetTime(time);
}


// RUSR (rebus-user)
//------------------
void
PuzHandler::SetRUSR(const ByteArray & data)
{
    std::vector<wxString> strs;
    ByteArray::const_iterator it = data.begin();
    for (ByteArray::const_iterator it = data.begin();
         it != data.end();
         ++it)
    {
        wxString str;
        if (*it != '\0')
            while (*it != '\0')
                str.append(static_cast<wxChar>(*it++));
        strs.push_back(str);
    }
    wxASSERT(strs.size()
        == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());
    SetTextRebus(strs);
}


// RTBL and GRBS (rebus-table, grid-rebus)
//----------------------------------------
void
PuzHandler::SetSolutionRebus(const ByteArray & table, const ByteArray & grid)
{
    // An important note here:
    // In the grid rebus section, the index is actually 1 greater than the real
    // number.
    // Note sure why.

    wxASSERT(grid.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());


    // Create the rebus table
    // Format: ' ' index ':' string ';'
    //   - Index can be multiple characters
    std::map<unsigned char, wxString> rebusTable;

    ByteArray::const_iterator table_it = table.begin();

    while (table_it != table.end())
    {
        wxString key;
        wxString value;

        ++table_it;         // Throw away ' '

        // Read the index
        while (*table_it != ':')
        {
            key.Append(static_cast<wxChar>(*table_it));
            ++table_it;
        }

        long index;
        if (! key.ToLong(&index))
            throw PuzLoadError(_T("Invalid rebus table key"));

        // The index value in the rebus-table section is 1 less than the
        // index in the grid-rebus, so we need add 1 here.
        ++index;

        ++table_it;         // Throw away ':'

        // Read the string value
        while (*table_it != ';')
        {
            value.Append(static_cast<wxChar>(*table_it));
            ++table_it;
        }

        ++table_it;         // Throw away ';' . . . done with this entry

        rebusTable[static_cast<unsigned char>(index)] = value;
    }

    HandlerBase::SetSolutionRebus(rebusTable, grid);
}




//------------------------------------------------------------------------------
// Section writing
//------------------------------------------------------------------------------

void
PuzHandler::WriteSections()
{
    WriteGEXT();
    WriteLTIM();
    WriteRUSR();
    WriteSolutionRebus();

    // Write the unknown sections
    std::vector<XPuzzle::section>::iterator s_it;
    for (s_it  = m_puz->m_extraSections.begin();
         s_it != m_puz->m_extraSections.end();
         ++s_it)
    {
        WriteSection(s_it->name, s_it->data);
    }
}


void
PuzHandler::WriteSection(const wxString & name, const ByteArray & data)
{
    size_t length = data.size();
    unsigned short cksum  = Checksummer::cksum_region(data, 0);
    Write(name);
    Write(&length, 2);
    Write(&cksum, 2);
    Write(&data[0], length);
    Write("\0", 1);
}


// GEXT
//-----
void
PuzHandler::WriteGEXT()
{
    ByteArray data = m_puz->m_grid.GetGext();
    if (data.has_data())
        WriteSection(_T("GEXT"), data);
}


// LTIM (lit-time?)
//-----------------
void
PuzHandler::WriteLTIM()
{
    if (m_puz->m_time != 0)
        WriteSection(_T("LTIM"),
                     wxString::Format(_T("%d,%d"), m_puz->m_time, 0) );
}


// RUSR (user-rebus)
//------------------
void
PuzHandler::WriteRUSR()
{
    ByteArray data;

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasTextRebus())
            data.push_string(square->GetText());
        data.push_back('\0');
    }
    if (data.has_data())
        WriteSection(_T("RUSR"), data);
}



// RTBL and GRBS (rebus-table, grid-rebus)
//--------------------------------------------------
void
PuzHandler::WriteSolutionRebus()
{
    std::map<wxString, wxByte> tableMap;
    ByteArray rebus;
    ByteArray table; // Write this as we go

    // Using a starting index like this works, but it's not quite accurate
    // I think Across lite loads the rebus-table into memory and uses it
    // like a symbol table, so that the index for a given string is 
    // preserved across saves.
    // I doubt that feature is essential, and it's a lot easier to just
    // remake the table starting from 1 when we need it.
    wxByte index = 1;


    std::map<wxString, wxByte>::iterator table_it;

    // Assemble the grid rebus string
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->HasSolutionRebus())
        {
            rebus.push_back(0);
        }
        else
        {
            table_it = tableMap.find(square->GetSolution());
            if (table_it == tableMap.end()) // This is a new entry
            {
                // The grid-rebus section adds 1 for every index
                tableMap[square->GetSolution()] = index + 1;
                rebus.push_back(index + 1);

                // Add to the rebusTable (uses the actual index)
                table.push_back(' ');
                table.push_string(wxString::Format(_T("%d"), index));
                table.push_back(':');
                table.push_string(square->GetSolution());
                table.push_back(';');

                ++index;
            }
            else // This string is already in the map
            {
                rebus.push_back(table_it->second);
            }
        }
    }


    if (rebus.has_data() && table.has_data())
    {
        WriteSection(_T("GRBS"), rebus);
        WriteSection(_T("RTBL"), table);
    }
}
