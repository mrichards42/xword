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

#ifndef PUZ_WORD_H
#define PUZ_WORD_H

#include <memory>
#include "puzstring.hpp"
#include "Square.hpp"
#include "iterator.hpp"

namespace puz {

class WordImpl;

// Default functor for Find functions
static bool FIND_ANY_SQUARE(const Square * square) { return true; }

// A word
// This is essentially a linked-list with an efficient implementation for
// words that have squares in line.
class PUZ_API Word
{
public:
    // Constructors
    Word();
    Word(Square * start, Square * end);
    Word(const Word & other);
    ~Word();

    // Operators
    Word & operator=(const Word & other);

    bool Contains(const Square * square) const;
    // Return the index of square.  -1 means not found
    int FindSquare(const Square * square) const;
    short GetDirection() const;
    bool empty() const;

    // Element access
    Square * front() const;
    Square * back() const;

    void push_back(Square * square);
    void push_front(Square * square);
    void pop_back();
    void pop_front();

    // Iterators
    square_iterator begin() const;
    square_iterator end() const;
    square_reverse_iterator rbegin() const;
    square_reverse_iterator rend() const;


    // Search functions
    square_iterator find(Square * start)
    {
        square_iterator it = begin();
        square_iterator end_ = end();
        while (it != end_ && it != start)
            ++it;
        return it;
    }

    square_reverse_iterator rfind(Square * start)
    {
        square_reverse_iterator it = rbegin();
        square_reverse_iterator end_ = rend();
        while (it != end_ && it != start)
            ++it;
        return it;
    }


    template<typename IT, typename FUNC>
    Square * FindSquare(IT begin, IT end, FUNC findFunc)
    {
        for (IT it = begin; it != end; ++it)
            if (findFunc(&*it))
                return &*it;
        return NULL;
    }

    template<typename FUNC>
    Square * FindSquare(FUNC findFunc)
    {
        return FindSquare(begin(), end(), findFunc);
    }

    template<typename FUNC>
    Square * FindSquare(Square * start, FUNC findFunc)
    {
        return FindSquare(find(start), end(), findFunc);
    }

    template <typename FUNC>
    Square * FindNextSquare(Square * start,
                            FUNC findFunc,
                            FindDirection direction = NEXT)
    {
        if (direction == NEXT)
        {
            square_iterator it = find(start);
            square_iterator end_ = end();
            if (it == end_)
                return NULL;
            ++it;
            return FindSquare(it, end_, findFunc);
        }
        else
        {
            square_reverse_iterator it = rfind(start);
            square_reverse_iterator end_ = rend();
            if (it == end_)
                return NULL;
            ++it;
            return FindSquare(it, end_, findFunc);
        }
    }

    template <typename FUNC>
    Square * FindPrevSquare(Square * start, FUNC findFunc)
    {
        return FindNextSquare(start, findFunc, PREV);
    }

    // Overloads
    Square * FindNextSquare(Square * start)
    {
        return FindNextSquare(start, FIND_ANY_SQUARE, NEXT);
    }

    Square * FindPrevSquare(Square * start)
    {
        return FindPrevSquare(start, FIND_ANY_SQUARE);
    }

protected:
    std::auto_ptr<WordImpl> m_impl;
};

} // namespace puz

#endif // PUZ_WORD_H
