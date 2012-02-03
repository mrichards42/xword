/****************************************************************************** 
                       luacurl for XWord

   Modified for XWord by
   - Mike Richards <mrichards42@gmx.com>

   Changes:
   - Add support for cURL's progress callback
   - Remove support for curl http forms (not important for XWord, and I
     couldn't get it to compile)
   - Add support for CURLOPT_COOKIELIST

   All changes to the original file are marked with 
   XWORD START
   XWORD END

 ******************************************************************************/

 
/****************************************************************************** 
                       cURL_Lua, Lua bindings for cURL
 ******************************************************************************
   
   Released under the GNU/GPL license or MIT license (at your option), 
   no warranties.

   Original Author: 
   	- Enrico Tassi <gareuselesinge@users.sourceforge.net>
  
   status: 
   	- binds from 7.9.5 to 7.18.1
   
   changelog:
   	- revamped to libcurl 7.18.1
        - fixed large file support
   	- added a Makefile based on libtool as suggested in the debian 
	  package of lua5.1
   	- first public release
   
   todo:
	- WRITE_CB,READ_CB,DEBUG_CB must be identifyed by a unique pointer, but
	  using CURL* + OFFSET may not be the case... think a bit more
	- CURLINFO ???
	- CURLOPT_COPYPOSTFIELDS 

   missing:
	- CURLOPT_IOCTLFUNCTION/DATA
	- CURLOPT_SOCKOPTFUNCTION/DATA
	- CURLOPT_OPENSOCKETFUNCTION/DATA 
	- CURLOPT_SEEKFUNCTION/DATA
	  
 ******************************************************************************
   $Id: lua-curl.c,v 1.6 2008/10/25 14:38:29 gareuselesinge Exp $
 ******************************************************************************/ 
#include <lua.h>
#include <lauxlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

#include "luabind.h"
#include "version.h"

#if ! defined (LUA_VERSION_NUM) || LUA_VERSION_NUM < 501
  #include "compat-5.1.h"
#endif

#define CURL_EASY_META_NAME "curleasy.type"

/* think more if this means unicity... maybe store in the bag some pointers */
/* this need that a has size > bigger_offset */

#define CURL_WRITECB_OFF(a)	((void*)(((char*)a)+0))
#define CURL_READCB_OFF(a)	((void*)(((char*)a)+1))
#define CURL_HEADCB_OFF(a)	((void*)(((char*)a)+2))

/* XWORD START */
#define CURL_PROGRESSCB_OFF(a)	((void*)(((char*)a)+3))
/* XWORD END */

/* to check the curl version on the fly, this is a v >= LIBCURL_VERSION */
#define CURL_NEWER(M,m,p) ((p + (m << 8) + (M << 16)) <= LIBCURL_VERSION_NUM)
#define CURL_OLDER(M,m,p) ((p + (m << 8) + (M << 16)) > LIBCURL_VERSION_NUM)

/* some compatibility for name aliases */
#ifndef CURLOPT_WRITEDATA
	#define CURLOPT_WRITEDATA CURLOPT_FILE
#endif
#ifndef CURLOPT_READDATA
	#define CURLOPT_READDATA  CURLOPT_INFILE 
#endif
#ifndef CURLOPT_HEADERDATA
	#define CURLOPT_HEADERDATA CURLOPT_WRITEHEADER
#endif

/* XWORD START */
/* CURLOPT_PROGRESSDATA has is its own #define in curl/curl.h */
/* XWORD END */

/* strings put in the bag, vectorialized for faster/shorter access */
	
        #define STR_CAINFO 		0
        #define STR_COOKIE 		1
        #define STR_COOKIEFILE 		2
        #define STR_COOKIEJAR 		3
/* XWORD START */
        #define STR_COOKIELIST 		4
/* XWORD END */
        #define STR_CUSTOMREQUEST 	5
        #define STR_EGDSOCKET 		6
        #define STR_FTPPORT 		7
        #define STR_INTERFACE 		8
        #define STR_KRB4LEVEL 		9
        #define STR_POSTFIELDS 		10
        #define STR_PROXY 		    11
        #define STR_PROXYUSERPWD 	12
        #define STR_RANDOM_FILE 	13
        #define STR_RANGE 		    14
        #define STR_REFERER 		15
        #define STR_SSLCERT 		16
        #define STR_SSLCERTPASSWD 	17
        #define STR_SSLCERTTYPE 	18
        #define STR_SSLENGINE 		19	
        #define STR_SSLKEY 		    20
        #define STR_SSLKEYTYPE 		21
        #define STR_SSL_CIPHER_LIST 22
        #define STR_URL 		    23
        #define STR_USERAGENT 		24
        #define STR_USERPWD 		25

	#define STR_LAST STR_USERPWD

#if CURL_NEWER(7,9,8)
	#define STR_CAPATH 		25
	#undef STR_LAST
	#define STR_LAST STR_CAPATH
#endif
#if CURL_NEWER(7,10,0)
	#define STR_ENCODING 		26
	#undef STR_LAST
	#define STR_LAST STR_ENCODING
#endif
#if CURL_NEWER(7,10,3)
	#define STR_PRIVATE 		27
	#undef STR_LAST
	#define STR_LAST STR_PRIVATE
#endif
#if CURL_NEWER(7,11,0)
	#define STR_NETRC_FILE 		28
	#undef STR_LAST
	#define STR_LAST STR_NETRC_FILE
#endif

	#define STR_SIZE (STR_LAST + 1)



/******************************************************************************
 * we need to keep with us the CURL handler plus some buffers
 * 
 */ 
struct L_curl_bag {
	CURL* handler;
	char* strings[STR_SIZE];
	char err[CURL_ERROR_SIZE];
	struct curl_slist  * headlist;
#if CURL_NEWER(7,9,6)
	struct curl_httppost *post;
#endif
};

/******************************************************************************
 * table created with this script:
 * 
 * cat /usr/include/curl/curl.h | grep "^ *CINIT(" | \
 *  	sed "s/CINIT(/{\"OPT_/" | sed -r "s/, +/\",CURLOPTTYPE_/" | \
 *	sed "s/, / + /" | sed "s/),/},/" > curlopt.h
 *
 */ 
static const struct L_const curl_easy_c [] = {
#include "curlopt.h"
  {NULL,0}
};

