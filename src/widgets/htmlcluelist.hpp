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


// This is a modified version of wxHtmlListBox,
// from $(WXWIN)/include/htmllbox.h

// Changes:
// * No support for links
// * Better support for wxWindow::SetFont

#ifndef MY_HTML_CLUE_LISTBOX_H
#define MY_HTML_CLUE_LISTBOX_H

#include <wx/vlbox.h>               // base class
#include <wx/html/htmlwin.h>

class wxHtmlCell;
class wxHtmlWinParser;
class HtmlClueListBoxCache;
class HtmlClueListBoxStyle;

extern const wxChar * HtmlClueListBoxNameStr;

// ----------------------------------------------------------------------------
// HtmlClueListBox
// ----------------------------------------------------------------------------

class HtmlClueListBox
    : public wxVListBox,
      public wxHtmlWindowInterface
{
    DECLARE_ABSTRACT_CLASS(HtmlClueListBox)
public:
    // constructors and such
    // ---------------------

    // default constructor, you must call Create() later
    HtmlClueListBox();

    // normal constructor which calls Create() internally
    HtmlClueListBox(wxWindow *parent,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxString& name = HtmlClueListBoxNameStr);

    // really creates the control and sets the initial number of items in it
    // (which may be changed later with SetItemCount())
    //
    // the only special style which may be specified here is wxLB_MULTIPLE
    //
    // returns true on success or false if the control couldn't be created
    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = HtmlClueListBoxNameStr);

    // destructor cleans up whatever resources we use
    virtual ~HtmlClueListBox();

    // override some base class virtuals
    virtual void RefreshLine(size_t line);
    virtual void RefreshLines(size_t from, size_t to);
    virtual void RefreshAll();
    virtual void SetItemCount(size_t count);

    // Font handling.
    virtual bool SetFont(const wxFont & font);
    // Color handling
    virtual bool SetForegroundColour(const wxColour & color);

protected:
    // this method must be implemented in the derived class and should return
    // the body (i.e. without <html>) of the HTML for the given item
    virtual wxString OnGetItem(size_t n) const = 0;

    // this function may be overridden to decorate HTML returned by OnGetItem()
    virtual wxString OnGetItemMarkup(size_t n) const;


    // this method allows to customize the selection appearance: it may be used
    // to specify the colour of the text which normally has the given colour
    // colFg when it is inside the selection
    //
    // by default, the original colour is not used at all and all text has the
    // same (default for this system) colour inside selection
    virtual wxColour GetSelectedTextColour(const wxColour& colFg) const;

    // this is the same as GetSelectedTextColour() but allows to customize the
    // background colour -- this is even more rarely used as you can change it
    // globally using SetSelectionBackground()
    virtual wxColour GetSelectedTextBgColour(const wxColour& colBg) const;


    // we implement both of these functions in terms of OnGetItem(), they are
    // not supposed to be overridden by our descendants
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
    virtual void OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const;
    virtual wxCoord OnMeasureItem(size_t n) const;

    // event handlers
    void OnSize(wxSizeEvent& event);

    // common part of all ctors
    void Init();

    // ensure that the given item is cached
    void CacheItem(size_t n) const;

private:
    // wxHtmlWindowInterface methods:
    virtual void SetHTMLWindowTitle(const wxString& title);
    virtual void OnHTMLLinkClicked(const wxHtmlLinkInfo& link);
    virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type,
                                                 const wxString& url,
                                                 wxString *redirect) const;
    virtual wxPoint HTMLCoordsToWindow(wxHtmlCell *cell,
                                       const wxPoint& pos) const;
    virtual wxWindow* GetHTMLWindow();
    virtual wxColour GetHTMLBackgroundColour() const;
    virtual void SetHTMLBackgroundColour(const wxColour& clr);
    virtual void SetHTMLBackgroundImage(const wxBitmap& bmpBg);
    virtual void SetHTMLStatusText(const wxString& text);
    virtual wxCursor GetHTMLCursor(HTMLCursor type) const;

private:
    // this class caches the pre-parsed HTML to speed up display
    HtmlClueListBoxCache *m_cache;

    // HTML parser we use
    wxHtmlWinParser *m_htmlParser;

    // rendering style for the parser which allows us to customize our colours
    HtmlClueListBoxStyle *m_htmlRendStyle;


    // it calls our GetSelectedTextColour() and GetSelectedTextBgColour()
    friend class HtmlClueListBoxStyle;
    friend class HtmlClueListBoxWinInterface;


    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(HtmlClueListBox)
};


#endif // MY_HTML_LISTBOX_H
