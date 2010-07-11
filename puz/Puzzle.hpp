// This file is part of XWord    
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
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

#include <string>
#include <vector>
#include "Grid.hpp"
#include <iostream>

namespace puz {

class PUZ_API Puzzle
{
public:
    struct PUZ_API FileHandlerDesc;
    class PUZ_API Clue;
    class PUZ_API ClueList;

    explicit Puzzle()
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false)
    {}

    explicit Puzzle(const std::string & filename,
                    const FileHandlerDesc * desc = NULL)
        : m_time(0),
          m_isTimerRunning(false),
          m_version(13),
          m_isOk(false)
    {
        Load(filename, desc);
    }

    ~Puzzle() {}

    void Load(const std::string & filename, const FileHandlerDesc * handler = NULL);
    void Save(const std::string & filename, const FileHandlerDesc * handler = NULL);

    void Clear();
    bool IsOk()        const { return m_isOk; }
    void SetOk(bool doit=true) { m_isOk = doit; }
    bool IsScrambled() const { return m_grid.IsScrambled(); }
    short GetVersion() const { return m_version; }

    // Verify that the puzzle is valid and raise an exception if it is not.
    void TestOk() const;

    // Getter / Setters
    //-----------------
    int GetTime() const { return m_time; }
    void SetTime(int time) { m_time = time; }

    bool IsTimerRunning() const { return m_isTimerRunning; }
    void SetTimerRunning(bool doit) { m_isTimerRunning = doit; }

    const std::string & GetTitle() const { return m_title; }
    void SetTitle(const std::string & title) { m_title = title; }

    const std::string & GetAuthor() const { return m_author; }
    void SetAuthor(const std::string & author) { m_author = author; }

    const std::string & GetCopyright() const { return m_copyright; }
    void SetCopyright(const std::string & copyright) { m_copyright = copyright; }

    const std::string & GetNotes() const { return m_notes; }
    void SetNotes(const std::string & notes) { m_notes = notes; }

    const std::string & GetError() const { return m_lastError; }
    void SetError(const std::string & err) { m_lastError = err; }
    void ClearError() { m_lastError.clear(); }
    bool HasError() const { return ! m_lastError.empty(); }

    // Clues
    //------
    ClueList & GetAcross() { return m_across; }
    void SetAcross(const ClueList & across) { m_across = across; }

    ClueList & GetDown() { return m_down; }
    void SetDown(const ClueList & down) { m_down = down; }

    // All clues in order
    void GetClueList(std::vector<std::string> * clues) const;
    // The non-const version attempts to setup the grid if it is not already setup.
    void GetClueList(std::vector<std::string> * clues);

    void SetClueList(const std::vector<std::string> & clues);

    // Call this after SetAcross / SetDown if you are unable to determine the clue numbers
    void RenumberClues();

    Grid & GetGrid() { return m_grid; }
    void SetGrid(const Grid & grid) { m_grid = grid; }


    //--------------------------------------------------------------------
    // Clue and ClueList
    //--------------------------------------------------------------------
    // This makes access to the clues nicer:
    class PUZ_API Clue
    {
    public:
        explicit Clue(int num=-1, const std::string & str="")
            : m_num(num), m_str(str)
        {}

        const std::string & Text()   const { return m_str; }
              std::string & Text()         { return m_str; }
              int        Number() const { return m_num; }

        int m_num;
        std::string m_str;
        bool operator==(const Puzzle::Clue & other) const
            { return other.m_num == m_num && other.m_str == m_str; }

        // This makes sorting a lot easier
        bool operator<(const Puzzle::Clue & other) const
            { return m_num < other.m_num; }
    };


    class PUZ_API ClueList : public std::vector<Puzzle::Clue>
    {
        typedef std::vector<Puzzle::Clue> _base;
    public:
        // Basic constructor
        Puzzle::ClueList() : _base() {}

        // Clue finding
        //-------------
        const_iterator
        Find(int num) const
        {
            const_iterator it;
            for (it = begin(); it != end(); ++it)
                if (it->Number() == num)
                    break;
            return it;
        }

        iterator
        Find(int num)
        {
            iterator it;
            for (it = begin(); it != end(); ++it)
                if (it->Number() == num)
                    break;
            return it;
        }
    };


    // -------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------
    int m_time;
    bool m_isTimerRunning;

    std::string m_title;
    std::string m_author;
    std::string m_copyright;
    std::string m_notes;

    ClueList m_across;
    ClueList m_down;

    Grid m_grid;


    // Extra unknown sections for persistence between load and save
    struct PUZ_API section;
    std::vector<section> m_extraSections;

    // -------------------------------------------------------------------
    // Load / Save
    // -------------------------------------------------------------------
    typedef void (*FileHandler)(Puzzle *, const std::string &, void *);
    typedef void (*CppLoadHandler)(Puzzle *, std::istream &);
    typedef void (*CppSaveHandler)(Puzzle *, std::ostream &);

    // The load and save functions are passed a void pointer with whatever
    // data the handler needs. This is kind of hackish and mostly just
    // for lua, but it might be useful at some other point.
    struct PUZ_API FileHandlerDesc
    {
        FileHandler handler;
        const char * ext;
        const char * desc;
        void * data;
    };

    static const FileHandlerDesc sm_loadHandlers[];
    static const FileHandlerDesc sm_saveHandlers[];

    static bool CanLoad(const std::string & filename);
    static bool CanSave(const std::string & filename);

    static const FileHandlerDesc * FindLoadHandler(const std::string & ext);
    static const FileHandlerDesc * FindSaveHandler(const std::string & ext);

protected:
    bool m_isOk;
    short m_version;

    // Const / non-const helpers
    void DoGetClueList(std::vector<std::string> * clues) const;

private:
    std::string m_lastError;
};



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
    m_across.clear();
    m_down.clear();
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
