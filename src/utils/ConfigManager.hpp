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
#include <wx/memconf.h>
#include <list>
#include <map>
#include <set>
#include <algorithm>
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
    friend class ConfigManagerBase;
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

    virtual ~ConfigGroup() {}

    virtual void Update(wxEvtHandler * h = NULL)
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
            (*it)->Update(h);
    }

    virtual void RemoveCallbacks(wxEvtHandler * h)
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
            (*it)->RemoveCallbacks(h);
    }

    // Copy values from other config to this config
    virtual void Copy(const ConfigGroup & other);

    ConfigGroup * FindChild(const wxString & name)
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
        {
            if ((*it)->m_name == name)
                return *it;
        }
        return NULL;
    }

    wxString m_name;
    std::list<ConfigGroup *> m_children;
    ConfigManagerBase * m_cfg;

protected:
    // Update our ConfigLists
    virtual void UpdateLists()
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
            (*it)->UpdateLists();
    }

private: // Can't assign or copy
    ConfigGroup(ConfigGroup &);
    ConfigGroup & operator=(const ConfigGroup &);
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
            : MyGroup(&m_group) // Pass the pointer to our base ConfigGroup
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

    wxEvtHandler-derived classes can attach callbacks that will be called when
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

static const bool CONFIG_DUMMY = true;

class ConfigManagerBase
{
public:
    ConfigManagerBase(bool useDummy = false)
        : m_group(this),
          m_config(NULL),
          m_autoUpdate(true),
          m_isDummy(useDummy)
    {
        if (useDummy)
            m_config = new wxMemoryConfig;
    }

    virtual ~ConfigManagerBase()
    {
        if (m_isDummy)
            delete GetConfig();
    }

    void SetConfig(wxConfigBase * cfg)
    {
        wxASSERT_MSG(! m_isDummy, "ConfigManager already has a dummy wxMemory Config");
        m_config = cfg;
        // If we have ConfigLists, update their values.
        m_group.UpdateLists();
    }
    wxConfigBase * GetConfig() { return m_config; }
    const wxConfigBase * GetConfig() const { return m_config; }
    void AutoUpdate(bool doit) { m_autoUpdate = doit; }
    bool AutoUpdate() const { return m_autoUpdate; }
    const ConfigGroup & GetGroup() const { return m_group; }

    class ConversionError : public std::exception {};
    class CopyError : public std::exception {};

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
    inline OUT_T Convert(const IN_T & val) const { return val; }

public:
    // Get and Set template functions.
    // These functions use TMP described above to deduce the return type and
    // the proper conversion (if any) using the value type to the function.

    template <typename T>
    inline T Get(const wxString & path, const T & default_val) const
    {
        typename AdaptedType<T>::type ret;
        if (m_config->Read(path, &ret))
        {
            try {
                return Convert<typename AdaptedType<T>::type, T>(ret);
            } catch (ConversionError &) {
                // default
            }
        }
        return default_val;
    }

    template <typename T>
    inline void Set(const wxString & path, const T & val)
    {
        m_config->Write(path, Convert<T, typename AdaptedType<T>::type>(val));
    }

    inline void Update(wxEvtHandler * h = NULL) { m_group.Update(h); }
    inline void RemoveCallbacks(wxEvtHandler * h) { m_group.RemoveCallbacks(h); }

    inline void Copy(const ConfigManagerBase & other)
    {
        m_group.Copy(other.m_group);
    }

protected:
    wxConfigBase * m_config;
    bool m_autoUpdate;
    ConfigGroup m_group;
    bool m_isDummy;
};



// ConfigGroup functions (that use ConfigManagerBase)

inline void ConfigGroup::Copy(const ConfigGroup & other)
{
    if (m_name != other.m_name ||
        m_children.size() != other.m_children.size())
    {
        throw ConfigManagerBase::CopyError();
    }
    // Iterate all the groups within and copy the values over
    std::list<ConfigGroup *>::iterator group;
    std::list<ConfigGroup *>::const_iterator otherGroup;
    group = m_children.begin();
    otherGroup = other.m_children.begin();
    for (;
        group != m_children.end();
        ++group, ++otherGroup)
    {
        (*group)->Copy(**otherGroup);
    }
}

