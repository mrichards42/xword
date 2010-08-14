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

#include "Puzzle.hpp"
#include "Checksummer.hpp"
#include "puzstring.hpp"
#include "util.hpp"
#include "StreamWrapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace puz {

static void LoadSections(Puzzle * puz, istream_wrapper & f);

void LoadPuz(Puzzle * puz, const string_t & filename, void * /* dummy */)
{
    std::ifstream stream(filename.c_str(), std::ios::in | std::ios::binary);
    if (stream.fail())
        throw FatalFileError(std::string("Unable to open file: ") + encode_utf8(filename));
    istream_wrapper f(stream);

    const unsigned short c_primary = f.ReadShort();
    if (strcmp(f.ReadString(12).c_str(), "ACROSS&DOWN") != 0)
        throw FileMagicError();

    const unsigned short c_cib = f.ReadShort();
    unsigned char c_masked[8];
    f.ReadCharArray(c_masked, 8);

    // Version is "[major].[minor]\0"
    // We can read puzzles of 1.[anything]
    std::string versionstr = f.ReadString(4);
    if (versionstr[0] != '1' || ! isdigit(versionstr[2]))
        throw VersionError();

    const unsigned short version = 10 + versionstr[2] - 0x30;

    f.Skip(2); // 1 unknown short
    const unsigned short c_grid = f.ReadShort();
    f.Skip(2 * 6); // 6 noise shorts

    const unsigned char width  = f.ReadChar();
    const unsigned char height = f.ReadChar();

    const unsigned short num_clues = f.ReadShort();
    const unsigned short grid_type = f.ReadShort();
    const unsigned short grid_flag = f.ReadShort();

    puz->GetGrid().SetSize(width, height);
    puz->GetGrid().SetCksum(c_grid);
    puz->GetGrid().SetType(grid_type);
    puz->GetGrid().SetFlag(grid_flag);

    // Read user text and solution
    std::string solution = f.ReadString(width * height);
    std::string text     = f.ReadString(width * height);

    // Set the grid's solution and text
    std::string::iterator sol_it  = solution.begin();
    std::string::iterator text_it = text.begin();
    for (Square * square = puz->GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        // Solution
        if (*sol_it == '.' || *sol_it == ':' && puz->m_grid.IsDiagramless())
            square->SetSolution(puz::Square::Black);
        else if (*sol_it == '-')
            square->SetSolution(puz::Square::Blank);
        else
            square->SetSolution(decode_puz(std::string(1, *sol_it)));
        ++sol_it;

        // Text
        if (square->IsBlack() || *text_it == ':' && puz->m_grid.IsDiagramless())
            square->SetText(puz::Square::Black);
        else if (*text_it == '-' || *text_it == 0)
            square->SetText(puz::Square::Blank);
        else
            square->SetText(decode_puz(std::string(1, *text_it)));
            if (islower(*text_it))
                square->AddFlag(FLAG_PENCIL);
        ++text_it;
    }
    assert(sol_it == solution.end() && text_it == text.end());

    // General puzzle info
    puz->SetTitle(decode_puz(f.ReadString()));
    puz->SetAuthor(decode_puz(f.ReadString()));
    puz->SetCopyright(decode_puz(f.ReadString()));

    // Clues
    std::vector<string_t> clues;
    clues.reserve(num_clues);
    // Save unaltered clues for the checksums
    std::vector<std::string> cksum_clues;
    cksum_clues.reserve(num_clues);
    for (size_t i = 0; i < num_clues; ++i)
    {
        cksum_clues.push_back(f.ReadString());
        clues.push_back(decode_puz(cksum_clues.back()));
    }

    puz->SetAllClues(clues);

    // Notes
    std::string notes = f.ReadString();
    puz->SetNotes(decode_puz(notes));

    puz->SetOk(true);

    // Try to load the extra sections (i.e. GEXT, LTIM, etc).
    try {
        LoadSections(puz, f);
    }
    catch (SectionError & err) {
        puz->SetError(err.what());
    }
    catch (std::ios::failure &) {
        puz->SetError("EOF in a section");
        // No error; we just won't process all the sections
    }

    // Test the checksums
    Checksummer cksum;
    cksum.SetVersion(version);
    cksum.SetAuthor(encode_puz(puz->GetAuthor()));
    cksum.SetTitle(encode_puz(puz->GetTitle()));
    cksum.SetCopyright(encode_puz(puz->GetCopyright()));

    // Use clues and notes from the actual file because the Puzzle methods unescape XML chars.
    cksum.SetClues(cksum_clues);
    cksum.SetNotes(notes);

    // Use the grid and solution from the actual file, because Across Lite
    // doesn't save rebus solutions correctly all the time.
    cksum.SetSolution(solution);
    cksum.SetGridText(text);

    cksum.SetWidth(puz->GetGrid().GetWidth());
    cksum.SetHeight(puz->GetGrid().GetHeight());
    cksum.SetGridType(puz->GetGrid().GetType());
    cksum.SetGridFlag(puz->GetGrid().GetFlag());

    // We're going to test both 1.3 and 1.2 as versions because some files
    // with notepads don't have the correct version . . .
    if (! cksum.TestChecksums(c_cib, c_primary, c_masked))
    {
        if (! puz->m_notes.empty())
        {
            cksum.SetVersion( (version == 13 ? 12 : 13) );
            if (! cksum.TestChecksums(c_cib, c_primary, c_masked))
                throw ChecksumError();
        }
        else
            throw ChecksumError();
    }
}


