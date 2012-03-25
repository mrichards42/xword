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

#include "MetadataCtrl.hpp"
#include "MyFrame.hpp"
#include "utils/string.hpp"
#include "App.hpp" // GetApp
#include <wx/tokenzr.h>
#ifdef XWORD_USE_LUA
#   include "../lua/luapuz/bind/luapuz_puz_Puzzle.hpp"
#endif

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

IMPLEMENT_DYNAMIC_CLASS(MetadataCtrl, HtmlText)

bool
MetadataCtrl::Create(wxWindow * parent,
                   wxWindowID id,
                   const wxString & displayFormat,
                   const wxPoint & position,
                   const wxSize & size,
                   long style,
                   const wxString & name)
{
    if (! HtmlText::Create(parent, id, wxEmptyString, position, size, style, name))
        return false;
    // Find a MyFrame instance somewhere in the parent hierarchy
    while (parent)
    {
        MyFrame * frame = wxDynamicCast(parent, MyFrame);
        if (frame)
        {
            m_frame = frame;
            break;
        }
        parent = parent->GetParent();
    }
    SetBackgroundColour(*wxWHITE);
    SetDisplayFormat(displayFormat);
    return true;
}

MetadataCtrl::~MetadataCtrl()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}

wxString
MetadataCtrl::FormatLabel()
{
    if (! m_frame)
        return wxEmptyString;
    wxString result;
    const puz::Puzzle & puz = m_frame->GetPuzzle();
    wxStringTokenizer tok(m_displayFormat, _T("%"), wxTOKEN_RET_EMPTY_ALL);
    bool ismeta = false; // Is the current token a metadata value?
    while (tok.HasMoreTokens())
    {
        if (! ismeta)
        {
            result << tok.GetNextToken();
            ismeta = true; // The next token is a metadata value
        }
        else // We're in the middle of a percent-delimited string
        {
            wxString str = tok.GetNextToken();
            if (str.empty()) // This is a literal %
            {
                result << _T("%");
            }
            else // This is a metadata value
            {
#ifdef XWORD_USE_LUA
                // Replace %meta% with puzzle:GetMeta("meta")
                // while escaping quotes
                str.Replace(_T("\""), _T("\\\""));
                result << _T("puzzle:GetMeta(\"") << str << _T("\")");
#else
                result << puz2wx(puz.GetMeta(wx2puz(str)));
#endif
            }
            ismeta = false; // The next token is plain text
        }
    }
#ifdef XWORD_USE_LUA
    // Process this with lua
    wxLuaState & luastate = m_frame->GetwxLuaState();
    if (! luastate.Ok())
        return wxEmptyString;
    lua_State * L = luastate.GetLuaState();
    // Compile a lua function taking a puzzle argument
    if (result.Find(_T("return")) == -1) // Make sure we're returning something
        result = _T("return ") + result;
    wxLuaCharBuffer code(wxString::Format(_T("return function (puzzle) %s end"), result));
    // Compile and run the code
    if (luaL_loadbuffer(L, code.GetData(), code.Length(), "") == 0)
    {
        if (lua_pcall(L, 0, 1, 0) == 0)
        {
            if (lua_isfunction(L, -1)) // Make sure we got a function
            {
                // Call this function
                luapuz_pushPuzzle(L, &m_frame->GetPuzzle());
                if (lua_pcall(L, 1, 1, 0) == 0)
                {
                    // Check the result
                    if (luastate.lua_IsString(-1))
                    {
                        result = luastate.lua_TowxString(-1);
                        lua_pop(L, 1);
                        return result;
                    }
                    // Clean up the stack
                    lua_pop(L, 1);
                }
            }
            else // Clean up the stack
            {
                lua_pop(L, 1);
            }
        }
    }
    else
    {
        // Return the error
        result = luastate.lua_TowxString(-1);
        lua_pop(L, 1);
        return result;
    }
    return wxEmptyString;
#else // ! XWORD_USE_LUA
    return result;
#endif // XWORD_USE_LUA
}
