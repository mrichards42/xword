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


#include "MyAuiToolBar.hpp"
#include "../utils/ToolManager.hpp"
#include <boost/foreach.hpp>

void
MyAuiToolBar::OnCustomize(wxCommandEvent & evt)
{
    wxDialog * dlg = new wxDialog((wxWindow*)this, wxID_ANY, wxString(_T("Choose Items")), wxDefaultPosition);
    wxSizer * sizer = new wxGridSizer(0, 4, 20, 20);
    BOOST_FOREACH(ToolInfo & tool, GetToolManager()->GetTools())
    {
        wxSizer * s = new wxBoxSizer(wxVERTICAL);
        if (tool.HasBitmap())
            s->Add(new wxStaticBitmap(dlg, wxID_ANY, tool.GetBitmap(24, 24)), 0, wxEXPAND);
        s->Add(new wxStaticText(dlg, wxID_ANY, tool.GetLabelText(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 1, wxEXPAND|wxALIGN_CENTER);
        wxSizer * s2 = new wxBoxSizer(wxHORIZONTAL);
        wxCheckBox * check = new wxCheckBox(dlg, tool.GetId(), _T(""));
        check->SetValue(tool.IsAttached(this));
        s2->Add(check, 0, wxEXPAND);
        s2->Add(s, 1, wxEXPAND);
        sizer->Add(s2, 1, wxEXPAND);
    }
    wxSizer * border = new wxBoxSizer(wxVERTICAL);
    border->Add(sizer, 0, wxEXPAND|wxALL, 20);
    dlg->SetSizerAndFit(border);
    dlg->SetExtraStyle(0); // don't block events
    dlg->ShowModal();
    dlg->Destroy();
}


void
MyAuiToolBar::OnCheckBox(wxCommandEvent & evt)
{
    if (evt.IsChecked()) {
        GetToolManager()->GetTool(evt.GetId())->Add(this, 24, 24);
        Realize();
    }
}