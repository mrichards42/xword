#ifndef CURL_LUA_H
#define CURL_LUA_H

int luacurl_freepops_open_and_init(lua_State* L);
int luacurl_freepops_open(lua_State* L);

// to be loaded from the toplevel (calls init too)
int luaopen_acurl(lua_State* L);

#endif
