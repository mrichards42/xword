/////////////////////////////////////////////////////////////////////////////
// Name:        wxlbind.h
// Purpose:     wxLuaBinding
// Author:      Ray Gilbert, John Labenski, J Winwood
// Created:     14/11/2001
// Copyright:   Ray Gilbert
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXLBIND_H_
#define _WXLBIND_H_

#include "wxlua/include/wxldefs.h"

#ifdef GetObject
    #undef GetObject // MSVC defines this
#endif

class WXDLLIMPEXP_WXLUA wxLuaBinding;
class WXDLLIMPEXP_WXLUA wxLuaState;
struct WXDLLIMPEXP_WXLUA wxLuaBindClass;

// ----------------------------------------------------------------------------
// wxLua binding defines, enums, and structs
// ----------------------------------------------------------------------------

// The inbuilt wxLua types (WXLUA_TXXX) corresponding to Lua types (LUA_TXXX)
//   are shifted to be positive values.
// The Binding wxLua types are positive integers generated automatically when
//   wxLua is initialized and start at WXLUA_T_MAX+1.
// *Use the function bool wxlua_iswxuserdatatype(wxl_type) if you want to
//   differentiate between the two.
//
// Note that WXLUA_TUNKNOWN is used as an initialiser for class types
//   and is used as an end marker for the wxLuaArgType array that
//   represents function prototype argument types in the wxLuaBindCFunc struct
//   and it must always be 0.

// wxLua types for Lua types
#define WXLUA_TUNKNOWN         0  // unset and invalid, not a LUA_TXXX
#define WXLUA_TNONE            1  // LUA_TNONE -1
#define WXLUA_TNIL             2  // LUA_TNIL 0
#define WXLUA_TBOOLEAN         3  // LUA_TBOOLEAN 1
#define WXLUA_TLIGHTUSERDATA   4  // LUA_TLIGHTUSERDATA 2
#define WXLUA_TNUMBER          5  // LUA_TNUMBER 3
#define WXLUA_TSTRING          6  // LUA_TSTRING 4
#define WXLUA_TTABLE           7  // LUA_TTABLE 5
#define WXLUA_TFUNCTION        8  // LUA_TFUNCTION 6
#define WXLUA_TUSERDATA        9  // LUA_TUSERDATA 7
#define WXLUA_TTHREAD          10 // LUA_TTHREAD 8
#define WXLUA_TINTEGER         11 // LUA_TNUMBER but integer only, not a LUA_TXXX
#define WXLUA_TCFUNCTION       12 // LUA_TFUNCTION & lua_iscfunction(), not a LUA_TXXX

#define WXLUA_T_MAX            12 // Max of the Lua WXLUA_TXXX values
#define WXLUA_T_MIN            0  // Min of the Lua WXLUA_TXXX values

#define WXLUATYPE_NULL         13 // C++ NULL, is full wxLua type with metatable

// Check that the Lua LUA_TXXX types are what they used to be
#if (LUA_TNONE != -1) || (LUA_TNIL != 0) || (LUA_TBOOLEAN != 1) || (LUA_TLIGHTUSERDATA != 2) || \
    (LUA_TNUMBER != 3) || (LUA_TSTRING != 4) || (LUA_TTABLE != 5) || (LUA_TFUNCTION != 6) || \
    (LUA_TUSERDATA != 7) || (LUA_TTHREAD != 8)
#   error "Lua has changed it's LUA_TXXX defines."
#endif

// Is this wxLua type one of the basic Lua types
#define WXLUAT_IS_LUAT(wxl_type) (((wxl_type) >= WXLUA_T_MIN) && ((wxl_type) <= WXLUA_T_MAX))

// Blindly convert the lua_type to the wxlua_type. Note: WXLUA_TXXX = LUA_TXXX + 2
// *** See wxlua_luatowxluatype() for a better function ***
#define LUAT_TO_WXLUAT(luatype) ((luatype) + 2)