template <typename T>
class ConfigList : public ConfigGroup
{
public:
    ConfigList(ConfigManagerBase * cfg)
    : ConfigGroup(cfg)
    {}
    
    ConfigList(ConfigGroup * parent, const wxString & name)
    : ConfigGroup(parent, name)
    {}
    
    virtual ~ConfigList()
    {
        // Delete the pointers from m_children
        clear(false);
    }
    
public:
    // Copy values from other config to this config
    virtual void Copy(const ConfigGroup & other)
    {
        if (m_name != other.m_name)
            throw ConfigManagerBase::CopyError();
        
        // This is a bit of a convoluted process, since we need to avoid
        // unnecessary deletion of the pointers in m_children (in case someone
        // added a callback).
        
        // Keep track of which entries we have copied
        std::set<ConfigGroup *> entries(m_children.begin(), m_children.end());
        // Copy the entires
        std::list<ConfigGroup *>::const_iterator it;
        for (it = other.m_children.begin(); it != other.m_children.end(); ++it)
        {
            // Find the old entry
            std::list<ConfigGroup *>::iterator old;
            for (old = m_children.begin(); old != m_children.end(); ++old)
            {
                if ((*old)->m_name == (*it)->m_name)
                {
                    entries.erase(*old);
                    (*old)->Copy(**it);
                    break;
                }
            }
            if (old == m_children.end()) // Not found
            {
                // Create a new entry
                // Get the name of this entry by removing the prefix of the
                // parent's name.
                wxString name;
                (*it)->m_name.StartsWith(other.m_name + _T("/"), &name);
                T * entry = push_back(name);
                // Fill it with values from the other config
                entry->Copy(**it);
            }
        }
        // Remove entries that were not copied
        std::set<ConfigGroup *>::iterator old;
        for (old = entries.begin(); old != entries.end(); ++old)
        {
            std::list<ConfigGroup *>::iterator item =
            std::find(m_children.begin(), m_children.end(), *old);
            if (item != m_children.end())
                m_children.remove(*item);
        }
    }
    
protected:
    // An iterator that automatically casts its pointers
    template <class ITERATOR>
    class iterator_t : public ITERATOR
    {
    public:
        iterator_t() : ITERATOR() {}
        iterator_t(ITERATOR & it) : ITERATOR(it) {}
        iterator_t(const ITERATOR & it) : ITERATOR(it) {}
        
        T & operator *() { return *cast(ITERATOR::operator *()); }
        const T & operator *() const { return *cast(ITERATOR::operator *()); }
        
        T * operator->() { return cast(*ITERATOR::operator->()); }
        const T * operator->() const { return cast(*ITERATOR::operator->()); }
        
    protected:
        T * cast(ConfigGroup * group) { return dynamic_cast<T *>(group); }
        const T * cast(ConfigGroup * group) const { return dynamic_cast<const T *>(group); }
    };
    
public:
    typedef iterator_t<std::list<ConfigGroup *>::iterator> iterator;
    typedef iterator_t<std::list<ConfigGroup *>::const_iterator> const_iterator;
    
    iterator begin() { return iterator(m_children.begin()); }
    const_iterator begin() const { return const_iterator(m_children.begin()); }
    iterator end() { return iterator(m_children.end()); }
    const_iterator end() const { return const_iterator(m_children.end()); }
    
    T * push_back(const wxString & name)
    {
        // This constructor automatically adds it to m_children
        return new T(this, name);
    }
    
    bool remove(const wxString & name)
    {
        m_cfg->GetConfig()->DeleteEntry(name);
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
        {
            if ((*it)->m_name == name)
            {
                ConfigGroup * group = *it;
                m_children.erase(it);
                delete group;
                return true;
            }
        }
        return false;
    }
    
    void clear(bool delete_entries = true)
    {
        std::list<ConfigGroup *>::iterator it;
        for (it = m_children.begin(); it != m_children.end(); ++it)
        {
            // Remove from the config
            if (delete_entries)
                m_cfg->GetConfig()->DeleteEntry((*it)->m_name);
            // We own the pointers in m_children
            delete *it;
        }
        m_children.clear();
    }
    
