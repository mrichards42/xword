// This file is part of XWord
// Copyright (C) 2014 Mike Richards ( mrichards42@gmx.com )
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

#include "xwordlua.hpp"
#include "xwordluatask.hpp"
#include "paths.hpp"

// Initialize xword lua stuff:
// Set package.path and package.cpath.
// Require the luapuz library.
// Create an xword table with standard paths.
// Set TASK_INIT global function to initialize secondary threads.
void lua_openxword(lua_State * L)
{
    // Set package.path and package.cpath
    lua_getglobal(L, "package");

    lua_pushstring(L, wx2lua(GetLuaPath()));
    lua_setfield(L, -2, "path");

    lua_pushstring(L, wx2lua(GetLuaCPath()));
    lua_setfield(L, -2, "cpath");

    lua_pop(L, 1);

    // Open the luapuz library
    // NB: We have to use require instead of directly calling luaopen_luapuz
    // since luapuz is loaded as a shared library.
    lua_getglobal(L, "require");
    lua_pushstring(L, "luapuz");
    if (lua_pcall(L, 1, 0, 0) != 0) {
        if (lua_isstring(L, -1))
            wxLogDebug("Error loading luapuz library: %s", lua_tostring(L, -1));
        else
            wxLogDebug("Unknown error loading luapuz library");
        lua_pop(L, 1);
    }

    // Set values in xword table:
    //    configdir
    //    scriptsdir
    //    imagesdir
    //    userdatadir
    //    isportable
    //    firstrun
    lua_getglobal(L, "xword");
    if (! lua_istable(L, -1)) // Create the table if it doesn't exist
    {
        lua_newtable(L);
        lua_pushvalue(L, -1); // Copy since setglobal will pop the table
        lua_setglobal(L, "xword");
    }

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

    lua_pushboolean(L, wxGetApp().FirstRun());
    lua_setfield(L, -2, "firstrun");

    lua_pop(L, 1);

    // Setup luatask init function
    lua_pushcfunction(L, lua_openwxtask);
    lua_setglobal(L, "TASK_INIT");
}
