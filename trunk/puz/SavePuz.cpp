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
#include "puzstring.hpp"
#include "Checksummer.hpp"
#include "StreamWrapper.hpp"
#include "util.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace puz {

static const int SAVE_VERSION = 13;
static const char * SAVE_VERSION_STRING = "1.3\0";

static void SaveSections(Puzzle * puz, ostream_wrapper & f);

void SavePuz(Puzzle * puz, const string_t & filename, void * /* dummy */)
{
    // Make sure we can actually save this puzzle
    // Conditions:
    //    * No blank solutions
    //    * All squares that are at the start
    //      of a word must have a clue
    //    * Only flags that work with Across Lite
    //        * No colored squares
    //        * No missing squares

    for (const Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->IsSolutionBlank() ||
            square->GetFlag() & ~ ACROSS_LITE_MASK ||
            (square->GetWordStart(ACROSS) == square &&
                square->GetWordEnd(ACROSS) != square &&
                ! square->HasClue(ACROSS)) ||
            (square->GetWordStart(DOWN) == square &&
                square->GetWordEnd(DOWN) != square &&
                ! square->HasClue(DOWN)))
        {
            throw ConversionError();
        }
    }

    // Checksums
    unsigned short c_cib;
    unsigned short c_primary;
    unsigned char c_masked[8];

    // This will check to make sure we have no formatted clues or notes.
    Checksummer cksum(*puz, SAVE_VERSION);
    cksum.GetChecksums(&c_cib, &c_primary, c_masked);

    const std::vector<std::string> & clues = cksum.GetClues();

    std::ofstream stream(filename.c_str(), std::ios::out | std::ios::binary);
    if (stream.fail())
        throw FatalFileError(std::string("Unable to open file: ") + encode_utf8(filename));
    ostream_wrapper f(stream);

    // Header
    f.Write(c_primary);
    f.Write("ACROSS&DOWN\0", 12);
    f.Write(c_cib);
    f.Write(&c_masked[0], 8);
    f.Write(SAVE_VERSION_STRING, 4);
    f.Skip(2); // 1 unknown short
    f.Write(puz->m_grid.GetCksum());
    f.Skip(12); // 6 noise shorts

    // Puzzle information
    f.Put(puz->m_grid.GetWidth());
    f.Put(puz->m_grid.GetHeight());

    f.Write(clues.size());

    f.Write(puz->m_grid.GetType());
    f.Write(puz->m_grid.GetFlag());

    // Puzzle data
    f.Write(cksum.GetSolution()); // Checksummer has already calculated
    f.Write(cksum.GetGridText()); // these, so we'll reused them.
    f.WriteNulTerminated(encode_puz(puz->m_title));
    f.WriteNulTerminated(encode_puz(puz->m_author));
    f.WriteNulTerminated(encode_puz(puz->m_copyright));

    std::vector<std::string>::const_iterator it;
    for (it = clues.begin(); it != clues.end(); ++it)
        f.WriteNulTerminated(*it);

    f.WriteNulTerminated(cksum.GetNotes());

    SaveSections(puz, f);
}


static void WriteGEXT(Puzzle * puz, ostream_wrapper & f);
static void WriteLTIM(Puzzle * puz, ostream_wrapper & f);
static void WriteRUSR(Puzzle * puz, ostream_wrapper & f);
static void WriteSolutionRebus(Puzzle * puz, ostream_wrapper & f);
static void WriteSection(ostream_wrapper & f,
                         const std::string & name,
                         const std::string & data);

void SaveSections(Puzzle * puz, ostream_wrapper & f)
{
    WriteGEXT(puz, f);
    WriteLTIM(puz, f);
    WriteRUSR(puz, f);
    WriteSolutionRebus(puz, f);

    // Write the unknown sections
    std::vector<Puzzle::section>::iterator it;
    for (it  = puz->m_extraSections.begin();
         it != puz->m_extraSections.end();
         ++it)
    {
        WriteSection(f, it->name, it->data);
    }

}


void WriteSection(ostream_wrapper & f,
                  const std::string & name,
                  const std::string & data)
{
    f.Write(name);
    f.Write(data.size());
    f.Write(Checksummer::cksum_region(data, 0));
    f.WriteNulTerminated(data);
}


void WriteGEXT(Puzzle * puz, ostream_wrapper & f)
{
    std::string data;
    data.reserve(puz->m_grid.GetWidth() * puz->m_grid.GetHeight());
    bool hasData = false;

    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        const char flag = square->GetFlag() & ACROSS_LITE_MASK;
        data.push_back(flag);
        if (! hasData && flag != 0)
            hasData = true;
    }
    if (hasData)
        WriteSection(f, "GEXT", data);
}


void WriteLTIM(Puzzle * puz, ostream_wrapper & f)
{
    if (puz->m_time == 0 && ! puz->m_isTimerRunning)
        return;
    std::ostringstream data;
    data << puz->m_time << "," << (puz->m_isTimerRunning ? 0 : 1);
    WriteSection(f, "LTIM", data.str());
}


void WriteRUSR(Puzzle * puz, ostream_wrapper & f)
{
    std::string data;
    bool hasData = false;

    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasTextRebus())
        {
            data.append(encode_puz(square->GetText()));
            if (! hasData)
                hasData = true;
        }
        data.push_back('\0');
    }
    if (hasData)
        WriteSection(f, "RUSR", data);
}



void WriteSolutionRebus(Puzzle * puz, ostream_wrapper & f)
{
    std::map<string_t, unsigned char> tableMap;
    std::string rebus;
    std::ostringstream table; // Write this as we go
    bool rebusHasData = false;
    bool tableHasData = false;

    // Using a starting index like this works, but it's not quite accurate
    // I think Across lite loads the rebus-table into memory and uses it
    // like a symbol table, so that the index for a given string is
    // preserved across saves.
    // I doubt that feature is essential, and it's a lot easier to just
    // remake the table starting from 1 when we need it.
    unsigned char index = 1;


    // Assemble the grid rebus string
    for (Square * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->HasSolutionRebus())
        {
            rebus.push_back(0);
            continue;
        }

        if (! rebusHasData)
            rebusHasData = true;

        // See if this rebus entry is already in the table.
        std::map<string_t, unsigned char>::iterator table_it;
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

            if (! tableHasData)
                tableHasData = true;

            // Add to the rebusTable (uses the actual index)
            table.width(2);
            table << static_cast<unsigned short>(index);
            table.width(1);
            table << ':';
            table << encode_puz(square->GetSolution());
            table << ';';

            ++index;
        }
    }


    if (rebusHasData && tableHasData)
    {
        WriteSection(f, "GRBS", rebus);
        WriteSection(f, "RTBL", table.str());
    }
}

} // namespace puz
