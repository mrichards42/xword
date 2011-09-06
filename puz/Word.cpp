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

#include "Word.hpp"
#include "Square.hpp"
#include "iterator.hpp"

namespace puz {

bool Word::Contains(const Square * square) const
{
    for (const_square_iterator it = begin(); it != end(); ++it)
        if (&*it == square)
            return true;
    return false;
}

// Default implementation returns a calculated direction
short Word::GetDirection() const
{
    return puz::GetDirection(*front(), *back());
}

const Square * Word::front() const
{
    return const_cast<Word *>(this)->front();
}

const Square * Word::back() const
{
    return const_cast<Word *>(this)->back();
}


// Default iterator implementations
square_iterator Word::begin()
{
    return square_iterator(new_iterator_impl(front()));
}

square_iterator Word::end()
{
    iterator_impl * it = new_iterator_impl(back());
    it->increment();
    return square_iterator(it);
}

square_reverse_iterator Word::rbegin()
{
    return square_reverse_iterator(new_iterator_impl(back()));
}

square_reverse_iterator Word::rend()
{
    iterator_impl * it = new_iterator_impl(front());
    it->decrement();
    return square_reverse_iterator(it);
}

// Const overloads
const_square_iterator Word::begin() const
{
    return const_square_iterator(new_iterator_impl(front()));
}

const_square_iterator Word::end() const
{
    iterator_impl * it = new_iterator_impl(back());
    it->increment();
    return const_square_iterator(it);
}

const_square_reverse_iterator Word::rbegin() const
{
    return const_square_reverse_iterator(new_iterator_impl(back()));
}

const_square_reverse_iterator Word::rend() const
{
    iterator_impl * it = new_iterator_impl(front());
    it->decrement();
    return const_square_reverse_iterator(it);
}


// const_cast is OK here, because the square_iterator class will prevent
// non-const access to a const iterator.
iterator_impl * Word::new_iterator_impl(const Square * square) const
{
    return const_cast<Word *>(this)->new_iterator_impl(
        const_cast<Square *>(square));
}


StraightWord::StraightWord(Square * start, Square * end)
    : m_front(start), m_back(end)
{
    short direction = puz::GetDirection(*start, *end);
    if ((direction % 45) != 0)
        throw NoWord();
    m_direction = static_cast<GridDirection>(direction);
}

// Straight word iterator
class straight_word_impl : public iterator_impl
{
public:
    straight_word_impl(GridDirection dir, Square * square)
        : m_direction(dir), m_square(square)
    {}

    virtual iterator_impl * clone()
    {
        return new straight_word_impl(m_direction, m_square);
    }

    virtual void increment()
    {
        m_square = m_square->Next(m_direction);
    }

    virtual void decrement()
    {
        m_square = m_square->Prev(m_direction);
    }

    virtual Square * ptr() { return m_square; }
    virtual Square & ref() { return *m_square; }

protected:
    GridDirection m_direction;
    Square * m_square;
};

iterator_impl * StraightWord::new_iterator_impl(Square * square)
{
    return new straight_word_impl(m_direction, square);
}


} // namespace puz