/******************************************************************************
 * table created with this script:
 * 
 *  cat /usr/include/curl/curl.h | grep "^ *CURL_NETRC_[A-Z]*," | \
 *	cut -f 1 -d "," |  \
 *	awk '{print "{\"" $1 "\", (int)" $1 "}," }' | \
 *	sed "s/CURL_//" > curl_netrcopt.h
 */
#if CURL_NEWER(7,9,8)
static const struct L_const curl_easy_netrc_c [] = {
#include "curl_netrcopt.h"
  {NULL,0}
};
#endif

/******************************************************************************
 * table created with this script:
 * 
 *  cat /usr/include/curl/curl.h | grep "CURLAUTH_" | \
 *		sed "s/#define *CURL/{\"/" | sed "s/ *\/\*.*\*\///" | \
 *		sed "s/ /\",/" | sed "s/$$/},/" > curl_authopt.h
 */
#if CURL_NEWER(7,10,6)
static const struct L_const curl_easy_auth_c [] = {
#include "curl_authopt.h"
  {NULL,0}
};
#endif

/******************************************************************************
 * table created with this script:
 * 
 *  cat /usr/include/curl/curl.h | grep -B10 '} curl_ftpauth' | \
 *		grep "CURLFTPAUTH_" | \
 *		grep -v "LAST.*never use" | \
 *		sed "s/^ *CURL//" | sed "s/^\([^,]*\),.*$$/{\"\1\",CURL\1},/" \
 *		> curl_ftpauthopt.h
 */
#if CURL_NEWER(7,12,2)
static const struct L_const curl_easy_ftpauth_c [] = {
#include "curl_ftpauthopt.h"
  {NULL,0}
};
#endif

/******************************************************************************
 * table created with this script:
 * 
 *  cat /usr/include/curl/curl.h | grep -B10 '} curl_ftpmethod' | \
 *		grep "CURLFTPMETHOD_" | \
 *		grep -v "LAST.*never use" | \
 *		sed "s/^ *CURL//" | sed "s/^\([^,]*\),.*$$/{\"\1\",CURL\1},/" \
 *		> curl_ftpmethods.h
 */
#if CURL_NEWER(7,15,2)
static const struct L_const curl_easy_ftpmethods_c [] = {
#include "curl_ftpmethods.h"
  {NULL,0}
};
#endif

/******************************************************************************
 * table created by hand:
 * 
 */
static const struct L_const curl_easy_httpver_c [] = {
  {"HTTP_VERSION_NONE",CURL_HTTP_VERSION_NONE},
  {"HTTP_VERSION_1_0",CURL_HTTP_VERSION_1_0},
  {"HTTP_VERSION_1_1",CURL_HTTP_VERSION_1_1},
  {NULL,0}
};
/******************************************************************************
 * table created by hand:
 * 
 */
#if CURL_NEWER(7,11,0)
static const struct L_const curl_easy_ftpssl_c [] = {
  {"FTPSSL_NONE",CURLFTPSSL_NONE},
  {"FTPSSL_TRY",CURLFTPSSL_TRY},
  {"FTPSSL_CONTROL",CURLFTPSSL_CONTROL},
  {"FTPSSL_ALL",CURLFTPSSL_ALL},
  {NULL,0}
};
#endif
/******************************************************************************
 * table created by hand:
 * 
 */
static const struct L_const curl_easy_closepolicy_c [] = {
  {"CLOSEPOLICY_LEAST_RECENTLY_USED",CURLCLOSEPOLICY_LEAST_RECENTLY_USED},
  {"CLOSEPOLICY_OLDEST",CURLCLOSEPOLICY_OLDEST},
  {NULL,0}
};
/******************************************************************************
 * table created by hand:
 * 
 */
#if CURL_NEWER(7,10,8)
static const struct L_const curl_easy_ipresolve_c [] = {
  {"IPRESOLVE_WHATEVER",CURL_IPRESOLVE_WHATEVER},
  {"IPRESOLVE_V4",CURL_IPRESOLVE_V4},
  {"IPRESOLVE_V6",CURL_IPRESOLVE_V6},
  {NULL,0}
};
#endif
/******************************************************************************
 * table created by hand:
 * 
 */
#if CURL_NEWER(7,10,0)
static const struct L_const curl_easy_proxytype_c [] = {
  {"PROXY_HTTP",CURLPROXY_HTTP},
  {"PROXY_SOCKS5",CURLPROXY_SOCKS5},
  {NULL,0}
};
#endif
/******************************************************************************
 * table created with this script:
 * 
 * cat  /usr/include/curl/curl.h | grep "^ *CFINIT" | \
 *		grep -v "CFINIT(NOTHING)" | sed "s/CFINIT(//" | \
 *		sed "s/),/ ,/" | \
 *		awk '{print "{\"FORM_" $1 "\",CURLFORM_" $1 "},"  }' > \
 *		curl_form.h 
 */
static const struct L_const curl_easy_form_c [] = {
#include "curl_form.h"
  {NULL,0}
};


/******************************************************************************
 * checks and returns a CURL* handler from the first position in the stack
 * 
 */ 
static CURL* L_checkcurleasy(lua_State*L)
{
  void* tmp = luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");
  return ((struct L_curl_bag*)tmp)->handler;
}

/******************************************************************************
 * checks and returns the userdata
 *  
 */ 
static struct L_curl_bag* L_checkcurluserdata(lua_State*L)
{
  void* tmp = luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");
  return ((struct L_curl_bag*)tmp);
}

/******************************************************************************
 * maps a curl option to the right bag.strings[] element
 * 
 */ 
