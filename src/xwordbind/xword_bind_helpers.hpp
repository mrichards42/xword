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


//----------------------------------------------------------------------------
// Clue List
//----------------------------------------------------------------------------


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



//----------------------------------------------------------------------------
// Puzzle Errors
//----------------------------------------------------------------------------
#include "../puz/HandlerBase.hpp" // puzzle exceptions

// Throw a puzzle loading error to lua
void xword_puz_error(lua_State * L,
                     const wxString & message,
                     const wxString & type,
                     bool fatal)
{
    // Create the table (and push on to the stack)
    lua_newtable(L);

    // Add the message
    lua_pushnumber(L, 1);
    wxlua_pushwxString(L, message);
    lua_settable(L, -3);

    // Add the type
    lua_pushnumber(L, 2);
    wxlua_pushwxString(L, type);
    lua_settable(L, -3);

    // Add the fatal flag
    lua_pushnumber(L, 3);
    lua_pushboolean(L, fatal);
    lua_settable(L, -3);

    // Throw the lua error on the top of the stack (the table we just created).
    lua_error(L);
}

#define _CATCH_PUZ_EXCEPTION(name)                                            \
    catch(Puz ## name ## Error & error)                                       \
    {                                                                         \
        xword_puz_error(L, error.message, _T(#name) _T("Error"), false);   \
    }

#define _CATCH_FATAL_PUZ_EXCEPTION(name)                                      \
    catch(Puz ## name ## Error & error)                                       \
    {                                                                         \
        xword_puz_error(L, error.message, _T(#name) _T("Error"), true);    \
    }

void
xword_handle_exceptions(lua_State * L)
{
    // Rethrow the previous exception
    try
    {
        throw;
    }

    _CATCH_PUZ_EXCEPTION(Checksum)
    _CATCH_PUZ_EXCEPTION(Section)
    _CATCH_PUZ_EXCEPTION(Data)
    _CATCH_PUZ_EXCEPTION(File)

    _CATCH_FATAL_PUZ_EXCEPTION(Type)
    _CATCH_FATAL_PUZ_EXCEPTION(Header)

    // Special formatting for some errors
    catch(FatalPuzFileError & error)
    {
        xword_puz_error(L, error.message, _T("FileError"), true);
    }

    // The base XWord errors
    catch(FatalPuzError & error)
    {
        xword_puz_error(L, error.message, _T("Error"), true);
    }
    catch(BasePuzError & error)
    {
        xword_puz_error(L, error.message, _T("Error"), false);
    }

    // Non-XWord errors
    catch (std::exception & error)
    {
        xword_puz_error(L, wxString(error.what(), wxConvCurrent), _T("std::exception"), true);
    }
    catch (...)
    {
        xword_puz_error(L, _T("Unknown error"), _T("Unknown"), true);
    }
}

#undef _CATCH_PUZ_EXCEPTION
#undef _CATCH_FATAL_PUZ_EXCEPTION


#endif // XWORD_BIND_HELP_H
