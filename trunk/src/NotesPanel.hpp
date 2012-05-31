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

#include <wx/html/htmlwin.h>
#include "App.hpp" // config

// An HtmlWindow that respects font and colors
class NotesPanel : public wxHtmlWindow
{
public:
    NotesPanel(wxWindow * parent, wxWindowID id)
        : wxHtmlWindow(parent, id),
          m_foreground(*wxBLACK)
    {
        ConfigManager::Notes_t & notes =
            wxGetApp().GetConfigManager().Notes;
        notes.font.AddCallback(this, &NotesPanel::SetFont);
        notes.foregroundColor.AddCallback(this, &NotesPanel::SetForegroundColour);
        notes.backgroundColor.AddCallback(this, &NotesPanel::SetBackgroundColour);
    }

    virtual ~NotesPanel()
    {
        wxGetApp().GetConfigManager().RemoveCallbacks(this);
    }

    virtual bool SetFont(const wxFont & font)
    {
        SetFonts(font.GetFaceName(), wxEmptyString, NULL);
        return wxHtmlWindow::SetFont(font);
    }

    virtual bool SetForegroundColour(const wxColour & colour)
    {
        m_foreground = colour;
        return SetPage(m_html);
    }

    virtual bool SetPage(const wxString & source)
    {
        m_html = source;
        return wxHtmlWindow::SetPage(
            _T("<HTML><BODY TEXT=")
                + m_foreground.GetAsString(wxC2S_HTML_SYNTAX) + _T(">")
            + m_html + _T("</BODY></HTML>"));
    }

protected:
    wxString m_html;
    wxColour m_foreground;
};
