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


#ifndef CHARACTERS_DLG_H
#define CHARACTERS_DLG_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include <wx/minifram.h>

#include "wxFB_Dialogs.h"

class MyFrame;
class SizedText;

class CharactersPanel
    : public wxFB_CharactersPanel
{
public:
    CharactersPanel(MyFrame * parent);
    void AddCharacter(wxChar ch);

protected:
    MyFrame * m_frame;
    int m_ctrlCount;

    SizedText * NewStaticText(const wxString & label = _T(""), wxWindowID = wxID_ANY);
    void OnLeftDown(wxMouseEvent & evt);
    void OnMouseOver(wxMouseEvent & evt);
    void OnMouseOut(wxMouseEvent & evt);
    void OnSetFocus(wxFocusEvent & evt);
    void OnSize(wxSizeEvent & evt);
};


#endif // CHARACTERS_DLG_H
