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
#include <memory>

namespace puz {

// There is a basic deficiency in this iterator implementation in that
// const iterators aren't really possible.
// Storing a pointer to iterator_impl makes it so that Square is always
// returned non-const.  I don't *really* think that is a problem given that
// we don't ever use a const Grid (where the Squares are actually stored).

// Really this just opens up the big can of worms that is the entire
// design of const-correctness in the puz library.

// base class for iterator implementation
class iterator_impl
{
public:
    virtual iterator_impl * clone()=0;
    virtual ~iterator_impl() {}
    virtual void increment()=0;
    virtual void decrement()=0;
    virtual Square * ptr()=0;
    virtual Square & ref()=0;
};

// A null iterator
class null_iterator_impl : public iterator_impl
{
public:
    null_iterator_impl() {}
    null_iterator_impl * clone() { return new null_iterator_impl(); }
    void increment() {}
    void decrement() {}
    Square * ptr() { return NULL; }
    Square & ref() { throw std::exception(); }
};

// ----------------------------------------------------------------------------
// The square iterator (wrapper for implementation classes)
// ----------------------------------------------------------------------------
template <typename Square_T, bool INC = true>
    class square_iterator_t
{
public:
    typedef square_iterator_t<Square_T, INC> self_t;
    // This allows conversion between square_iterator and const_square_iterator
    friend class square_iterator_t<const Square, INC>;


    square_iterator_t<Square_T, INC>()
        : m_impl(new null_iterator_impl())
    {}

    explicit square_iterator_t<Square_T, INC>(iterator_impl * it)
        : m_impl(it)
    {}

    // Copy constructor: make a copy of m_impl
    template <typename OTHER>
    square_iterator_t<Square_T, INC>(const OTHER & other)
        : m_impl(other.m_impl->clone())
    {}

    // Assignment
    template <typename OTHER>
    self_t & operator=(const OTHER & other)
    {
        m_impl.reset(other.m_impl->clone());
        return *this;
    }

    // Comparison
    template <typename OTHER>
    bool operator==(const square_iterator_t<OTHER, INC> & other) const
    {
        return m_impl->ptr() == other.m_impl->ptr();
    }

    template <typename OTHER>
    bool operator!=(const square_iterator_t<OTHER, INC> & other) const
    {
        return m_impl->ptr() != other.m_impl->ptr();
    }

    bool operator==(const Square * other) const
    {
        return m_impl->ptr() == other;
    }

    bool operator!=(const Square * other) const
    {
        return m_impl->ptr() != other;
    }

    // Prefix operators
    self_t & operator++() { increment(); return *this; }
    self_t & operator--() { decrement(); return *this; }

    // Postfix operators
    self_t operator++(int) { self_t it = *this; increment(); return it; }
    self_t operator--(int) { self_t it = *this; decrement(); return it; }

    // Pointer operators
    Square & operator*() const { return m_impl->ref(); }
    Square * operator->() const { return &*m_impl->ptr(); }

protected:
    std::auto_ptr<iterator_impl> m_impl;

    // Template-specific functions
    void increment();
    void decrement();
};

// Typedefs
typedef square_iterator_t<Square, true> square_iterator;
typedef square_iterator_t<Square, false> square_reverse_iterator;

// Swap increment/decrement for reverse iterators
inline void square_iterator::increment() { m_impl->increment(); }
inline void square_iterator::decrement() { m_impl->decrement(); }
inline void square_reverse_iterator::increment() { m_impl->decrement(); }
inline void square_reverse_iterator::decrement() { m_impl->increment(); }

// Basic iterator implementation
// This is a drop-in replacement for the usual construction:
// for (square = grid.First(); square != NULL; square = square->Next())
// Instead you can use
// for (square_iterator it = grid.begin(); it != grid.end(); ++it)
class basic_iterator_impl : public iterator_impl
{
public:
    basic_iterator_impl(Square * square = NULL, GridDirection dir = ACROSS)
        : m_square(square),
          m_direction(dir)
    {}
    virtual iterator_impl * clone()
        { return new basic_iterator_impl(m_square, m_direction); }
    virtual void increment() { m_square = m_square->Next(m_direction); }
    virtual void decrement() { m_square = m_square->Prev(m_direction); }
    virtual Square * ptr() { return m_square; }
    virtual Square & ref() { return *m_square; }

protected:
    Square * m_square;
    GridDirection m_direction;
};

} // namespace puz

#endif // PUZ_ITERATOR_H
