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


#ifndef LAYOUT_DLG_H
#define LAYOUT_DLG_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "../MyFrame.hpp"


// Note that this should only be used as a modal dialog because
// wxSingleChoiceDialog is modal


class LayoutDialog
    : public wxSingleChoiceDialog
{
public:
    LayoutDialog(MyFrame * frame,
                 const wxString & message,
                 const wxString & caption,
                 const wxArrayString & choices,
                 const wxArrayString & layouts)
        : wxSingleChoiceDialog(frame, message, caption, choices),
          m_frame(frame),
          m_layouts(layouts)
    {
        wxASSERT(choices.size() == layouts.size());
    }

    ~LayoutDialog() {}

protected:
    void OnSelect(wxCommandEvent & evt)
    {
        m_frame->LoadLayoutString(m_layouts[evt.GetSelection()], true);
    }

    // Keep a pointer to the frame so we can dynamically update it.
    MyFrame * m_frame;

    wxArrayString m_layouts;

    DECLARE_EVENT_TABLE()
};



BEGIN_EVENT_TABLE(LayoutDialog, wxSingleChoiceDialog)
    EVT_LISTBOX    (wxID_ANY, LayoutDialog::OnSelect)
END_EVENT_TABLE()

#endif // LAYOUT_DLG_H