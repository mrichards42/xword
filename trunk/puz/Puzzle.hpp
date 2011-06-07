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


#ifndef PUZ_PUZZLE_H
#define PUZ_PUZZLE_H

#include "Grid.hpp"
#include "Clue.hpp"
#include "Word.hpp"
#include "puzstring.hpp"
#include <vector>
#include <cassert>
#include <memory>

namespace puz {

class PUZ_API Puzzle
{
public:
    struct PUZ_API FileHandlerDesc;

    explicit Puzzle()
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false),
          m_formatData(NULL)
    {}

    explicit Puzzle(const std::string & filename,
                    const FileHandlerDesc * desc = NULL)
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false),
          m_formatData(NULL)
    {
        Load(filename, desc);
    }

    ~Puzzle() {}

    void Load(const std::string & filename,
              const FileHandlerDesc * handler = NULL);

    void Save(const std::string & filename,
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
    void SetNotes(const string_t & notes) { m_notes = notes; }

    // Words
    const Word * FindWord(const puz::Square * square) const;
          Word * FindWord(const puz::Square * square);
    const Word * FindWord(const puz::Square * square, short direction) const;
          Word * FindWord(const puz::Square * square, short direction);

    Word MakeWord(int x1, int y1, int x2, int y2);
    Word MakeWord(const Square * start, const Square * end)
    {
        return MakeWord(start->GetCol(), start->GetRow(),
                        end->GetCol(), end->GetRow());
    }
    Word * AddWord(int x1, int y1, int x2, int y2)
        { return AddWord(MakeWord(x1, y1, x2, y2)); }
    Word * AddWord(const Square * start, const Square * end)
        { return AddWord(MakeWord(start, end)); }
    Word * AddWord(const Word & word)
        { m_words.push_back(word); return &m_words.back(); }

    const WordList & GetWords() const { return m_words; }
          WordList & GetWords() { return m_words; }

    // Clues
    //------
    // Set Square::HasClue(dir) for each numbered clue in the given direction.
    const Clues & GetClues() const { return m_clues; }
          Clues & GetClues()       { return m_clues; }

    const Clue * FindClue(const puz::Square * square) const;
          Clue * FindClue(const puz::Square * square);
    const Clue * FindClue(const puz::Word * word) const;
          Clue * FindClue(const puz::Word * word);

    bool HasClueList(const string_t & direction) const
        { return m_clues.HasClueList(direction); }

    const ClueList & GetClueList(const string_t & direction) const
        { return m_clues.GetClueList(direction); }
    ClueList & GetClueList(const string_t & direction)
          { return m_clues.GetClueList(direction); }

    ClueList & SetClueList(const string_t & direction, const ClueList & clues)
          { return m_clues.SetClueList(direction, clues); }

    // Grid
    // ----
    const Grid & GetGrid() const { return m_grid; }
          Grid & GetGrid()       { return m_grid; }
    void SetGrid(const Grid & grid) { m_grid = grid; }

    // Grid / clue / word numbering algorithms
    //----------------------------------------
    void NumberGrid();
    // NumberClues assumes that Across and Down are in order but not numbered.
    void NumberClues();
    // GenerateWords requires that all clues are "Across", "Down", or "Diagonal",
    // and all clue numbers have a matching square number
    void GenerateWords();
    // Writes the clues in "Across Lite" order given a vector of clues.
    void SetAllClues(const std::vector<string_t> & clues);
    // Was this puzzle set up with NumberGrid and NumberClues?
    // Clues may only be "Across" and "Down", words must start and end
    // where they are expected to, and there must be no unclued words.
    bool UsesNumberAlgorithm() const;

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
    WordList m_words;


    // -------------------------------------------------------------------
    // Load / Save
    // -------------------------------------------------------------------
    typedef void (*FileHandler)(Puzzle *, const std::string &, void *);

    // The load and save functions are passed a void pointer with whatever
    // data the handler needs. This is kind of hackish and mostly just
    // for lua, but it might be useful at some other point.
    struct PUZ_API FileHandlerDesc
    {
        FileHandler handler;
        const char * ext;
        const char_t * desc;
        void * data;
    };

    static const FileHandlerDesc sm_loadHandlers[];
    static const FileHandlerDesc sm_saveHandlers[];

    static bool CanLoad(const std::string & filename);
    static bool CanSave(const std::string & filename);

    static const FileHandlerDesc * FindLoadHandler(const std::string & ext);
    static const FileHandlerDesc * FindSaveHandler(const std::string & ext);

    // Load/save data
    // This class should be derived from to implement a puzzle format-specific
    // userdata that can be used to save unknown sections.
    class FormatData
    {
    public:
        FormatData() {}
        virtual ~FormatData() {}
    };

    void SetFormatData(FormatData * data)
    {
        assert(! m_formatData.get());
        m_formatData.reset(data);
    }

    FormatData * GetFormatData() { return m_formatData.get(); }


protected:
    bool m_isOk;
    short m_version;
    std::auto_ptr<FormatData> m_formatData;

private:
    void TestClueList(const string_t & direction);
    void DoLoad(const std::string & filename, const FileHandlerDesc * desc);
};



inline void
Puzzle::Clear()
{
    m_isOk = false;
    m_grid.Clear();
    m_clues.clear();
    m_words.clear();
    m_title.clear();
    m_author.clear();
    m_copyright.clear();
    m_notes.clear();
    m_time = 0;
    m_isTimerRunning = false;
}

} // namespace puz

#endif // PUZ_PUZZLE_H
