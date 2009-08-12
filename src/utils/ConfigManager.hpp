// This file is part of XWord
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
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
#include <map>



// A class for representing default values.  Note that everything is
// accessed and set by const reference so that macros can be used
// to define the real classes later
class ConfigDefault
{
public:
    ConfigDefault() {}
    virtual ~ConfigDefault() {}
    virtual void SetData(const wxString & WXUNUSED(data))
        { wxFAIL_MSG(_T("Not Implemented")); }
    virtual void SetData(const bool & WXUNUSED(data))
        { wxFAIL_MSG(_T("Not Implemented")); }
    virtual void SetData(const double & WXUNUSED(data))
        { wxFAIL_MSG(_T("Not Implemented")); }
    virtual void SetData(const long & WXUNUSED(data))
        { wxFAIL_MSG(_T("Not Implemented")); }

    virtual const wxString & GetString() const
        { wxFAIL_MSG(_T("Not Implemented")); return wxEmptyString; }
    virtual const bool & GetBool() const
        { wxFAIL_MSG(_T("Not Implemented")); return false; }
    virtual const double & GetDouble() const
        { wxFAIL_MSG(_T("Not Implemented")); return 0; }
    virtual const long & GetLong() const
        { wxFAIL_MSG(_T("Not Implemented")); return 0; }
};


// Macros for adding, reading, writing, and getting the default
// for config items.

// Every type should have the following functions, where TYPE is
// replaced by the type name:
//     void Add      (const wxString & path, const TYPE & default);
//     void AddTYPE  (const wxString & path, const TYPE & default);
//     TYPE ReadTYPE (const wxString & path);
//     bool Write    (const wxString & path, const TYPE & value);
//     bool WriteTYPE(const wxString & path, const TYPE & value);
//     void GetDefaultTYPE(const wxString & path) const;

#define CFG_TYPE(type, type_name)                                              \
    /* The ConfigXXXX class */                                                 \
    class Config ## type_name : public ConfigDefault                           \
    {                                                                          \
    public:                                                                    \
        Config ## type_name(const type & data) { SetData(data); }              \
                                                                               \
        virtual void SetData(const type & data) { m_data = data; }             \
        virtual const type & Get ## type_name() const { return m_data; }       \
                                                                               \
    protected:                                                                 \
        type m_data;                                                           \
    };                                                                         \
                                                                               \
    /* Add functions */                                                        \
    void Add(const wxString & path, const type & default_value)                \
    {                                                                          \
        Add ## type_name(path, default_value);                                 \
    }                                                                          \
    void Add ## type_name(const wxString & path, const type & default_value)   \
    {                                                                          \
        m_items[GetPathOf(path)] = new Config ## type_name(default_value);     \
    }                                                                          \
                                                                               \
    /* Read function */                                                        \
    type Read ## type_name(const wxString & path)                              \
    {                                                                          \
        wxASSERT(m_config != NULL);                                            \
        type ret;                                                              \
        if (m_config->Read(GetPathOf(path), &ret))                             \
            return ret;                                                        \
        return GetDefault ## type_name(path);                                  \
    }                                                                          \
                                                                               \
    /* Write functions */                                                      \
    bool Write(const wxString & path, const type & value)                      \
    {                                                                          \
        return Write ## type_name(path, value);                                \
    }                                                                          \
    bool Write ## type_name(const wxString & path, const type & value)         \
    {                                                                          \
        wxASSERT(m_config != NULL);                                            \
        return m_config->Write(GetPathOf(path), value);                        \
    }                                                                          \
                                                                               \
    /* GetDefault function */                                                  \
    type GetDefault ## type_name(const wxString & path) const                  \
    {                                                                          \
        wxASSERT(m_items.find(GetPathOf(path)) != m_items.end());              \
        return m_items.find(GetPathOf(path))->second->Get ## type_name();      \
    }

// Macro for types that aren't natively representable by a wxConfig.
// Example functions that must be created for this macro:
//     static wxFont   StringToFont(const wxString & str);
//     static wxString FontToString(const wxFont & font);

// Note that these types will not have Write() and AddEntry() functions,
// but only WriteTYPE() and AddTYPE() functions.
#define CFG_CONVERTED_TYPE(type, type_name, conv_type)                         \
                                                                               \
    /* Add function */                                                         \
    void Add ## type_name(const wxString & path,                               \
                          const type & default_value)                          \
    {                                                                          \
        Add ## conv_type( path, type_name ## To ## conv_type(default_value) ); \
    }                                                                          \
                                                                               \
    /* Read function */                                                        \
    type Read ## type_name(const wxString & path)                              \
    {                                                                          \
        return conv_type ## To ## type_name( Read ## conv_type(path) );        \
    }                                                                          \
                                                                               \
    /* Write function */                                                       \
    bool Write ## type_name(const wxString & path,                             \
                            const type & value)                                \
    {                                                                          \
        return Write ## conv_type(path, type_name ## To ## conv_type(value) ); \
    }                                                                          \
                                                                               \
    /* GetDefault function */                                                  \
    type GetDefault ## type_name(const wxString & path) const                  \
    {                                                                          \
        return conv_type ## To ## type_name( GetDefault ## conv_type(path) );  \
    }                                                                          \



class ConfigManager
{
public:
    explicit ConfigManager(wxConfigBase * config = NULL)
        : m_config(config),
          m_path(_T("/"))
    { }

    ~ConfigManager()
    {
        for (std::map<wxString, ConfigDefault*>::iterator it = m_items.begin();
             it != m_items.end();
             ++it)
        {
            delete it->second;
        }
    }

    void SetConfig(wxConfigBase * config) { m_config = config; }

    void SetPath(const wxString & path) { m_path = GetPathOf(path); }
    const wxString & GetPath() const { return m_path; }

    // AddXXX, ReadXXX, WriteXXX, and GetDefaultXXX functions
    CFG_TYPE(wxString,   String)
    CFG_TYPE(bool,       Bool)
    CFG_TYPE(double,     Double)
    CFG_TYPE(long,       Long)

    // Extra data types
    //-----------------

    // wxFont
    static wxString FontToString(const wxFont & font)
        { return font.GetNativeFontInfoUserDesc(); }

    static wxFont StringToFont(const wxString & str)
    {
        wxFont ret;
        ret.SetNativeFontInfoUserDesc(str);
        return ret;
    }

    CFG_CONVERTED_TYPE(wxFont, Font, String);


    // wxColour
    static wxString ColorToString(const wxColour & color)
        { return color.GetAsString(); }
    static wxColour StringToColor(const wxString & str)
        { return wxColour(str); }

    CFG_CONVERTED_TYPE(wxColour, Color, String);

private:
    wxConfigBase * m_config;
    wxString m_path;
    wxString GetPathOf(const wxString & name) const
    {
        if (name.StartsWith(_T("/")))
            return name;
        return m_path + _T("/") + name;
    }

    std::map<wxString, ConfigDefault*> m_items;
};



#undef CFG_TYPE
#undef CFG_CONVERTED_TYPE

#endif // CONFIG_MGR_H