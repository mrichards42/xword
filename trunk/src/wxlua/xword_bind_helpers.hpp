#ifndef XWORD_BIND_HELP_H
#define XWORD_BIND_HELP_H

#include <algorithm> // clue list sorting

// This file should *only* be included from xword_bind.cpp
// It does *not* have the requisite headers for wxLua, because they will be
// included before this file.

//----------------------------------------------------------------------------
// XWord wxString conversion
//----------------------------------------------------------------------------

#define _XWORD_STRING_CONV wxCSConv(wxFONTENCODING_CP1252)


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



// Helper function for GetAcross / GetDown
// Push a lua table on to the stack given an XPuzzle::ClueList
int xword_pushClueList(lua_State *L, const XPuzzle::ClueList & clues)
{
    // Create the table (and push on to the stack)
    lua_newtable(L);

    for (XPuzzle::ClueList::const_iterator it = clues.begin();
         it != clues.end();
         ++it)
    {
        lua_pushnumber(L, it->Number());
        xword_pushwxString(L, it->Text());
        // t[number] = text
        lua_settable(L, -3); // -3 = third from the top of the stack
    }

    return 1; // One object on the stack for lua.
}




// Helper function for SetAcross / SetDown
// Fill in the ClueList with values from the table at the stack index
int xword_table2ClueList(lua_State *L, int index, XPuzzle::ClueList * clues)
{
    if (! lua_istable(L, index))
        wxlua_argerror(L, index, wxT("a table"));

    clues->clear();

    // Iterate the table
    lua_pushnil(L);  /* first key */
    while (lua_next(L, index) != 0)
    {
        // key is index -2
        // value is index -1
        int number = wxlua_getnumbertype(L, -2);
		wxString text = xword_getwxStringtype(L, -1);
        clues->push_back(XPuzzle::Clue(number, text));

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    // Sort the clues, because there is no guarantee that the lua table is
    // sorted.
    std::sort(clues->begin(), clues->end());

    return 0; // No objects on the stack for lua.
}




#endif // XWORD_BIND_HELP_H

