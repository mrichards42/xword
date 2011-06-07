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

#ifndef CLUE_PROMPT_H
#define CLUE_PROMPT_H

#include <wx/control.h>
#include <wx/html/htmlwin.h>

#include "puz/Clue.hpp"

class wxHtmlCell;
class wxHtmlWinParser;

class CluePrompt
    : public wxControl,
      public wxHtmlWindowInterface
{
public:
    CluePrompt() : m_parser(NULL), m_cell(NULL), m_padding(5) {}

    CluePrompt(wxWindow * parent,
              wxWindowID id,
              const wxString & label = wxEmptyString,
              const wxString & displayFormat = _T("%N. %T"),
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              long style = wxBORDER_NONE,
              const wxString & name = _T("CluePrompt"))
        : m_parser(NULL), m_cell(NULL), m_padding(5)
    {
        Create(parent, id, label, displayFormat, position, size, style, name);
    }

    ~CluePrompt();

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & label = wxEmptyString,
                const wxString & displayFormat = _T("%N. %T"),
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = wxBORDER_NONE,
                const wxString & name = _T("CluePrompt"));

    const wxString & GetDisplayFormat() const { return m_displayFormat; }
    void SetDisplayFormat(const wxString & format)
        { m_displayFormat = format; }

    void Clear()
    {
        wxControl::SetLabel(wxEmptyString);
        LayoutCell();
        Refresh();
    }

    void SetClue(const puz::Clue * clue);

    bool SetFont(const wxFont & font);
    wxCoord GetPadding() const { return m_padding; }
    void SetPadding(wxCoord padding) { m_padding = padding; }


protected:
    wxString m_displayFormat;
    wxCoord m_padding;
    wxHtmlCell *m_cell;
    wxHtmlWinParser *m_parser;

    void LayoutCell();
    void Parse(const wxString & label, int pointSize, const wxString & faceName);

    void OnPaint(wxPaintEvent & evt);
    void OnSize(wxSizeEvent & evt)
    {
        LayoutCell();
        Refresh();
        evt.Skip();
    }

    virtual wxBorder GetDefaultBorder() const { return wxBORDER_NONE; }

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(CluePrompt)
    DECLARE_DYNAMIC_CLASS(CluePrompt)

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

};

#endif // CLUE_PROMPT_H
