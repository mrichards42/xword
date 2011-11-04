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

#include "puz.hpp"

#include "Puzzle.hpp"
#include "Checksummer.hpp"
#include "puzstring.hpp"
#include "utils/streamwrapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

namespace puz {

static void LoadSections(Puzzle * puz, istream_wrapper & f);

void LoadPuz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    std::ifstream stream(filename.c_str(), std::ios::in | std::ios::binary);
    if (stream.fail())
        throw FileError(filename);
    istream_wrapper f(stream);

    const unsigned short c_primary = f.ReadShort();
    if (strcmp(f.ReadString(12).c_str(), "ACROSS&DOWN") != 0)
        throw FileTypeError("puz");

    const unsigned short c_cib = f.ReadShort();
    unsigned char c_masked[8];
    f.ReadCharArray(c_masked, 8);

    // Version is "[major].[minor]\0"
    // We can read puzzles of 1.[anything]
    std::string versionstr = f.ReadString(4);
    if (versionstr[0] != '1' || ! isdigit(versionstr[2]))
        throw LoadError("Unknown puz version.");

    const unsigned short version = 10 + versionstr[2] - 0x30;

    f.Skip(2); // 1 unknown short
    const unsigned short c_grid = f.ReadShort();
    f.Skip(2 * 6); // 6 noise shorts

    const unsigned char width  = f.ReadChar();
    const unsigned char height = f.ReadChar();

    const unsigned short num_clues = f.ReadShort();
    const unsigned short grid_type = f.ReadShort();
    const unsigned short grid_flag = f.ReadShort();

    puz->GetGrid().SetCksum(c_grid);
    puz->GetGrid().SetType(grid_type);
    puz->GetGrid().SetFlag(grid_flag);
    puz->GetGrid().SetSize(width, height);

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
        if (square->IsBlack() && ! puz->m_grid.IsDiagramless())
            square->SetText(puz::Square::Black);
        else if (*text_it == '-' || *text_it == 0)
            square->SetText(puz::Square::Blank);
        else if (puz->m_grid.IsDiagramless() && (*text_it == '.' || *text_it == ':'))
        {
            // Black squares in a diagramless puzzle.
            if (*text_it == '.')
                square->SetText(puz::Square::Black);
            else if (*text_it == ':')
                square->SetText(puz::Square::Blank);
        }
        else
        {
            square->SetText(decode_puz(std::string(1, *text_it)));
            if (islower(*text_it))
                square->AddFlag(FLAG_PENCIL);
        }
        ++text_it;
    }
    assert(sol_it == solution.end() && text_it == text.end());
    puz->NumberGrid();

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
        clues.push_back(escape_xml(decode_puz(cksum_clues.back())));
    }

    puz->SetAllClues(clues);

    // Notes
    std::string notes = f.ReadString();
    puz->SetNotes(escape_xml(decode_puz(notes)));

    puz->SetOk(true);

    // Try to load the extra sections (i.e. GEXT, LTIM, etc).
    try {
        LoadSections(puz, f);
    }
    catch (std::ios::failure &) {
        // EOF here doesn't matter.
    }


    // Don't even bother with the checksums, since we check the validity
    // of the puzzle anyways
}


//------------------------------------------------------------------------------
// Load the sections
//------------------------------------------------------------------------------

static bool LoadGEXT(Puzzle * puz, const std::string & data);
static void UnLoadGEXT(Puzzle * puz);
static bool LoadCHKD(Puzzle * puz, const std::string & data);
static void UnLoadCHKD(Puzzle * puz);
static bool LoadLTIM(Puzzle * puz, const std::string & data);
static void UnLoadLTIM(Puzzle * puz);
static bool LoadRUSR(Puzzle * puz, const std::string & data);
static void UnLoadRUSR(Puzzle * puz);
static bool LoadSolutionRebus(Puzzle * puz,
                              const std::string & table,
                              const std::string & grid);
static void UnLoadSolutionRebus(Puzzle * puz);

