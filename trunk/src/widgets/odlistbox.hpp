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


#ifndef OWNER_DRAWN_LIST_BOX_H
#define OWNER_DRAWN_LIST_BOX_H

#if wxUSE_CONTROLS

#include "ctrlwitems.hpp"     // base classes
#include "vlboxcache.hpp"

//==============================================================================
// Note that this is still an abstract base class because the wxCachedVListBox
// drawing functions are still pure virtual.
//
// Overrides (- = optional override):
//
//     void    OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const
//   - void    OnDrawItem      (wxDC & dc, const wxRect & rect, size_t n) const
//   - void    OnDrawSeparator (wxDC & dc,       wxRect & rect, size_t n) const
//   - wxCoord OnMeasureItem   (wxDC & dc, size_t n) const
//
// NB: OnMeasureItem provides wxDC as a parameter (different from wxVListBox)
//==============================================================================

static const wxChar * wxOwnerDrawnListBoxNameStr = _T("OwnerDrawnListBox");

// At some point we can use another parent class . . . not yet
template<typename T = wxString, class _Parent = wxCachedVListBox>
class WXDLLEXPORT wxOwnerDrawnListBox
    : public _Parent,
      public wxItemContainer_T<T>
{
public:
    // Constructors
    // --------------

    wxOwnerDrawnListBox() { Init(); }

    wxOwnerDrawnListBox(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        int n = 0, const T choices[] = NULL,
                        long style = 0,
                        const wxString & name = wxOwnerDrawnListBoxNameStr)
    {
        Init();
        Create(parent, id, pos, size, n, choices, style, name);
    }

    wxOwnerDrawnListBox(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        const std::vector<T> & choices,
                        long style = 0,
                        const wxString & name = wxOwnerDrawnListBoxNameStr)
    {
        Init();
        Create(parent, id, pos, size, choices, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                int n = 0, const T choices[] = NULL,
                long style = 0,
                const wxString & name = _T("OwnerDrawnListBox"))
    {
        if (! _Parent::Create(parent, id, pos, size, style, name))
            return false;
        int i;
        for (i = 0; i < n; ++i)
            m_items.push_back(choices[i]);
        UpdateCount();
        return true;
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<T> & choices,
                long style = 0,
                const wxString & name = _T("OwnerDrawnListBox"))
    {
        if (! _Parent::Create(parent, id, pos, size, style, name))
            return false;
        Append(choices);
        return true;
    }

    virtual ~wxOwnerDrawnListBox() {}


    // Overrides for wxItemContainer_T
    // -----------------------------------

    virtual void SetItem(unsigned int n, const T & item)
        { m_items.at(n) = item; RefreshLine(n); }

    void SetSelection(int n)   { _Parent::SetSelection(n); }
    int  GetSelection() const  { return _Parent::GetSelection(); }

    virtual void Clear()
    {
        m_items.clear();
        UpdateCount();
    }

    virtual void Delete(unsigned int n)
    {
        m_heights.erase (m_heights.begin()  + n);
        m_bmpCache.erase(m_bmpCache.begin() + n);
        m_items.erase   (m_items.begin()    + n);
        UpdateCount();
    }


    // Optimized append
    void Append(const std::vector<T> & items)
    {
        for(std::vector<T>::const_iterator it = items.begin();
            it != items.end();
            ++it)
        {
            m_items.push_back(*it);
        }
        UpdateCount();
    }

    // Text Color
    void SetSelectionForeground(const wxColour & color)
        { m_selectionForeground = color; }

    const wxColour & GetSelectionForeground() const
        { return m_selectionForeground; }

protected:
    void Init()
    {
        SetSelectionForeground(
            wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );

        SetSelectionBackground(
            wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
    }

    void UpdateCount()
    {
        SetItemCount(m_items.size());
        OnUpdateCount();
        if (!this->IsFrozen())
            RefreshAll();
    }

    virtual int DoAppend(const T & item)
    {
        m_items.push_back(item);
        UpdateCount();
        return GetCount() - 1;
    }

    virtual int DoInsert(const T & item, unsigned int pos)
    {
        OnInsert(pos);
        m_heights.insert(m_heights.begin() + pos, -1);
        m_bmpCache.insert(m_bmpCache.begin() + pos, wxNullBitmap);
        m_items.insert(m_items.begin() + pos, item);
        UpdateCount();
        return pos;
    }

    // Needed for caching items, because DoInsert is messy with a template
    // parameter
    virtual void OnInsert(size_t pos) { }

    virtual void InvalidateCache()
    {
        wxCachedVListBox::InvalidateCache();
        OnUpdateCount();
    }

    // Override this if you need to recalculate anything when an item is
    // added, inserted, or removed, or when the cache is invalidated.
    // This function is called after the vectors are changed, but before a
    // redraw.
    virtual void OnUpdateCount() { }

    // Overrides for wxCachedVListBox
    // ------------------------------

    // Make these protected so they aren't accessible publicly
    virtual void SetItemCount(size_t count) { _Parent::SetItemCount(count); }
    virtual void SetLineCount(size_t count) { _Parent::SetLineCount(count); }

    // Note that this is a different overload (with a DC, which is helpful)
    virtual wxCoord OnMeasureItem(size_t n) const
        { return OnMeasureItem(wxMemoryDC(), n); }
    virtual wxCoord OnMeasureItem(wxDC & dc, size_t n) const
        { return dc.GetCharHeight(); }

    // We can't know how to draw unless the type is wxString
    virtual void OnDrawItem(wxDC & dc,
                            const wxRect & rect,
                            size_t n) const = 0;

    // Default behavior
    virtual void OnDrawBackground(wxDC & dc,
                                  const wxRect & rect,
                                  size_t n) const;

    // Text color
    wxColour m_selectionForeground;

    DECLARE_ABSTRACT_CLASS(wxOwnerDrawnListBox)
    DECLARE_NO_COPY_CLASS(wxOwnerDrawnListBox)
};



// Drawing functions
template <typename T, class _Parent>
void
wxOwnerDrawnListBox<T, _Parent>::OnDrawBackground(wxDC & dc,
                                                  const wxRect & rect,
                                                  size_t n) const
{
    if (IsSelected(n)) {
        dc.SetBrush(wxBrush(GetSelectionBackground()));
        dc.SetPen  (wxPen  (GetSelectionBackground()));
        dc.DrawRectangle(rect);
    }
    // else the background is already filled in
}







// wxWidgets RTTI
//---------------
// This is the equivalent of IMPLEMENT_ABSTRACT_CLASS, except that the
// implementation can't happen with the macro because this is a templated class
template <typename T, class _Parent>
wxClassInfo
wxOwnerDrawnListBox<T, _Parent>
    ::ms_classInfo( _T("wxOwnerDrawnListBox"),
                    &_Parent::ms_classInfo,
                    NULL,
                    (int) sizeof(wxOwnerDrawnListBox<T, _Parent>),
                    (wxObjectConstructorFn) NULL );

template <typename T, class _Parent>
wxClassInfo *
wxOwnerDrawnListBox<T, _Parent>::GetClassInfo() const
{
    return &wxOwnerDrawnListBox<T, _Parent>::ms_classInfo;
}





// Macros that make using cached items easier
//-------------------------------------------
// Defines behaviors for
//      Clear
//      Delete
//      Insert
//      InvalidateCache
//      SetItemCount
// that follow changes in m_items

#define DECLARE_CACHED_ITEM_1(item1, default1,                   \
                               parent)                           \
    protected:                                                   \
        virtual void Clear()                                     \
            { item1.clear();                                     \
              parent::Clear(); }                                 \
        virtual void Delete(size_t pos)                          \
            { item1.erase(item1.begin() + pos);                  \
              parent::Delete(pos); }                             \
        virtual void OnInsert(size_t pos)                        \
            { item1.insert(item1.begin() + pos, default1);       \
              parent::OnInsert(pos); }                           \
        virtual void InvalidateCache()                           \
            { item1.assign(GetCount(), default1);                \
              parent::InvalidateCache(); }                       \
        virtual void SetItemCount(size_t count)                  \
            { item1.resize(count, default1);                     \
              parent::SetItemCount(count); }

#define DECLARE_CACHED_ITEM_2(item1, default1,                   \
                              item2, default2,                   \
                              parent)                            \
    protected:                                                   \
        virtual void Clear()                                     \
            { item1.clear();                                     \
              item2.clear();                                     \
              parent::Clear(); }                                 \
        virtual void Delete(size_t pos)                          \
            { item1.erase(item1.begin() + pos);                  \
              item2.erase(item2.begin() + pos);                  \
              parent::Delete(pos); }                             \
        virtual void OnInsert(size_t pos)                        \
            { item1.insert(item1.begin() + pos, default1);       \
              item2.insert(item2.begin() + pos, default2);       \
              parent::OnInsert(pos); }                           \
        virtual void InvalidateCache()                           \
            { item1.assign(GetCount(), default1);                \
              item2.assign(GetCount(), default2);                \
              parent::InvalidateCache(); }                       \
        virtual void SetItemCount(size_t count)                  \
            { item1.resize(count, default1);                     \
              item2.resize(count, default2);                     \
              parent::SetItemCount(count); }

