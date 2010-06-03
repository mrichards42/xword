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

#ifndef XWORD_LUA_H
#define XWORD_LUA_H

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

static void xword_setup_lua_paths(wxLuaState & lua)
{
    lua_State * L = lua.GetLuaState();

    // push the package table
    lua_getglobal(L, "package");

    // package.path
    lua_pushstring(L, "path");
    lua_pushstring(L, wx2lua(GetLuaPath()));
    lua_settable(L, -3);

    // package.cpath
    lua_pushstring(L, "cpath");
    lua_pushstring(L, wx2lua(GetLuaCPath()));
    lua_settable(L, -3);

    // pop the package table
    lua_pop(L, 1);
}

#endif // XWORD_LUA_H
