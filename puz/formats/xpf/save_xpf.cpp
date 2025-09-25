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
#include <fstream>

namespace puz {

//-----------------------------------------------------------------------------
// SaveXPF
//-----------------------------------------------------------------------------
void SaveXPF(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    const Grid & grid = puz->GetGrid();

    if (grid.IsScrambled())
        throw ConversionError("XPF does not support scrambled puzzles");
    if (! grid.HasSolution())
        throw ConversionError("XPF does not support puzzles without a solution");
    for (const Square * square = puz->GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasImage())
            throw ConversionError("XPF does not support puzzles with background images.");
    }

    xml::document doc;
    xml::node puzzles = doc.append_child("Puzzles");
    puzzles.append_attribute("Version") = "1.0";
    xml::node puzzle = puzzles.append_child("Puzzle");
    // Metadata
    puz::Puzzle::metamap_t & meta = puz->GetMetadata();
    puz::Puzzle::metamap_t::iterator it;
    for (it = meta.begin(); it != meta.end(); ++it)
    {
        if (it->first == puzT("notes")) // "notes" -> "Notepad"
            xml::Append(puzzle, "Notepad", it->second);
        else
            xml::Append(puzzle, xml::CamelCase(it->first).c_str(), it->second);
    }

    // Grid
    if (grid.GetType() == TYPE_DIAGRAMLESS)
        xml::Append(puzzle, "Type", "diagramless");

    // Grid Size
    xml::node size = puzzle.append_child("Size");
    xml::Append(size, "Rows", puz::ToString(grid.GetHeight()));
    xml::Append(size, "Cols", puz::ToString(grid.GetWidth()));