#define DECLARE_CACHED_ITEM_3(item1, default1,                   \
                              item2, default2,                   \
                              item3, default3,                   \
                              parent)                            \
    protected:                                                   \
        virtual void Clear()                                     \
            { item1.clear();                                     \
              item2.clear();                                     \
              item3.clear();                                     \
              parent::Clear(); }                                 \
        virtual void Delete(size_t pos)                          \
            { item1.erase(item1.begin() + pos);                  \
              item2.erase(item2.begin() + pos);                  \
              item3.erase(item3.begin() + pos);                  \
              parent::Delete(pos); }                             \
        virtual void OnInsert(size_t pos)                        \
            { item1.insert(item1.begin() + pos, default1);       \
              item1.insert(item2.begin() + pos, default2);       \
              item1.insert(item3.begin() + pos, default3);       \
              parent::OnInsert(pos); }                           \
        virtual void InvalidateCache()                           \
            { item1.assign(GetCount(), default1);                \
              item2.assign(GetCount(), default2);                \
              item3.assign(GetCount(), default3);                \
              parent::InvalidateCache(); }                       \
        virtual void SetItemCount(size_t count)                  \
            { item1.resize(count, default1);                     \
              item2.resize(count, default2);                     \
              item3.resize(count, default3);                     \
              parent::SetItemCount(count); }


#endif // wxUSE_CONTROLS

#endif // OWNER_DRAWN_LIST_BOX_H