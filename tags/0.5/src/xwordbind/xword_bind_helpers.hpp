#ifndef XWORD_BIND_HELP_H
#define XWORD_BIND_HELP_H

#include <algorithm> // clue list sorting

// This file should *only* be included from xword_bind.cpp
// It does *not* have the requisite headers for wxLua, because they will be
// included before this file.

//----------------------------------------------------------------------------
// XWord wxString conversion
//----------------------------------------------------------------------------

#define _XWORD_STRING_CONV wxConvUTF8


void xword_pushwxString(lua_State *L, const wxString & str)
{
    lua_pushstring(L, str.mb_str(_XWORD_STRING_CONV));
}

// This code is taken (almost) verbatim from wxlstate.cpp (line 1408)
// The first return is changed to reflect the XWord's wxString conversion
wxString LUACALL xword_getwxStringtype(lua_State *L, int stack_idx)
{
    if (wxlua_isstringtype(L, stack_idx))
        return wxString(lua_tostring(L, stack_idx), _XWORD_STRING_CONV);
    else if (wxlua_iswxuserdata(L, stack_idx))
    {
        int stack_type = wxluaT_type(L, stack_idx);

        if (wxluaT_isderivedtype(L, stack_type, *p_wxluatype_wxString) >= 0)
        {
            wxString* wxstr = (wxString*)wxlua_touserdata(L, stack_idx, false);
            wxCHECK_MSG(wxstr, wxEmptyString, wxT("Invalid userdata wxString"));
            return *wxstr;
        }
    }

    wxlua_argerror(L, stack_idx, wxT("a 'string' or 'wxString'"));

    return wxEmptyString;
}

#undef _XWORD_STRING_CONV

#endif // XWORD_BIND_HELP_H
