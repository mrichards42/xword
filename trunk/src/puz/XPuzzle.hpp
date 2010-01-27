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


#ifndef X_PUZZLE_H
#define X_PUZZLE_H

#include <wx/string.h>
#include <vector>
#include "XGrid.hpp"
#include "../utils/ByteArray.hpp"

// Friends
class XPuzzleModule;
class HandlerBase;

class XPuzzle
{
    friend class XPuzzleModule;
    friend class HandlerBase;
public:

    class Clue;
    class ClueList;

    explicit XPuzzle(const wxString & filename = wxEmptyString)
    {
        m_modified = false;
        m_time = 0;
        m_isTimerRunning = false;
        m_version = 13;
        m_isOk = false;
        if (! filename.empty())
            Load(filename);
    }

    ~XPuzzle() {}

    bool Load(const wxString & filename, wxString ext = wxEmptyString);
    bool Save(const wxString & filename, wxString ext = wxEmptyString);

    void Clear();
    bool IsOk()        const { return m_isOk; }
    void SetOk(bool doit=true) { m_isOk = doit; }
    bool IsScrambled() const { return m_grid.IsScrambled(); }
    short GetVersion() const { return m_version; }

    // Getter / Setters
    //-----------------
    const wxString & GetFilename() const { return m_filename; }
    void SetFilename(const wxString & filename) { m_filename = filename; }

    bool GetModified() const { return m_modified; }
    void SetModified(bool modified) { m_modified = modified; }

    int GetTime() const { return m_time; }
    void SetTime(int time) { m_time = time; }

    const wxString & GetTitle() const { return m_title; }
    void SetTitle(const wxString & title) { m_title = title; }

    const wxString & GetAuthor() const { return m_author; }
    void SetAuthor(const wxString & author) { m_author = author; }

    const wxString & GetCopyright() const { return m_copyright; }
    void SetCopyright(const wxString & copyright) { m_copyright = copyright; }

    const wxString & GetNotes() const { return m_notes; }
    void SetNotes(const wxString & notes) { m_notes = notes; }

    // Clues
    //------
    ClueList & GetAcross() { return m_across; }
    void SetAcross(const ClueList & across) { m_across = across; }

    ClueList & GetDown() { return m_down; }
    void SetDown(const ClueList & down) { m_down = down; }

    // The non-const version attempts to setup the grid if it is not already setup.
    bool GetClueList(std::vector<wxString> * clues);
    bool GetClueList(std::vector<wxString> * clues) const;

    // Return false if the clues don't match the grid
    bool SetClueList(const std::vector<wxString> & clues);

    // Call this after SetAcross / SetDown if you are unable to determine the clue numbers
    void RenumberClues();

    XGrid & GetGrid() { return m_grid; }

    // XGrid has pointers to its data members.  These won't be copied, and the pointers will
    // refer to non-existant XSquares once the grid parameter is destroyed.  Therefore, we need
    // to setup the grid independently in this function.
    void SetGrid(const XGrid & grid) { m_grid = grid; m_grid.SetupGrid(); }


    //--------------------------------------------------------------------
    // Clue and ClueList
    //--------------------------------------------------------------------
    // This makes access to the clues nicer:
    class XPuzzle::Clue
    {
    public:
        Clue(int num, const wxString & str)
            : m_num(num), m_str(str)
        {}

        const wxString & Text()   const { return m_str; }
              wxString & Text()         { return m_str; }
              int        Number() const { return m_num; }

        int m_num;
        wxString m_str;
        bool operator==(const XPuzzle::Clue & other) const
            { return other.m_num == m_num && other.m_str == m_str; }

        // This makes sorting a lot easier
        bool operator<(const XPuzzle::Clue & other) const
            { return m_num < other.m_num; }
    };


    class XPuzzle::ClueList : public std::vector<XPuzzle::Clue>
    {
        typedef std::vector<XPuzzle::Clue> _base;
    public:
        // Basic constructor
        ClueList() : _base() {}

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


    //--------------------------------------------------------------------
    // Members
    //--------------------------------------------------------------------
    wxString m_filename;
    bool m_modified;

    int m_time;
    bool m_isTimerRunning;

    wxString m_title;
    wxString m_author;
    wxString m_copyright;
    wxString m_notes;

    ClueList m_across;
    ClueList m_down;

    XGrid m_grid;


    // Extra unknown sections for persistence between load and save
    struct section;
    std::vector<section> m_extraSections;

    static wxString GetLoadTypeString();
    static wxString GetSaveTypeString();
    static bool CanLoad(const wxString & ext);
    static bool CanSave(const wxString & ext);

    static void AddHandler(HandlerBase * handler);

protected:
    bool m_isOk;
    short m_version;

    // Const / non-const helpers
    bool DoGetClueList(std::vector<wxString> * clues) const;

private:
    // Load / save stuff
    bool Load(const wxString & filename, HandlerBase * handler);
    bool Save(const wxString & filename, HandlerBase * handler);

    static HandlerBase * PromptForSaveHandler(const wxString & message);
    static HandlerBase * PromptForLoadHandler(const wxString & message);

    static HandlerBase * GetHandler(const wxString & ext);

    static void InitHandlers();
    static void CleanUpHandlers();

    static std::vector<HandlerBase *> sm_handlers;
};



struct XPuzzle::section
{
    section(const wxString & a_name, const ByteArray & a_data)
        : name(a_name), data(a_data)
    {}
    wxString name;
    ByteArray data;
};



inline void
XPuzzle::Clear()
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


#endif // X_PUZZLE_H