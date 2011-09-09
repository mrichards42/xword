#ifndef LUAPUZ_TRACKING_H
#define LUAPUZ_TRACKING_H

extern "C" {
#include "lua.h"
}

void luapuz_registerTable(lua_State * L, const char * name);
void luapuz_registerWeakTable(lua_State * L, const char * name);


// ---------------------------------------------------------------------------
// Tracked userdata objects
// ---------------------------------------------------------------------------

extern const char * luapuz_tracked_objects_key;

LUAPUZ_API void luapuz_track_object(lua_State * L, void * objptr);
LUAPUZ_API void luapuz_untrack_object(lua_State * L, void * objptr);
LUAPUZ_API bool luapuz_is_tracked_object(lua_State * L, void * objptr);
LUAPUZ_API bool luapuz_push_tracked_object(lua_State * L, void * objptr);

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

extern const char * luapuz_enumerations_key;

struct luapuz_enumReg {
  const char *label;
  lua_Number value;
};

// Register an enumeration to the table at the top of the stack.
void luapuz_registerEnum(lua_State * L,
                         const char * name,
                         const luapuz_enumReg * e);

// Check whether the value at the given index belongs to the specified enumeration
// and return it.
LUAPUZ_API int luapuz_checkEnum(lua_State * L, int idx, const char * enum_name);

// Check whether the value at the given index belongs to the specified enumeration
// and return it.
LUAPUZ_API bool luapuz_isEnum(lua_State * L, int idx, const char * enum_name);

// ---------------------------------------------------------------------------
// Load Save
// ---------------------------------------------------------------------------

extern const char * luapuz_loadsave_key;

#endif // LUAPUZ_TRACKING_H
