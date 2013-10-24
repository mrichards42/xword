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


#ifndef METADATA_FORMAT_H
#define METADATA_FORMAT_H

#include "wxFB_MetadataFormat.h"
#include <wx/popupwin.h>

class MetadataFormatDialog : public wxFB_MetadataFormatDialog
{
public:
    MetadataFormatDialog(wxWindow * parent)
        : wxFB_MetadataFormatDialog(parent)
    {}
    void UpdateResult();
    void UpdateLayout();
protected:
    void OnUseLua(wxCommandEvent & evt);
    void OnTextChanged(wxCommandEvent & evt);
};


class MetadataFormatHelpPanel : public wxFB_MetadataFormatHelpPanel
{
public:
    MetadataFormatHelpPanel(wxWindow * parent, const wxArrayString & fields);
};


class MetadataFormatHelpPopup : public wxPopupTransientWindow
{
public:
    MetadataFormatHelpPopup(wxWindow * parent, const wxArrayString & fields);
protected:
    MetadataFormatHelpPanel * m_panel;
};

#endif // METADATA_FORMAT_H
