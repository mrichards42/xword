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

#include <sstream>
#include "Puzzle.hpp"
#include "puzstring.hpp"
#include "parse/json.hpp"

namespace puz {

class ipuzParser : public json::Parser
{
public:
    virtual bool DoLoadPuzzle(Puzzle * puz, json::Value * root);

    void SetStyle(Square & square, json::Value * style_value);
    std::map<string_t, json::Map *> m_style_map;
    std::map<string_t, string_t> m_color_map;
};

void LoadIpuzStream(Puzzle * puz, const std::string & filename, std::istream & stream)
{
    if (stream.fail())
        throw FileError(filename);
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

void LoadIpuz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    std::ifstream stream(filename.c_str());
    LoadIpuzStream(puz, filename, stream);
}


void LoadIpuzString(Puzzle * puz, const std::string & data)
{
    std::istringstream stream(data);
    std::string filename("<json>");
    LoadIpuzStream(puz, filename, stream);
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

void ipuzParser::SetStyle(Square & square, json::Value * style_value)
{
    if (! style_value)
        return;
    // Get the style as a map
    json::Map * style = NULL;
    if (style_value->IsString()) // Named styles
        style = m_style_map[style_value->AsString()];
    else if (style_value->IsMap())
        style = style_value->AsMap();
    if (! style)
        return;

    // Named styles
    string_t name = style->GetString(puzT("named"), puzT(""));
    if (! name.empty())
        SetStyle(square, m_style_map[name]);

    // Set styles
    if (style->GetString(puzT("shapebg"), puzT("")) == puzT("circle"))
        square.SetCircle();
    if (style->GetBool(puzT("highlight"), false))
        square.SetHighlight();

    string_t barred = style->GetString(puzT("barred"), puzT(""));
    if (barred.find(puzT("T")) != string_t::npos)
        square.m_bars[BAR_TOP] = true;
    if (barred.find(puzT("L")) != string_t::npos)
        square.m_bars[BAR_LEFT] = true;
    if (barred.find(puzT("R")) != string_t::npos)
        square.m_bars[BAR_RIGHT] = true;
    if (barred.find(puzT("B")) != string_t::npos)
        square.m_bars[BAR_BOTTOM] = true;

    if (style->Contains(puzT("mark"))) {
        json::Map* mark = style->GetMap(puzT("mark"));
        if (mark->Contains(puzT("TL")))
            square.m_mark[MARK_TL] = mark->GetString(puzT("TL"), puzT(""));
        if (mark->Contains(puzT("TR")))
            square.m_mark[MARK_TR] = mark->GetString(puzT("TR"), puzT(""));
        if (mark->Contains(puzT("BL")))
            square.m_mark[MARK_BL] = mark->GetString(puzT("BL"), puzT(""));
        if (mark->Contains(puzT("BR")))
            square.m_mark[MARK_BR] = mark->GetString(puzT("BR"), puzT(""));
    }

    // Color
    string_t color = style->GetString(puzT("color"), puzT(""));
    if (!color.empty()) {
        if (color.length() == 6) {
            // Six-digit hex value.
            square.SetColor(color);
        }
        else {
            // Integer indicating an arbitrary app-defined color.
            // The spec recommends supporting "at least 16", with 0 as black and others as non-black.
            // For now, just support 0 and treat others as "highlight".
            if (color == puzT("0"))
                square.SetColor(0, 0, 0);
            else
                square.SetHighlight();
        }
    }
}

/**
 * Returns whether any clues contain custom words.
 *
 * Note that we cannot use Clues.HasWords() yet because constructing valid ClueLists requires
 * knowing whether we're using 0-based or 1-based coordinates.
 */
bool HasWords(json::Map* cluelists) {
    json::Map::iterator cl_it;
    for (cl_it = cluelists->begin(); cl_it != cluelists->end(); ++cl_it)
    {
        ClueList cluelist;
        json::Array* clues = cl_it->second->AsArray();
        json::Array::iterator clue_it;
        for (clue_it = clues->begin(); clue_it != clues->end(); ++clue_it)
        {
            json::Value* clueVal = (*clue_it);
            if (!clueVal->IsSimple() && !clueVal->IsArray())
            {
                json::Map* clue = (*clue_it)->AsMap();
                if (clue->Contains(puzT("cells"))) {
                    return true;
                }
            }
        }
    }
    return false;
}

/** Returns whether any cell is invalid when applying the given offset to cell coordinates. */
bool AnyCellIsInvalid(Grid& grid, json::Map* cluelists, int offset) {
    json::Map::iterator cl_it;
    for (cl_it = cluelists->begin(); cl_it != cluelists->end(); ++cl_it)
    {
        ClueList cluelist;
        json::Array* clues = cl_it->second->AsArray();
        json::Array::iterator clue_it;
        for (clue_it = clues->begin(); clue_it != clues->end(); ++clue_it)
        {
            json::Value* clueVal = (*clue_it);
            if (!clueVal->IsSimple() && !clueVal->IsArray())
            {
                json::Map* clue = (*clue_it)->AsMap();
                if (clue->Contains(puzT("cells"))) {
                    json::Array* cells = clue->GetArray(puzT("cells"));
                    json::Array::iterator cell_it;
                    for (cell_it = cells->begin(); cell_it != cells->end(); ++cell_it) {
                        json::Array* cellVal = (*cell_it)->AsArray();
                        int x = ToInt(cellVal->GetNumber(0)) + offset;
                        int y = ToInt(cellVal->GetNumber(1)) + offset;
                        if (x < 0 || x > grid.GetWidth() - 1 || y < 0 || y > grid.GetHeight() - 1) {
                            return true;
                        }
                        Square& square = grid.At(x, y);
                        if (square.IsBlack() || square.IsMissing()) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool ipuzParser::DoLoadPuzzle(Puzzle * puz, json::Value * root)
{
    json::Map * doc = root->AsMap();
    try {
        // Check kind from most to least specific until we find a match
        json::Array * kinds = doc->PopArray(puzT("kind"));
        std::vector<json::Value*>::iterator i = kinds->end();
        bool known_type = false;
        while (i != kinds->begin() && !known_type) {
            string_t kind = (*(--i))->AsString();
            if (kind.substr(kind.size() - 2) == puzT("#1"))
                kind = kind.substr(0, kind.size() - 2);
            if (kind == puzT("http://ipuz.org/crossword") ||
                kind == puzT("http://ipuz.org/crossword/crypticcrossword")) {
                // Regular crossword
                known_type = true;
            } else if (kind == puzT("http://ipuz.org/crossword/diagramless")) {
                // Diagramless crossword
                known_type = true;
                puz->GetGrid().SetType(TYPE_DIAGRAMLESS);
            } else if (kind == puzT("http://crosswordnexus.com/ipuz/coded")) {
                // Coded crossword
                known_type = true;
                puz->GetGrid().SetType(TYPE_CODED);
            }
        }
        if (!known_type) {
            throw LoadError("Unsupported ipuz kind");
        }
    }
    catch (json::BaseError &) {
        throw FileTypeError("ipuz");
    }

    // Metadata
    puz->SetTitle(doc->PopString(puzT("title"), puzT("")), /* is_html */ true);
    puz->SetAuthor(doc->PopString(puzT("author"), puzT("")), /* is_html */ true);
    puz->SetCopyright(doc->PopString(puzT("copyright"), puzT("")), /* is_html */ true);
    string_t notes = doc->PopString(puzT("notes"), puzT(""));
    if (! notes.empty())
        puz->SetNotes(notes, /* is_html */ true);
    else
        puz->SetNotes(doc->PopString(puzT("intro"), puzT("")), /* is_html */ true);

    // Read the styles into a style map
    if (doc->Contains(puzT("styles")))
    {
        json::Map * styles = doc->GetMap(puzT("styles"));
        json::Map::iterator style;
        for (style = styles->begin(); style != styles->end(); ++style)
            m_style_map[style->first] = style->second->AsMap();
    }

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
                    SetStyle(square, map->Get(puzT("style")));
                square.SetText(map->GetString(puzT("value"), puzT("")));
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
    else
    {
        puz->GetGrid().SetFlag(FLAG_NO_SOLUTION);
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

    // Heuristically determine whether this puzzle uses 0-based or 1-based coordinates.
    // Due to an ambiguity in the ipuz specification, both coordinate systems have appeared in the
    // wild. We can apply some heuristics to guess the right basis. A future version of the ipuz
    // spec should resolve the ambiguity.
    int offset = -1;
    if (HasWords(cluelists)) {
        bool hasInvalid0Coordinates = AnyCellIsInvalid(puz->GetGrid(), cluelists, 0);
        bool hasInvalid1Coordinates = AnyCellIsInvalid(puz->GetGrid(), cluelists, -1);
        // If the coordinates are all valid with 0-based indexing and invalid with 1-based indexing,
        // we use 0-based indexing. In any other case, we default to 1-based indexing as we used
        // before.
        if (!hasInvalid0Coordinates && hasInvalid1Coordinates) {
            offset = 0;
        }
    }

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
                    cluelist.push_back(Clue(puzT(""), clueVal->AsString(), /* is_html */ true));
                else if (clueVal->IsArray())
                {
                    json::Array * clue = (*clue_it)->AsArray();
                    cluelist.push_back(Clue(clue->GetString(0),
                                            clue->GetString(1),
                                            /* is_html */ true));
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

                    Clue outClue(
                        clue->GetString(puzT("number"), puzT("")),
                        text,
                        /* is_html */ true
                    );

                    if (clue->Contains(puzT("cells"))) {
                        json::Array * cells = clue->GetArray(puzT("cells"));
                        Word word;
                        json::Array::iterator cell_it;
                        for (cell_it = cells->begin(); cell_it != cells->end(); ++cell_it) {
                            json::Array * cellVal = (*cell_it)->AsArray();
                            word.push_back(&puz->GetGrid().At(
                                ToInt(cellVal->GetNumber(0)) + offset,
                                ToInt(cellVal->GetNumber(1)) + offset));
                        }
                        outClue.SetWord(word);
                    }

                    cluelist.push_back(outClue);
                }
            }
            puz->SetClueList(cl_it->first, cluelist);
        }
    }

    puz->SetFormatData(new ipuzData(root));
    return true; // puz owns json::Value * root
}

} // namespace puz
