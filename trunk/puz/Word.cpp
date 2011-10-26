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

#include <sstream>

#include "Word.hpp"
#include "Square.hpp"
#include "iterator.hpp"

namespace puz {

// ----------------------------------------------------------------------------
// Implementation classes
// ----------------------------------------------------------------------------

// The base implementation class
class WordImpl
{
public:
    WordImpl() {}
    virtual WordImpl * clone()=0;
    virtual ~WordImpl() {}

    virtual bool Contains(const puz::Square * square) const=0;
    virtual int FindSquare(const puz::Square * square) const=0;
    virtual short GetDirection() const=0;

    // Element access
    virtual Square * front() const=0;
    virtual Square * back() const=0;

    // Throw an exception if push_back/front doesn't work
    virtual void push_back(Square * square)=0;
    virtual void push_front(Square * square)=0;
    virtual void pop_back()=0;
    virtual void pop_front()=0;

    // Iterators
    virtual iterator_impl * get_begin() const=0;
    virtual iterator_impl * get_end() const=0;
    virtual iterator_impl * get_rbegin() const=0;
    virtual iterator_impl * get_rend() const=0;
};


// A word with a start and end square and a defined direction.
class StraightImpl : public WordImpl
{
public:
    StraightImpl(Square * start, Square * end)
        : m_front(start), m_back(end)
    {
        short direction = puz::GetDirection(*start, *end);
        if ((direction % 45) != 0)
            throw NoWord();
        m_direction = static_cast<GridDirection>(direction);
    }

    WordImpl * clone() { return new StraightImpl(m_front, m_back); }

    bool Contains(const Square * square) const
    {
        return square->IsBetween(m_front, m_back);
    }

    int FindSquare(const Square * square) const
    {
        if (square->IsBetween(m_front, m_back))
        {
            int x = abs(square->GetCol() - m_front->GetCol());
            if (x > 0)
                return x;
            return abs(square->GetRow() - m_front->GetRow());
        }
        return -1;
    }

    short GetDirection() const { return m_direction; }

    void push_back(Square * square)
    {
        if (m_back->Next(m_direction) == square)
            m_back = square;
        else
            throw NoWord();
    }

    void push_front(Square * square)
    {
        if (m_front->Prev(m_direction) == square)
            m_front = square;
        else
            throw NoWord();
    }

    void pop_front() { m_front = m_front->Next(m_direction); }
    void pop_back() { m_back = m_back->Prev(m_direction); }

    Square * front() const { return m_front; }
    Square * back() const { return m_back; }

    // Iterators
    iterator_impl * get_begin() const
    {
        return new basic_iterator_impl(front(), m_direction);
    }

    iterator_impl * get_end() const
    {
        return new basic_iterator_impl(back()->Next(m_direction), m_direction);
    }

    iterator_impl * get_rbegin() const
    {
        return new basic_iterator_impl(back(), m_direction);
    }

    iterator_impl * get_rend() const
    {
        return new basic_iterator_impl(front()->Prev(m_direction), m_direction);
    }

    Square * m_front;
    Square * m_back;
    GridDirection m_direction;
};


// A word that is a linked-list of arbitrary squares.
// I'm having trouble wrapping a STL iterator from std::list using
// the current square_iterator system, so I've just reimplemented
// a linked-list here
class ListImpl : public WordImpl
{
protected:
    struct Node
    {
        Node(Node * p = NULL, Node * n = NULL, Square * s = NULL)
            : prev(p), next(n), square(s)
        {
        }
        Node * prev;
        Node * next;
        Square * square;
    };

    // Iterator implementation class
    class list_iterator_impl : public iterator_impl
    {
    public:
        list_iterator_impl(Node * n) : node(n) {}
        iterator_impl * clone() { return new list_iterator_impl(node); }
        void increment() { node = node->next; };
        void decrement() { node = node->prev; };
        Square * ptr() { return node->square; }
        Square & ref() { return *node->square; }
        Node * node;
    };

public:
    ListImpl()
    {
        m_head.next = &m_head;
        m_head.prev = &m_head;
    }

    ListImpl(WordImpl * other)
    {
        m_head.next = &m_head;
        m_head.prev = &m_head;
        square_iterator it = square_iterator(other->get_begin());
        square_iterator end = square_iterator(other->get_end());
        for (; it != end; ++it)
            push_back(&*it);
    }

