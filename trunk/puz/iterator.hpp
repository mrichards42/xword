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

#ifndef PUZ_ITERATOR_H
#define PUZ_ITERATOR_H

#include "Square.hpp"
#include "memory"

namespace puz {

// base class for iterator implementation
class iterator_impl
{
public:
    iterator_impl() {}
    virtual iterator_impl * clone()=0;
    virtual void increment()=0;
    virtual void decrement()=0;
    virtual Square * ptr()=0;
    virtual Square & ref()=0;
};

// ----------------------------------------------------------------------------
// The square iterator
// ----------------------------------------------------------------------------
template <typename Square_T, bool INC = true>
    class square_iterator_t
{
public:
    typedef square_iterator_t<Square_T, INC> self_t;

    explicit square_iterator_t<Square_T, INC>(iterator_impl * it = NULL)
        : m_impl(it)
    {}

    // Copy constructor: make a copy of m_impl
    square_iterator_t<Square_T, INC>(const self_t & other)
        : m_impl(other.m_impl.get() ? other.m_impl->clone() : NULL)
    {}

    // Assignment
    self_t & operator=(const self_t & other)
    {
        m_impl.reset(other.m_impl.get() ? other.m_impl->clone() : NULL);
        return *this;
    }

    // Comparison
    bool operator==(const self_t & other) const
    {
        return m_impl->ptr() == other.m_impl->ptr();
    }

    bool operator!=(const self_t & other) const
    {
        return m_impl->ptr() != other.m_impl->ptr();
    }

    // Prefix operators
    self_t & operator++()
    {
        increment();
        return *this;
    }

    self_t & operator--()
    {
        decrement();
        return *this;
    }

    // Postfix operators
    self_t operator++(int)
    {
        self_t it = *this;
        increment();
        return it;
    }

    self_t operator--(int)
    {
        self_t it = *this;
        decrement();
        return it;
    }

    // Pointer operators
    Square_T & operator*() { return m_impl->ref(); }
    Square_T * operator->() { return &*m_impl->ptr(); }

protected:
    std::auto_ptr<iterator_impl> m_impl;

    // Template-specific functions
    void increment() { m_impl->increment(); }
    void decrement() { m_impl->decrement(); }
};

// Swap increment/decrement for reverse iterators
void square_iterator_t<Square, false>::increment()
{
    m_impl->decrement();
}

void square_iterator_t<Square, false>::decrement()
{
    m_impl->increment();
}

void square_iterator_t<const Square, false>::increment()
{
    m_impl->decrement();
}

void square_iterator_t<const Square, false>::decrement()
{
    m_impl->increment();
}

typedef square_iterator_t<Square, true> square_iterator;
typedef square_iterator_t<const Square, true> const_square_iterator;
typedef square_iterator_t<Square, false> square_reverse_iterator;
typedef square_iterator_t<const Square, false> const_square_reverse_iterator;
/*
// ----------------------------------------------------------------------------
// Reverse iterator
// ----------------------------------------------------------------------------

template <typename Square_T>
    class square_reverse_iterator_t
{
public:
    explicit square_reverse_iterator_t<Square_T>(iterator_impl * it = NULL) : m_impl(it) {}

    // Copy constructor: make a copy of m_impl
    square_reverse_iterator_t<Square_T>(const square_reverse_iterator_t<Square_T> & other)
        : m_impl(other.m_impl.get() ? other.m_impl->clone() : NULL)
    {}

    // Assignment
    square_reverse_iterator_t<Square_T> & operator=(const square_reverse_iterator_t<Square_T> other)
    {
        m_impl.reset(other.m_impl.get() ? other.m_impl->clone() : NULL);
        return *this;
    }

    // Prefix operators
    square_reverse_iterator_t<Square_T> & operator++()
    {
        m_impl->decrement();
        return *this;
    }

    square_reverse_iterator_t<Square_T> & operator--()
    {
        m_impl->increment();
        return *this;
    }

    // Postfix operators
    square_reverse_iterator_t<Square_T> operator++(int)
    {
        square_reverse_iterator_t<Square_T> it = *this;
        m_impl->decrement();
        return it;
    }

    square_reverse_iterator_t<Square_T> operator--(int)
    {
        square_reverse_iterator_t<Square_T> it = *this;
        m_impl->increment();
        return it;
    }

    // Pointer operators
    Square_T & operator*() { return m_impl->ref(); }
    Square_T * operator->() { return &*m_impl->ptr(); }

protected:
    std::auto_ptr<iterator_impl> m_impl;
};

typedef square_reverse_iterator_t<Square> square_reverse_iterator;
typedef square_reverse_iterator_t<const Square> const_square_reverse_iterator;
*/

// ----------------------------------------------------------------------------
// Implementation classes
// ----------------------------------------------------------------------------

// Basic iterator
// This is a drop-in replacement for the usual construction:
// for (square = grid.First(); square != NULL; square = square->Next())
// Instead you can use
// for (square_iterator it = grid.begin(); it != grid.end(); ++it)
class grid_iterator_impl : public iterator_impl
{
public:
    grid_iterator_impl(Square * square = NULL) : m_square(square) {}

    virtual iterator_impl * clone()
    {
        return new grid_iterator_impl(m_square);
    }

    virtual void increment()
    {
        if (m_square == NULL)
            throw std::out_of_range("Square out of range");
        m_square = m_square->Next();
    }

    virtual void decrement()
    {
        if (m_square == NULL)
            throw std::out_of_range("Square out of range");
        m_square = m_square->Prev();
    }

    virtual Square * ptr() { return m_square; }
    virtual Square & ref() { return *m_square; }

protected:
    Square * m_square;
};

} // namespace puz

#endif // PUZ_ITERATOR_H

