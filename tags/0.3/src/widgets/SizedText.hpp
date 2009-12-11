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


// TODO:
//   - Sometimes the text wraps at the wrong place

#ifndef MY_SIZED_TEXT_CTRL_H
#define MY_SIZED_TEXT_CTRL_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "../utils/wrap.hpp"

// Enums
//------
enum SizedTextWrapMode
{
    ST_WRAP     = 0x00,
    ST_TRUNCATE = 0x01
};

class SizedText
    : public wxControl
{
public:
    SizedText() {}

    SizedText(wxWindow * parent,
              wxWindowID id,
              const wxString & label = wxEmptyString,
              const wxPoint & position = wxDefaultPosition,
              const wxSize & size = wxDefaultSize,
              long style = wxBORDER_NONE | wxALIGN_CENTER | ST_WRAP,
              const wxString & name = _T("SizedText"))
    {
        Create(parent, id, label, position, size, style, name);
    }

    virtual ~SizedText() {}

    bool Create(wxWindow * parent,
                wxWindowID id,
                const wxString & label = wxEmptyString,
                const wxPoint & position = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                long style = wxBORDER_NONE | wxALIGN_CENTER | ST_WRAP,
                const wxString & name = _("SizedText"));

    void SetLabel(const wxString & label)
    {
        m_fullLabel = label;
        ResizeLabel();
    }

    bool SetFont(const wxFont & font)
    {
        bool ret = wxControl::SetFont(font);
        ResizeLabel();
        return ret;
    }

    const wxString & GetFullLabel() { return m_fullLabel; }

    bool IsTruncated() const { return HasFlag(ST_TRUNCATE); }
    bool IsWrapped()   const { return ! IsTruncated(); }

    void SetWrapMode(SizedTextWrapMode mode)
    {
        SetWindowStyle(GetWindowStyle() & ~ (ST_WRAP | ST_TRUNCATE) | mode);
        Refresh();
    }

    void SetAlign(long align)
    {
        SetWindowStyle(GetWindowStyle() & ~ (wxALIGN_MASK) | align);
        Refresh();
    }

protected:
    void ResizeLabel();
    void WrapLabel();
    void TruncateLabel();

    void OnSize(wxSizeEvent & evt) { ResizeLabel(); evt.Skip(); }
    void OnPaint(wxPaintEvent & WXUNUSED(evt));

    wxString m_fullLabel;

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(SizedText)
    DECLARE_DYNAMIC_CLASS(SizedText)
};

#endif // MY_SIZED_TEXT_CTRL_H
