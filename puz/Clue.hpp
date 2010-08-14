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


#ifndef PUZ_CLUE_H
#define PUZ_CLUE_H

#include <vector>
#include <map>
#include "puzstring.hpp"
#include <cassert>

namespace puz {

// A clue, with a number and text pointer.
// The clue number is stored as a string, and can be any text.
class PUZ_API Clue
{
public:
    explicit Clue(const string_t & _number = puzT(""),
                  const string_t & _text = puzT(""))
    {
        SetNumber(_number);
        SetText(_text);
    }

    explicit Clue(int _number,
                  const string_t & _text = puzT(""))
    {
        SetNumber(_number);
        SetText(_text);
    }

    void SetText  (const string_t & _text);
    void SetNumber(const string_t & _number);
    void SetNumber(int _number);

    const string_t & GetText()   const { return text; }
    const string_t & GetNumber() const { return number; }

    // -1 if this is an invalid number.
    int GetInt() const { return m_int; }

    bool operator==(const Clue & other) const
        { return other.number == number && other.text == text; }

    // This makes sorting a lot easier
    bool operator<(const Clue & other) const
    {
        assert(GetInt() != 0 && other.GetInt() != 0);
        return GetInt() < other.GetInt();
    }

    string_t number;
    string_t text;

protected:
    int m_int;
};



class PUZ_API ClueList : public std::vector<Clue>
{
public:
    // Basic constructor
    ClueList() : std::vector<Clue>() {}

    // Find by clue number (string)
    const_iterator Find(const string_t & number) const;
    iterator Find(const string_t & number);

    // Find by clue number (int)
    const_iterator Find(int num) const;
    iterator Find(int num);
};


class Puzzle; // friend

// This class holds all of the clues
class PUZ_API Clues
{
    friend class Puzzle;
public:
    Clues() {}

    const ClueList & GetAcross() const { return m_across; }
          ClueList & GetAcross()       { return m_across; }
    const ClueList & GetDown()   const { return m_down; }
          ClueList & GetDown()         { return m_down; }

    ClueList & GetClues(const string_t & direction);
    const ClueList & GetClues(const string_t & direction) const;
    bool HasClues(const string_t & direction) const;

    void clear()
    {
        m_across.clear();
        m_down.clear();
        m_otherClues.clear();
    }

protected:
    ClueList m_across;
    ClueList m_down;

    // This holds nonstandard clues, e.g. "Diagonal"
    typedef std::map<string_t, ClueList> cluemap_t;
    cluemap_t m_otherClues;
};

} // namespace puz

#endif // PUZ_CLUE_H