//------------------------------------------------------------------------------
// Load the sections
//------------------------------------------------------------------------------

static void LoadGEXT(Puzzle * puz, const std::string & data);
static void UnLoadGEXT(Puzzle * puz);
static void LoadLTIM(Puzzle * puz, const std::string & data);
static void UnLoadLTIM(Puzzle * puz);
static void LoadRUSR(Puzzle * puz, const std::string & data);
static void UnLoadRUSR(Puzzle * puz);
static void LoadSolutionRebus(Puzzle * puz,
                              const std::string & table,
                              const std::string & grid);
static void UnLoadSolutionRebus(Puzzle * puz);

#define LOAD_SECTION(name)                              \
    data = sections[#name];                             \
    if (! data.empty())                                 \
    {                                                   \
        try {                                           \
            Load##name(puz, data);                      \
        }                                               \
        catch (std::exception & err) {                  \
            puz->SetError(err.what());                  \
            UnLoad##name(puz);                          \
        }                                               \
        catch (...) {                                   \
            puz->SetError("Unknown section error");     \
            UnLoad##name(puz);                          \
        }                                               \
    }                                                   \
    sections.erase(#name);

void LoadSections(Puzzle * puz, istream_wrapper & f)
{
    std::map<std::string, std::string> sections;

    std::string title;
    try {
        // Read all the sections
        // If an error occurs while reading a section, skip
        // that section.
        for (;;)
        {
            // An extra section is defined as:
            // Title (4 chars)
            // Section length   (le-short)
            // Section checksum (le-short)

            try {
                title = f.ReadString(4);
            }
            catch(std::ios::failure &) {
                break;
            }

            unsigned short length = f.ReadShort();
            unsigned short c_section = f.ReadShort();

            std::string data = f.ReadString(length);

            // Check the nul-terminator
            if (f.ReadChar() != 0)
                throw SectionError("Missing nul-terminator in an extra section");

            // Test the checksum
            if (c_section != Checksummer::cksum_region(data, 0))
            {
                puz->SetError(std::string("Bad checksum in puz section: ") + title);
                // This is not an exception, we just won't process the section
                continue;
            }

            sections[title] = data;
        }
    }
    catch(std::ios::failure &) {
        if (title.empty() || title.at(0) == 0)
            puz->SetError("EOF in a puz section");
        else
            puz->SetError(std::string("EOF in puz section: ") + title);
    }

    // Fill in the puzzle data
    std::string data;

    LOAD_SECTION(GEXT)
    LOAD_SECTION(LTIM)
    LOAD_SECTION(RUSR)

    // Solution rebus needs RTBL and GRBS
    std::string table = sections["RTBL"];
    data = sections["GRBS"];
    if (! data.empty() && ! table.empty())
    {
        try {
            LoadSolutionRebus(puz, table, data);
        }
        catch (...) {
            UnLoadSolutionRebus(puz);
        }
        sections.erase("RTBL");
        sections.erase("GRBS");
    }
    if (data.empty())
        sections.erase("GRBS");
    if (table.empty())
        sections.erase("RTBL");

    // Add the remaining unknown sections
    for (std::map<std::string, std::string>::iterator it = sections.begin();
         it != sections.end();
         ++it)
    {
        puz->m_extraSections.push_back(Puzzle::section(it->first, it->second));
    }
}

#undef LOAD_SECTION


//------------------------------------------------------------------------------
// GEXT (square flags)
//------------------------------------------------------------------------------

void LoadGEXT(Puzzle * puz, const std::string & data)
{
    std::istringstream stream(data);
    istream_wrapper f(stream);

    std::string::const_iterator it = data.begin();
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetFlag(f.ReadChar());
    }
    if (! f.CheckEof())
        throw SectionError("Too Many values in GEXT");
}

// Rollback changes
void UnLoadGEXT(Puzzle * puz)
{
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetFlag(FLAG_CLEAR);
    }
}

