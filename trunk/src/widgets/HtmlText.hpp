// This file is part of XWord
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#ifndef HTML_TEXT_H
#define HTML_TEXT_H

#include <wx/control.h>
#include <wx/html/htmlwin.h>

class wxHtmlCell;
class wxHtmlWinParser;

enum HtmlTextStyles
{
    HT_WRAP     = 0x00,
    HT_TRUNCATE = 0x01,
    HT_NOWRAP = HT_TRUNCATE,

    HT_FILL = 0x00, // Fill space by changing the font size
    HT_FIXED = 0x02 // Don't adjust the font size to fill available space
};


class HtmlText
    : public wxControl,
      public wxHtmlWindowInterface
{
public:
    HtmlText()
        : m_parser(NULL), m_cell(NULL), m_padding(5),
          m_minFontSize(6), m_maxFontSize(150)
    {}

    HtmlText(wxWindow * parent,
             wxWindowID id,
             const wxString & label = wxEmptyString,
             const wxPoint & position = wxDefaultPosition,
             const wxSize & size = wxDefaultSize,
             long style = 0,
             const wxString & name = _T("HtmlText"))
        : m_parser(NULL), m_cell(NULL), m_padding(5),
          m_minFontSize(6), m_maxFontSize(150)
    {
        Create(parent, id, label, position, size, style, name);
    }

    ~HtmlText();

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & label = wxEmptyString,
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = 0,
                const wxString & name = _T("HtmlText"));

    void Clear()
    {
        wxControl::SetLabel(wxEmptyString);
        LayoutCell();
        Refresh();
    }

    bool SetFont(const wxFont & font);
    void SetMinFontSize(int size) { m_minFontSize = size; }
    int GetMinFontSize() const { return m_minFontSize; }
    void SetMaxFontSize(int size) { m_maxFontSize = size; }
    int GetMaxFontSize() const { return m_maxFontSize; }

    wxCoord GetPadding() const { return m_padding; }
    void SetPadding(wxCoord padding) { m_padding = padding; }


protected:
    wxCoord m_padding;
    wxHtmlCell *m_cell;
    wxHtmlWinParser *m_parser;

    int m_minFontSize, m_maxFontSize, m_lastFontSize;

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
    DECLARE_NO_COPY_CLASS(HtmlText)
    DECLARE_DYNAMIC_CLASS(HtmlText)

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

#endif // HTML_TEXT_H