    bool empty() const { return m_children.empty(); }
    
protected:
    // Update m_children to reflect the current config state
    virtual void UpdateLists()
    {
        clear(false);
        wxConfigBase * cfg = m_cfg->GetConfig();
        // Enumeration variables
        long index;
        wxString name;
        cfg->SetPath(m_name);
        if (cfg->GetFirstGroup(name, index))
        {
            do
            {
                push_back(name);
            }
            while (cfg->GetNextGroup(name, index));
        }
        cfg->SetPath(_T("/"));
    }
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
    inline T Get() const { return GetConfig()->Get(m_name, m_default); }

    inline void Set(const T & val)
    {
        ConfigManagerBase * cfg = GetConfig();
        const bool different = cfg->SetIfChanged<T>(m_name, val);
        if (different && cfg->AutoUpdate())
            Update(val);
    }

    // operators that substitute for Get and Set
    inline T operator()() const { return Get(); }
    inline void operator=(const T & val) { Set(val); }


    // Callback functions
    template<typename OBJ, typename FUNC>
    void AddCallback(OBJ * obj, FUNC func)
    {
        // Add a callback to the callbacks multimap.
        wxEvtHandler * h = static_cast<wxEvtHandler*>(obj);
        m_callbacks.insert(pair_t(h, new Callback<OBJ, FUNC>(obj, func)));
    }

    void RemoveCallbacks(wxEvtHandler * handler)
    {
        // Delete the callbacks and erase the map entries for this window.
        std::pair<typename map_t::iterator, typename map_t::iterator> range =
            m_callbacks.equal_range(handler);
        for (typename map_t::iterator it = range.first; it != range.second; ++it)
            delete it->second;
        m_callbacks.erase(range.first, range.second);
    }

    // Run the callbacks for this value
    inline void Update(wxEvtHandler * h = NULL) { Update(Get(), h); }
    inline void Update(T val, wxEvtHandler * h = NULL)
    {
        typename map_t::iterator it;
        for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
        {
            if (h == NULL || it->first == h)
                it->second->Call(val);
        }
    }

    // Copy values from other to this
    inline void Copy(const ConfigGroup & other_)
    {
        const ConfigValue<T> * other = dynamic_cast<const ConfigValue<T> *>(&other_);
        if (! other || m_name != other->m_name)
            throw ConfigManagerBase::CopyError();
        Set(other->Get());
    }

protected:
    ConfigManagerBase * GetConfig() { return m_cfg; }
    const ConfigManagerBase * GetConfig() const { return m_cfg; }
    T m_default;
    typedef std::multimap<wxEvtHandler *, CallbackBase*> map_t;
    typedef std::pair<wxEvtHandler *, CallbackBase*> pair_t;
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
ConfigManagerBase::Convert<wxFont, wxString>(const wxFont & font) const
{
    return font.GetNativeFontInfoDesc();
}

// String to Font conversion
template <>
inline wxFont
ConfigManagerBase::Convert<wxString, wxFont>(const wxString & str) const
{
    wxFont font;
    // We user to user NativeFontInfoUserDesc instead of NativeFontInfoDesc,
    // So try both here.
    if (! font.SetNativeFontInfo(str))
        if (! font.SetNativeFontInfoUserDesc(str))
            throw ConfigManagerBase::ConversionError();
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
ConfigManagerBase::Convert<wxColour, wxString>(const wxColour & color) const
{
    return color.GetAsString();
}

// String to Color conversion
template <>
inline wxColour
ConfigManagerBase::Convert<wxString, wxColour>(const wxString & str) const
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
ConfigManagerBase::Convert<wxPoint, wxString>(const wxPoint & pt) const
{
    return wxString::Format(_T("%d, %d"), pt.x, pt.y);
}

// String to Point conversion
template <>
inline wxPoint
ConfigManagerBase::Convert<wxString, wxPoint>(const wxString & str) const
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
