/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


// TODO:
//   - Sometimes the text wraps at inappropriate places

#ifndef MY_SIZED_TEXT_CTRL_H
#define MY_SIZED_TEXT_CTRL_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wrap.hpp>


static const bool ST_TRUNCATE = true;

class SizedText
    : public wxControl
{
public:
    explicit SizedText(wxWindow * parent, int id = wxID_ANY, const wxString & label = wxEmptyString, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, int style = 0, bool trunc = false);
    ~SizedText();

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


protected:
    void ResizeLabel();
    void OnSize(wxSizeEvent & evt)   { ResizeLabel(); evt.Skip(); }
    void OnPaint(wxPaintEvent & WXUNUSED(evt));

    wxString m_fullLabel;
    bool m_truncate;

    DECLARE_EVENT_TABLE()
};



#endif // MY_SIZED_TEXT_CTRL_H
