/******************************************************************************
 * $Id: luabind.h,v 1.4 2008/10/25 11:58:03 gareuselesinge Exp $
 * This file is part of FreePOPs (http://www.freepops.org)                    *
 * This file is distributed under the terms of GNU GPL license or, at your    *
 * option, MIT license.                                                       *
 ******************************************************************************/


/******************************************************************************/
 /*!
  * \file   luabind.h
  * \brief functions to help in creating bindings
  * 
  * \author Name <gareuselesinge@users.sourceforge.net>
  */
/******************************************************************************/
#ifndef LUABIND_H
#define LUABIND_H

// fill an array of this this with {"NAME",value} and terminate it with {NULL,0}
// and pass it to L_openconst
struct L_const{
	char* name;
	unsigned int value;
};
// fill an array of this this with {"NAME",value} and 
// terminate it with {NULL,NULL}
// and pass it to L_openconst_with_type
struct L_Tuserdata{
	char* name;
	void* data;
};

// a more flexibla luaL_error
void L_error(lua_State* L, char* msg, ...);

// returns the table at position n in the stack
int  L_tablesize(lua_State* L,int n);

// checks if we received n arguments, if not we raise an error
void L_checknarg(lua_State* L,int n,char* msg);

// given a table on top of the stack we add all constats to it
void L_openconst(lua_State* L,const struct L_const* t);

// given a table on top of the stack we add all constats with type to it
void L_openTconst(lua_State* L,const struct L_Tuserdata* t,const char * type);


// since there is no luaL_checklightuserdata
void * L_checkludata(lua_State* L,int n);

#endif
