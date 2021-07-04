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
#include "puzstring.hpp"
#include "Checksummer.hpp"
#include "utils/streamwrapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace puz {

static const int SAVE_VERSION = 13;
static const char * SAVE_VERSION_STRING = "1.3\0";

static const int UTF8_SAVE_VERSION = 20;
static const char * UTF8_SAVE_VERSION_STRING = "2.0\0";

static void SaveSections(Puzzle * puz, ostream_wrapper & f);

void SavePuz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    // Make sure we can actually save this puzzle
    // Conditions:
    //    * No blank solutions
    //    * All squares that are at the start
    //      of a word must have a clue
    //    * Only flags that work with Across Lite
    //        * No colored squares
    //        * No missing squares

    if (! puz->UsesNumberAlgorithm())
        throw ConversionError("This puzzle uses features not supported in a .puz file.");

    for (const Square * square = puz->GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        if (square->IsSolutionBlank()
            || square->GetFlag() & ~ (ACROSS_LITE_MASK | FLAG_CORRECT | FLAG_THEME)
            || square->HasImage())
        {
            throw ConversionError();
        }
    }

    // If any of the text strings use UTF-8, then we save as version 2.0. Otherwise, we use 1.3 for
    // wider compatibility. For simplicity, check all metadata, even if it doesn't end up being
    // saved in the .puz.
    bool uses_utf8 = false;
    Puzzle::metamap_t& meta = puz->GetMetadata();
    Puzzle::metamap_t::iterator metamap_it;
    for (metamap_it = meta.begin(); metamap_it != meta.end(); ++metamap_it)
    {
        if (!can_encode_puz(metamap_it->second)) {
            uses_utf8 = true;
            break;
        }
    }
    if (!uses_utf8) {
        Clues clues = puz->GetClues();
        Clues::iterator cluelist_it;
        for (cluelist_it = clues.begin(); cluelist_it != clues.end(); ++cluelist_it)
        {
            ClueList& cluelist = cluelist_it->second;
            ClueList::iterator clue;
            for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
            {
                if (!can_encode_puz(clue->GetText())) {
                    uses_utf8 = true;
                    break;
                }
            }
            if (uses_utf8)
                break;
        }
    }

    int save_version;
    const char * save_version_string;
    if (uses_utf8)
    {
        save_version = UTF8_SAVE_VERSION;
        save_version_string = UTF8_SAVE_VERSION_STRING;
    }
    else
    {
        save_version = SAVE_VERSION;
        save_version_string = SAVE_VERSION_STRING;
    }

    // Checksums
    unsigned short c_cib;
    unsigned short c_primary;
    unsigned char c_masked[8];

    // This will check to make sure we have no formatted clues or notes.
    Checksummer cksum(*puz, save_version);
    cksum.GetChecksums(&c_cib, &c_primary, c_masked);

    const std::vector<std::string> & clues = cksum.GetClues();

    std::ofstream stream(filename.c_str(), std::ios::out | std::ios::binary);
    if (stream.fail())
        throw FileError(filename);
    ostream_wrapper f(stream);

    // Header
    f.Write(c_primary);
    f.Write("ACROSS&DOWN\0", 12);
    f.Write(c_cib);
    f.Write(&c_masked[0], 8);
    f.Write(save_version_string, 4);
    f.Skip(2); // 1 unknown short
    f.Write(puz->GetGrid().GetCksum());
    f.Skip(12); // 6 noise shorts

    // Puzzle information
    f.Put(puz->GetGrid().GetWidth());
    f.Put(puz->GetGrid().GetHeight());

    f.Write(clues.size());

    f.Write(puz->GetGrid().GetType());
    f.Write(puz->GetGrid().GetFlag());

    // Puzzle data, as already calculated by Checksummer
    f.Write(cksum.GetSolution());
    f.Write(cksum.GetGridText());
    f.WriteNulTerminated(cksum.GetTitle());
    f.WriteNulTerminated(cksum.GetAuthor());
    f.WriteNulTerminated(cksum.GetCopyright());

    std::vector<std::string>::const_iterator it;
    for (it = clues.begin(); it != clues.end(); ++it)
        f.WriteNulTerminated(*it);

    f.WriteNulTerminated(cksum.GetNotes());

    SaveSections(puz, f);
}


static void WriteGEXT(Puzzle * puz, ostream_wrapper & f);
static void WriteCHKD(Puzzle * puz, ostream_wrapper & f);
static void WriteLTIM(Puzzle * puz, ostream_wrapper & f);
static void WriteRUSR(Puzzle * puz, ostream_wrapper & f);
static void WriteSolutionRebus(Puzzle * puz, ostream_wrapper & f);
static void WriteSection(ostream_wrapper & f,
                         const std::string & name,
                         const std::string & data);

void SaveSections(Puzzle * puz, ostream_wrapper & f)
{
    WriteGEXT(puz, f);
    WriteCHKD(puz, f);
    WriteLTIM(puz, f);
    WriteRUSR(puz, f);
    WriteSolutionRebus(puz, f);

    // Write the unknown sections
    PuzData * data = dynamic_cast<PuzData *>(puz->GetFormatData());
    if (data)
    {
        std::vector< std::pair<std::string, std::string> >::iterator it;
        for (it  = data->extraSections.begin();
             it != data->extraSections.end();
             ++it)
        {
            WriteSection(f, it->first, it->second);
        }
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
    data.reserve(puz->GetGrid().GetWidth() * puz->GetGrid().GetHeight());
    bool hasData = false;

    for (Square * square = puz->GetGrid().First();
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



void WriteCHKD(Puzzle * puz, ostream_wrapper & f)
{
    // Checked square data
    std::string data;
    data.reserve(puz->GetGrid().GetWidth() * puz->GetGrid().GetHeight());
    bool hasData = false;

    for (Square * square = puz->GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        const bool correct = square->HasFlag(FLAG_CORRECT);
        data.push_back(correct ? 1 : 0);
        if (! hasData && correct)
            hasData = true;
    }
    if (hasData)
        WriteSection(f, "CHKD", data);
}



void WriteLTIM(Puzzle * puz, ostream_wrapper & f)
{
    if (puz->GetTime() == 0 && ! puz->IsTimerRunning())
        return;
    std::ostringstream data;
    data << puz->GetTime() << "," << (puz->IsTimerRunning() ? 0 : 1);
    WriteSection(f, "LTIM", data.str());
}


void WriteRUSR(Puzzle * puz, ostream_wrapper & f)
{
    std::string data;
    bool hasData = false;

    for (Square * square = puz->GetGrid().First();
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
    for (Square * square = puz->GetGrid().First();
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
