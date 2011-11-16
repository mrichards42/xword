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

#ifndef CONFIG_MGR_H
#define CONFIG_MGR_H

#include <wx/config.h>
#include <list>
#include <map>
#include <wx/event.h> // wxEvtHandler, wxWindowDestroyEvent



class ConfigManagerBase;

// A single group in a path.
// e.g. in the path "/LastRun/CalculatedValues/MaxValue"
//    Each value separated by a "/" gets a group
//    ( "", "LastRun", "CalculatedValues", "MaxValue" )
// The "" is the base group, called m_group in ConfigManagerBase.
// The last entry, "MaxValue" should be of type ConfigValue, and is
//    responsible for getting and setting the config value.
class ConfigGroup
{
public:
    ConfigGroup(ConfigManagerBase * cfg)
        : m_name(_T("")),
          m_cfg(cfg)
    {
    }

    ConfigGroup(ConfigGroup * parent, const wxString & name)
        : m_name(parent->m_name + _T("/") + name),
          m_cfg(parent->m_cfg)
    {
        if (parent)
            parent->m_children.push_back(this);
    }

    virtual void Update()
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
            (*it)->Update();
    }

    virtual void RemoveCallbacks(wxWindow * win)
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
            (*it)->RemoveCallbacks(win);
    }

    wxString m_name;
    std::list<ConfigGroup *> m_children;
    ConfigManagerBase * m_cfg;
};


/*  ***************************************************************************
    The ConfigManager base class.
    ***************************************************************************

    To use ConfigManager, first call SetConfig() with a wxConfigBase pointer.

    ConfigManagerBase can be used as-is, by using the Get and Set functions:
        T Get(wxString path, T default);
        void Set(wxString path, T value);

    However, the power in this class can be fully taken advantage of if
    nested classes derived from ConfigGroup are used to create a hierarchy:

    class ConfigManager : public ConfigManagerBase
    {
    public:
        ConfigManager()
            : MyGroup(&m_group) // Pass the pointer to our bas ConfigGroup
        {}

        class MyGroup : public ConfigGroup
        {
        public:
            MyGroup(ConfigGroup * parent)
                : ConfigGroup(parent),
                  // ConfigValue(this, path, default_value)
                  MyString(this, _T("StringPath"), _T("Default string")),
                  MyLong(this, _T("LongPath"), 15),
                  MyBool(this, _T("BoolPath"), false),
                  MyDouble(this, _T("DoublePath"), 1.2),
                  MyFont(this, _T("FontPath"), *wxSWISS_FONT)
            {}

            ConfigString MyString;
            ConfigLong MyLong;
            ConfigBool MyBool;
            ConfigDouble MyDouble;
            ConfigFont MyFont;
        } Group;
    };

    This class could be used as follows:
        ConfigManager config;
        wxString val = config.Group.MyString.Get();
        config.Group.MyString.Set(_T("New Value"));

    Alternatively, operators () and = can be used for Get and Set, respectively.
        long val = config.Group.MyLong();
        config.Group.MyLong = 100000L;

    wxWindow-derived classes can attach callbacks that will be called when
    a value is Set().
        MyFrame * frame;
        config.Group.MyBool.AddCallback(frame, &MyFrame::Maximize);
        config.Group.MyBool.Set(true); // This calls MyFrame::Maximize

    To disable this behavior, call AutoUpdate(false):
        config.AutoUpdate(false);
        config.Group.MyBool.Set(false); // This does not call MyFrame::Maximize

    To force the callbacks to be called use Update()
        config.Update(); // Calls all callbacks in the config hierarchy.
*/

class ConfigManagerBase
{
public:
    ConfigManagerBase()
        : m_group(this),
          m_config(NULL),
          m_autoUpdate(true)
    {
    }

    void SetConfig(wxConfigBase * cfg) { m_config = cfg; }
    void AutoUpdate(bool doit) { m_autoUpdate = doit; }
    bool AutoUpdate() const { return m_autoUpdate; }

protected:
    // A little metaprogramming :)

