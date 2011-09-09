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

#include "ipuz.hpp"

#include "Puzzle.hpp"
#include "puzstring.hpp"
#include "parse/json.hpp"

namespace puz {

class ipuzParser : public json::Parser
{
public:
    virtual bool DoLoadPuzzle(Puzzle * puz, json::Value * root);
};

void LoadIpuz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    std::ifstream stream(filename.c_str());
    if (stream.fail())
        throw FatalFileError(std::string("Unable to open file: ") + filename);
    // Throw away "ipuz("
    char test[6];
    stream.read(test, 5);
    test[5] = '\0';
    if (std::string("ipuz(") != test)
        stream.seekg(0);
    // Load the file
    ipuzParser parser;
    parser.LoadPuzzle(puz, stream);
}

// Parse an ipuz enumeration value and return a string to append to
// a clue, in the format "n n n ..."
string_t ParseEnumeration(const string_t & str)
{
    // Find an unescaped character
    struct _find_first_of_unescaped
    {
        _find_first_of_unescaped(const string_t & str, char_t esc)
            : str(str),
              escape_char(esc)
        {}

        size_t operator()(const string_t & chars, size_t start_index = 0)
        {
            size_t index = start_index;
            while (index != string_t::npos)
            {
                index = str.find_first_of(chars, index);
                if (index == string_t::npos || index == start_index)
                    break;
                else if (str[index - 1] != escape_char)
                    break;
                else // Index was escaped
                    ++index; // Search again, starting from the next character.
            }
            return index;
        }

        char_t escape_char;
        const string_t & str;

    } find_first_of_unescaped(str, puzT('`'));

    // Find the end of the cue (the first unescaped =)
    size_t start = find_first_of_unescaped(puzT("="));
    if (start == string_t::npos)
        start = 0;
    else
        ++start;

    string_t ret;
    while (start != string_t::npos)
    {
        // Look for an unescaped number
        start = find_first_of_unescaped(puzT("1234567890"), start);
        size_t end = str.find_first_not_of(puzT("1234567890"), start);
        if (start != end)
            ret.append(str.substr(start, end-start)).append(puzT(" "));
        start = end;
    }
    if (! ret.empty())
        return ret.substr(0, ret.size() - 1); // Extra space at the end
    return ret;
}


