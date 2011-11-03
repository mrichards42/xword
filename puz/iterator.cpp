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

#ifndef _WINDOWS

#include "iterator.hpp"

namespace puz {

// Increment/Decrement for forward iterators
template<> void square_iterator_t<Square, true>::increment()
{
    m_impl->increment();
}

template<> void square_iterator_t<Square, true>::decrement()
{
    m_impl->decrement();
}

template<> void square_iterator_t<const Square, true>::increment()
{
    m_impl->increment();
}

template<> void square_iterator_t<const Square, true>::decrement()
{
    m_impl->decrement();
}

// Increment/Decrement for reverse iterators
template<> void square_iterator_t<Square, false>::increment()
{
    m_impl->decrement();
}

template<> void square_iterator_t<Square, false>::decrement()
{
    m_impl->increment();
}

template<> void square_iterator_t<const Square, false>::increment()
{
    m_impl->decrement();
}

template<> void square_iterator_t<const Square, false>::decrement()
{
    m_impl->increment();
}

} // namespace puz
#endif // _WINDOWS