static unsigned int L_CURLoption2vect(lua_State*L,CURLoption opt){

switch (opt) {
        case CURLOPT_CAINFO: return STR_CAINFO;
        case CURLOPT_COOKIE: return STR_COOKIE;
        case CURLOPT_COOKIEFILE: return STR_COOKIEFILE;
        case CURLOPT_COOKIEJAR: return STR_COOKIEJAR;
/* XWORD START */
        case CURLOPT_COOKIELIST: return STR_COOKIELIST;
/* XWORD END */
        case CURLOPT_CUSTOMREQUEST: return STR_CUSTOMREQUEST;
        case CURLOPT_EGDSOCKET: return STR_EGDSOCKET;
        case CURLOPT_FTPPORT: return STR_FTPPORT;
        case CURLOPT_INTERFACE: return STR_INTERFACE;
        case CURLOPT_KRB4LEVEL: return STR_KRB4LEVEL;
        case CURLOPT_POSTFIELDS: return STR_POSTFIELDS;
        case CURLOPT_PROXY: return STR_PROXY;
        case CURLOPT_PROXYUSERPWD: return STR_PROXYUSERPWD;
        case CURLOPT_RANDOM_FILE: return STR_RANDOM_FILE;
        case CURLOPT_RANGE: return STR_RANGE;
        case CURLOPT_REFERER: return STR_REFERER;
        case CURLOPT_SSLCERT: return STR_SSLCERT;
        case CURLOPT_SSLCERTPASSWD: return STR_SSLCERTPASSWD;
        case CURLOPT_SSLCERTTYPE: return STR_SSLCERTTYPE;
        case CURLOPT_SSLENGINE: return STR_SSLENGINE;
        case CURLOPT_SSLKEY: return STR_SSLKEY;
        case CURLOPT_SSLKEYTYPE: return STR_SSLKEYTYPE;
        case CURLOPT_SSL_CIPHER_LIST: return STR_SSL_CIPHER_LIST;
        case CURLOPT_URL: return STR_URL;
        case CURLOPT_USERAGENT: return STR_USERAGENT;
        case CURLOPT_USERPWD: return STR_USERPWD;
#if CURL_NEWER(7,9,8)
	case CURLOPT_CAPATH: return STR_CAPATH;
#endif
#if CURL_NEWER(7,10,0)
	case CURLOPT_ENCODING: return STR_ENCODING;
#endif
#if CURL_NEWER(7,10,3)
	case CURLOPT_PRIVATE: return STR_PRIVATE;
#endif
#if CURL_NEWER(7,11,0)
	case CURLOPT_NETRC_FILE: return STR_NETRC_FILE;
#endif
 	default: L_error(L,"Unsupported string in bag");
}

return 0;
}

/******************************************************************************
 * checks and returns a string field from the first position in the stack
 * 
 */ 
static char** L_checkcurlstring(lua_State*L,CURLoption opt)
{
  struct L_curl_bag* tmp = (struct L_curl_bag*)
	  luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");

  return &(tmp->strings[L_CURLoption2vect(L,opt)]);
}
/******************************************************************************
 * checks and returns the header slist
 * 
 */ 
static struct curl_slist ** L_checkcurlheadlist(lua_State*L)
{
  struct L_curl_bag* tmp = (struct L_curl_bag*)
	  luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");

  return &(tmp->headlist);
}

/******************************************************************************
 * checks and returns the err field from the first position in the stack
 * 
 */ 
static char* L_checkcurlerr(lua_State*L)
{
  void* tmp = luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");
  return ((struct L_curl_bag*)tmp)->err;
}

/******************************************************************************
 * checks and returns the post field from the first position in the stack
 * 
 */
#if CURL_NEWER(7,9,6)
static struct curl_httppost **L_checkcurlpost(lua_State*L){
  void* tmp = luaL_checkudata(L,1,CURL_EASY_META_NAME);
  luaL_argcheck(L,tmp != NULL,1,"curleasy expected");
  return &((struct L_curl_bag*)tmp)->post;

}
#endif
/******************************************************************************
 * checks if c is_in t and returns it
 *
 */ 
static long L_checkconst(lua_State* L,
		const struct L_const* t,const char* t_nam, int c){
int i,found;
long int con;

if( lua_type(L,c) != LUA_TNUMBER)
	L_error(L,"Expecting a %s value, got %s",t_nam,
		lua_typename(L,lua_type(L,c)));

con = (long int)lua_tonumber(L,c);

for ( i = 0,found = 0 ; t[i].name != NULL ; i++){
	if( t[i].value == con){
		found = 1;
		break;
	}
}

if(found == 1)
	return con;
else {
	L_error(L,"Expecting a %s value, got something else",t_nam);
}
	
return -1;
}

/******************************************************************************
 * checks if c is <= \sigma t and returns it
 *
 */ 
static long int L_checkconst_mask(lua_State* L,
		const struct L_const* t,const char* t_nam, int c){
int con;

if( lua_type(L,c) != LUA_TNUMBER )
	L_error(L,"Expecting a %s value, got nothing",t_nam);

/* FIXME: think a check */
con = (long int)lua_tonumber(L,c);

return con;
}

/******************************************************************************
 * checks, builds and return a string list
 *
 */ 
static struct curl_slist * L_checkslist(lua_State* L,int tab_index) {
	
struct curl_slist * sl = NULL;

/* since we manipulate the stack we want tab_index in absolute */
if ( tab_index < 0 )
	tab_index = lua_gettop(L) + 1 + tab_index;
	
/* a slist must be a LUA table */
luaL_argcheck(L,lua_istable(L,tab_index),tab_index,"expecting a table");

/* create the slist */
sl = NULL;

/* traverse the table */
lua_pushnil(L);
while( lua_next(L,tab_index) != 0 ){
	const char * val;
	
	/* now we have: ...old_stack... | key:int | val:string */
	if ( lua_type(L,-1) != LUA_TSTRING) {
		curl_slist_free_all(sl);
		L_error(L,"this table must be a string list");
	}
	if ( lua_type(L,-2) != LUA_TNUMBER ) {
		curl_slist_free_all(sl);
		L_error(L,"this table is a list, keys must be unused");
	}

	/* get the string */
	val = lua_tostring(L,-1);
	
	/* pop val */
	lua_pop(L,1);
	
	/* store it in the list */
	sl = curl_slist_append(sl,val);
}

return sl;
}

/******************************************************************************
 * curl_easy_perform
 *
 */ 
static int luacurl_easy_perform(lua_State* L) {
CURL* c = L_checkcurleasy(L);
CURLcode rc = curl_easy_perform(c);

L_checknarg(L,1,"perform wants 1 argument (self)");

if ( rc == CURLE_OK ) {
	lua_pushnumber(L,(lua_Number)rc);
	lua_pushnil(L);
}else{
	lua_pushnumber(L,(lua_Number)rc);
	lua_pushstring(L,L_checkcurlerr(L));
}

return 2;
}