bool ipuzParser::DoLoadPuzzle(Puzzle * puz, json::Value * root)
{
    json::Map * doc = root->AsMap();
    // Check version
    if (doc->PopString(puzT("version")) != puzT("http://ipuz.org/v1"))
        throw VersionError("Unreadable ipuz version");
    // Check kind
    string_t kind = doc->PopArray(puzT("kind"))->at(0)->AsString();
    if (kind.substr(kind.size() - 2) == puzT("#1"))
        kind = kind.substr(0, kind.size() - 2);
    if (kind != puzT("http://ipuz.org/crossword"))
        throw VersionError("Not a crossword puzzle");

    // Metadata
    puz->SetTitle(doc->PopString(puzT("title"), puzT("")));
    puz->SetAuthor(doc->PopString(puzT("author"), puzT("")));
    puz->SetCopyright(doc->PopString(puzT("copyright"), puzT("")));
    puz->SetNotes(doc->PopString(puzT("intro"), puzT(""))
                  + doc->PopString(puzT("notes"), puzT("")));

    // Grid
    string_t block_str = doc->PopString(puzT("block"), puzT("#"));
    string_t empty_str = doc->PopString(puzT("empty"), puzT("0"));

    json::Map * dim = doc->GetMap(puzT("dimensions"));
    puz->GetGrid().SetSize(
        ToInt(dim->GetNumber(puzT("width"))),
        ToInt(dim->GetNumber(puzT("height")))
    );
    // Set all squares to missing to start
    {
        for (Square * square = puz->GetGrid().First();
             square != NULL;
             square = square->Next())
        {
            square->SetMissing();
        }
    }

    // Iterating the json grid is kind of a pain, and this ugly macro makes it
    // a bit easier.
#define GRID_FOREACH(grid_val, block)                                        \
    {                                                                        \
        json::Array * grid = grid_val;                                       \
        size_t r = 0;                                                        \
        json::Array::iterator row_it;                                        \
        for (row_it = grid->begin(); row_it != grid->end(); ++row_it)        \
        {                                                                    \
            size_t c = 0;                                                    \
            json::Array * row = (*row_it)->AsArray();                        \
            json::Array::iterator cell_it;                                   \
            for (cell_it = row->begin(); cell_it != row->end(); ++cell_it)   \
            {                                                                \
                json::Value * cell = *cell_it;                               \
                Square & square = puz->GetGrid().At(c, r);                   \
                block;                                                       \
                ++c;                                                         \
            }                                                                \
            ++r;                                                             \
        }                                                                    \
    }

    // Block / clue number layout
    GRID_FOREACH(doc->GetArray(puzT("puzzle")),
    {
        if (! cell->IsNull())
        {
            square.SetMissing(false);
            string_t val;
            if (! cell->IsMap())
                val = cell->AsString();
            else
            {
                json::Map * map = cell->AsMap();
                val = map->GetString(puzT("cell"), empty_str);
                if (map->Contains(puzT("style")))
                {
                    json::Value * v_style = map->Get(puzT("style"));
                    if (v_style->IsMap())
                    {
                        json::Map * style = v_style->AsMap();
                        if (style->GetString(puzT("shapebg"), puzT(""))
                                == puzT("circle"))
                            square.SetCircle();
                        if (style->GetBool(puzT("highlight"), false))
                            square.SetHighlight();
                        // TODO: colors
                    }
                }
            }
            if (val == block_str)
                square.SetSolution(square.Black);
            else if (val != empty_str)
                square.SetNumber(val);
        }
    });

    // Solution
    if (doc->Contains(puzT("solution")))
    {
        GRID_FOREACH(doc->GetArray(puzT("solution")),
        {
            if (! cell->IsNull())
            {
                string_t val;
                if (! cell->IsMap())
                    val = cell->AsString();
                else
                    val = cell->AsMap()->GetString(puzT("value"), empty_str);
                if (val == block_str)
                    square.SetSolution(square.Black);
                else if (val != empty_str)
                    square.SetSolution(val);
            }
        });
    }

    // User grid
    if (doc->Contains(puzT("saved")))
    {
        GRID_FOREACH(doc->GetArray(puzT("saved")),
        {
            if (! cell->IsNull())
            {
                string_t val;
                if (! cell->IsMap())
                    val = cell->AsString();
                else
                    val = cell->AsMap()->GetString(puzT("value"), empty_str);
                if (val == block_str)
                    square.SetText(square.Black);
                else if (val != empty_str)
                    square.SetText(val);
            }
        });
    }


    // Clues
    json::Map * cluelists = doc->GetMap(puzT("clues"));
    {
        json::Map::iterator cl_it;
        for (cl_it = cluelists->begin(); cl_it != cluelists->end(); ++cl_it)
        {
            ClueList cluelist;
            json::Array * clues = cl_it->second->AsArray();
            json::Array::iterator clue_it;
            for (clue_it = clues->begin(); clue_it != clues->end(); ++clue_it)
            {
                json::Value * clueVal = (*clue_it);
                if (clueVal->IsSimple())
                    cluelist.push_back(Clue(puzT(""), clueVal->AsString()));
                else if (clueVal->IsArray())
                {
                    json::Array * clue = (*clue_it)->AsArray();
                    cluelist.push_back(Clue(clue->GetString(0),
                                            clue->GetString(1)));
                }
                else
                {
                    json::Map * clue = (*clue_it)->AsMap();

                    string_t enum_ = ParseEnumeration(
                        clue->GetString(puzT("enumeration"), puzT(""))
                    );
                    string_t text = clue->GetString(puzT("clue"));
                    if (! enum_.empty())
                        text.append(puzT(" ("))
                            .append(enum_)
                            .append(puzT(")"));

                    cluelist.push_back(Clue(
                        clue->GetString(puzT("number"), puzT("")),
                        text
                    ));
                }
            }
            puz->SetClueList(cl_it->first, cluelist);
        }
    }

    puz->SetFormatData(new ipuzData(root));
    return true; // puz owns json::Value * root
}

} // namespace puz