// Returns true if the wxLua type is for a wxLua binding type and not a
//   generic WXLUA_TXXX type. This means that there might be a metatable for
//   this type in the wxlua_lreg_types_key of the LUA_REGISTRYINDEX table.
#define wxlua_iswxuserdatatype(wxl_type) ((wxl_type) > WXLUA_T_MAX)

// Variables used in the wxLuaArgType member of the wxLuaBindCFunc for
// Lua types. The binding generator uses these and generates new ones for
// classes and structs as specified in the bindings.
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TNONE;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TNIL;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TBOOLEAN;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TLIGHTUSERDATA;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TNUMBER;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TSTRING;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TTABLE;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TFUNCTION;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TUSERDATA;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TTHREAD;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TINTEGER;
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_TCFUNCTION;

// The NULL wxLua type is not part of the bindings, but is installed
// by the wxLuaState since it may be used by various bindings and does not
// rely on wxWidgets.
extern WXDLLIMPEXP_DATA_WXLUA(int) wxluatype_NULL;                 // wxLua type for NULL pointer
extern WXDLLIMPEXP_DATA_WXLUA(wxLuaBindClass) wxLuaBindClass_NULL; // for NULL pointer

// Copies of wxLua types that are used very often.
// Note that we do not use the original since we may not be linked
// to the binding library that defines them.
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxEvent;       // wxLua type for wxEvents
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxWindow;      // wxLua type for wxWindows
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxString;      // wxLua type for wxStrings
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxArrayString; // wxLua type for wxArrayString
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxSortedArrayString; // wxLua type for wxSortedArrayString
extern WXDLLIMPEXP_DATA_WXLUA(int*) p_wxluatype_wxArrayInt;    // wxLua type for wxArrayInt

// ----------------------------------------------------------------------------
// wxLuaArgType a pointer to a declared wxLua type, see wxLuaBindCFunc::argtypes
// ----------------------------------------------------------------------------

typedef int* wxLuaArgType;      // address of wxLua class type (a pointer to it)
extern WXDLLIMPEXP_DATA_WXLUA(wxLuaArgType) g_wxluaargtypeArray_None[1]; // = {0}

// ----------------------------------------------------------------------------
// wxLuaMethod_Type: Values for the wxLuaBindMethod::method_type and wxLuaBindCFunc::method_type
// ----------------------------------------------------------------------------
enum wxLuaMethod_Type
{
    WXLUAMETHOD_CONSTRUCTOR = 0x0001, // constructor
    WXLUAMETHOD_METHOD      = 0x0002, // class member function
    WXLUAMETHOD_CFUNCTION   = 0x0004, // global C function (not part of a class)
    WXLUAMETHOD_GETPROP     = 0x0008, // Get %property funcName, read
    WXLUAMETHOD_SETPROP     = 0x0010, // Set %property funcName, write

    WXLUAMETHOD_STATIC      = 0x1000, // Class member function is static

    WXLUAMETHOD_DELETE      = 0x2000, // This is the delete function that wxLua has generated
                                      // to delete this class and is not part of the
                                      // original class.

    WXLUAMETHOD_ENCAPSULATE = 0x4000, // This class is not derived from a wxObject

    WXLUAMETHOD_CHECKED_OVERLOAD = 0x10000, // Class method has been checked to see if it is
                                            // overloaded function from the base class by
                                            // wxLuaBinding::InitAllBindings().
                                            // wxLuaBindMethod::basemethod is !NULL
                                            // if an overloaded function was found.

    WXLUAMETHOD_SEARCH_MASK = 0xFFFF,    // Helper for wxLuaBinding::GetClassMethod(), ignore WXLUAMETHOD_CHECKED_OVERLOAD
    WXLUAMETHOD_SORT_MASK   = 0xFFFF,    // Helper for wxLuaBinding::InitAllBindings(), ignore WXLUAMETHOD_CHECKED_OVERLOAD
};

