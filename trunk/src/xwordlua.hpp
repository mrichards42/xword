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

#ifndef XWORD_LUA_H
#define XWORD_LUA_H

// This must be included after #include <wx/wx.h>

// Lua!

// wxLua binding initialization
#define wxLUA_USEBINDING_WXLUASOCKET 0
#define wxLUA_USEBINDING_WXGL 0
#define wxLUA_USEBINDING_WXMEDIA 0
#define wxLUA_USEBINDING_WXRICHTEXT 0

#include "wxlua/include/wxlua.h"
#include "wxbind/include/wxbinddefs.h"

WXLUA_DECLARE_BIND_ALL
// XWord binding initialization
extern bool wxLuaBinding_xword_init();

#define XWORD_LUA_IMPLEMENT_BIND_ALL \
        WXLUA_IMPLEMENT_BIND_ALL     \
        wxLuaBinding_xword_init();

/*
// %override wxLua_function_addgcobject
// %function void addgcobject(wxObject * object)
static int LUACALL _addgcobject(lua_State *L)
{
    wxObject * o = (wxObject *)wxlua_touserdata(L, 1, false);
    if (! wxluaO_isgcobject(L, o))
        wxluaO_addgcobject(L, o);
    return 0;
}
*/


static void xword_setup_lua(wxLuaState & lua)
{
    lua_State * L = lua.GetLuaState();

    // Set package.path and package.cpath
    lua_getglobal(L, "package");

    lua_pushstring(L, wx2lua(GetLuaPath()));
    lua_setfield(L, -2, "path");

    lua_pushstring(L, wx2lua(GetLuaCPath()));
    lua_setfield(L, -2, "cpath");

    lua_pop(L, 1);


    // Set values in xword table:
    //    configdir
    //    scriptsdir
    //    imagesdir
    //    userdatadir
    //    isportable
    lua_getglobal(L, "xword");

    lua_pushstring(L, wx2lua(GetConfigDir().c_str()));
    lua_setfield(L, -2, "configdir");

    lua_pushstring(L, wx2lua(GetScriptsDir().c_str()));
    lua_setfield(L, -2, "scriptsdir");

    lua_pushstring(L, wx2lua(GetImagesDir().c_str()));
    lua_setfield(L, -2, "imagesdir");

    lua_pushstring(L, wx2lua(GetUserDataDir().c_str()));
    lua_setfield(L, -2, "userdatadir");

    lua_pushboolean(L, wxGetApp().IsPortable());
    lua_setfield(L, -2, "isportable");

    lua_pushstring(L, wx2lua(XWORD_VERSION_STRING));
    lua_setfield(L, -2, "version");

    lua_pop(L, 1);

    /*
        The only thing I was using this for was the wxFSFile pointer returned
        by wxFileSyetem::OpenFile(), and I just fixed the problem at the
        source


    // Add a addgcobject method to the wxlua table.
    // Ideally we wouldn't need this, but I think there are a few bugs in
    // the way that wxLua decides what to gc.
    lua_getglobal(L, "wxlua");

    lua_pushcfunction(L, _addgcobject);
    lua_setfield(L, -2, "addgcobject");

    lua_pop(L, 1);
    */
}

#endif // XWORD_LUA_H
