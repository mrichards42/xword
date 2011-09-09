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

#include "xpf.hpp"

#include "Puzzle.hpp"
#include "Clue.hpp"
#include "puzstring.hpp"
#include "parse/xml.hpp"

namespace puz {

class XPFParser : public xml::Parser
{
public:
    virtual bool DoLoadPuzzle(Puzzle * puz, xml::document & doc);
protected:
    // Return the square from this node
    Square * GetSquare(Puzzle * puz, const xml::node &);
    Square * RequireSquare(Puzzle * puz, const xml::node &);
};


void LoadXPF(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    XPFParser parser;
    parser.LoadFromFilename(puz, filename);
}

inline Square *
XPFParser::GetSquare(Puzzle * puz, const xml::node & node)
{
    return puz->GetGrid().AtNULL(
        node.attribute("Col").as_int() - 1,
        node.attribute("Row").as_int() - 1
    );
}

inline Square *
XPFParser::RequireSquare(Puzzle * puz, const xml::node & node)
{
    int col = node.attribute("Col").as_int();
    int row = node.attribute("Row").as_int();
    if (col == 0 || row == 0)
        throw FatalFileError("Missing Col or Row");

    try {
        return &puz->GetGrid().At(col - 1, row - 1);
    }
    catch (...)
    {
        throw InvalidGridCell("Square is out of range");
    }
}


bool XPFParser::DoLoadPuzzle(Puzzle * puz, xml::document & doc)
{
    Grid & grid = puz->GetGrid();

    xml::node puzzle = RequireChild(doc.child("Puzzles"), "Puzzle");

    // Metadata
    puz->SetTitle(GetText(puzzle, "Title"));
    puz->SetAuthor(GetText(puzzle, "Author"));
    puz->SetCopyright(GetText(puzzle, "Copyright"));
    puz->SetNotes(GetText(puzzle, "Notepad"));

    // Grid
    if (GetText(puzzle, "Type") == puzT("diagramless"))
        grid.SetType(TYPE_DIAGRAMLESS);

    xml::node size = RequireChild(puzzle, "Size");
    int height = ToInt(GetText(size, "Rows"));
    int width = ToInt(GetText(size, "Cols"));
    if (width <= 0 || height <= 0)
        throw FatalFileError("Invalid grid size");
    grid.SetSize(width, height);

    // Answers
    {
        Square * square = grid.First();
        xml::node row = RequireChild(puzzle, "Grid").child("Row");
        int row_n = 0;
        for (; row; row = row.next_sibling("Row"))
        {
            if (row_n >= height)
                throw FatalFileError("Too many rows in grid");
            string_t text = GetText(row);
            if (text.size() != width)
                throw FatalFileError("Wrong number of squares in Row");
            string_t::iterator it;
            for (it = text.begin(); it != text.end(); ++it)
            {
                assert(square);
                switch (*it)
                {
                    case puzT('~'):
                        square->SetMissing();
                        break;
                    case puzT('.'):
                        square->SetBlack();
                        break;
                    default:
                        square->SetSolution(string_t(1, *it));
                        break;
                }
                square = square->Next();
            }
            ++row_n;
        }
        if (row_n < height - 1)
            throw FatalFileError("Not enough rows in grid");
        assert(! square);
    }

    // Circles
    {
        xml::node circle = puzzle.child("Circles").child("Circle");
        for (; circle; circle = circle.next_sibling("Circle"))
        {
            Square * square = GetSquare(puz, circle);
            if (! square)
                continue;
            square->SetCircle();
        }
    }

    // Rebus
    {
        xml::node rebus = puzzle.child("RebusEntries").child("Rebus");
        for (; rebus; rebus = rebus.next_sibling("Rebus"))
        {
            Square * square = GetSquare(puz, rebus);
            if (! square)
                continue;
            string_t plain = GetAttribute(rebus, "Short");
            string_t rebus_text = GetText(rebus);

            if (rebus_text.empty())
            {
                continue;
            }
            else if (plain.empty())
            {
                if (square->IsSolutionBlank())
                    square->SetSolutionRebus(rebus_text);
                else
                    square->SetSolution(rebus_text);
            }
            else
            {
                square->SetSolution(rebus_text, Square::ToPlain(plain));
            }
        }
    }

    // Shade
    {
        xml::node shade = puzzle.child("Shades").child("Shade");
        for (; shade; shade = shade.next_sibling("Shade"))
        {
            Square * square = GetSquare(puz, shade);
            if (! square)
                continue;
            string_t color = GetText(shade);
            if (color == puzT("gray"))
                square->SetHighlight();
            else
                square->SetColor(color);
        }
    }

    // Clues
    {
        bool clueAlgorithm = false;
        std::vector<string_t> all_clues;
        Clues & clues = puz->GetClues();

        xml::node clue = puzzle.child("Clues").child("Clue");
        for (; clue; clue = clue.next_sibling("Clue"))
        {
            string_t clue_text = GetText(clue);
            all_clues.push_back(clue_text);

            Square * square = GetSquare(puz, clue);
            string_t dir = GetAttribute(clue, "Dir");
            string_t num = GetAttribute(clue, "Num");
            if (! square || dir.empty() || num.empty())
                clueAlgorithm = true;
            else
            {
                square->SetNumber(num);
                clues[dir].push_back(Clue(num, clue_text));
            }
        }
        if (clueAlgorithm)
            puz->SetAllClues(all_clues);
    }

    // User grid
    {
        int row_n = 0;
        Square * square = grid.First();
        xml::node row = puzzle.child("UserGrid").child("Row");
        if (! row)
            row = puzzle.child("User").child("Grid").child("Row");
        for (; row; row = row.next_sibling("Row"))
        {
            if (row_n >= height)
                break;
            string_t text = GetText(row);
            if (text.size() != width)
                break;
            string_t::iterator it;
            for (it = text.begin(); it != text.end(); ++it)
            {
                assert(square);
                square->SetText(string_t(1, *it));
                square = square->Next();
            }
            ++row_n;
        }
    }

    // Rebus
    {
        xml::node rebus = puzzle.child("UserRebusEntries").child("Rebus");
        if (! rebus)
            rebus = puzzle.child("User").child("RebusEntries").child("Rebus");
        for (; rebus; rebus = rebus.next_sibling("Rebus"))
        {
            Square * square = GetSquare(puz, rebus);
            if (! square)
                continue;
            string_t rebus_text = GetText(rebus);

            if (rebus_text.empty())
                continue;
            else
                square->SetText(rebus_text);
        }
    }

    // Flags
    {
        xml::node flag = puzzle.child("SquareFlags").child("Flag");
        if (! flag)
            flag = puzzle.child("User").child("Flags").child("Flag");
        for (; flag; flag = flag.next_sibling("Flag"))
        {
            Square * square = GetSquare(puz, flag);
            if (! square)
                continue;
            if (flag.attribute("Pencil").value() == std::string("true"))
                square->AddFlag(FLAG_PENCIL);
            if (flag.attribute("Incorrect").value() == std::string("true"))
                square->AddFlag(FLAG_BLACK);
            if (flag.attribute("Revealed").value() == std::string("true"))
                square->AddFlag(FLAG_REVEALED);
            if (flag.attribute("X").value() == std::string("true"))
                square->AddFlag(FLAG_X);
        }
    }

    // Timer
    {
        xml::node timer = puzzle.child("Timer");
        if (! timer)
            timer = puzzle.child("User").child("Timer");
        puz->SetTime(timer.attribute("Seconds").as_int());
        puz->SetTimerRunning(timer.attribute("Running").value() == std::string("true"));
    }

    return false; // Delete the doc
}

} // namespace puz

