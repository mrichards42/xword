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


#ifndef PUZ_CLUE_H
#define PUZ_CLUE_H

#include <vector>
#include <map>
#include <memory>
#include "puzstring.hpp"
#include "Word.hpp"
#include <cassert>

namespace puz {

class ClueList;

// A clue, with a number, text, and a word.
// The clue number is stored as a string, and can be any text.
class PUZ_API Clue
{
    friend class ClueList;
public:
    explicit Clue(const string_t & num_ = puzT(""),
                  const string_t & text_ = puzT(""),
                  const bool is_html_ = false)
    {
        SetNumber(num_);
        SetText(text_, is_html_);
    }

    explicit Clue(const string_t & num_,
                  const string_t & text_,
                  Word word_,
                  const bool is_html_ = false)
        : word(word_)
    {
        SetNumber(num_);
        SetText(text_, is_html_);
    }

    explicit Clue(int num_, const string_t & text_ = puzT(""), const bool is_html_ = false)
    {
        SetNumber(num_);
        SetText(text_, is_html_);
    }

    explicit Clue(int num_,
                  const string_t & text_,
                  Word word_,
                  const bool is_html_ = false)
        : word(word_)
    {
        SetNumber(num_);
        SetText(text_, is_html_);
    }

    void SetText(const string_t & text_, const bool is_html_ = false);
    void SetNumber(const string_t & num_);
    void SetNumber(int num_);
    void SetWord(const Word & word_) { word = word_; }

    const string_t & GetText() const { return text; }
    const string_t & GetNumber() const { return number; }
    const Word & GetWord() const { return word; }
    Word & GetWord() { return word; }

    // -1 if this is an invalid number.
    int GetInt() const { return m_int; }

    // This makes sorting a lot easier
    bool operator<(const Clue & other) const
    {
        assert(GetInt() != 0 && other.GetInt() != 0);
        return GetInt() < other.GetInt();
    }

    string_t number;
    string_t text;
    Word word;

protected:
    int m_int;
};


class PUZ_API ClueList : public std::vector<Clue>
{
public:
    // Basic constructor
    explicit ClueList(const string_t & title = puzT("")) : m_title(title) {}

    const string_t & GetTitle() const { return m_title; }
    void SetTitle(const string_t & title) { m_title = title; }

    // Find by clue number (string)
    const Clue * Find(const string_t & number) const;
    Clue * Find(const string_t & number);

    // Find by clue number (int)
    const Clue * Find(int num) const;
    Clue * Find(int num);

    // Find by square
    const Clue * Find(const puz::Square * square) const;
    Clue * Find(const puz::Square * square);

    // Find by word
    const Clue * Find(const puz::Word * word) const;
    Clue * Find(const puz::Word * word);

protected:
    string_t m_title;;
};


// This class holds all of the clues
class PUZ_API Clues : public std::vector<std::pair<string_t, ClueList> >
{
public:
    Clues() {}

    const ClueList & GetAcross() const { return GetClueList(puzT("Across")); }
          ClueList & GetAcross()       { return GetClueList(puzT("Across")); }
    const ClueList & GetDown()   const { return GetClueList(puzT("Down")); }
          ClueList & GetDown()         { return GetClueList(puzT("Down")); }

    ClueList & GetClueList(const string_t & direction);
    const ClueList & GetClueList(const string_t & direction) const;

    bool HasClueList(const string_t & direction) const;
    bool HasWords() const; // Do any clues have words?

    ClueList & operator[](const string_t & direction);

    ClueList & SetClueList(const string_t & direction, const ClueList & cluelist)
    {
        // Normalize different capitalizations of "Across" and "Down", since these have special meaning.
        // We still retain the original direction in the title field.
        string_t canonical_direction;
        if (CaseInsensitiveEquals(direction, puzT("Across")))
            canonical_direction = puzT("Across");
        else if (CaseInsensitiveEquals(direction, puzT("Down")))
            canonical_direction = puzT("Down");
        else
            canonical_direction = direction;

        operator[](canonical_direction) = cluelist;
        ClueList & ret = operator[](canonical_direction);
        if (ret.GetTitle().empty())
            ret.SetTitle(direction);
        return ret;
    }

    std::vector<std::pair<string_t, ClueList> >::iterator find(const string_t& direction);
    std::vector<std::pair<string_t, ClueList> >::const_iterator find(const string_t& direction) const;
};

} // namespace puz

#endif // PUZ_CLUE_H