// ----------------------------------------------------------------------------
// wxLuaBindCFunc - Defines a single function for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindCFunc
{
    lua_CFunction lua_cfunc;   // C function that implements the method or property
    int           method_type; // enum wxLuaMethod_Type flags for this function
    int           minargs;     // Min number of required args
    int           maxargs;     // Max number of args allowed
    wxLuaArgType* argtypes;    // Array of wxLua types representing each argument, NULL terminated.
};

// ----------------------------------------------------------------------------
// wxLuaBindMethod - Defines a method or property (a function) for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindMethod
{
    const char*      name;          // Name of the method or property
    int              method_type;   // enum wxLuaMethod_Type flags for this method.
                                    //   Note that each func has own type, this is ored values of them.
    wxLuaBindCFunc*  wxluacfuncs;   // Array of C functions for this method
    int              wxluacfuncs_n; // Number of C functions (overloaded > 1) for this method
    wxLuaBindMethod* basemethod;    // Overloaded method from the base class, else NULL.
                                    //   See comments for WXLUAMETHOD_CHECKED_OVERLOAD
};

// ----------------------------------------------------------------------------
// wxLuaBindNumber - Defines a numeric value for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindNumber
{
    const char* name;          // name
    double      value;         // numeric value
};

extern WXDLLIMPEXP_DATA_WXLUA(wxLuaBindNumber) g_wxluanumberArray_None[1]; // = {{0,0}}

// ----------------------------------------------------------------------------
// wxLuaBindString - Defines a wxWidgets wxChar* string for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindString
{
    const char*   name;        // name
    const wxChar* value;       // string value
};

// ----------------------------------------------------------------------------
// wxLuaBindEvent - Defines a wxWidgets wxEventType for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindEvent
{
    const char*        name;      // Name of the wxEventType, e.g. "wxEVT_COMMAND_MENU_SELECTED"
    const wxEventType* eventType; // wxWidgets event type, e.g. &wxEVT_COMMAND_MENU_SELECTED
    int*               wxluatype; // wxLua class type that wxWidgets uses for this wxEventType,
                                  //   e.g. &wxluatype_wxCommandEvent
};

// ----------------------------------------------------------------------------
// wxLuaBindEvent - Defines an object or pointer for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindObject
{
    const char*  name;      // Name of the object or pointer
    int*         wxluatype; // wxLua class type of the object or pointer.
    const void*  objPtr;    // Pointer to the object, e.g. &wxDefaultPosition
    const void** pObjPtr;   // Pointer to the object pointer, e.g. (const void **)&wxThePenList
};

// ----------------------------------------------------------------------------
// wxLuaBindClass - Defines a C++ class or struct for wxLua
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_WXLUA wxLuaBindClass
{
    const char*      name;           // Name of the class or struct
    wxLuaBindMethod* wxluamethods;   // Pointer to methods for this class
    int              wxluamethods_n; // Number of methods
    wxClassInfo*     classInfo;      // Pointer to the wxClassInfo associated with this class or NULL.
    int*             wxluatype;      // wxLua class type for userdata
    const char**     baseclassNames; // Names of base classes, or NULL if no base classes.
                                     // This is a NULL terminated array of strings.
    wxLuaBindClass** baseBindClasses;// Pointers to the base classes or NULL for no base classes.
                                     // This member is set after all the bindings are
                                     // registered since the base class may be from
                                     // a different module and so it can't be set at compile time.
                                     // This array is one shorter than the baseclassNames array
                                     // and you should use it rather than this to check the length
                                     // since any one of these may be NULL if the
                                     // library or module with the base class is not loaded.

    wxLuaBindNumber* enums;          // Class member enums or NULL if none
    int              enums_n;        // number of enums
};

// ----------------------------------------------------------------------------
// C functions for the metatable for wxLua userdata installed by the wxLuaBinding
// ----------------------------------------------------------------------------

// Generic delete function for userdata binding objects
WXDLLIMPEXP_WXLUA int LUACALL wxlua_userdata_delete(lua_State *L);

