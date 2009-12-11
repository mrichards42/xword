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
        throw PuzTypeError(_T("This is not a valid Across puzzle file."));

    Read(&h.c_cib, 2);
    Read(h.c_masked, 8);

    Read(h.version, 4);

    if (h.version[0] != '1')
        throw PuzHeaderError(_T("This puzzle uses a later version that this ")
                           _T("version of XWord can't read."));
    if (h.version[2] < '1' || h.version[2] > '9')
         throw PuzHeaderError(_T("Bad puzzle version"));

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

    SetGridCksum(h.c_grid);
    SetGridType(h.puz_type);
    SetGridFlag(h.puz_flag);

    // Read in the solution and text
    ByteArray solution(h.width * h.height);
    Read(&solution[0], h.width * h.height);

    ByteArray gridText(h.width * h.height);
    Read(&gridText[0], h.width * h.height);

    // Set the grid's solution and text
    ByteArray::iterator sol_it  = solution.begin();
    ByteArray::iterator text_it = gridText.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetSolution(static_cast<wxChar>(*sol_it));
        if (*text_it == _T('-'))
            square->SetText(_T(""));
        else
        {
            square->SetText    (static_cast<wxChar>(*text_it));
            if (wxIslower(*text_it))
                square->AddFlag(XFLAG_PENCIL);
        }
        ++text_it;
        ++sol_it;
    }
    wxASSERT(sol_it == solution.end() && text_it == gridText.end());

    SetupGrid();

    m_puz->m_title       = ReadString();
    m_puz->m_author      = ReadString();
    m_puz->m_copyright   = ReadString();

    for (size_t i = 0; i < h.num_clues; ++i)
        m_puz->m_clues.push_back(ReadString());

    SetupClues();

    m_puz->m_notes = ReadString();

    // At this point we've read in all the essential puzzle information.
    // Even if loading the sections fails miserably, or if the checksums
    // aren't correct, we can still display a puzzle.
    // Any functions we call from here on out are welcome to throw
    // exceptions, and our caller is welcome to catch those exceptions
    // and still try to display the puzzle.
    SetReadErrorsFatal(false);

    wxString sectionError;
    bool checksumsOk;

    // Try to load the extra sections (i.e. GEXT, LTIM, etc).
    try
    {
        LoadSections();
        sectionError = wxEmptyString;
    }
    catch (PuzDataError & error)
    {
        sectionError = error.what();
    }


    // Test the checksums

    Checksummer cksum(*m_puz, version);

    // Use the grid and solution from the actual file, because Across Lite
    // doesn't save rebus solutions correctly all the time.
    cksum.SetSolution(solution);
    cksum.SetGridText(gridText);

    checksumsOk = cksum.TestChecksums(h.c_cib, h.c_primary, h.c_masked);
    if (! checksumsOk)
    {
        // We're going to test both 1.3 and 1.2 as versions because some files
        // with notepads don't have the correct version . . .
        cksum.SetVersion( (version == 13 ? 12 : 13) );
        checksumsOk = cksum.TestChecksums(h.c_cib, h.c_primary, h.c_masked);
    }

    // Throw errors if there are any
    if (! checksumsOk)
        throw PuzChecksumError(_T("File checksums don't match."));
    if (! sectionError.empty())
        throw PuzSectionError(sectionError);

    m_puz->SetOk(checksumsOk && sectionError.empty());
}



//------------------------------------------------------------------------------
// DoSave implementation
//------------------------------------------------------------------------------

void
PuzHandler::DoSave()
{
    // NB: Make sure to use c strings (without _T() macro) when writing
    // the file so that they show up correctly in unicode build.  If _T() is
    // used under unicode build, it will add extra nuls between letters.

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

    Write("1.3\0", 4);

    Write("\0\0", 2);
    unsigned short temp = m_puz->m_grid.GetCksum();
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
    temp = m_puz->m_grid.GetType();
    Write(&temp, 2);
    temp = m_puz->m_grid.GetFlag();
    Write(&temp, 2);


    // Write the puzzle's contents
    ByteArray grid;
    ByteArray solution;
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        grid.push_back(square->GetPlainText());
        solution.push_back(square->GetPlainSolution());
    }
    Write(solution);
    Write(grid);
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
        Read(&length, 2);
        if (length == 0)
            throw PuzSectionError(_T("Length of %s region is 0"),
                                  title.to_string().c_str());

        unsigned short ck_section;
        Read(&ck_section, 2);

        ByteArray data(length);
        Read(&data[0], length);

        if (ck_section != Checksummer::cksum_region(data, 0))
            throw PuzSectionError(_T("Checksum does not match for %s region"),
                                  title.to_string().c_str());

        if (m_inStream->GetC() != '\0')
            throw PuzSectionError(_T("Missiong nul-terminator for %s region"),
                                  title.to_string().c_str());

        sections[title.to_string()] = data;
    }

    // Write the sections
    ByteArray data;

    data = sections[_T("GEXT")];
    if (data.has_data())
        SetGEXT(data);
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