    // For types natively supported by wxConfigBase (wxString, bool,
    // long, double), this works transparently.

    // To define a type that must be adapted to one of the native types,
    // do the following (examples for wxFont and wxColour are at the bottom
    // of this document):

    //  (1) Define an explicitly specialized struct AdaptedType<T> for your type
    //      that defines a typedef for the basic type:
    //          template <> struct AdaptedType<T> { typedef NATIVE_T type; }
    template <typename T> struct AdaptedType { typedef T type; };

    //  (2) Define two Convert<IN_T, OUT_T> functions to convert your type to
    //      the native type, and the native type to your type:
    //
    //          template <> NATIVE_T Convert<T, NATIVE_T>(const T & )
    //              { return your type converted to the native type; }
    //
    //          template <> T Convert<NATIVE_T, T>(const NATIVE_T & )
    //              { return the native type converted to your type; }

    template <typename IN_T, typename OUT_T>
    inline OUT_T Convert(const IN_T & val) { return val; }

public:
    // Get and Set template functions.
    // These functions use TMP described above to deduce the return type and
    // the proper conversion (if any) using the value type to the function.

    template <typename T>
    inline T Get(const wxString & path, const T & default_val)
    {
        typename AdaptedType<T>::type ret;
        if (m_config->Read(path, &ret))
            return Convert<typename AdaptedType<T>::type, T>(ret);
        return default_val;
    }

    template <typename T>
    inline void Set(const wxString & path, const T & val)
    {
        m_config->Write(path, Convert<T, typename AdaptedType<T>::type>(val));
    }

    inline void Update() { m_group.Update(); }
    inline void RemoveCallbacks(wxWindow * win) { m_group.RemoveCallbacks(win); }

protected:
    wxConfigBase * m_config;
    bool m_autoUpdate;
    ConfigGroup m_group;
};


/*  ***************************************************************************
    The ConfigValue template class.
    ***************************************************************************

    Usage is described in ConfigManagerBase.
    
*/
template <typename T>
class ConfigValue
    : public wxEvtHandler, // Needed for wxWindowDestroyEvent
      public ConfigGroup
{
    // Callback class
    class CallbackBase
    {
    public:
        virtual ~CallbackBase() {}
        inline virtual void Call(T) =0;
    };

    template<typename OBJ, typename FUNC>
    class Callback : public CallbackBase
    {
    public:
        Callback(OBJ * obj, FUNC func) : m_obj(obj), m_func(func) {}
        inline virtual void Call(T val) { (m_obj->*m_func)(val); }
		OBJ * m_obj;
        FUNC m_func;
    };

public:
    ConfigValue(ConfigGroup * parent,
                const wxString & name,
                T default_val)
        : wxEvtHandler(),
          ConfigGroup(parent, name),
          m_default(default_val)
    {
    }

    ~ConfigValue()
    {
        // Callback windows should remove themselves.
        wxASSERT(m_callbacks.empty());
        typename map_t::iterator it;
        for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
            delete it->second;
    }

    // Get and set functions
    inline T Get() { return GetConfig()->Get(m_name, m_default); }

    inline void Set(const T & val)
    {
        ConfigManagerBase * cfg = GetConfig();
        cfg->Set<T>(m_name, val);
        if (cfg->AutoUpdate())
            Update(val);
    }

    // operators that substitute for Get and Set
    inline T operator()() { return Get(); }
    inline void operator=(const T & val) { Set(val); }


    // Callback functions
    template<typename OBJ, typename FUNC>
    void AddCallback(OBJ * obj, FUNC func)
    {
        // Add a callback to the callbacks multimap.
        wxWindow * win = static_cast<wxWindow*>(obj);
        m_callbacks.insert(pair_t(win, new Callback<OBJ, FUNC>(obj, func)));
    }

    void RemoveCallbacks(wxWindow * win)
    {
        // Delete the callbacks and erase the map entries for this window.
        std::pair<typename map_t::iterator, typename map_t::iterator> range =
            m_callbacks.equal_range(win);
        for (typename map_t::iterator it = range.first; it != range.second; ++it)
            delete it->second;
        m_callbacks.erase(range.first, range.second);
    }

    // Run the callbacks for this value
    inline void Update() { Update(Get()); }
    inline void Update(T val)
    {
        typename map_t::iterator it;
        for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
            it->second->Call(val);
    }

protected:
    ConfigManagerBase * GetConfig() { return m_cfg; }
    T m_default;
    typedef std::multimap<wxWindow *, CallbackBase*> map_t;
    typedef std::pair<wxWindow *, CallbackBase*> pair_t;
    map_t m_callbacks;
};

