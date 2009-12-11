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

    wxString m_filename;
    bool m_modified;

    int m_time;
    bool m_isTimerRunning;

    class Clue;
    typedef std::vector<Clue> ClueList;

    wxString m_title;
    wxString m_author;
    wxString m_copyright;
    wxString m_notes;

    // m_clues holds the actual clues
    std::vector<wxString> m_clues;

    // m_across and m_down hold copies of the clues (ref-counted)
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


// This just makes access to the clues nicer:
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
    m_clues.clear();
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