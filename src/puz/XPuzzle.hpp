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
#include "PuzLoader.hpp"


class XPuzzle
{
public:
    explicit XPuzzle(const wxString & filename = wxEmptyString)
    {
        m_isOk = false;
        if (! filename.empty())
            Load(filename);
    }

    ~XPuzzle() {}

    bool Load(const wxString & filename, const wxString & ext = wxEmptyString);
    bool Save(const wxString & filename, const wxString & ext = wxEmptyString);

    void Clear();
    bool IsOk() { return m_isOk; }

    wxString m_filename;
    bool m_modified;

    int m_time;
    bool m_complete;

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

    // This is here so we don't have to include HandlerCommon.hpp
    // There are issues with the timing of includes, etc.
    typedef std::vector<unsigned char> ByteArray;

    struct section
    {
        section(const wxString & a_name, const ByteArray & a_data)
            : name(a_name), data(a_data)
        {}
        wxString name;
        ByteArray data;
    };

    // Extra random sections . . . just save them and append to the file later
    std::vector<section> m_extraSections;

protected:
    bool m_isOk;
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




inline bool
XPuzzle::Load(const wxString & filename, const wxString & ext)
{
    m_modified = false;
    m_isOk = PuzLoader::Load(this, filename, ext);
    return m_isOk;
}


inline bool
XPuzzle::Save(const wxString & filename, const wxString & ext)
{
    m_modified = false;
    return PuzLoader::Save(this, filename, ext);
}


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
    m_complete = false;
    m_extraSections.clear();
}


#endif // X_PUZZLE_H