// memory deallocation function for created wxLuaBindClass defined objects, Lua's __gc metatable index
WXDLLIMPEXP_WXLUA int LUACALL wxlua_wxLuaBindClass__gc(lua_State *L);
// Lua 'set table' function for created wxLuaBindClass defined objects, Lua's __newindex metatable index
WXDLLIMPEXP_WXLUA int LUACALL wxlua_wxLuaBindClass__newindex(lua_State *L);
// Lua 'get table' function for created wxLuaBindClass defined objects, Lua's __index metatable index
WXDLLIMPEXP_WXLUA int LUACALL wxlua_wxLuaBindClass__index(lua_State *L);
// Lua 'tostring' function for created wxLuaBindClass defined objects, Lua's __tostring metatable index
WXDLLIMPEXP_WXLUA int LUACALL wxlua_wxLuaBindClass__tostring(lua_State *L);

// ----------------------------------------------------------------------------
// Overloaded binding function call helper functions.
// ----------------------------------------------------------------------------

// Redirect a Lua function call to 1 wxLuaBindCFunc from a list of overloaded functions.
// The 1st upvalue must be a wxLuaBindMethod.
int LUACALL wxlua_callOverloadedFunction(lua_State* L);
// Redirect a Lua function call to 1 wxLuaBindCFunc from a list of overloaded functions
// declared in the wxLuaBindMethod.
WXDLLIMPEXP_WXLUA int LUACALL wxlua_callOverloadedFunction(lua_State* L, struct wxLuaBindMethod* wxlMethod);
// Get a human readable string of the Lua args (items on the stack) for a function call
WXDLLIMPEXP_WXLUA wxString wxlua_getLuaArgsMsg(lua_State* L, int start_stack_idx, int end_stack_idx);
// Get a human readable wxString of the wxLuaArgType arrays for the functions in the method
WXDLLIMPEXP_WXLUA wxString wxlua_getBindMethodArgsMsg(lua_State* L, struct wxLuaBindMethod* wxlMethod);

// ----------------------------------------------------------------------------
// wxLuaObject - Wraps a reference to a Lua object reference inside a
//   wxObject-derived class so that a Lua object can be used for userdata.
// Also with a simple extension by a proxy member value it can be used
//   to provide pointers to the wxValidator classes.
// ----------------------------------------------------------------------------

enum wxLuaObject_Type
{
    wxLUAOBJECT_NONE     = 0, // nothing is allocated
    wxLUAOBJECT_BOOL     = 1, // bool allocated
    wxLUAOBJECT_INT      = 2, // int allocated
    wxLUAOBJECT_STRING   = 4, // wxString allocated
    wxLUAOBJECT_ARRAYINT = 8  // wxArrayInt allocated
};

class WXDLLIMPEXP_WXLUA wxLuaObject : public wxObject, wxClientData
{
public:
    // Wrap the item at the lua_State's stack index and create a reference to it
    // in the wxlua_lreg_refs_key registy table
    wxLuaObject(const wxLuaState& wxlState, int stack_idx);

    virtual ~wxLuaObject();

    // Get the value of the reference object and push it onto the stack.
    // (or a proxy if the object has been aliased for a wxValidator class.)
    // returns true if the object is valid and has a reference, returns false
    // on failure and nothing is pushed on the stack.
    bool GetObject();
    // Remove any existing reference and allocate another.
    // You cannot call this after calling GetXXXPtr(), but only if this wraps a
    // stack item.
    void SetObject(int stack_idx);

    // The following methods are used by the wxValidator interface
    // Call GetObject() so that it's on the stack then try to get the value of
    // the object as the specified type and set the member variable equal to it
    // and return a pointer to member variable to a function that wants
    // a pointer to read/write from/to.
    // You may only call only one of these per instance of a wxLuaObject class.
    bool       *GetBoolPtr();
    int        *GetIntPtr();
    wxString   *GetStringPtr();
    wxArrayInt *GetArrayPtr();

    // Return a flag value that indicated whether the
    // object is being used by a wxValidator class (else wxLUAOBJECT_NONE).
    wxLuaObject_Type GetAllocationFlag() const { return m_alloc_flag; }

