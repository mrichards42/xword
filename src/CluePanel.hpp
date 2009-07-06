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


#ifndef MY_CLUE_PANEL_H
#define MY_CLUE_PANEL_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "ClueListBox.hpp"

class CluePanel
    : public wxPanel
{
public:
    explicit CluePanel(wxWindow* parent, wxWindowID id, const wxString & heading, bool direction, long style = wxBORDER_NONE);
    ~CluePanel();

    wxStaticText * m_heading;
    ClueListBox * m_clueList;

private:
    // Check to see if the panel is very small
    // If it is, display the clue box next to the heading
    void OnSize(wxSizeEvent & evt);

    DECLARE_EVENT_TABLE()
};


#endif MY_CLUE_PANEL_H