    // Answers
    {
        xml::node grid_node = puzzle.append_child("Grid");
        xml::node row;
        std::string row_text;
        row_text.reserve(grid.GetWidth());
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->IsFirst(ACROSS))
                row = grid_node.append_child("Row");
            if (square->IsMissing())
                row_text.append(1, '~');
            else if (square->IsBlack())
                row_text.append(1, char(Square::Black[0]));
            else if (square->IsSolutionBlank())
                row_text.append(1, ' ');
            else
                row_text.append(1, square->GetPlainSolution());
            if (square->IsLast(ACROSS))
            {
                xml::SetText(row, row_text.c_str());
                row_text.clear();
            }
        }
    }

    // Circles
    {
        xml::node circles = puzzle.append_child("Circles");
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->HasCircle())
            {
                xml::node circle = circles.append_child("Circle");
                circle.append_attribute("Row") = square->GetRow() + 1;
                circle.append_attribute("Col") = square->GetCol() + 1;
            }
        }
        if (circles.empty())
            puzzle.remove_child(circles);
    }

    // Rebus
    {
        xml::node rebus = puzzle.append_child("RebusEntries");
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->HasSolutionRebus())
            {
                xml::node entry = rebus.append_child("Rebus");
                entry.append_attribute("Row") = square->GetRow() + 1;
                entry.append_attribute("Col") = square->GetCol() + 1;
                entry.append_attribute("Short") =
                    std::string(1, square->GetPlainSolution()).c_str();
                xml::SetText(entry, square->GetSolution());
            }
        }
        if (rebus.empty())
            puzzle.remove_child(rebus);
    }

    // Shades
    {
        xml::node shades = puzzle.append_child("Shades");
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->HasColor())
            {
                xml::node shade = shades.append_child("Shade");
                shade.append_attribute("Row") = square->GetRow() + 1;
                shade.append_attribute("Col") = square->GetCol() + 1;
                if (square->HasHighlight())
                    xml::SetText(shade, "gray");
                else
                    xml::SetText(shade, square->GetHtmlColor());
            }
        }
        if (shades.empty())
            puzzle.remove_child(shades);
    }

    // Clues
    {
        xml::node clues_node = puzzle.append_child("Clues");
        Clues & clues = puz->GetClues();
        Clues::iterator clues_it;
        for (clues_it = clues.begin(); clues_it != clues.end(); ++clues_it)
        {
            ClueList & cluelist = clues_it->second;
            ClueList::iterator it;
            for (it = cluelist.begin(); it != cluelist.end(); ++it)
            {
                xml::node clue = clues_node.append_child("Clue");
                // Find the clue direction
                if (! puz->IsDiagramless())
                {
                    const Word & word = it->GetWord();
                    switch (word.GetDirection())
                    {
                    case ACROSS:
                        clue.append_attribute("Dir") = "Across";
                        break;
                    case DOWN:
                        clue.append_attribute("Dir") = "Down";
                        break;
                    case DIAGONAL_SW:
                        clue.append_attribute("Dir") = "Diagonal";
                        break;
                    default:
                        throw ConversionError("XPF clues must be Across, Down, or Diagonal");
                        break;
                    }
                    clue.append_attribute("Row") = word.front()->GetRow() + 1;
                    clue.append_attribute("Col") = word.front()->GetCol() + 1;
                }
                else // diagramless
                {
                    puz::string_t direction = clues_it->first;
                    if (direction == puzT("Across"))
                        clue.append_attribute("Dir") = "Across";
                    else if (direction == puzT("Down"))
                        clue.append_attribute("Dir") = "Down";
                    else if (direction == puzT("Diagonal"))
                        clue.append_attribute("Dir") = "Diagonal";
                    else
                        throw ConversionError("XPF clues must be Across, Down, or Diagonal");
                }
                clue.append_attribute("Num") = encode_utf8(it->GetNumber()).c_str();
                // Clue formatting needs to be escaped if it is XHTML.
                // xml::SetInnerXML(clue, it->GetText());
                xml::SetText(clue, it->GetText());
            }
        }
    }

    // User Grid
    {
        xml::node usergrid = puzzle.append_child("UserGrid");
        xml::node row;
        std::string row_text;
        row_text.reserve(grid.GetWidth());
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->IsFirst(ACROSS))
                row = usergrid.append_child("Row");
            if (square->IsMissing())
                row_text.append(1, '~');
            else if (square->IsBlack())
                row_text.append(1, char(Square::Black[0]));
            else if (square->IsBlank())
                row_text.append(1, ' ');
            else
                row_text.append(1, square->GetPlainText());
            if (square->IsLast(ACROSS))
            {
                xml::SetText(row, row_text.c_str());
                row_text.clear();
            }
        }
        if (usergrid.empty())
            puzzle.remove_child(usergrid);
    }

    // User Rebus
    {
        xml::node rebus = puzzle.append_child("UserRebusEntries");
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->HasTextRebus())
            {
                xml::node entry = rebus.append_child("Rebus");
                entry.append_attribute("Row") = square->GetRow() + 1;
                entry.append_attribute("Col") = square->GetCol() + 1;
                entry.append_attribute("Short") =
                    std::string(1, square->GetPlainText()).c_str();
                xml::SetText(entry, square->GetText());
            }
        }
        if (rebus.empty())
            puzzle.remove_child(rebus);
    }

    // Square Flags
    {
        xml::node flags = puzzle.append_child("SquareFlags");
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            if (square->GetFlag() != 0)
            {
                xml::node entry = flags.append_child("Flag");
                entry.append_attribute("Row") = square->GetRow() + 1;
                entry.append_attribute("Col") = square->GetCol() + 1;
                if (square->HasFlag(FLAG_PENCIL))
                    entry.append_attribute("Pencil") = "true";
                if (square->HasFlag(FLAG_BLACK))
                    entry.append_attribute("Checked") = "true";
                if (square->HasFlag(FLAG_REVEALED))
                    entry.append_attribute("Revealed") = "true";
                if (square->HasFlag(FLAG_X))
                    entry.append_attribute("Incorrect") = "true";
                if (square->HasFlag(FLAG_CORRECT))
                    entry.append_attribute("Correct") = "true";
            }
        }
        if (flags.empty())
            puzzle.remove_child(flags);
    }

    // Timer
    {
        if (puz->GetTime() != 0)
        {
            xml::node timer = puzzle.append_child("Timer");
            timer.append_attribute("Seconds") = puz->GetTime();
            if (puz->IsTimerRunning())
                timer.append_attribute("Running") = "true";
        }
    }

    doc.save_file(filename.c_str());
}

} // namespace puz