//------------------------------------------------------------------------------
// LTIM (timer)
//------------------------------------------------------------------------------

void LoadLTIM(Puzzle * puz, const std::string & data)
{
    std::istringstream stream(data);
    istream_wrapper f(stream);

    // Split the string at the ','
    const std::string timestring = f.ReadString(',');

    int time = atoi(timestring.c_str());
    if (time == 0 && ! timestring.empty() && timestring[0] != '0')
        throw SectionError("Incorrect LTIM value");

    const std::string runningstring = f.ReadString(-1);
    int isTimerRunning = atoi(runningstring.c_str());
    if (isTimerRunning == 0 && ! runningstring.empty() && runningstring[0] != '0')
        throw SectionError("Incorrect LTIM value");

    puz->m_isTimerRunning = (isTimerRunning == 0);
    puz->m_time = time;
}

// Rollback changes
void UnLoadLTIM(Puzzle * puz)
{
    puz->m_isTimerRunning = false;
    puz->m_time = 0;
}


//------------------------------------------------------------------------------
// RUSR (user rebus grid)
//------------------------------------------------------------------------------
void LoadRUSR(Puzzle * puz, const std::string & data)
{
    // RUSR is a series of strings (each nul-terminated) that represent any
    // user grid rebus entries.  If the rebus is a symbol, it is enclosed
    // in '[' ']'.

    std::istringstream stream(data);
    istream_wrapper f(stream);

    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        std::string str = f.ReadString();

        if (str.empty())
            continue;

        try
        {
            square->SetText(decode_puz(str));
        }
        catch (InvalidString &)
        {
            // Try to press on anyways
        }
    }
    if (! f.CheckEof())
        throw SectionError("Too many values in RUSR");
}

void UnLoadRUSR(Puzzle * puz)
{
    // Nothing to do here really . . .
    // We'll just have to stick with partially complete user rebus
}


//------------------------------------------------------------------------------
// RTBL and GRBS (solution grid)
//------------------------------------------------------------------------------

void LoadSolutionRebus(Puzzle * puz,
                       const std::string & table,
                       const std::string & grid)
{
    // NB: In the grid rebus section (GRBS), the index is 1 greater than the
    // index in the rebus table section (RTBL).

    if (grid.size() !=  puz->m_grid.GetWidth() * puz->m_grid.GetHeight())
        throw SectionError("GRBS size is not equal to the grid size");

    // Read the rebus table (RTBL)
    // Format: index ':' string ';'
    //   - Index is a number, padded to two digits with a space if needed.
    std::map<unsigned char, std::string> rebusTable;

    std::istringstream tstream(table);
    istream_wrapper table_stream(tstream);

    for (;;)
    {
        std::string key;

        // Read the index
        try {
            key = table_stream.ReadString(':');
        }
        catch(std::ios::failure &) {
            break;
        }

        int index = atoi(key.c_str());
        if (index == 0 && key != " 0")
            throw SectionError("Invalid rebus table key");

        // The index value in the rebus-table section is 1 less than the
        // index in the grid-rebus, so we need add 1 here.
        ++index;

        std::string value = table_stream.ReadString(';');

        rebusTable[static_cast<unsigned char>(index)] = value;
    }
    if (! table_stream.CheckEof())
        throw SectionError("Poorly formed RTBL");


    // Set the grid rebus solution
    std::istringstream gstream(grid);
    istream_wrapper grid_stream(gstream);

    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        const unsigned char index = grid_stream.ReadChar();
        if (index > 0)
        {
            // Look for this index in the rebus table
            std::map<unsigned char, std::string>::const_iterator it;
            it = rebusTable.find(index);
            if (it == rebusTable.end())
                throw SectionError("Invalid value in GRBS section");

            // Don't overwrite the plain solution
            square->SetSolutionRebus(decode_puz(it->second));
        }
    }
    if (! grid_stream.CheckEof())
        throw SectionError("Too many values in GRBS section");
}

void UnLoadSolutionRebus(Puzzle * puz)
{
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetSolutionRebus(string_t(1, char_t(square->GetPlainSolution())));
    }
}

} // namespace puz
