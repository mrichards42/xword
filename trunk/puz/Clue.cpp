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

#include "Clue.hpp"
#include "exceptions.hpp"
#include "util.hpp"

namespace puz {

// ---------------------------------------------------------------------------
// Clue
// ---------------------------------------------------------------------------

void Clue::SetNumber(const string_t & _number)
{
    number = _number;
    EscapeXML(number);
    m_int = ToInt(number);
}

void Clue::SetNumber(int _number)
{
    m_int = _number;
    number = ToString(_number);
}

void Clue::SetText(const string_t & _text)
{
    SetFormatted(text, _text);
}

// ---------------------------------------------------------------------------
// ClueList
// ---------------------------------------------------------------------------

ClueList::const_iterator ClueList::Find(int number) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetInt() == number)
            break;
    return it;
}

ClueList::iterator ClueList::Find(int number)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetInt() == number)
            break;
    return it;
}

ClueList::const_iterator ClueList::Find(const string_t & number) const
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetNumber() == number)
            break;
    return it;
}

ClueList::iterator ClueList::Find(const string_t & number)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
        if (it->GetNumber() == number)
            break;
    return it;
}

// ---------------------------------------------------------------------------
// Clues
// ---------------------------------------------------------------------------

ClueList & Clues::GetClues(const string_t & direction)
{
    if (direction == puzT("Across"))
        return m_across;
    else if (direction == puzT("Down"))
        return m_down;
    else
    {
        cluemap_t::iterator it;
        it = m_otherClues.find(direction);
        if (it == m_otherClues.end())
            throw NoClues(std::string("Unkonw clue direction: ") + encode_utf8(direction));
        return it->second;
    }
}

const ClueList & Clues::GetClues(const string_t & direction) const
{
    if (direction == puzT("Across"))
        return m_across;
    else if (direction == puzT("Down"))
        return m_down;
    else
    {
        cluemap_t::const_iterator it;
        it = m_otherClues.find(direction);
        if (it == m_otherClues.end())
            throw NoClues(std::string("Unkonw clue direction: ") + encode_utf8(direction));
        return it->second;
    }
}


bool Clues::HasClues(const string_t & direction) const
{
    try {
        GetClues(direction);
        return true;
    }
    catch (NoClues &) {
        return false;
    }
}

} // namespace puz