    ~ListImpl()
    {
        Node * n = m_head.next;
        while (n != &m_head)
        {
            Node * next = n->next;
            delete n;
            n = next;
        }
    }

    WordImpl * clone() { return new ListImpl(this); }

    bool Contains(const Square * square) const
    {
        return FindSquare(square) != -1;
    }

    int FindSquare(const Square * square) const
    {
        const_square_iterator it = const_square_iterator(get_begin());
        const_square_iterator end_ = const_square_iterator(get_end());
        int i = 0;
        for (; it != end_; ++it)
        {
            if (it == square)
                return i;
            ++i;
        }
        return -1;
    }

    short GetDirection() const
    {
        return puz::GetDirection(*front(), *back());
    }

    Square * front() const { return first_node()->square; }
    Square * back() const { return last_node()->square; }

    void insert(Square * square, Node * before)
    {
        Node * n = new Node(before->prev, before, square);
        before->prev->next = n;
        before->prev = n;
    }

    void remove(Node * n)
    {
        if (n == &m_head)
            return;
        Node * next = n->next;
        Node * prev = n->prev;
        next->prev = prev;
        prev->next = next;
        delete n;
    }

    void push_back(Square * square)
    {
        insert(square, &m_head);
    }

    void push_front(Square * square)
    {
        insert(square, first_node());
    }

    void pop_back()
    {
        remove(last_node());
    }

    void pop_front()
    {
        remove(first_node());
    }

    // Iterators
    iterator_impl * get_begin() const
        { return new list_iterator_impl(first_node()); }
    iterator_impl * get_end() const
        { return new null_iterator_impl(); }
    iterator_impl * get_rbegin() const
        { return new list_iterator_impl(last_node()); }
    iterator_impl * get_rend() const
        { return new null_iterator_impl(); }

    Node m_head;
    Node * first_node() const { return m_head.next; }
    Node * last_node() const { return m_head.prev; }
};


// ----------------------------------------------------------------------------
// Word implementation
// ----------------------------------------------------------------------------

// Constructors
Word::Word()
    : m_impl(new ListImpl)
{
}

Word::Word(Square * start, Square * end)
    : m_impl(new StraightImpl(start, end))
{
}

Word::Word(const Word & other)
    : m_impl(other.m_impl->clone())
{
}

Word::~Word() { }

// Operators
Word & Word::operator=(const Word & other)
{
    m_impl.reset(other.m_impl->clone());
    return *this;
}

bool Word::Contains(const Square * square) const
{
    return square && m_impl->Contains(square);
}

int Word::FindSquare(const Square * square) const
{
    if (square)
        return m_impl->FindSquare(square);
    return -1;
}

short Word::GetDirection() const { return m_impl->GetDirection(); }

bool Word::empty() const { return front() == NULL; }

// Element access
Square * Word::front() const { return m_impl->front(); }
Square * Word::back() const { return m_impl->back(); }

// Try to push_back/front
// If it doesn't work, convert to ListImpl, then do it.
void Word::push_back(Square * square)
{
    try
    {
        m_impl->push_back(square);
    }
    catch (NoWord &)
    {
        m_impl.reset(new ListImpl(m_impl.get()));
        m_impl->push_back(square);
    }
}

void Word::push_front(Square * square)
{
    try
    {
        m_impl->push_front(square);
    }
    catch (NoWord &)
    {
        m_impl.reset(new ListImpl(m_impl.get()));
        m_impl->push_front(square);
    }
}

void Word::pop_back() { m_impl->pop_back(); }
void Word::pop_front() { m_impl->pop_front(); }

// ----------------------------------------------------------------------------
// Iterators
// ----------------------------------------------------------------------------

square_iterator Word::begin() const
{
    return square_iterator(m_impl->get_begin());
}

square_iterator Word::end() const
{
    return square_iterator(m_impl->get_end());
}

square_reverse_iterator Word::rbegin() const
{
    return square_reverse_iterator(m_impl->get_rbegin());
}

square_reverse_iterator Word::rend() const
{
    return square_reverse_iterator(m_impl->get_rend());
}

} // namespace puz
