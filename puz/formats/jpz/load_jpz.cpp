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

#include "jpz.hpp"

#include "Puzzle.hpp"
#include "Clue.hpp"
#include "puzstring.hpp"
#include "utils/minizip.hpp"
#include <sstream>
#include "parse/base64.hpp"

namespace puz {

class jpzParser : public xml::Parser
{
public:
    virtual bool DoLoadPuzzle(Puzzle * puz, xml::document & doc);

    // Return the square from this element
    Square * GetSquare(Puzzle * puz, xml::node &);
    Square * RequireSquare(Puzzle * puz, xml::node &);
};


void LoadJpz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    jpzParser parser;

    // Open a zip archive
    unzip::Archive zip(filename);
    if (! zip)
    {
        parser.LoadFromFilename(puz, filename);
        return;
    }
    // Browse the archive
    unzip::File f = zip.First();
    int n_files = zip.GetFileCount();
    while (f)
    {
        // Read the file
        f.Open();
        std::ostringstream stream;
        for (;;)
        {
            char buf[1024];
            int chars_read = f.Read(buf, 1024);
            stream.write(buf, chars_read);
            if (chars_read <= 0)
                break;
        }
        if (n_files == 1)
        {
            parser.LoadFromString(puz, stream.str().c_str());
            return;
        }
        else
        {
            try {
                parser.LoadFromString(puz, stream.str().c_str());
                return;
            }
            catch (...) {
                // Try the next file
            }
        }
        f.Next();
    }
    throw FileTypeError("jpz");
}

Square *
jpzParser::RequireSquare(Puzzle * puz, xml::node & node)
{
    string_t x = GetAttribute(node, "x");
    string_t y = GetAttribute(node, "y");
    if (x.find(puzT("-")) != string_t::npos || y.find(puzT("-")) != string_t::npos)
        throw LoadError("Spanned cells are not supported.");
    if (x.empty() || x.empty())
        throw LoadError("Cell missing x or y");
    try
    {
        // x and y are 1-based
        return &puz->GetGrid().At(ToInt(x) - 1, ToInt(y) - 1);
    }
    catch (...)
    {
        throw LoadError("Cell is out of range");
    }
}

Square *
jpzParser::GetSquare(Puzzle * puz, xml::node & node)
{
    try {
        return RequireSquare(puz, node);
    }
    catch(...) {
        return NULL;
    }
}


void parse_range(const string_t & str, int * val1, int * val2)
{
    int index = str.find(puzT("-"));
    if (index == string_t::npos)
    {
        *val1 = ToInt(str);
        *val2 = *val1;
    }
    else
    {
        *val1 = ToInt(str.substr(0, index));
        *val2 = ToInt(str.substr(index+1));
    }
}

Word make_word(Puzzle * puz, const string_t & x, const string_t & y)
{
    int x1, x2;
    parse_range(x, &x1, &x2);
    if (x1 == -1 || x2 == -1)
        throw LoadError("Invalid cell range in word.");
    int y1, y2;
    parse_range(y, &y1, &y2);
    if (y1 == -1 || y2 == -1)
        throw LoadError("Invalid cell range in word.");
    Grid & grid = puz->GetGrid();
    // JPZ squares are 1-based.
    return Word(&grid.At(x1-1, y1-1), &grid.At(x2-1, y2-1));
}