// GEXT (grid-extra?)
//-------------------
void
PuzHandler::SetGEXT(const ByteArray & data)
{
    ByteArray::const_iterator it = data.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetFlag(*it++);
    }
    wxASSERT(it == data.end());
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
        throw PuzSectionError(_T("Missing ',' in LTIM section"));

    long time;
    if (! str.Left(index).ToLong(&time))
        throw PuzSectionError(_T("Incorrect time value"));

    long isTimerRunning;
    if (! str.Mid(index+1).ToLong(&isTimerRunning))
        throw PuzSectionError(_T("Incorrect time value"));

    m_puz->m_isTimerRunning = (isTimerRunning == 0);
    m_puz->m_time = time;
}


// RUSR (rebus-user)
//------------------
void
PuzHandler::SetRUSR(const ByteArray & data)
{
    // RUSR is a series of strings (each nul-terminated) that represent any user
    // grid rebus entries.  If the rebus is a symbol, it is enclosed in '[' ']'.

    ByteArray::const_iterator it = data.begin();

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        wxString str;
        while (*it != '\0')
            str.append(static_cast<wxChar>(*it++));
        ++it;

        if (str.empty())
            continue;

        // Make sure symbols are entered correctly
        if (str.at(0) == _T('['))
        {
            if (str.at(3) != _T(']'))
                throw PuzSectionError(_T("Missing ']' in RUSR section"));

            wxChar num = str.at(1);
            if (num > 255)
                throw PuzSectionError(_T("Invalid entry in RUSR section"));
        }

        square->SetText(str);
    }
    wxASSERT(it == data.end());
}


// RTBL and GRBS (rebus-table, grid-rebus)
//----------------------------------------
void
PuzHandler::SetSolutionRebus(const ByteArray & table, const ByteArray & grid)
{
    // NB: In the grid rebus section (GRBS), the index is 1 greater than the
    // index in the rebus table section (RTBL).

    wxASSERT(grid.size()
             == m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());


    // Read the rebus table (RTBL)
    // Format: index ':' string ';'
    //   - Index is a number, padded to two digits with a space if needed.
    std::map<unsigned char, wxString> rebusTable;

    ByteArray::const_iterator table_it = table.begin();

    while (table_it != table.end())
    {
        wxString key;
        wxString value;

        // Read the index
        while (*table_it != ':')
        {
            key.Append(static_cast<wxChar>(*table_it));
            ++table_it;
        }

        long index;
        if (! key.ToLong(&index))
            throw PuzSectionError(_T("Invalid rebus table key"));

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


    // Set the grid rebus solution
    ByteArray::const_iterator rebus_it = grid.begin();

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (*rebus_it > 0)
        {
            std::map<unsigned char, wxString>::const_iterator it;
            it = rebusTable.find(*rebus_it);
            if (it == rebusTable.end())
                throw PuzSectionError(_T("Invalid value in GRBS section"));

            // Make sure we're not overwriting the plain solution for
            // unscrambling
            if (m_puz->m_grid.IsScrambled())
                square->SetSolution(it->second, square->GetPlainSolution());
            else
                square->SetSolution(it->second);
        }
        ++rebus_it;
    }
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
    ByteArray data;
    data.reserve(m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        data.push_back(square->GetFlag());
    }
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
                     wxString::Format(_T("%d,%d"),
                                      m_puz->m_time,
                                      m_puz->m_isTimerRunning ? 0 : 1) );
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


    // Assemble the grid rebus string
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->HasSolutionRebus())
        {
            rebus.push_back(0);
            continue;
        }

        std::map<wxString, wxByte>::iterator table_it;
        table_it = tableMap.find(square->GetSolution());

        if (table_it != tableMap.end())
        {
            rebus.push_back(table_it->second);
        }
        else // We need to add this entry to the map
        {
            // The grid-rebus section adds 1 for every index
            tableMap[square->GetSolution()] = index + 1;
            rebus.push_back(index + 1);

            // Add to the rebusTable (uses the actual index)
            table.push_string(wxString::Format(_T("%2d"), index));
            table.push_back(':');
            table.push_string(square->GetSolution());
            table.push_back(';');

            ++index;
        }
    }


    if (rebus.has_data() && table.has_data())
    {
        WriteSection(_T("GRBS"), rebus);
        WriteSection(_T("RTBL"), table);
    }
}
