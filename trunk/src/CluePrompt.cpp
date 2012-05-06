// This file is part of XWord    
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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


#include "CluePrompt.hpp"
#include "App.hpp" // GetApp

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>


IMPLEMENT_DYNAMIC_CLASS(CluePrompt, HtmlText)

bool
CluePrompt::Create(wxWindow * parent,
                   wxWindowID id,
                   const wxString & displayFormat,
                   const wxPoint & position,
                   const wxSize & size,
                   long style,
                   const wxString & name)
{
    if (! MetadataCtrl::Create(parent, id, displayFormat, position, size, style, name))
        return false;

    // Config
    ConfigManager::CluePrompt_t & prompt =
        wxGetApp().GetConfigManager().CluePrompt;
    prompt.font.AddCallback(this, &CluePrompt::SetFont);
    prompt.backgroundColor.AddCallback(this, &CluePrompt::SetBackgroundColour);
    prompt.foregroundColor.AddCallback(this, &CluePrompt::SetForegroundColour);
    prompt.displayFormat.AddCallback(this, &CluePrompt::SetDisplayFormat);
    prompt.alignment.AddCallback(this, &CluePrompt::SetAlignment);
    prompt.useLua.AddCallback(this, &CluePrompt::SetLuaFormat);
    return true;
}

CluePrompt::~CluePrompt()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}
