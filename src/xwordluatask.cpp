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

#include "xwordluatask.hpp"
#include "xwordlua.hpp" // lua_openxword
extern "C" {
#include <lauxlib.h>
}
#include <wxlua\wxllua.h> // lua2wx()
#include <wx/app.h> // wxTheApp

// Thread event
wxDEFINE_EVENT(EVT_LUATASK, wxCommandEvent);

// Redefine task.post to use wx events in secondary threads
// task.post(id, data, flags)
int task_post(lua_State * L) {
    // Get params
    long id = (long)luaL_checknumber(L, 1);
    wxString data = lua2wx(luaL_checkstring(L, 2));
    long flags = (long)luaL_optinteger(L, 3, 0);
    // Don't bother with the id, and assume it will always be 1
    wxCommandEvent * evt = new wxCommandEvent(EVT_LUATASK);
    evt->SetInt(flags);
    evt->SetString(data.c_str());
    wxTheApp->QueueEvent(evt);
    // Always return 0, as this should never fail
    lua_pushnumber(L, 0);
    return 1;
};

// Open wx stuff from a secondary thread
int lua_openwxtask(lua_State * L) {
    // Open xword functions
    lua_openxword(L);
    // Redefine task.post
    lua_getglobal(L, "task");
    lua_pushcfunction(L, task_post);
    lua_setfield(L, -2, "post");
    lua_pop(L, 1);
    return 0;
}