/* luacurl callbacks */
#define LUACURL_CB_FUNC_START(luacb, cb, usercb)                              \
static size_t  L_callback_ ## luacb (                                         \
			void *ptr,size_t size,size_t nmemb,void *stream){                 \
lua_State* L = (lua_State*)stream;                                            \
size_t rc;                                                                    \
size_t dimension = size * nmemb;                                              \
                                                                              \
L_checknarg(L,1,"we expect the callback to have a curl handler on the stack");\
                                                                              \
/* find the lua closure */                                                    \
/* XXX we assume the c:perform() leaves c on the stack */                     \
lua_pushlightuserdata(L,CURL_ ## cb ## _OFF(L_checkcurleasy(L)));             \
lua_rawget(L,LUA_REGISTRYINDEX);                                              \
                                                                              \
                                                                              \
/* call it */                                                                 \
lua_pushlstring(L,(const char *)ptr,dimension);                               \
lua_pushnumber(L,dimension);                                                  \
lua_call(L,2,2);                                                              \
                                                                              \
L_checknarg(L,3,"we expect the callback to return 2 arguments");              \
                                                                              \
if (lua_type(L,-2) != LUA_TNUMBER)                                            \
	L_error(L," ## usercb must return: (number,errror_message or nil) "       \
		"but the first one is not a number");                                 \
                                                                              \
rc = (size_t)lua_tonumber(L,-2);

/* The user can insert code that checks the return code here */

#define LUACURL_CB_FUNC_END()                                                \
lua_pop(L,2);                                                                \
                                                                             \
return rc;                                                                   \
}

/******************************************************************************
 * curl write callback
 *
 */
LUACURL_CB_FUNC_START(writedata, WRITECB, write_cb)
/*
if( rc != dimension  ) {
	if ( lua_type(L,-1) == LUA_TSTRING)
		L_error(L,"write_cb returned %d that is not the expected %d"
		 ", error message: '%s'",rc,dimension,lua_tostring(L,-1));
	else
		L_error(L,"write_cb returned %d that is not the expected %d"
		 ", no error message",rc,dimension);
}
*/
LUACURL_CB_FUNC_END()


/******************************************************************************
 * curl write header callback
 *
 */ 
LUACURL_CB_FUNC_START(writehead, HEADCB, head_cb)
/*
if( rc != dimension  ) {
	if ( lua_type(L,-1) == LUA_TSTRING)
		L_error(L,"head_cb returned %d that is not the expected %d"
		 ", error message: '%s'",rc,dimension,lua_tostring(L,-1));
	else
		L_error(L,"head_cb returned %d that is not the expected %d"
		 ", no error message",rc,dimension);
}
*/
LUACURL_CB_FUNC_END()

/******************************************************************************
 * curl read callback
 *
 */ 
LUACURL_CB_FUNC_START(readdata, READCB, read_cb)
/* Check the return code */
if(rc != 0) {
	/* we have data to send */
	if ( rc > dimension )
		L_error(L,"read_rc must return a size <= than the number "
			"that received in input");
	if ( lua_type(L,-1) != LUA_TSTRING)
		L_error(L,"read_cb must return a string as the second "
			"value, not a %s",lua_typename(L,lua_type(L,-1)));
	if ( rc != lua_strlen(L,-1) )
		L_error(L,"read_cb must return a size and a string, "
			"and the size must be the string size");
	memcpy(ptr,lua_tostring(L,-1),rc);
}
LUACURL_CB_FUNC_END()


/* XWORD START */

/******************************************************************************
 * curl progress callback
 *
 */
/* We can't use the LUA_CB_FUNC_START template because the progress function
   recieves different parameters than the read / write functions */

/* The lua_State is passed as the CURLOPT_PROGRESSDATA parameter (void * ptr) */
static size_t  L_callback_progressdata (void * ptr,
                                        double dltotal, double dlnow,
                                        double ultotal, double ulnow)
{
    lua_State* L = (lua_State*)ptr;
    size_t rc;

    L_checknarg(L, 1,
        "we expect the callback to have a curl handler on the stack");

    /* find the lua closure */
    /* XXX we assume the c:perform() leaves c on the stack */
    lua_pushlightuserdata(L,CURL_PROGRESSCB_OFF(L_checkcurleasy(L)));
    lua_rawget(L,LUA_REGISTRYINDEX);

    /* call it */
    lua_pushnumber(L, dltotal);
    lua_pushnumber(L, dlnow);
    lua_pushnumber(L, ultotal);
    lua_pushnumber(L, ulnow);

    lua_call(L, 4, 2);

    L_checknarg(L,3,"we expect the callback to return 2 arguments");

    if (lua_type(L,-2) != LUA_TNUMBER)
        L_error(L,"progress_cb must return: (number,errror_message or nil) "
                   "but the first one is not a number");

    rc = (size_t)lua_tonumber(L,-2);
    lua_pop(L,2);

    return rc;
}

/* XWORD END */


/******************************************************************************
 * CURLOPT_HTTPPOST parser
 *
 */ 

/* XWORD START */

/* The code that allocates a dynamic array
       struct curl_forms forms[forms_size];
   doesn't compile for me, so I'm just going to comment this chunk out */

/* XWORD END */

#if CURL_NEWER(7,9,8) && defined(XWORD_USE_HTTPPOST)
static CURLcode L_httppost(CURL* c,CURLoption opt,lua_State* L){
/* we assume we hve stack: || c | opt | table 
 *
 * table is a table of tables since we assume the function is called:
 * c:setopt(curl.OPT_HTTPPOST,{
 *	{curl.FORM_COPYNAME,"name1",
 *	 curl.FORM_COPYCONTENTS,"data1",
 *	 curl.FORM_CONTENTTYPE,"Content-type: text/plain",
 *	 curl.FORM_END},
 *	{curl.FORM_COPYNAME,"name2",
 *	 curl.FORM_COPYCONTENTS,"data2",
 *	 curl.FORM_CONTENTTYPE,"Content-type: text/plain",
 *	 curl.FORM_END}
 * })
 *
 */
struct curl_httppost *post = NULL, *last = NULL;
#if CURL_NEWER(7,9,8)
CURLFORMcode rc = CURL_FORMADD_OK;
#else
int rc = CURL_FORMADD_OK;
#endif

CURLcode rc_opt = CURLE_OK;

/* check for the table */
if( ! lua_istable(L,3) )
	L_error(L,"expecting a table, got %s",lua_typename(L,lua_type(L,3)));

/* outer loop to travers the table list */
lua_pushnil(L);
while( lua_next(L,3) != 0 ){
	/* now we have: ...old_stack... | key:int | val:table 
	 * and we traverse the internal table
	 */
	int forms_size = L_tablesize(L,5)/2+1;
	struct curl_forms forms[forms_size]; /* not ANSI */
        int position = 0;
	
	lua_pushnil(L);
	while( lua_next(L,5) != 0 ){
		CURLformoption o = (CURLformoption)
			L_checkconst(L,curl_easy_form_c,"CURLformoption",7);
		switch(o){
			case CURLFORM_BUFFER:
			case CURLFORM_BUFFERPTR:
			case CURLFORM_FILENAME:
			case CURLFORM_CONTENTTYPE: /* sould be ok */
			case CURLFORM_FILE:
			case CURLFORM_FILECONTENT:
			case CURLFORM_PTRCONTENTS:
			case CURLFORM_COPYCONTENTS:
			case CURLFORM_PTRNAME:
			case CURLFORM_COPYNAME:{
				forms[position].option = o;
				lua_pop(L,1);
				if(lua_next(L,5) == 0)
					L_error(L,
					 "incomplete FORM, value missed");
				forms[position].value = luaL_checkstring(L,7);
			}break;
			
			case CURLFORM_BUFFERLENGTH:{
				forms[position].option = o;
				lua_pop(L,1);
				if(lua_next(L,5) == 0)
					L_error(L,
					 "incomplete FORM, value missed");
				forms[position].value = (char*)
#ifdef __UWORD_TYPE
					(__UWORD_TYPE) 
#endif
					/* not sure this makes sense */
					luaL_checkint(L,7);
			}break;
						   
			case CURLFORM_CONTENTHEADER:{
				/* we need a damned bag here! */			    			L_error(L,"not implemented, use "
					"CURLFORM_CONTENTTYPE instead");
			}break;

			case CURLFORM_END:{
				forms[position].option = o;		  
			}break;

			case CURLFORM_ARRAY:{
				L_error(L,"You can't use CURLFORM_ARRAY");
			}break;
			
			default:{
				L_error(L,"invalid CURLFORM_");	
			}break;
		}
		position++;
		lua_pop(L,1);
	}

	if ( (position<forms_size && forms[position].option != CURLFORM_END) ||
	     (position==forms_size && forms[position-1].option != CURLFORM_END))
		L_error(L,"You must end a form with CURLFORM_END");
	
	rc = curl_formadd(&post,&last,CURLFORM_ARRAY,forms,CURLFORM_END);
	if( rc != CURL_FORMADD_OK) {
		char* desc = NULL;
		switch(rc){
			case CURL_FORMADD_MEMORY:
				desc="the FormInfo allocation fails";break;
			case CURL_FORMADD_OPTION_TWICE:
				desc="one option is given twice for one Form";
				break;
			case CURL_FORMADD_NULL:
				desc="a null pointer was given for a char";
				break;
 			case CURL_FORMADD_UNKNOWN_OPTION:
				desc="an unknown option was used";break;
			case CURL_FORMADD_INCOMPLETE:
				desc="some FormInfo is not complete (or error)";
				break;
 			case CURL_FORMADD_ILLEGAL_ARRAY:
				desc="an illegal option is used in an "
					"array (internal)";
				break;
			default: desc = "Unknown error";break;
		}
		L_error(L,"Invalid form '%s'",desc);
	}
	lua_pop(L,1);
}

rc_opt = curl_easy_setopt(c,opt,post);

if( *L_checkcurlpost(L) != NULL)
	curl_formfree(*L_checkcurlpost(L));

*L_checkcurlpost(L) = post;
	
return rc_opt;
}
#endif
/******************************************************************************
 * curl_easy_setopt
 *
 */ 
static int luacurl_easy_setopt(lua_State* L) {
CURL* c = L_checkcurleasy(L);
CURLoption opt = (CURLoption)L_checkconst(L,curl_easy_c,"CURLoption",2);
CURLcode rc = CURLE_OK;

L_checknarg(L,3,"setopt wants 3 argument (self,opt,val)");

switch(opt) {
	/* long */
#if CURL_NEWER(7,18,0)
	case CURLOPT_PROXY_TRANSFER_MODE:
#endif
#if CURL_NEWER(7,17,1)
	case CURLOPT_POST301:	
#endif
#if CURL_NEWER(7,16,4)
	case CURLOPT_NEW_FILE_PERMS:
	case CURLOPT_NEW_DIRECTORY_PERMS:
#endif
#if CURL_NEWER(7,16,2)
	case CURLOPT_HTTP_CONTENT_DECODING:
	case CURLOPT_HTTP_TRANSFER_DECODING:
	case CURLOPT_TIMEOUT_MS:
	case CURLOPT_CONNECTTIMEOUT_MS:
#endif
#if CURL_NEWER(7,16,0)
	case CURLOPT_SSL_SESSIONID_CACHE:
#endif
#if CURL_NEWER(7,15,2)
	case CURLOPT_FTP_FILEMETHOD:
#endif
#if CURL_NEWER(7,15,2)
	case CURLOPT_CONNECT_ONLY:
	case CURLOPT_LOCALPORT:
	case CURLOPT_LOCALPORTRANGE:
#endif
#if CURL_NEWER(7,15,0)
	case CURLOPT_FTP_SKIP_PASV_IP:
#endif
#if CURL_NEWER(7,14,1)
	case CURLOPT_IGNORE_CONTENT_LENGTH:
#endif
#if CURL_NEWER(7,12,2)
	case CURLOPT_FTPSSLAUTH:
#endif
	case CURLOPT_SSLENGINE_DEFAULT:
	case CURLOPT_SSLVERSION:
	case CURLOPT_SSL_VERIFYPEER:
	case CURLOPT_SSL_VERIFYHOST:
	case CURLOPT_TRANSFERTEXT:
	case CURLOPT_CRLF:
	case CURLOPT_RESUME_FROM:
	case CURLOPT_FILETIME:
	case CURLOPT_NOBODY:
	case CURLOPT_INFILESIZE:
	case CURLOPT_UPLOAD:
#if CURL_NEWER(7,10,8)
	case CURLOPT_MAXFILESIZE:
#endif
	case CURLOPT_TIMECONDITION:
	case CURLOPT_TIMEVALUE:
	case CURLOPT_TIMEOUT:
	case CURLOPT_LOW_SPEED_LIMIT:
	case CURLOPT_LOW_SPEED_TIME:
	case CURLOPT_MAXCONNECTS:
	case CURLOPT_FRESH_CONNECT:
	case CURLOPT_FORBID_REUSE:
	case CURLOPT_CONNECTTIMEOUT:
	case CURLOPT_FTPLISTONLY:
	case CURLOPT_FTPAPPEND:
	case CURLOPT_HEADER:
	case CURLOPT_NOPROGRESS:
#if CURL_NEWER(7,10,0)		
	case CURLOPT_NOSIGNAL:
	case CURLOPT_BUFFERSIZE:
#endif		
	case CURLOPT_FAILONERROR:
	case CURLOPT_PROXYPORT:
	case CURLOPT_HTTPPROXYTUNNEL:
	case CURLOPT_DNS_CACHE_TIMEOUT:
	case CURLOPT_DNS_USE_GLOBAL_CACHE:
	case CURLOPT_PORT:
#if CURL_NEWER(7,11,2)
	case CURLOPT_TCP_NODELAY:
#endif
	case CURLOPT_AUTOREFERER:
	case CURLOPT_FOLLOWLOCATION:
#if CURL_NEWER(7,10,4)
	case CURLOPT_UNRESTRICTED_AUTH:
#endif
	case CURLOPT_MAXREDIRS:
	case CURLOPT_PUT:
	case CURLOPT_POST:
	case CURLOPT_POSTFIELDSIZE:
#if CURL_NEWER(7,9,7)		
	case CURLOPT_COOKIESESSION:
#endif
	case CURLOPT_HTTPGET:
	case CURLOPT_VERBOSE:
#if CURL_NEWER(7,10,7)
	case CURLOPT_FTP_CREATE_MISSING_DIRS:
#endif
#if CURL_NEWER(7,10,8)
	case CURLOPT_FTP_RESPONSE_TIMEOUT:
#endif
#if CURL_NEWER(7,10,5)
	case CURLOPT_FTP_USE_EPRT:
#endif
	case CURLOPT_FTP_USE_EPSV:{
		long par = luaL_checklong(L,3);
		rc = curl_easy_setopt(c,opt,par);
	}break;
				  
	/* curl_off_t */
#if CURL_NEWER(7,15,5)
	case CURLOPT_MAX_SEND_SPEED_LARGE:
	case CURLOPT_MAX_RECV_SPEED_LARGE:
#endif
#if CURL_NEWER(7,11,1)
	case CURLOPT_POSTFIELDSIZE_LARGE:
	case CURLOPT_RESUME_FROM_LARGE:
	case CURLOPT_INFILESIZE_LARGE:
	case CURLOPT_MAXFILESIZE_LARGE:{
		curl_off_t o = (curl_off_t)luaL_checknumber(L,3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#endif
				       
	/* char* */
	case CURLOPT_ERRORBUFFER:{
		/* not used since the lua perform returns it */
		L_error(L,"not used, lua returns the error string "
			"as the second arg if something fails");
	}break; 
#if CURL_NEWER(7,17,1)
	case CURLOPT_SSH_HOST_PUBLIC_KEY_MD5:
#endif
#if CURL_NEWER(7,15,5)
	case CURLOPT_FTP_ALTERNATIVE_TO_USER:
#endif
#if CURL_NEWER(7,14,1)
	case CURLOPT_COOKIELIST:
#endif
#if CURL_NEWER(7,13,0)
	case CURLOPT_FTP_ACCOUNT:
#endif
	case CURLOPT_SSLCERT:
	case CURLOPT_SSLCERTTYPE:
	case CURLOPT_SSLCERTPASSWD: /* alias CURLOPT_SSLKEYPASSWD */
	case CURLOPT_SSLKEY:
	case CURLOPT_SSLKEYTYPE:
	case CURLOPT_SSLENGINE:
	case CURLOPT_CAINFO:
#if CURL_NEWER(7,9,8)				 
	case CURLOPT_CAPATH:
#endif				 
	case CURLOPT_RANDOM_FILE:
	case CURLOPT_EGDSOCKET:
	case CURLOPT_SSL_CIPHER_LIST:
	case CURLOPT_KRB4LEVEL:
#if CURL_NEWER(7,10,3)
	case CURLOPT_PRIVATE:
#endif
	case CURLOPT_RANGE:
	case CURLOPT_CUSTOMREQUEST:
	case CURLOPT_FTPPORT:
	case CURLOPT_PROXY:
	case CURLOPT_INTERFACE:
#if CURL_NEWER(7,11,0)	
	case CURLOPT_NETRC_FILE:
#endif
	case CURLOPT_USERPWD:
	case CURLOPT_PROXYUSERPWD:
#if CURL_NEWER(7,10,0)				 
	case CURLOPT_ENCODING:
#endif
	case CURLOPT_POSTFIELDS:
	case CURLOPT_REFERER:
	case CURLOPT_USERAGENT:
	case CURLOPT_COOKIE:
	case CURLOPT_COOKIEFILE:
	case CURLOPT_COOKIEJAR:
	case CURLOPT_URL: {
		const char* str = luaL_checkstring(L,3);
		char **u = L_checkcurlstring(L,opt);
		free(*u);
		*u = ((str == NULL) ? NULL : strdup(str));
		rc = curl_easy_setopt(c,opt,*u);
	}break;

	/* function ? think more how many type we need here ? */
#if CURL_NEWER(7,9,6)			 
	case CURLOPT_DEBUGFUNCTION:{
		L_error(L,"FIX: Not implemented");			   
	}break;
#endif
#if CURL_NEWER(7,10,6)
	case CURLOPT_SSL_CTX_FUNCTION:
#endif

/* XWORD START */

/* The callback functions (use a macro as a template, it's much easier) */
#define LUACURL_CB_FUNC(func, cb, curlopt, luacb)                             \
    case CURLOPT_ ## func :{                                                  \
        /* we expect a function */                                            \
        if ( ! lua_isfunction(L,3) )                                          \
            L_error(L,"Expecting a function");                                \
        /* we store it somewere, maybe in the registry */                     \
        lua_pushlightuserdata(L,CURL_ ## cb ## _OFF(c));                      \
        lua_pushvalue(L,-2);                                                  \
        lua_rawset(L,LUA_REGISTRYINDEX);                                      \
        /* we save the registry key in the C function bag */                  \
        rc = curl_easy_setopt(c,CURLOPT_ ## curlopt,L);                       \
        /* check for errors */                                                \
        if( rc != CURLE_OK) {                                                 \
            L_error(L,"%s",L_checkcurlerr(L));                                \
        }                                                                     \
        /* we attach the function to a C function that calls it */            \
        rc = curl_easy_setopt(c,                                              \
            CURLOPT_ ## func,L_callback_ ## luacb);               \
    }break;

    LUACURL_CB_FUNC(PROGRESSFUNCTION, PROGRESSCB, PROGRESSDATA, progressdata)
    LUACURL_CB_FUNC(READFUNCTION,     READCB,     READDATA,     readdata)
    LUACURL_CB_FUNC(HEADERFUNCTION,   HEADCB,     WRITEHEADER,  writehead)
    LUACURL_CB_FUNC(WRITEFUNCTION,    WRITECB,    WRITEDATA,    writedata)
#undef LUACURL_CB_FUNC
/* XWORD END */

	/* void* */
#if CURL_NEWER(7,10,6)
	case CURLOPT_SSL_CTX_DATA:
#endif
#if CURL_NEWER(7,9,6)
	case CURLOPT_DEBUGDATA:
#endif
	case CURLOPT_WRITEHEADER:
	case CURLOPT_PROGRESSDATA:
	case CURLOPT_READDATA:
	case CURLOPT_WRITEDATA:{
		L_error(L,"This option must not be used,"
			"use Lua's lexical scoping closure instead");
	}break;
	
	/* FILE* */
	case CURLOPT_STDERR:{
		L_error(L,"Not implemented");		    
		/* it is not hard to put a FILE* in L_curl_bag and open a
		   new FILE* on request made by file name and not FILE*...
		   but not really useful I think */
	}break;

	/* constants */
	case CURLOPT_NETRC:{
#if CURL_NEWER(7,9,8)
		enum CURL_NETRC_OPTION o=(enum CURL_NETRC_OPTION)
			L_checkconst(L,curl_easy_netrc_c,"CURL_NETRC_OPTION",3);
#else
		long o = luaL_checklong(L,3);
#endif
		rc = curl_easy_setopt(c,opt,o);
	}break;
#if CURL_NEWER(7,10,7)			   
	case CURLOPT_PROXYAUTH:
#endif
#if CURL_NEWER(7,10,6)
	case CURLOPT_HTTPAUTH:{
		long int o= L_checkconst_mask(L,
			curl_easy_auth_c,"CURLAUTH_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#endif
	case CURLOPT_HTTP_VERSION:{
		long int o = L_checkconst(L,
			curl_easy_httpver_c,"CURL_HTTP_VERSION_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#if CURL_NEWER(7,11,0)				  
	case CURLOPT_FTP_SSL:{
		long int o = L_checkconst(L,
			curl_easy_ftpssl_c,"CURLFTPSSL_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#endif
	case CURLOPT_CLOSEPOLICY:{
		long int o = L_checkconst(L,
			curl_easy_closepolicy_c,"CURLCLOSEPOLICY_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#if CURL_NEWER(7,10,8)
	case CURLOPT_IPRESOLVE:{
		long int o = L_checkconst(L,
	  		curl_easy_ipresolve_c,"CURL_IPRESOLVE_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#endif
/* FIXME: not sure of this */
#if CURL_NEWER(7,10,0)
	case CURLOPT_PROXYTYPE:{
		long int o = L_checkconst(L,
	  		curl_easy_proxytype_c,"CURLPROXY_*",3);
		rc = curl_easy_setopt(c,opt,o);
	}break;
#endif
		 
	/* slist */
#if CURL_NEWER(7,9,8)			    
	case CURLOPT_HTTPPOST:{
        #ifdef XWORD_USE_HTTPPOST /* The code is buggy, don't use it for now */
		rc = L_httppost(c,opt,L);	      
        #else
        L_error(L, "not implemented");
        #endif /* XWORD_USE_HTTPPOST */
	}break;
#endif
	case CURLOPT_QUOTE:
	case CURLOPT_POSTQUOTE:
#if CURL_NEWER(7,9,5)
	case CURLOPT_PREQUOTE:
#endif
#if CURL_NEWER(7,10,3)			      
	case CURLOPT_HTTP200ALIASES:
#endif
	case CURLOPT_HTTPHEADER:{
		struct curl_slist ** old=NULL;
		struct curl_slist * sl = L_checkslist(L,3);
		rc = curl_easy_setopt(c,opt,sl);
		
		old = L_checkcurlheadlist(L);
		
		curl_slist_free_all(*old);
		*old = sl;
	}break;

	/* share handle */
#if CURL_NEWER(7,10,0)
	case CURLOPT_SHARE:{
		L_error(L,"not implemented");		   
	}break;
#endif

	/* deprecated */
#if CURL_OLDER(7,10,8)
	case CURLOPT_PASSWDFUNCTION:
	case CURLOPT_PASSWDDATA:
#endif
			   {
		L_error(L,"deprecated and not implemented");
	}break;
	
			   
	/* default */
	default:{
		L_error(L,"invalid CURLOPT_");
	}break;
}

/* check for errors */
if( rc != CURLE_OK) {
	L_error(L,"setopt: '%s'",L_checkcurlerr(L));
}

return 0;
}

/******************************************************************************
 * curl_easy_init
 *
 */ 
static int luacurl_easy_init(lua_State* L) {

CURL * tmp = NULL;
struct L_curl_bag* c = NULL;
CURLcode rc = CURLE_OK;
int i;

tmp = curl_easy_init();

if ( tmp == NULL) {
	L_error(L,"curl_easy_init() returned NULL");
}
	
c = (struct L_curl_bag*)lua_newuserdata(L,sizeof(struct L_curl_bag));
luaL_getmetatable(L,CURL_EASY_META_NAME);
lua_setmetatable(L,-2);

c->handler = tmp;
for(i = 0 ; i < STR_SIZE ; i++)
	c->strings[i] = NULL;
c->headlist = NULL;	
#if CURL_NEWER(7,9,6)
c->post=NULL;
#endif
rc = curl_easy_setopt(tmp,CURLOPT_ERRORBUFFER,c->err);

/* check for errors */
if( rc != CURLE_OK) {
	L_error(L,"unable to call CURLOPT_ERRORBUFFER (%d)",rc);
}

return 1;
}

/******************************************************************************
 * curl_easy_cleanup
 *
 */ 
static int luacurl_easy_cleanup(lua_State* L) {
struct L_curl_bag* c = L_checkcurluserdata(L);
int i;

curl_easy_cleanup(c->handler);
for(i = 0 ; i < STR_SIZE ; i++)
	free(c->strings[i]);
curl_slist_free_all(c->headlist);
#if CURL_NEWER(7,9,6)
if(c->post != NULL)
	curl_formfree(c->post);	
#endif

lua_pushlightuserdata(L,CURL_WRITECB_OFF(c));
lua_pushnil(L);
lua_rawset(L,LUA_REGISTRYINDEX);

lua_pushlightuserdata(L,CURL_READCB_OFF(c));
lua_pushnil(L);
lua_rawset(L,LUA_REGISTRYINDEX);

lua_pushlightuserdata(L,CURL_HEADCB_OFF(c));
lua_pushnil(L);
lua_rawset(L,LUA_REGISTRYINDEX);

/* XWORD START */
lua_pushlightuserdata(L,CURL_PROGRESSCB_OFF(c));
lua_pushnil(L);
lua_rawset(L,LUA_REGISTRYINDEX);
/* XWORD END */

return 0;
}

/******************************************************************************
 * curl_escape
 *
 */ 
static int luacurl_escape(lua_State* L) {
const char* data = luaL_checkstring(L,1);
size_t len = lua_strlen(L,1);
char * tmp;
L_checknarg(L,1,"escape wants 1 argument (string)");

tmp = curl_escape(data,len);

lua_pushstring(L,tmp);

#if CURL_NEWER(7,10,0)
curl_free(tmp);
#else
free(tmp);
#endif
	
return 1;
}

/******************************************************************************
 * curl_unescape
 *
 */ 
static int luacurl_unescape(lua_State* L) {
const char* data = luaL_checkstring(L,1);
size_t len = lua_strlen(L,1);
char * tmp;
L_checknarg(L,1,"unescape wants 1 argument (string)");

tmp = curl_unescape(data,len);

lua_pushstring(L,tmp);

#if CURL_NEWER(7,10,0)
curl_free(tmp);
#else
free(tmp);
#endif
	
return 1;
}


/******************************************************************************
 * curl_version
 *
 */ 
static int luacurl_version(lua_State* L) {
L_checknarg(L,0,"version wants no arguments");

lua_pushstring(L,curl_version());

return 1;
}

/******************************************************************************
 * curl_version_info
 *
 */ 
#if CURL_NEWER(7,10,0)
static int luacurl_version_info(lua_State* L) {
curl_version_info_data *d = curl_version_info(CURLVERSION_NOW);	
int i;
L_checknarg(L,0,"version_info wants no arguments");

lua_newtable(L);

lua_pushstring(L,"version");
lua_pushstring(L,d->version);
lua_settable(L,-3);

lua_pushstring(L,"version_num");
lua_pushnumber(L,d->version_num);
lua_settable(L,-3);

lua_pushstring(L,"host");
lua_pushstring(L,d->host);
lua_settable(L,-3);

lua_pushstring(L,"ssl_version");
lua_pushstring(L,d->ssl_version);
lua_settable(L,-3);

lua_pushstring(L,"ssl_version_num");
lua_pushnumber(L,d->ssl_version_num);
lua_settable(L,-3);

lua_pushstring(L,"features");
lua_pushnumber(L,d->features);
lua_settable(L,-3);

lua_pushstring(L,"libz_version");
lua_pushstring(L,d->libz_version);
lua_settable(L,-3);

lua_pushstring(L,"protocols");
lua_newtable(L);
for(i=0;d->protocols[i] != NULL;i++){
	lua_pushnumber(L,i+1);
	lua_pushstring(L,d->protocols[i]);
	lua_settable(L,-3);
}
lua_pushstring(L,"n");
lua_pushnumber(L,i);
lua_settable(L,-3);

lua_settable(L,-3);


return 1;
}
#endif

/******************************************************************************
 * curl.* functions
 *
 */ 
static const struct luaL_reg curl_f [] = {
  {"easy_init",luacurl_easy_init},
  {"escape",luacurl_escape},
  {"unescape",luacurl_unescape},
  {"version",luacurl_version},
#if CURL_NEWER(7,10,0)
  {"version_info",luacurl_version_info},
#endif
  {NULL,NULL}
};

/******************************************************************************
 * c:* functions
 *
 */ 
static const struct luaL_reg curl_easy_m [] = {
  {"setopt",luacurl_easy_setopt},
  {"perform",luacurl_easy_perform},
  {NULL,NULL}
};


/******************************************************************************
 * open the luacurl library
 * you need to call curl_global_init manually
 *
 */ 
int luacurl_open(lua_State* L) {

	luaL_newmetatable(L,CURL_EASY_META_NAME);
	
	lua_pushstring(L,"__gc");
	lua_pushcfunction(L,luacurl_easy_cleanup);
	lua_settable(L,-3);
	
	lua_pushstring(L,"__index");
	lua_pushvalue(L,-2);
	lua_settable(L,-3);
	
	luaL_register(L,NULL,curl_easy_m);
	luaL_register(L,"curl",curl_f);

	lua_pushliteral(L,LUACURL_VERSION);
	lua_setfield(L,-2,"_VERSION");
	lua_pushliteral(L,LUACURL_AUTHOR);
	lua_setfield(L,-2,"_AUTHOR");
	lua_pushliteral(L,LUACURL_LICENSE);
	lua_setfield(L,-2,"_LICENSE");
	
	L_openconst(L,curl_easy_c);
	L_openconst(L,curl_easy_httpver_c);
	L_openconst(L,curl_easy_form_c);
	L_openconst(L,curl_easy_closepolicy_c);
#if CURL_NEWER(7,9,8)
	L_openconst(L,curl_easy_netrc_c);
#endif
#if CURL_NEWER(7,10,0)	
	L_openconst(L,curl_easy_proxytype_c);
#endif
#if CURL_NEWER(7,10,6)
	L_openconst(L,curl_easy_auth_c);
#endif	
#if CURL_NEWER(7,10,8)
	L_openconst(L,curl_easy_ipresolve_c);
#endif
#if CURL_NEWER(7,11,0)
	L_openconst(L,curl_easy_ftpssl_c);
#endif
#if CURL_NEWER(7,12,2)
	L_openconst(L,curl_easy_ftpauth_c);
#endif	
#if CURL_NEWER(7,15,2)
	L_openconst(L,curl_easy_ftpmethods_c);
#endif	

	return 1;
}

/******************************************************************************
 * opens the luacurl library and calls curl_global_init(CURL_GLOBAL_ALL)
 * use this if you have not initialized cURL in the C code
 * 
 */ 
int luacurl_open_and_init(lua_State* L) {

	/* can be called multiple times without troubles */
	curl_global_init(CURL_GLOBAL_ALL);
	
	return luacurl_open(L);
}

LUALIB_API int luaopen_luacurl(lua_State* L) { return luacurl_open_and_init(L); }

/* eof */