// -----------------------------------------------------------------------
// Basic (unconverted) types
// -----------------------------------------------------------------------
typedef ConfigValue<wxString> ConfigString;
typedef ConfigValue<bool>     ConfigBool;
typedef ConfigValue<long>     ConfigLong;
typedef ConfigValue<double>   ConfigDouble;

// -----------------------------------------------------------------------
// wxFont adapted type
// -----------------------------------------------------------------------
#include <wx/font.h>

// Tell ConfigManagerBase that this should be converted to a wxString
template<>
struct ConfigManagerBase::AdaptedType<wxFont> { typedef wxString type; };

// Font to String conversion
template <>
inline wxString
ConfigManagerBase::Convert<wxFont, wxString>(const wxFont & font)
{
    return font.GetNativeFontInfoUserDesc();
}

// String to Font conversion
template <>
inline wxFont
ConfigManagerBase::Convert<wxString, wxFont>(const wxString & str)
{
    wxFont font;
    font.SetNativeFontInfoUserDesc(str);
    return font;
}

// The ConfigValue typedef
typedef ConfigValue<wxFont> ConfigFont;


// -----------------------------------------------------------------------
// wxColour adapted type
// -----------------------------------------------------------------------
#include <wx/colour.h>

// The ConfigValue typedef
typedef ConfigValue<wxColour> ConfigColor;

// Tell ConfigManagerBase that this should be converted to a wxString
template<>
struct ConfigManagerBase::AdaptedType<wxColour> { typedef wxString type; };

// Color to String conversion
template <>
inline wxString
ConfigManagerBase::Convert<wxColour, wxString>(const wxColour & color)
{
    return color.GetAsString();
}

// String to Color conversion
template <>
inline wxColour
ConfigManagerBase::Convert<wxString, wxColour>(const wxString & str)
{
    return wxColour(str);
}

// -----------------------------------------------------------------------
// wxPoint adapted type
// -----------------------------------------------------------------------
#include <wx/gdicmn.h>
#include <wx/tokenzr.h>
// The ConfigValue typedef
typedef ConfigValue<wxPoint> ConfigPoint;

// Tell ConfigManagerBase that this should be converted to a wxString
template<>
struct ConfigManagerBase::AdaptedType<wxPoint> { typedef wxString type; };

// Point to String conversion
template <>
inline wxString
ConfigManagerBase::Convert<wxPoint, wxString>(const wxPoint & pt)
{
    return wxString::Format(_T("%d, %d"), pt.x, pt.y);
}

// String to Point conversion
template <>
inline wxPoint
ConfigManagerBase::Convert<wxString, wxPoint>(const wxString & str)
{
    wxArrayString tokens = wxStringTokenize(str, _T(", "), wxTOKEN_STRTOK);
    long x, y;
    if (tokens.size() != 2 ||
        ! (tokens[0].ToLong(&x) && tokens[1].ToLong(&y)) )
    {
        return wxPoint();
    }

    return wxPoint(x, y);
}

#endif // CONFIG_MGR_H
