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

#include "metadataformat.hpp"
#include "../MetadataCtrl.hpp"
#include "../App.hpp"

void MetadataFormatDialog::UpdateResult()
{
    MyFrame * frame = wxGetApp().GetFrame();
    wxString value = MetadataCtrl::FormatLabel(m_format->GetValue(), frame, m_useLua->GetValue());
    // Tool tip is the whole result
    m_result->SetToolTip(value);
    // Label is the first line
    wxString label = value.BeforeFirst(_T('\n'));
    if (label.size() != value.size())
        label << _T("...");
    m_result->SetLabel(label);
}

void MetadataFormatDialog::UpdateLayout()
{
    // Hide or show the function(puzzle) ... end text
    const bool isChecked = m_useLua->GetValue();
    m_sizer->Show(m_functionStart, isChecked);
    m_sizer->Show(m_functionEnd, isChecked);
    m_sizer->GetItem(m_format)->SetBorder(isChecked ? 15 : 0);
    Layout();
}

void MetadataFormatDialog::OnTextChanged(wxCommandEvent & evt)
{
    UpdateResult();
    evt.Skip();
}

void MetadataFormatDialog::OnUseLua(wxCommandEvent & evt)
{
    UpdateLayout();
    UpdateResult();
}




MetadataFormatHelpPanel::MetadataFormatHelpPanel(wxWindow * parent,
                                                 const wxArrayString & fields)
    : wxFB_MetadataFormatHelpPanel(parent)
{
    // Create the metadata fields
    MyFrame * frame = wxGetApp().GetFrame();
    wxFont fieldFont = GetFont();
    fieldFont.SetFamily(wxFONTFAMILY_TELETYPE);
    fieldFont.SetFaceName("consolas");
    bool hasFields = false;
    for (size_t i = 0; i < fields.Count(); ++i)
    {
        wxString field = fields.Item(i);
        wxString result = MetadataCtrl::GetMeta(field, frame);
        if (! result.empty())
        {
            hasFields = true;
            wxStaticText * text = new wxStaticText(this, wxID_ANY, _T("%") + field + _T("%"));
            text->SetFont(fieldFont);
            m_sizer->Add(text);
            wxStaticText * value = new wxStaticText(this, wxID_ANY, result, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            value->SetMaxSize(wxSize(GetCharWidth() * 25, -1));
            m_sizer->Add(value);

        }
    }
    if (hasFields)
    {
        m_sizer->Layout();
        m_text->Wrap(m_sizer->GetMinSize().x);
    }
    else
    {
        GetSizer()->Hide(m_sizer);
        m_text->Wrap(m_text->GetSize().x / 2);
    }
    Fit();
}

MetadataFormatHelpPopup::MetadataFormatHelpPopup(wxWindow * parent,
                                                   const wxArrayString & fields)
    : wxPopupTransientWindow(parent, wxBORDER_SIMPLE)
{
    m_panel = new MetadataFormatHelpPanel(this, fields);
    m_panel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    m_panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
    Fit();
}