// This file is part of XWord
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
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


// This is a templatized version of wxControlWithItems (wx/ctrsub.h)
//
// NB: These classes don't support ClientData . . . I just don't understand it
//     and it makes the code a pain in the ass

//=============================================================
// Usage:
//    Derive from wxControlWithItems_T<T> or wxItemContainer_T<T>
//    Override the following member functions (- = optional)
//          void SetItem(unsigned int n, const T & item);
//
//          void SetSelection(int n)
//          int  GetSelection() const
//        - bool SetItemSelection(const T & item)
//
//          void Clear()
//          void Delete(unsigned int n)
//
//          int  DoAppend(const T & item)
//          int  DoInsert(const T & item, unsigned int pos)
//        - void Append(const std::vector<T> & items)
//
//=============================================================



#ifndef CTRL_W_ITEMS_TEMPLATE_H
#define CTRL_W_ITEMS_TEMPLATE_H

#include "wx/defs.h"

#if wxUSE_CONTROLS

#include "wx/control.h"      // base class

#include <vector> // don't use wxArray because it is not a templated class

// ----------------------------------------------------------------------------
// wxItemContainer_T defines an interface which is implemented by all controls
// which have string subitems each of which may be selected.
//
// It is decomposed in wxItemContainerImmutable_T which omits all methods
// adding/removing items.
// ----------------------------------------------------------------------------

template<typename T>
class WXDLLEXPORT wxItemContainerImmutable_T
{
public:
    typedef T item_t;
    typedef typename std::vector<T> container_t;

    wxItemContainerImmutable_T() { }
    virtual ~wxItemContainerImmutable_T() { }

    // accessing items
    // -----------------

    virtual unsigned int GetCount() const { return m_items.size(); }
    bool IsEmpty() const { return m_items.empty(); }

    virtual const T & GetItem(unsigned int n) const  { return m_items.at(n); }
    virtual const container_t & GetItems() const  { return m_items; }
    virtual void SetItem(unsigned int n, const T & item) = 0;

    virtual int FindItem(const T & item) const
    {
        unsigned int count = GetCount();

        for (unsigned int i = 0; i < count ; ++i)
            if (GetItem(i) == item)
                return (int)i;

        return wxNOT_FOUND;
    }


    // selection
    // ---------

    virtual void SetSelection(int n) = 0;
    virtual int  GetSelection() const = 0;

    // set selection to the specified item, return false if not found
    virtual bool SetItemSelection(const T & item)
    {
        const int sel = FindItem(item);
        if (sel == wxNOT_FOUND)
            return false;

        SetSelection(sel);

        return true;
    }


    // this is the same as SetSelection( for single-selection controls but
    // reads better for multi-selection ones
    void Select(int n) { SetSelection(n); }


protected:
    // check that the index is valid
    inline bool IsValid(unsigned int n) const { return n < GetCount(); }
    inline bool IsValidInsert(unsigned int n) const { return n <= GetCount(); }

    container_t m_items;
};


template <typename T>
class WXDLLEXPORT wxItemContainer_T
    : public wxItemContainerImmutable_T<T>
{
public:
    typedef typename std::vector<T> container_t;
    wxItemContainer_T() { }
    virtual ~wxItemContainer_T() { }

    // adding items
    // ------------

    int Append(const T & item) { return DoAppend(item); }

    // append several items at once to the control (virtual for optimmizations)
    virtual void Append(const container_t & items)
    {
        typename container_t::const_iterator it;
        for (it = items.begin(); it != items.end(); ++it)
            Append(*it);
    }

    int Insert(const T & item, unsigned int pos) { return DoInsert(item, pos); }

    // deleting items
    // --------------
    virtual void Clear() = 0;
    virtual void Delete(unsigned int n) = 0;

protected:
    virtual int DoAppend(const T & item) = 0;
    virtual int DoInsert(const T & item, unsigned int pos) = 0;
};



template <typename T>
class WXDLLEXPORT wxControlWithItems_T
    : public wxControl,
      public wxItemContainer_T<T>
{
public:
    wxControlWithItems_T() { }
    virtual ~wxControlWithItems_T() { }

    // usually the controls like list/combo boxes have their own background
    // colour
    virtual bool ShouldInheritColours() const { return false; }

private:
    DECLARE_NO_COPY_CLASS(wxControlWithItems_T)
};


#endif // wxUSE_CONTROLS

#endif // CTRL_W_ITEMS_TEMPLATE_H