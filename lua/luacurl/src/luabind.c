/******************************************************************************
 * $Id: luabind.c,v 1.4 2008/10/25 11:58:03 gareuselesinge Exp $
 * This file is part of FreePOPs (http://www.freepops.org)                    *
 * This file is distributed under the terms of GNU GPL license or, at your    *
 * option,  MIT license .                                                     *
 ******************************************************************************/

/******************************************************************************
 * File description:
 *	hacks useful to create bindings
 * Notes:
 *	
 * Authors:
 * 	Name <gareuselesinge@users.sourceforge.net>
 ******************************************************************************/
#include <lua.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "luabind.h"

/******************************************************************************
 * DEBUG ONLY
 * 
 */ 
#define LINE_PREFIX "L: "	
static void L_printstack(lua_State* s)	
{
int i;

fprintf(stderr,"%slua stack image:\n",LINE_PREFIX);
for(i=lua_gettop(s) ; i > 0 ; i-- )
	{
	fprintf(stderr,"%sstack(%2d) : %s: ",LINE_PREFIX,i,
		lua_typename(s,lua_type(s,i)));
	switch(lua_type(s,i)){
		case LUA_TSTRING:
			fprintf(stderr," \"%s\"\n",lua_tostring(s,i));
		break;
		case LUA_TNUMBER:
			fprintf(stderr," %5.3f\n",lua_tonumber(s,i));
		break;
		case LUA_TBOOLEAN:
			fprintf(stderr," %s\n",
				lua_toboolean(s,i)==0?"true":"false");
		break;
		case LUA_TNIL:
			fprintf(stderr," nil\n");
		break;
		default:
			fprintf(stderr," ??\n");
		break;
	}
	}
fprintf(stderr,"%sstack( 0) : --bottom--\n\n",LINE_PREFIX);
}

/******************************************************************************
 * The error function
 * 
 */ 
void L_error(lua_State* L, char* msg, ...){
char buffer[1024];
va_list ap;
	
va_start(ap,msg);
vsnprintf(buffer,1024,msg,ap);
va_end(ap);

L_printstack(L);
luaL_error(L,buffer);
}

/******************************************************************************
 * calculates the size of a table
 *
 */ 
int L_tablesize(lua_State* L,int n){
int i = 0;

if ( !lua_istable(L,n))
	L_error(L,"expecting a table, "
		"not a %s",lua_typename(L,lua_type(L,-1)));

lua_pushnil(L);
while( lua_next(L,n) != 0 ){
	i++;
	lua_pop(L,1);
}

return i;
}
/******************************************************************************
 * check number of arguments
 *
 */ 
void L_checknarg(lua_State* L,int n,char* msg){
if( lua_gettop(L) != n )
	L_error(L,"Stack has %d values: '%s'",lua_gettop(L),msg);
}
/******************************************************************************
 * expects a table on top and adds all t fields to this table
 *
 */ 
void L_openconst(lua_State* L,const struct L_const* t) {
int i;
for ( i = 0 ; t[i].name != NULL ; i++){
	lua_pushstring(L,t[i].name);
	lua_pushnumber(L,(lua_Number)t[i].value);
	lua_settable(L,-3);
}

}
/******************************************************************************
 * expects a table on top and adds all t fields to this table
 * also the metatable typeshould exist
 */ 
void L_openTconst(lua_State* L,const struct L_Tuserdata* t,const char * type){
int i;
for ( i = 0 ; t[i].name != NULL ; i++){
	void * c;
	lua_pushstring(L,t[i].name);
	c = lua_newuserdata(L,sizeof(void*));
	*(void**)c = t[i].data;
	luaL_getmetatable(L,type);
	lua_setmetatable(L,-2);
	lua_settable(L,-3);
}

}
/*******************************************************************************
 * since there is no luaL_checklightuserdata
 *
 *
 */ 
void * L_checkludata(lua_State* L,int n){
if (! lua_islightuserdata(L,n))
	L_error(L,"Argument %d is %s, expected is lightuserdata",n,
		lua_typename(L,lua_type(L,n)));
return lua_touserdata(L,n);
}

