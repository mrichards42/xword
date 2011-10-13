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

#include "txt.hpp"

#include "Puzzle.hpp"
#include "puzstring.hpp"
#include "utils/streamwrapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

namespace puz {

void LoadTxt(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    std::ifstream stream(filename.c_str(), std::ios::in | std::ios::binary);
    if (stream.fail())
        throw FileError(filename);
    istream_wrapper f(stream);

    std::string version = f.ReadLine();
    if (version != "<ACROSS PUZZLE>" && version != "<ACROSS PUZZLE V2>")
        throw FileTypeError("txt");

    if (f.ReadLine() != "<TITLE>")
        throw LoadError("Missing <TITLE>");
    puz->SetTitle(TrimWhitespace(decode_utf8(f.ReadLine())));

    if (f.ReadLine() != "<AUTHOR>")
        throw LoadError("Missing <AUTHOR>");
    puz->SetAuthor(TrimWhitespace(decode_utf8(f.ReadLine())));

    if (f.ReadLine() != "<COPYRIGHT>")
        throw LoadError("Missing <COPYRIGHT>");
    string_t copyright = TrimWhitespace(decode_utf8(f.ReadLine()));
    if (! copyright.empty())
    {
        // Add the copyright symbol
        puz->SetCopyright(decode_utf8("\xc2\xa9 ") + copyright);
    }

    if (f.ReadLine() != "<SIZE>")
        throw LoadError("Missing <SIZE>");
    string_t size = TrimWhitespace(decode_utf8(f.ReadLine()));
    size_t x_index = size.find(puzT("x"));
    if (x_index == string_t::npos)
        throw LoadError("Missing 'x' in size specification");
    int width = ToInt(size.substr(0, x_index));
    int height = ToInt(size.substr(x_index + 1));
    if (width == -1 || height == -1)
        throw LoadError("Improper size specification.");
    puz->GetGrid().SetSize(width, height);


    // Read the grid into a string
    if (f.ReadLine() != "<GRID>")
        throw LoadError("Missing <GRID>");

    string_t solution;
    solution.reserve(width * height);
    bool mark = false;
    typedef std::pair<string_t, char> solution_pair;
    std::map<char_t, solution_pair> rebus_map;

    for (size_t i = 0; i < height; ++i)
        solution.append(TrimWhitespace(decode_utf8(f.ReadLine())));
    if (solution.size() < width * height)
        throw LoadError("Not enough squares in the grid");
    else if (solution.size() > width * height)
        throw LoadError("Too many squares in the grid");

    // Look for the rebus section
    std::string line = f.ReadLine();
    if (line == "<REBUS>")
    {
        line = f.ReadLine();
        if (line.find(';'))
        {
            // Flags line (only MARK is supported)
            if (line.find("MARK") != std::string::npos)
                mark = true;
            line = f.ReadLine();
        }
        // Markers . . . format is  ID:Long:Short
        for (;;)
        {
            size_t delim1 = line.find(':');
            if (delim1 == string_t::npos)
                break;
            size_t delim2 = line.find(':', delim1 + 1);
            if (delim2 == string_t::npos)
                break;
            string_t key = TrimWhitespace(decode_utf8(line.substr(0, delim1)));
            string_t long_solution = TrimWhitespace(decode_utf8(line.substr(delim1 + 1, delim2 - delim1 - 1)));
            string_t short_solution = TrimWhitespace(decode_utf8(line.substr(delim2 + 1)));
            if (! key.empty() && ! long_solution.empty())
            {
                if (short_solution.empty())
                    short_solution = long_solution;
                rebus_map[key[0]] = solution_pair(
                        long_solution, static_cast<char>(short_solution[0]));
            }
            line = f.ReadLine();
        }
    }

    // Set the solution
    string_t::iterator it;
    Square * square = puz->GetGrid().First();
    for (it = solution.begin(); it != solution.end(); ++it)
    {
        // Look in the rebus map
        char_t ch = *it;
        std::map<char_t, solution_pair>::iterator rebus = rebus_map.find(ch);
        if (rebus != rebus_map.end())
        {
            square->SetSolution(rebus->second.first, rebus->second.second);
        }
        else if (ch == puzT('.'))
        {
            square->SetSolution(Square::Black);
        }
        // Circle the square if it is lower case
        else if (mark && islower(static_cast<int>(ch)))
        {
            square->SetSolution(string_t(1, ch));
            square->SetCircle();
        }
        // Set the solution normally
        else
        {
            square->SetSolution(string_t(1, ch));
        }
        square = square->Next();
    }

    // Clues
    // Turn off exceptions on EOF, since we might reach EOF when reading
    // clues and notepad
    f.m_stream.exceptions(0);
    if (line != "<ACROSS>")
        throw LoadError("Missing <ACROSS>");
    ClueList & across = puz->SetClueList(puzT("Across"), ClueList());
    for (line = f.ReadLine(); line != "<DOWN>" && ! line.empty(); line = f.ReadLine())
        across.push_back(Clue(puzT(""), TrimWhitespace(decode_utf8(line))));

    if (line != "<DOWN>")
        throw LoadError("Missing <DOWN>");
    ClueList & down = puz->SetClueList(puzT("Down"), ClueList());
    for (line = f.ReadLine(); line != "<NOTEPAD>" && ! line.empty(); line = f.ReadLine())
        down.push_back(Clue(puzT(""), TrimWhitespace(decode_utf8(line))));

    puz->NumberGrid();
    puz->NumberClues();
    puz->GenerateWords();

    // The rest of the puzzle is notepad
    if (line == "<NOTEPAD>")
    {
        string_t notes;
        for (line = f.ReadLine(); ! line.empty(); line = f.ReadLine())
            notes.append(decode_utf8(line));
        puz->SetNotes(notes);
    }

    puz->SetOk(true);
}

} // namespace puz
