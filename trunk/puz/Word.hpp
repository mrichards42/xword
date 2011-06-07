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

#include <vector>
#include <list>
#include <algorithm>
#include "puzstring.hpp"
#include "Square.hpp"

namespace puz {

class PUZ_API Word: public std::vector<Square *> {
public:
    bool Contains(const puz::Square * square) const
    {
        return std::find(begin(), end(), square) != end();
    }

    short GetDirection() const
    {
        assert(! empty());
        if (empty())
            return ACROSS;
        return puz::GetDirection(*front(), *back());
    }

    // Searching
    //----------
    // Accepts functions (or function-objects) that subscribe to the template:
    //    bool Function(const Square *)
    template<typename IT, typename T>
    const Square * FindSquare(IT begin, IT end, T findFunc) const;

    template<typename T>
    const Square * FindSquare(const Square * start, T findFunc,
                              FindDirection increment = NEXT) const;

    template<typename T>
    const Square * FindSquare(T findFunc, FindDirection increment = NEXT) const;

    template<typename T>
    const Square * FindNextSquare(const Square * start,
                                  T findFunc,
                                  FindDirection increment = NEXT) const;

    template<typename T>
    Square * FindSquare(iterator begin, iterator end, T findFunc);

    // non-const overloads
    template<typename T>
    Square * FindSquare(Square * start, T findFunc,
                        FindDirection increment = NEXT);

    // FindSquare starting from the first square
    template<typename T>
    Square * FindSquare(T findFunc, FindDirection increment = NEXT);

    // Search starting from the next square
    template<typename T>
    Square * FindNextSquare(Square * start, T findFunc,
                            FindDirection increment = NEXT);
};


template <typename IT, typename T>
const Square *
Word::FindSquare(IT begin, IT end, T findFunc) const
{
    for (IT it = begin; it != end; ++it)
        if (findFunc(*it))
            return *it;
    return NULL;
}

template <typename T>
const Square *
Word::FindSquare(const Square * start,
                 T findFunc,
                 FindDirection increment) const
{
    if (increment == NEXT)
    {
        return FindSquare(
            find(begin(), end(), start),
            end(),
            findFunc
        );
    }
    else
    {
        return FindSquare(
            find(rbegin(), rend(), start),
            rend(),
            findFunc
        );
    }
}


template<typename T>
const Square *
Word::FindSquare(T findFunc, FindDirection increment) const
{
    if (increment == NEXT)
        return FindSquare(begin(), end(), findFunc);
    else
        return FindSquare(rbegin(), rend(), findFunc);
}


template <typename T>
const Square *
Word::FindNextSquare(const Square * start,
                     T findFunc,
                     FindDirection increment) const
{
    if (increment == NEXT)
    {
        const_iterator it = find(begin(), end(), start);
        const_iterator end_ = end();
        if (it == end_)
            return NULL;
        ++it;
        return FindSquare(it, end_, findFunc);
    }
    else
    {
        const_reverse_iterator it = find(rbegin(), rend(), start);
        const_reverse_iterator end_ = rend();
        if (it == end_)
            return NULL;
        ++it;
        return FindSquare(it, end_, findFunc);
    }
}




// non-const overloads
template <typename T>
Square *
Word::FindSquare(Square * start,
                 T findFunc,
                 FindDirection increment)
{
    return const_cast<Square *>(const_cast<const Word *>(this)->FindSquare(
        start, findFunc, increment
    ));
}


template<typename T>
Square *
Word::FindSquare(T findFunc, FindDirection increment)
{
    return const_cast<Square *>(const_cast<const Word *>(this)->FindSquare(
        findFunc, increment
    ));
}


template <typename T>
Square *
Word::FindNextSquare(Square * start,
                     T findFunc,
                     FindDirection increment)
{
    return const_cast<Square *>(const_cast<const Word *>(this)->FindNextSquare(
        start, findFunc, increment
    ));
}




class PUZ_API WordList : public std::list<Word> {
};

}

#endif // PUZ_WORD_H
