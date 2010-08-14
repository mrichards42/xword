// This file is part of XWord    
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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


#ifndef PUZ_PUZZLE_H
#define PUZ_PUZZLE_H

#include "Grid.hpp"
#include "Clue.hpp"
#include "puzstring.hpp"
#include <vector>

namespace puz {

class PUZ_API Puzzle
{
public:
    struct PUZ_API FileHandlerDesc;

    explicit Puzzle()
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false)
    {}

    explicit Puzzle(const string_t & filename,
                    const FileHandlerDesc * desc = NULL)
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false)
    {
        Load(filename, desc);
    }

    ~Puzzle() {}

    void Load(const string_t & filename,
              const FileHandlerDesc * handler = NULL);
    void Save(const string_t & filename,
              const FileHandlerDesc * handler = NULL);

    void Clear();
    bool IsOk()        const { return m_isOk; }
    void SetOk(bool ok=true) { m_isOk = ok; }
    bool IsScrambled() const { return m_grid.IsScrambled(); }
    short GetVersion() const { return m_version; }

    // Verify that the puzzle is valid and raise an exception if it is not.
    void TestOk();

    // Getter / Setters
    //-----------------
    int GetTime() const { return m_time; }
    void SetTime(int time) { m_time = time; }

    bool IsTimerRunning() const { return m_isTimerRunning; }
    void SetTimerRunning(bool doit) { m_isTimerRunning = doit; }

    const string_t & GetTitle() const { return m_title; }
    void SetTitle(const string_t & title) { m_title = title; }

    const string_t & GetAuthor() const { return m_author; }
    void SetAuthor(const string_t & author) { m_author = author; }

    const string_t & GetCopyright() const { return m_copyright; }
    void SetCopyright(const string_t & copyright) { m_copyright = copyright; }

    const string_t & GetNotes() const { return m_notes; }
    void SetNotes(const string_t & notes) { SetFormatted(m_notes, notes); }

    // errors are 8-bit strings
    const std::string & GetError() const { return m_lastError; }
    void SetError(const std::string & err) { m_lastError = err; }
    void ClearError() { m_lastError.clear(); }
    bool HasError() const { return ! m_lastError.empty(); }

    // Clues
    //------
    const ClueList & GetAcross() const { return m_clues.GetAcross(); }
          ClueList & GetAcross()       { return m_clues.GetAcross(); }
    void SetAcross(const ClueList & across) { m_clues.GetAcross() = across; }

    const ClueList & GetDown() const { return m_clues.GetDown(); }
          ClueList & GetDown()       { return m_clues.GetDown(); }
    void SetDown(const ClueList & down) { m_clues.GetDown() = down; }

    const ClueList & GetClues(const string_t & direction) const
        { return m_clues.GetClues(direction); }
    ClueList & GetClues(const string_t & direction)
          { return m_clues.GetClues(direction); }

    // Grid
    // ----
    const Grid & GetGrid() const { return m_grid; }
          Grid & GetGrid()       { return m_grid; }
    void SetGrid(const Grid & grid) { m_grid = grid; }

    // Grid / clue numbering algorithm
    //--------------------------------
    // Assumes that Across and Down are in order but not numbered.
    void NumberClues();
    // Assumes that there are no preexisting clues and writes the clues
    // in "Across Lite" order given a vector of clues.
    void SetAllClues(const std::vector<string_t> & clues);
    void NumberGrid();

    // -------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------
    int m_time;
    bool m_isTimerRunning;

    string_t m_title;
    string_t m_author;
    string_t m_copyright;
    string_t m_notes;

    Clues m_clues;

    Grid m_grid;

    // Extra unknown sections for persistence between load and save
    struct PUZ_API section;
    std::vector<section> m_extraSections;

    // -------------------------------------------------------------------
    // Load / Save
    // -------------------------------------------------------------------
    typedef void (*FileHandler)(Puzzle *, const string_t &, void *);

    // The load and save functions are passed a void pointer with whatever
    // data the handler needs. This is kind of hackish and mostly just
    // for lua, but it might be useful at some other point.
    struct PUZ_API FileHandlerDesc
    {
        FileHandler handler;
        const char_t * ext;
        const char_t * desc;
        void * data;
    };

    static const FileHandlerDesc sm_loadHandlers[];
    static const FileHandlerDesc sm_saveHandlers[];

    static bool CanLoad(const string_t & filename);
    static bool CanSave(const string_t & filename);

    static const FileHandlerDesc * FindLoadHandler(const string_t & ext);
    static const FileHandlerDesc * FindSaveHandler(const string_t & ext);

protected:
    bool m_isOk;
    short m_version;

private:
    std::string m_lastError;

    void TestClueList(const string_t & direction);
};



// Sections use 8-bit strings
struct PUZ_API Puzzle::section
{
    section(const std::string & a_name, const std::string & a_data)
        : name(a_name), data(a_data)
    {}
    std::string name;
    std::string data;
};



inline void
Puzzle::Clear()
{
    m_isOk = false;
    m_grid.Clear();
    m_clues.clear();
    m_title.clear();
    m_author.clear();
    m_copyright.clear();
    m_notes.clear();
    m_time = 0;
    m_isTimerRunning = false;
    m_extraSections.clear();
}

} // namespace puz

#endif // PUZ_PUZZLE_H