    wxLuaState GetwxLuaState() const;

protected:
    wxLuaState* m_wxlState;   // a pointer due to #include recursion.
    int         m_reference;  // reference in wxlua_lreg_refs_key registry table

    wxLuaObject_Type m_alloc_flag; // type of object for wxValidator interface

    union                          // object stored for wxValidator interface
    {
        bool        m_bool;
        int         m_int;
        wxString*   m_string;
        wxArrayInt* m_arrayInt;
    };

private:
    DECLARE_ABSTRACT_CLASS(wxLuaObject)
};

// ----------------------------------------------------------------------------
// wxLUA_DECLARE_ENCAPSULATION and wxLUA_IMPLEMENT_ENCAPSULATION
//     Declare the macros used to define and implement classes that
//     wrap non-wxObject derived pointers used by wxLua in the bindings.
//
// IMPEXPSYMBOL : similiar to WXDLLIMPEXP_WXBIND, you cannot leave this
//   parameter empty, but you may use WXLUA_NO_DLLIMPEXP and
//   WXLUA_NO_DLLIMPEXP_DATA(x) if you don't want DLL export symbols.
// className : name of the class to encapsulate (may be NameSpace::MyClass)
// objName : name to use in naming the encapsulation class (NameSpace_MyClass)
// ----------------------------------------------------------------------------

#define WXLUA_NO_DLLIMPEXP           // use if you don't want to export class
#define WXLUA_NO_DLLIMPEXP_DATA(x) x // use if you don't want to export data

