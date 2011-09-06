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

#include "Clue.hpp"
#include "exceptions.hpp"
#include <sstream>

namespace puz {

// ---------------------------------------------------------------------------
// Clue
// ---------------------------------------------------------------------------

void Clue::SetNumber(const string_t & _number)
{
    number = _number;
    m_int = ToInt(number);
}

void Clue::SetNumber(int _number)
{
    m_int = _number;
    number = ToString(_number);
}

void Clue::SetText(const string_t & _text)
{
    text = _text;
}

// ---------------------------------------------------------------------------
// ClueList
// ---------------------------------------------------------------------------

const Clue * ClueList::Find(int number) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetInt() == number)
            break;
    if (it == end())
        return NULL;
    return &*it;
}

Clue * ClueList::Find(int number)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetInt() == number)
            break;
    if (it == end())
        return NULL;
    return &*it;
}

const Clue * ClueList::Find(const string_t & number) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->number == number)
            break;
    if (it == end())
        return NULL;
    return &*it;
}

Clue * ClueList::Find(const string_t & number)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->number == number)
            break;
    if (it == end())
        return NULL;
    return &*it;
}


const Clue * ClueList::Find(const puz::Word * word) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->word == word)
            break;
    if (it == end())
        return NULL;
    return &*it;
}

Clue * ClueList::Find(const puz::Word * word)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->word == word)
            break;
    if (it == end())
        return NULL;
    return &*it;
}


const Clue * ClueList::Find(const puz::Square * square) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->word->Contains(square))
            break;
    if (it == end())
        return NULL;
    return &*it;
}

Clue * ClueList::Find(const puz::Square * square)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->word->Contains(square))
            break;
    if (it == end())
        return NULL;
    return &*it;
}


// ---------------------------------------------------------------------------
// Clues
// ---------------------------------------------------------------------------

ClueList & Clues::GetClueList(const string_t & direction)
{
    iterator it = find(direction);
    if (it == end())
        throw NoClues(std::string("Unknown clue direction: ") + encode_utf8(direction));
    return it->second;
}

const ClueList & Clues::GetClueList(const string_t & direction) const
{
    const_iterator it = find(direction);
    if (it == end())
        throw NoClues(std::string("Unknown clue direction: ") + encode_utf8(direction));
    return it->second;
}

bool Clues::HasClueList(const string_t & direction) const
{
    return find(direction) != end();
}

bool Clues::HasWords() const
{
    for (const_iterator it = begin(); it != end(); ++it)
    {
        ClueList::const_iterator clue;
        for (clue = it->second.begin(); clue != it->second.end(); ++clue)
            if (clue->GetWord() != NULL)
                return true;
    }
    return false;
}

ClueList & Clues::operator[](const string_t & direction)
{
    iterator it = find(direction);
    if (it == end())
        return insert(std::make_pair(direction, ClueList(direction))).first->second;
    return it->second;
}

} // namespace puz