bool jpzParser::DoLoadPuzzle(Puzzle * puz, xml::document & doc)
{
    Grid & grid = puz->GetGrid();

    xml::node applet = doc.child("crossword-compiler-applet");
    if (! applet)
        throw FileTypeError("jpz");
    xml::node puzzle = RequireChild(applet, "rectangular-puzzle");
    xml::node crossword = RequireChild(puzzle, "crossword");

    // Metadata
    xml::node meta = puzzle.child("metadata");
    puz->SetTitle(GetInnerXML(meta, "title"));
    puz->SetAuthor(GetInnerXML(meta, "creator"));
    puz->SetCopyright(GetInnerXML(meta, "copyright"));

    if (puz->GetTitle().empty())
        puz->SetTitle(GetInnerXML(applet, "title"));
    if (puz->GetCopyright().empty())
        puz->SetCopyright(GetInnerXML(applet, "copyright"));

    puz->SetNotes(GetInnerXML(puzzle, "instructions"));

    // Grid
    {
        xml::node grid_node = RequireChild(crossword, "grid");

        int width = grid_node.attribute("width").as_int();
        int height = grid_node.attribute("height").as_int();
        if (width < 1 || height < 1)
            throw LoadError("Invalid grid size");
        grid.SetSize(width, height);

        // Default all squares to missing, because missing cells don't have to
        // be specified.
        {
            Square * square;
            for (square = grid.First(); square != NULL; square = square->Next())
                square->SetMissing();
        }

        // Grid cells
        xml::node cell = RequireChild(grid_node, "cell");
        for (; cell; cell = cell.next_sibling("cell"))
        {
            Square * square = RequireSquare(puz, cell);
            // Find the square's type
            string_t type = GetAttribute(cell, "type");
            if (type == puzT("void"))
            {
                square->SetMissing();
            }
            else if (type == puzT("block"))
            {
                square->SetMissing(false);
                square->SetBlack();
            }
            else if (type == puzT("clue"))
            {
                throw LoadError("Clues inside squares are not supported.");
            }
            else // type == puzT("letter")
            {
                square->SetMissing(false);
                square->SetSolution(GetAttribute(cell, "solution"));
                square->SetText(GetAttribute(cell, "solve-state"));
                square->SetNumber(GetAttribute(cell, "number"));
                if (GetAttribute(cell, "background-shape") == puzT("circle"))
                    square->SetCircle();
                square->SetColor(GetAttribute(cell, "background-color"));
                string_t status = GetAttribute(cell, "solve-status");
                if (GetAttribute(cell, "hint") == puzT("true"))
                    square->SetText(square->GetSolution());
                if (status == puzT("revealed"))
                {
                    square->AddFlag(FLAG_REVEALED);
                    square->SetText(square->GetSolution());
                }
                else if (status == puzT("pencil"))
                    square->AddFlag(FLAG_PENCIL);
                // Extra square flags to keep track of checked and correct
                // letters
                // This is nonstandard, but doesn't break Crossword Solver.
                if (GetAttribute(cell, "checked") == puzT("true"))
                    square->AddFlag(FLAG_BLACK);
                if (GetAttribute(cell, "incorrect") == puzT("true"))
                    square->AddFlag(FLAG_X);
                if (GetAttribute(cell, "correct") == puzT("true"))
                    square->AddFlag(FLAG_CORRECT);
                // Top right number
                square->m_mark[MARK_TR] = GetAttribute(cell, "top-right-number");
            }
            // Image
            xml::node image = cell.child("background-picture");
            if (image)
            {
                square->m_imageformat = image.attribute("format").value();
                square->m_imagedata = base64_decode(
                    image.child_value("encoded-image"));
            }
        }
    }

    // Words
    std::map<string_t, Word> words;
    {
        xml::node xmlword = RequireChild(crossword, "word");
        for (; xmlword; xmlword = xmlword.next_sibling("word"))
        {
           // Look for a cell range
            string_t x = GetAttribute(xmlword, "x");
            string_t y = GetAttribute(xmlword, "y");
            // Ignore the delaration if X or Y is provided, but not both.
            Word word;
            if (! x.empty() && ! y.empty())
                word = make_word(puz, x, y);
            string_t id = GetAttribute(xmlword, "id");
            if (id.empty())
                throw LoadError("Each word must have an id");
            // TODO: hidden

            // Word cells
            xml::node cells = xmlword.child("cells");
            for (; cells; cells = cells.next_sibling("cells"))
            {
                // Make a word from this cell.
                Word cell_word = make_word(puz, GetAttribute(cells, "x"),
                                                GetAttribute(cells, "y"));
                square_iterator it;
                for (it = cell_word.begin(); it != cell_word.end(); ++it)
                    word.push_back(&*it);
            }
            // Add this word to the internal map of words.
            words[id] = word;
        }
    }

    // Clues
    {
        xml::node clues = crossword.child("clues");
        for (; clues; clues = clues.next_sibling("clues"))
        {
            string_t key = GetText(clues, "title");
            if (key.empty())
                throw LoadError("Each clue list must have a title");
            ClueList list(GetInnerXML(RequireChild(clues, "title")));
            xml::node clue = RequireChild(clues, "clue");
            for (; clue; clue = clue.next_sibling("clue"))
            {
                std::map<string_t, Word>::iterator it;
                it = words.find(GetAttribute(clue, "word"));
                if (it == words.end())
                    throw LoadError("Each clue must have a word");
                string_t text = GetInnerXML(clue);
                // Format is added after the clue in parentheses
                string_t format = GetAttribute(clue, "format");
                if (! format.empty())
                    text.append(puzT(" (")).append(format).append(puzT(")"));
                string_t number = GetAttribute(clue, "number");
                list.push_back(Clue(number, text, it->second));
            }
            puz->SetClueList(key, list);
        }
    }

    // Timer
    {
        xml::node timer = applet.child("applet-settings").child("timer");
        puz->SetTime(timer.attribute("initial-value").as_int());
        puz->SetTimerRunning(
            GetAttribute(timer, "start-on-load") == puzT("true")
        );
    }

    puz->SetFormatData(new JpzData(&doc));
    return true; // We own the doc
}

} // namespace puz