#define LOAD_SECTION(name)                              \
    data = sections[#name];                             \
    if (! data.empty())                                 \
        if (! Load##name(puz, data))                    \
            UnLoad##name(puz);                          \
    sections.erase(#name);

void LoadSections(Puzzle * puz, istream_wrapper & f)
{
    std::map<std::string, std::string> sections;

    std::string title;
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
        } catch (std::ios::failure &) {
            break;
        }

        unsigned short length = f.ReadShort();
        unsigned short c_section = f.ReadShort();

        std::string data = f.ReadString(length);

        // Check the nul-terminator
        if (f.ReadChar() != 0)
            break; // Don't throw an error

        // Test the checksum
        if (c_section != Checksummer::cksum_region(data, 0))
            continue; // Skip this section.

        sections[title] = data;
    }

    // Fill in the puzzle data
    std::string data;

    LOAD_SECTION(GEXT)
    LOAD_SECTION(CHKD)
    LOAD_SECTION(LTIM)
    LOAD_SECTION(RUSR)

    // Solution rebus needs RTBL and GRBS
    std::string table = sections["RTBL"];
    data = sections["GRBS"];
    if (! data.empty() && ! table.empty())
    {
        if (! LoadSolutionRebus(puz, table, data))
            UnLoadSolutionRebus(puz);
        sections.erase("RTBL");
        sections.erase("GRBS");
    }
    if (data.empty())
        sections.erase("GRBS");
    if (table.empty())
        sections.erase("RTBL");

    // Add the remaining unknown sections
    // If an exception is thrown, Puzzle::LoadPuzzle will clean up this pointer
    PuzData * extra = new PuzData;
    puz->SetFormatData(extra);
    std::map<std::string, std::string>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it)
        extra->extraSections.push_back(*it);
}

#undef LOAD_SECTION


//------------------------------------------------------------------------------
// GEXT (square flags)
//------------------------------------------------------------------------------

bool LoadGEXT(Puzzle * puz, const std::string & data)
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
        return false;
    return true;
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
// CHKD (correct squares) // This is my own extension
//------------------------------------------------------------------------------

bool LoadCHKD(Puzzle * puz, const std::string & data)
{
    std::istringstream stream(data);
    istream_wrapper f(stream);

    std::string::const_iterator it = data.begin();
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (f.ReadChar() != 0)
            square->AddFlag(FLAG_CORRECT);
    }
    if (! f.CheckEof())
        return false;
    return true;
}

// Rollback changes
void UnLoadCHKD(Puzzle * puz)
{
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->RemoveFlag(FLAG_CORRECT);
    }
}

//------------------------------------------------------------------------------
// LTIM (timer)
//------------------------------------------------------------------------------

bool LoadLTIM(Puzzle * puz, const std::string & data)
{
    std::istringstream stream(data);
    istream_wrapper f(stream);

    // Split the string at the ','
    const std::string timestring = f.ReadString(',');

    int time = atoi(timestring.c_str());
    if (time == 0 && ! timestring.empty() && timestring[0] != '0')
        return false;

    const std::string runningstring = f.ReadString(-1);
    int isTimerRunning = atoi(runningstring.c_str());
    if (isTimerRunning == 0 && ! runningstring.empty() && runningstring[0] != '0')
        return false;

    puz->m_isTimerRunning = (isTimerRunning == 0);
    puz->m_time = time;
    return true;
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
bool LoadRUSR(Puzzle * puz, const std::string & data)
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

        square->SetText(decode_puz(str));
    }
    if (! f.CheckEof())
        return false;
    return true;
}

void UnLoadRUSR(Puzzle * puz)
{
    // Nothing to do here really . . .
    // We'll just have to stick with partially complete user rebus
}


//------------------------------------------------------------------------------
// RTBL and GRBS (solution grid)
//------------------------------------------------------------------------------

bool LoadSolutionRebus(Puzzle * puz,
                       const std::string & table,
                       const std::string & grid)
{
    // NB: In the grid rebus section (GRBS), the index is 1 greater than the
    // index in the rebus table section (RTBL).

    if (grid.size() !=  puz->m_grid.GetWidth() * puz->m_grid.GetHeight())
        return false;

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
            return false;

        // The index value in the rebus-table section is 1 less than the
        // index in the grid-rebus, so we need add 1 here.
        ++index;

        std::string value = table_stream.ReadString(';');

        rebusTable[static_cast<unsigned char>(index)] = value;
    }
    if (! table_stream.CheckEof())
        return false;


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
                return false;

            // Don't overwrite the plain solution
            square->SetSolutionRebus(decode_puz(it->second));
        }
    }
    if (! grid_stream.CheckEof())
        return false;
    return true;
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