#define wxLUA_DECLARE_ENCAPSULATION(IMPEXPSYMBOL, className, objName) \
class IMPEXPSYMBOL wxLua_wxObject_##objName : public wxObject \
{ \
public: \
    wxLua_wxObject_##objName(className *p_##objName) : m_p##objName(p_##objName) {} \
    virtual ~wxLua_wxObject_##objName(); \
    className *m_p##objName; \
    DECLARE_ABSTRACT_CLASS(wxLua_wxObject_##objName) \
};

// we may not have fully defined the class/object in header so delete it in src.
#define wxLUA_IMPLEMENT_ENCAPSULATION(className, objName) \
IMPLEMENT_ABSTRACT_CLASS(wxLua_wxObject_##objName, wxObject) \
wxLua_wxObject_##objName::~wxLua_wxObject_##objName() \
{ \
    delete m_p##objName; \
}

#define wxGridCellWorkerDummyFriend wxGridCellWorkerDummyFriend; \
    public: \
    size_t GetRef() const { return m_nRef; }

#define wxGridCellAttrDummyFriend wxGridCellAttrDummyFriend; \
    public: \
    size_t GetRef() const { return m_nRef; }

#include "wx/grid.h"

// The wxGridWorker classes have protected destructors, use DecRef().
#define wxLUA_IMPLEMENT_wxGridCellWorker_ENCAPSULATION(className, objName) \
IMPLEMENT_ABSTRACT_CLASS(wxLua_wxObject_##objName, wxObject) \
wxLua_wxObject_##objName::~wxLua_wxObject_##objName() \
{ \
    m_p##objName->DecRef(); \
}

// ----------------------------------------------------------------------------
// wxLuaSmartStringArray - Wraps a "new" array of wxStrings with an automatic
//                         destructor that deletes them to make binding easier.
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_WXLUA wxLuaSmartStringArray
{
public:
    wxLuaSmartStringArray(wxString *strArr = NULL) : m_strArr(strArr) { }
    ~wxLuaSmartStringArray() { delete[] m_strArr; }

    void operator = (wxString *strArr) { m_strArr = strArr; }
    operator const wxString *() const { return m_strArr; }

private:
    wxString *m_strArr;
};

// ----------------------------------------------------------------------------
// wxLuaSmartIntArray - Wraps a "new" array of ints with an automatic
//                      destructor that deletes them to make binding easier.
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_WXLUA wxLuaSmartIntArray
{
public:
    wxLuaSmartIntArray(int* intArr = NULL) : m_intArr(intArr) { }
    ~wxLuaSmartIntArray() { delete[] m_intArr; }

    void operator = (int *intArr) { m_intArr = intArr; }
    operator int *() { return m_intArr; } // Note: not const for wxGLCanvas

private:
    int *m_intArr;
};

// ----------------------------------------------------------------------------
// wxLuaSmartwxArrayString - Wraps a "new" wxArrayString with an automatic
//                           destructor that deletes them to make binding easier.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_WXLUA wxLuaSmartwxArrayString : public wxObject
{
public:
    wxLuaSmartwxArrayString(const wxLuaSmartwxArrayString& arr) { Ref(arr); }
    wxLuaSmartwxArrayString(wxArrayString *arr, bool del);

    wxArrayString* GetArray() const;

    operator const wxArrayString *() const { return  GetArray(); }
    operator const wxArrayString &() const { return *GetArray(); }
    operator       wxArrayString &()       { return *GetArray(); }

    // You may have to cast the wxLuaSmartwxArrayString with (wxArrayString&)
    // e.g. wxLuaSmartwxArrayString arr; ((wxArrayString&)arr).Add(wxT("hello"));
    wxLuaSmartwxArrayString& operator = (const wxLuaSmartwxArrayString& arr)
    {
        Ref(arr);
        return *this;
    }
    wxLuaSmartwxArrayString& operator = (const wxArrayString& arr)
    {
        *GetArray() = arr;
        return *this;
    }
};

extern const WXDLLIMPEXP_DATA_WXLUA(wxLuaSmartwxArrayString) wxLuaNullSmartwxArrayString;

// ----------------------------------------------------------------------------
// wxLuaSmartwxSortedArrayString - Wraps a "new" wxSortedArrayString with an automatic
//                                 destructor that deletes them to make binding easier.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_WXLUA wxLuaSmartwxSortedArrayString : public wxObject
{
public:
    wxLuaSmartwxSortedArrayString(const wxLuaSmartwxArrayString& arr) { Ref(arr); }
    wxLuaSmartwxSortedArrayString(wxSortedArrayString *arr, bool del);

    wxSortedArrayString* GetArray() const;

    operator const wxSortedArrayString *() const { return  GetArray(); }
    operator const wxSortedArrayString &() const { return *GetArray(); }
    operator       wxSortedArrayString &()       { return *GetArray(); }

    // You may have to cast the wxLuaSmartwxSortedArrayString with (wxSortedArrayString&)
    // e.g. wxLuaSmartwxSortedArrayString arr; ((wxSortedArrayString&)arr).Add(wxT("hello"));
    wxLuaSmartwxSortedArrayString& operator = (const wxLuaSmartwxSortedArrayString& arr)
    {
        Ref(arr);
        return *this;
    }
};

// ----------------------------------------------------------------------------
// wxLuaSmartwxArrayInt - Wraps a "new" wxArrayInt with an automatic
//                        destructor to delete them to make binding easier
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_WXLUA wxLuaSmartwxArrayInt : public wxObject
{
public:
    wxLuaSmartwxArrayInt(const wxLuaSmartwxArrayInt& arr) { Ref(arr); }
    wxLuaSmartwxArrayInt(wxArrayInt *arr = NULL, bool del = true);

    wxArrayInt* GetArray() const;

    operator const wxArrayInt *() const { return  GetArray(); }
    operator const wxArrayInt &() const { return *GetArray(); }
    operator       wxArrayInt &()       { return *GetArray(); }

    // You may have to cast the wxLuaSmartwxArrayInt with (wxArrayInt&)
    // e.g. wxLuaSmartwxArrayInt arr; ((wxArrayInt&)arr).Add(5);
    wxLuaSmartwxArrayInt& operator = (const wxLuaSmartwxArrayInt& arr)
    {
        Ref(arr);
        return *this;
    }
};

// ----------------------------------------------------------------------------
// wxLuaBinding - binds classes, functions, objects, and event callbacks to
//                the wxLuaState.
// ----------------------------------------------------------------------------

// list of wxLua Bindings
WX_DECLARE_USER_EXPORTED_LIST(wxLuaBinding, wxLuaBindingList, WXDLLIMPEXP_WXLUA);

class WXDLLIMPEXP_WXLUA wxLuaBinding : public wxObject
{
public:
    wxLuaBinding();
    virtual ~wxLuaBinding() {}

    // Binds C Functions/Defines/Object/Events to a Lua table with binding's namespace.
    // The Lua table that the bindings were installed into is left on the top
    //   of the stack and you must pop it when done.
    virtual bool RegisterBinding(const wxLuaState& wxlState);

    // Create the metatable for the class and install it into the Lua registry.
    static bool InstallClassMetatable(lua_State* L, const wxLuaBindClass* wxlClass);
    // Install a single wxLuaBindClass struct into the table at the top
    // of the stack.
    static bool InstallClass(lua_State* L, const wxLuaBindClass* wxlClass);

    // -----------------------------------------------------------------------

    // Get/Set the binding name, a unique name of the binding.
    //   By default it is the "hook_cpp_namespace" used in the binding
    //   generator which needs to be a unique name.
    wxString GetBindingName() const { return m_bindingName; }
    void SetBindingName(const wxString& name) { m_bindingName = name; }

    // Get/Set the namespace table in Lua that this binding will be installed to
    wxString GetLuaNamespace() const { return m_nameSpace; }
    void SetLuaNamespace(const wxString& nameSpace) { m_nameSpace = nameSpace; }

    size_t GetClassCount() const        { return m_classCount; }
    wxLuaBindClass* GetClassArray()     { return m_classArray; }

    size_t GetNumberCount() const       { return m_numberCount; }
    wxLuaBindNumber* GetNumberArray()   { return m_numberArray; }

    size_t GetStringCount() const       { return m_stringCount; }
    wxLuaBindString* GetStringArray()   { return m_stringArray; }

    size_t GetEventCount() const        { return m_eventCount; }
    wxLuaBindEvent* GetEventArray()     { return m_eventArray; }

    size_t GetObjectCount() const       { return m_objectCount; }
    wxLuaBindObject* GetObjectArray()   { return m_objectArray; }

    size_t GetFunctionCount() const     { return m_functionCount; }
    wxLuaBindMethod* GetFunctionArray() { return m_functionArray; }

    // Is this wxLua type defined in this binding?
    bool HaswxLuaType(int wxl_type) const { return (wxl_type >= m_first_wxluatype) && (wxl_type <= m_last_wxluatype); }

    // -----------------------------------------------------------------------
    // These functions only look through the binding data of this wxLuaBinding

    // Find the wxLuaBindEvent with the wxEventType, returns NULL if not found.
    const wxLuaBindEvent* GetBindEvent(wxEventType eventType) const;
    // Look up the wxEventType name as a string, from the wxEventType number
    //  in the wxLuaBindEvent* struct list of this binding.
    wxString GetEventTypeName(wxEventType eventType) const;
    // Get the wxLuaBindClass that has this wxLua type, or NULL if none
    const wxLuaBindClass* GetBindClass(int wxl_type) const;
    // Get the wxLuaBindClass that has this name, or NULL if none
    const wxLuaBindClass* GetBindClass(const char* className) const;
    // Get the first wxLuaBindClass that has this wxLuaBindMethod
    const wxLuaBindClass* GetBindClass(const wxLuaBindMethod* wxlMethod) const;
    // Get the first wxLuaBindClass that has this wxLuaBindCFunc
    const wxLuaBindClass* GetBindClass(const wxLuaBindCFunc* wxlCFunc) const;

    // -----------------------------------------------------------------------
    // These functions search through the static wxLuaBinding::GetBindingList()
    // for the items.

    // Get the installed wxLuaBinding with the given
    //   wxLuaBinding::GetBindingName() or NULL for no match.
    static wxLuaBinding* GetLuaBinding(const wxString& bindingName);

    // Get wxLuaBindClass for given wxLua type using wxLuaBindClass::wxluatype,
    //   returns NULL on failure.
    static const wxLuaBindClass* FindBindClass(int wxl_type);
    // Get wxLuaBindClass that has this class name using wxLuaBindClass::name,
    //   returns NULL on failure.
    static const wxLuaBindClass* FindBindClass(const char* className);
    // Get the first wxLuaBindClass that has this particular wxLuaBindMethod
    //   returns NULL on failure.
    static const wxLuaBindClass* FindBindClass(const wxLuaBindMethod* wxlMethod);
    // Get the first wxLuaBindClass that has this particular wxLuaBindCFunc in its methods
    //   returns NULL on failure.
    static const wxLuaBindClass* FindBindClass(const wxLuaBindCFunc* wxlCFunc);

    // Get wxLuaBindEvent for given wxEventType (wxEvent::GetEventType()) by finding
    //   the matching wxLuaBindEvent::eventType.
    //   returns NULL on failure.
    static const wxLuaBindEvent* FindBindEvent(wxEventType eventType);

    // Get the wxLuaBinding that has this wxLuaBindMethod in its wxLuaBinding::GetFunctionArray().
    //   returns NULL on failure.
    static wxLuaBinding* FindMethodBinding(const wxLuaBindMethod* wxlMethod);

    // -----------------------------------------------------------------------
    // These functions search through the input struct

    // Lookup a wxLuaBindMethod function or property called methodName in the wxLuaBindClass
    // that is also of the wxLuaMethod_Type method_type. The method_type may be
    // ored values and the first match found is returned or NULL if not found.
    // If the wxLuaBindMethod cannot be found on the current class recurse through base classes
    //   if search_baseclasses.
    static wxLuaBindMethod* GetClassMethod(const wxLuaBindClass *wxlClass, const char *methodName,
                                           int method_type, bool search_baseclasses);

    // -----------------------------------------------------------------------

    // Get all the bindings that were initialized using the generated binding
    //   function wxLuaBinding_[binding name]_init().
    //   You can adjust the list *only* if you do not have any wxLuaStates
    //   created, otherwise the wxLua types will be out of sync.
    static wxLuaBindingList* GetBindingList() { return &sm_bindingList; }

    // -----------------------------------------------------------------------

    // Initialize all of the bindings by iterating the GetBindingList() and
    // setting the base classes and base class functions. This function
    // is automatically run by the wxLuaState and should not need to be called.
    static void InitAllBindings(bool force_update = false);

protected:
    void InitBinding(); // must called after subclassed version is created
                        // to sort the bindings appropriately

    // Register the classes, defines, strings, events, objects, and functions
    // stored in the binding arrays. The Lua table to install them into
    // must be at the top of the stack.
    virtual void DoRegisterBinding(const wxLuaState& wxlState) const;

    // binding objects
    size_t           m_classCount;
    wxLuaBindClass*  m_classArray;
    size_t           m_numberCount;
    wxLuaBindNumber* m_numberArray;
    size_t           m_stringCount;
    wxLuaBindString* m_stringArray;
    size_t           m_eventCount;
    wxLuaBindEvent*  m_eventArray;
    size_t           m_objectCount;
    wxLuaBindObject* m_objectArray;
    size_t           m_functionCount;
    wxLuaBindMethod* m_functionArray;

    wxString m_bindingName;     // A unique name of the binding
    wxString m_nameSpace;       // Lua table namespace e.g. "wx"
    int m_first_wxluatype;      // The first wxLua type allocated for a class
    int m_last_wxluatype;       // The last wxLua type of registered classes

    static wxLuaBindingList sm_bindingList;
    static bool             sm_bindingList_initialized;
    static int              sm_wxluatype_max;

    DECLARE_ABSTRACT_CLASS(wxLuaBinding)
};

#endif // _WXLBIND_H_
