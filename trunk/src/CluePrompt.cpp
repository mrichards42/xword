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
#include "utils/string.hpp"
#include "App.hpp" // GetApp

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>


IMPLEMENT_DYNAMIC_CLASS(CluePrompt, HtmlText)

bool
CluePrompt::Create(wxWindow * parent,
                   wxWindowID id,
                   const wxString & label,
                   const wxString & displayFormat,
                   const wxPoint & position,
                   const wxSize & size,
                   long style,
                   const wxString & name)
{
    if (! HtmlText::Create(parent, id, label, position, size, style, name))
        return false;

    SetDisplayFormat(displayFormat);

    // Config
    ConfigManager::CluePrompt_t & prompt =
        wxGetApp().GetConfigManager().CluePrompt;
    prompt.font.AddCallback(this, &CluePrompt::SetFont);
    prompt.backgroundColor.AddCallback(this, &CluePrompt::SetBackgroundColour);
    prompt.foregroundColor.AddCallback(this, &CluePrompt::SetForegroundColour);
    prompt.displayFormat.AddCallback(this, &CluePrompt::SetDisplayFormat);
    return true;
}

CluePrompt::~CluePrompt()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}

void
CluePrompt::SetClue(const puz::Clue * clue)
{
    wxString result;
    if (clue)
    {
        wxString number = puz2wx(clue->GetNumber());
        wxString text = puz2wx(clue->GetText());
        if (! number.empty())
        {
            result = m_displayFormat;

            // Deal with literal "%" (not that anyone would really want a literal
            // percent sign in their clue prompt . . .)
            result.Replace(_T("%%"), _T("%Z"));
            // Just in case the text has any literal "%"s, make sure we don't
            // accidentally overwrite them.
            text.Replace(_T("%"), _T("%Z"));

            // Clue Number
            result.Replace(_T("%N"), number);

            // Clue Direction
            /*
            if (direction == puz::ACROSS)
            {
                result.Replace(_T("%D"), _T("Across"));
                result.Replace(_T("%d"), _T("A"));
            }
            else
            {
                result.Replace(_T("%D"), _T("Down"));
                result.Replace(_T("%d"), _T("D"));
            }
            */

            // Clue Text
            result.Replace(_T("%T"), text);

            // Literal "%"
            result.Replace(_T("%Z"), _T("%"));
        }
    }

    SetLabel(result);
    LayoutCell();
    Refresh();
}
