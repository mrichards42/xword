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
#include <fstream>
#include "utils/minizip.hpp"
#include "parse/base64.hpp"

namespace puz {

// XML Writer class for pugixml
class xml_zip_writer 
    : public pugi::xml_writer
{
public:
    xml_zip_writer(zip::File & file) : m_file(file) {}

    virtual void write(const void* data, size_t size)
    {
        m_file.Write(data, size);
    }

protected:
    zip::File & m_file;
};

void AppendChildWithSpanWrapping(xml::node parent, int child_count, xml::node child) {
    if (child_count > 1 && child.type() == pugi::xml_node_type::node_pcdata) {
        // When writing mixed content, plain text must be wrapped in <span> tags.
        xml::node span = parent.append_child("span");
        span.append_copy(child);
    } else {
        parent.append_copy(child);
    }
}

//-----------------------------------------------------------------------------
// SaveJpz
//-----------------------------------------------------------------------------
void SaveJpz(Puzzle * puz, const std::string & filename, void * /* dummy */)
{
    const Grid & grid = puz->GetGrid();

    if (grid.IsScrambled())
        throw ConversionError("Jpz does not support scrambled puzzles");
    if (! grid.HasSolution())
        throw ConversionError("Jpz does not support puzzles without a solution");
    if (grid.GetType() == TYPE_DIAGRAMLESS)
        throw ConversionError("Can't save a diagramless puzzle as jpz.");

    xml::document doc;

    xml::node applet = doc.append_child("crossword-compiler-applet");
    applet.append_attribute("xmlns") =
        "http://crossword.info/xml/crossword-compiler-applet";

    // Read settings from the saved XML Doc, otherwise provide default
    // settings.
    JpzData * data = dynamic_cast<JpzData *>(puz->GetFormatData());
    if (data)
    {
        applet.append_copy(data->doc->child("crossword-compiler-applet")
                                    .child("applet-settings"));
    }
    if (! applet.child("applet-settings"))
    {
        xml::node settings = applet.append_child("applet-settings");
        settings.append_attribute("cursor-color") = "#FFFF00";
        settings.append_attribute("selected-cells-color") = "#C0C0C0";

        xml::node actions = settings.append_child("actions");
        actions.append_attribute("buttons-layout") = "below";
        actions.append_child("check").append_attribute("label") = "Check";
        actions.append_child("reveal-letter").append_attribute("label") =
                                                                "Reveal Letter";
        actions.append_child("reveal-word").append_attribute("label") =
                                                                "Reveal Word";
        actions.append_child("revert").append_attribute("label") = "Clear All";
        actions.append_child("solution").append_attribute("label") = "Solution";

        xml::node completion = settings.append_child("completion");
        completion.append_attribute("only-if-correct") = "true";
        string_t message = puz->GetMeta(puzT("completion"));
        if (message.empty()) {
            message = puzT("Congratulations, you have solved the puzzle!");
        }
        xml::SetText(completion, message);
    }
    // Timer
    applet.child("applet-settings").remove_child("timer");
    xml::node timer = applet.child("applet-settings").append_child("timer");
    timer.append_attribute("start-on-load") = puz->IsTimerRunning();
    timer.append_attribute("initial-value") = puz->GetTime();

    // On to the actual puzzle
    xml::node puzzle = applet.append_child("rectangular-puzzle");
    puzzle.append_attribute("xmlns") = 
        "http://crossword.info/xml/rectangular-puzzle";
    // Metadata
    xml::node metadata = puzzle.append_child("metadata");
    const Puzzle::metamap_t & puz_metadata = puz->GetMetadata();
    Puzzle::metamap_t::const_iterator it;
    for (it = puz_metadata.begin(); it != puz_metadata.end(); ++it)
    {
        // Author is "creator" in jpz
        if (it->first == puzT("author"))
            xml::SetInnerXML(metadata.append_child("creator"), it->second);
        // Notes are jpz "instructions" and are under the "puzzle" element
        else if (it->first != puzT("notes"))
            xml::SetInnerXML(metadata.append_child(puz::encode_utf8(it->first).c_str()), it->second);
    }

    xml::SetInnerXML(puzzle.append_child("instructions"), puz->GetNotes());

    const char* puzzle_root_tag;
    if (grid.IsAcrostic())
        puzzle_root_tag = "acrostic";
    else
        puzzle_root_tag = "crossword";
    xml::node crossword = puzzle.append_child(puzzle_root_tag);

    // Grid
    xml::node xmlgrid = crossword.append_child("grid");
    // Size
    xmlgrid.append_attribute("width") = int(grid.GetWidth());
    xmlgrid.append_attribute("height") = int(grid.GetHeight());
    // Meta
    xml::node gridlook = xmlgrid.append_child("grid-look");
    gridlook.append_attribute("numbering-scheme") = "normal";
    gridlook.append_attribute("grid-line-color") = "#000000";
    gridlook.append_attribute("block-color") = "#000000";
    gridlook.append_attribute("font-color") = "#000000";
    gridlook.append_attribute("number-color") = "#000000";
    // Cells
    {
        const Square * square;
        for (square = grid.First(); square; square = square->Next())
        {
            xml::node cell = xmlgrid.append_child("cell");
            cell.append_attribute("x") = square->GetCol() + 1; // 1-based
            cell.append_attribute("y") = square->GetRow() + 1;
            if (square->IsMissing())
            {
                cell.append_attribute("type") = "void";
            }
            else if (square->IsBlack() && !square->IsAnnotation())
            {
                cell.append_attribute("type") = "block";
                if (square->HasColor())
                    cell.append_attribute("background-color") =
                        encode_utf8(square->GetHtmlColor()).c_str();
            }
            else
            {
                if (square->IsAnnotation())
                    cell.append_attribute("type") = "clue";
                cell.append_attribute("solution") =
                    encode_utf8(square->GetSolution()).c_str();
                if (square->HasNumber())
                    cell.append_attribute("number") =
                        encode_utf8(square->GetNumber()).c_str();
                if (! square->IsBlank())
                    cell.append_attribute("solve-state") =
                        encode_utf8(square->GetText()).c_str();
                if (square->HasCircle())
                    cell.append_attribute("background-shape") = "circle";
                if (square->HasColor())
                    cell.append_attribute("background-color") =
                        encode_utf8(square->GetHtmlColor()).c_str();
                if (square->HasFlag(FLAG_REVEALED))
                {
                    cell.append_attribute("solve-status") = "revealed";
                    cell.append_attribute("hint") = "true";
                }
                else if (square->HasFlag(FLAG_PENCIL))
                    cell.append_attribute("solve-status") = "pencil";
                // Extra square flags to keep track of incorrect letters
                // This is nonstandard, but doesn't break Crossword Solver.
                if (square->HasFlag(FLAG_BLACK))
                    cell.append_attribute("checked") = "true";
                if (square->HasFlag(FLAG_X))
                    cell.append_attribute("incorect") = "true";
                if (square->HasFlag(FLAG_CORRECT))
                    cell.append_attribute("correct") = "true";
            }
            if (square->HasImage())
            {
                xml::node img = cell.append_child("background-picture");
                img.append_attribute("format") = square->m_imageformat.c_str();
                std::string encoded = base64_encode(
                    (const unsigned char *)square->m_imagedata.c_str(),
                                           square->m_imagedata.length());
                xml::SetText(img.append_child("encoded-image"), encoded.c_str());
            }
            if (! square->m_mark[MARK_TR].empty())
            {
                cell.append_attribute("top-right-number") =
                    encode_utf8(square->m_mark[MARK_TR]).c_str();
            }
            if (square->m_bars[BAR_TOP])
                cell.append_attribute("top-bar") = "true";
            if (square->m_bars[BAR_LEFT])
                cell.append_attribute("left-bar") = "true";
            if (square->m_bars[BAR_RIGHT])
                cell.append_attribute("right-bar") = "true";
            if (square->m_bars[BAR_BOTTOM])
                cell.append_attribute("bottom-bar") = "true";
        }
    }

    // Words
    std::map<const Word *, int> wordMap; // Map words to ids
    {
        int id = 1;
        Clues & clues = puz->GetClues();
        Clues::iterator clues_it;
        for (clues_it = clues.begin(); clues_it != clues.end(); ++clues_it)
        {
            ClueList & cluelist = clues_it->second;
            ClueList::iterator it;
            for (it = cluelist.begin(); it != cluelist.end(); ++it)
            {
                Word * word = &it->GetWord();
                xml::node xmlword = crossword.append_child("word");
                xmlword.append_attribute("id") = id;
                wordMap[word] = id;
                ++id;
                square_iterator it;
                for (it = word->begin(); it != word->end(); ++it)
                {
                    xml::node cell = xmlword.append_child("cells");
                    cell.append_attribute("x") = it->GetCol() + 1;
                    cell.append_attribute("y") = it->GetRow() + 1;
                }
            }
        }
    }

    // Clues
    {
        Clues & clues = puz->GetClues();
        Clues::iterator clues_it;
        for (clues_it = clues.begin(); clues_it != clues.end(); ++clues_it)
        {
            ClueList & cluelist = clues_it->second;
            xml::node xmlclues = crossword.append_child("clues");
            xml::SetInnerXML(xmlclues.append_child("title"),
                             cluelist.GetTitle());
            ClueList::iterator it;
            for (it = cluelist.begin(); it != cluelist.end(); ++it)
            {
                xml::node clue = xmlclues.append_child("clue");
                clue.append_attribute("number") =
                    encode_utf8(it->GetNumber()).c_str();
                clue.append_attribute("word") = wordMap[&it->GetWord()];
                xml::SetInnerXML(clue, it->GetText(), &AppendChildWithSpanWrapping);
            }
        }
    }

    // Save to a zip file.
    zip::Archive archive(filename);
    if (! archive)
        throw FileError(filename);
    // Open a zip in the archive with the filename part of the path
    zip::File & file = archive.OpenFile(
        filename.substr(filename.find_last_of("/\\") + 1));
    // The XML writer instance
    xml_zip_writer writer(file);
    doc.save(writer, "");
}

} // namespace puz
