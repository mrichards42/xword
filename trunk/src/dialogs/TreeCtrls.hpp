// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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


#ifndef CONFIG_INFO_H
#define CONFIG_INFO_H

// Defines template class ControlInfo for Adding appearance editing wxControls
// Defines BindChangingEvent() for binding value changed events of wxControls

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/wx.h>
#endif

#include "../config.hpp"
#include "colorchoice.hpp"
#include "fontpicker.hpp"

// Macros

#define DEFINE_CONTROL_EVENT_TRAITS(CTRL, EVT)      \
    template <> struct ControlEventTraits<CTRL>     \
    {                                               \
        static wxEventType type() { return EVT; }   \
    }

#define DEFINE_CONTROL_DESC(CTRL, TYPE, GET_FUNC, SET_FUNC)                   \
    template <>                                                               \
    struct ControlDesc<CTRL, TYPE> : public ControlDescBase<CTRL, TYPE>       \
    {                                                                         \
        CTRL * NewCtrl(wxWindow * p) { return new CTRL(p, wxID_ANY); }        \
        TYPE GetValue(const CTRL * ctrl) { return ctrl->GET_FUNC(); }         \
        void SetValue(CTRL * ctrl, const TYPE & val) { ctrl->SET_FUNC(val); } \
    }

// ----------------------------------------------------------------------------
// ControlEventTraits<CTRL>::type() -> wxEventType for a control changed event
// ----------------------------------------------------------------------------

template <typename T> struct ControlEventTraits;

DEFINE_CONTROL_EVENT_TRAITS(wxTextCtrl, wxEVT_TEXT);
DEFINE_CONTROL_EVENT_TRAITS(wxChoice,   wxEVT_CHOICE);
DEFINE_CONTROL_EVENT_TRAITS(wxCheckBox, wxEVT_CHECKBOX);
DEFINE_CONTROL_EVENT_TRAITS(wxRadioBox, wxEVT_RADIOBOX);
DEFINE_CONTROL_EVENT_TRAITS(wxSpinCtrl, wxEVT_SPINCTRL);
DEFINE_CONTROL_EVENT_TRAITS(FontPickerPanel,  wxEVT_FONTPICKER_CHANGED);
DEFINE_CONTROL_EVENT_TRAITS(wxOwnerDrawnComboBox, wxEVT_COMBOBOX);
DEFINE_CONTROL_EVENT_TRAITS(ColorChoice, wxEVT_COLOURPICKER_CHANGED);



// Bind the value changed event for the given wxControl
// Call as ::BindChangedEvent(wxControl, function, this)
//      or ::BindChangedEvent(wxControl, wxEventType, function, this)
template<typename CTRL, typename FUNCTION, typename SINK>
void BindChangedEvent(CTRL * handler, FUNCTION function, SINK sink)
{
    handler->Bind(ControlEventTraits<CTRL>::type(), function, sink);
}

template<typename EVT, typename FUNCTION, typename SINK>
void BindChangedEvent(wxWindow * handler, EVT eventType, FUNCTION function, SINK sink)
{
    handler->Bind(eventType, function, sink);
}

// ----------------------------------------------------------------------------
// ControlDesc<CTRL, TYPE>
// A template class for loading and saving config values
// ----------------------------------------------------------------------------
template <typename CTRL, typename T>
struct ControlDescBase
{
    typedef typename CTRL ctrlType;
    virtual CTRL * NewCtrl(wxWindow * parent)=0;
    virtual void LoadConfig(CTRL * ctrl, const ConfigValue<T> & cfg)
        { SetValue(ctrl, cfg.Get()); }
    virtual void SaveConfig(CTRL * ctrl, ConfigValue<T> & cfg)
        { cfg.Set(GetValue(ctrl)); }
    virtual T GetValue(const CTRL * ctrl)=0;
    virtual void SetValue(CTRL * ctrl, const T &val )=0;
};


template <typename CTRL, typename T> struct ControlDesc;

DEFINE_CONTROL_DESC(ColorChoice, wxColour, GetColor, SetColor);
DEFINE_CONTROL_DESC(wxChoice, long, GetSelection, SetSelection);
DEFINE_CONTROL_DESC(wxChoice, wxString, GetStringSelection, SetStringSelection);
DEFINE_CONTROL_DESC(FontPickerPanel, wxFont, GetSelectedFont, SetSelectedFont);
DEFINE_CONTROL_DESC(wxTextCtrl, wxString, GetValue, SetValue);
template <>
struct ControlDesc<wxCheckBox, bool> : public ControlDescBase<wxCheckBox, bool>
{
    wxCheckBox * NewCtrl(wxWindow * parent)
        { return new wxCheckBox(parent, wxID_ANY, ""); }
    bool GetValue(const wxCheckBox * ctrl) { return ctrl->GetValue(); }
    void SetValue(wxCheckBox * ctrl, const bool & val) { ctrl->SetValue(val); }
};


// ----------------------------------------------------------------------------
// Specializations of ControlDesc<> for specific controls
// ----------------------------------------------------------------------------
struct VAlignDesc : public ControlDesc<wxChoice, long>
{
    virtual wxChoice * NewCtrl(wxWindow * parent)
    {
        wxString choices[] = {"Top", "Middle", "Bottom"};
        return new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxSize(100, -1),
                            sizeof(choices) / sizeof(wxString), choices);
    }

    virtual void LoadConfig(wxChoice * ctrl, const ConfigLong & cfg)
    {
        long align = cfg.Get();
        if (align & wxALIGN_CENTER_VERTICAL)
            ctrl->SetSelection(1);
        else if (align & wxALIGN_BOTTOM)
            ctrl->SetSelection(2);
        else // wxALIGN_TOP
            ctrl->SetSelection(0);
    }
    virtual void SaveConfig(wxChoice * ctrl, ConfigLong & cfg)
    {
        int halign = cfg.Get() & (wxALIGN_CENTER_HORIZONTAL | wxALIGN_RIGHT);
        switch (ctrl->GetSelection())
        {
            case 1:
                cfg.Set(halign | wxALIGN_CENTER_VERTICAL);
                break;
            case 2:
                cfg.Set(halign | wxALIGN_BOTTOM);
                break;
            default:
                cfg.Set(halign | wxALIGN_TOP);
        }
    }
};


struct HAlignDesc : public ControlDesc<wxChoice, long>
{
    virtual wxChoice * NewCtrl(wxWindow * parent)
    {
        wxString choices[] = {"Left", "Center", "Right"};
        return new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxSize(100, -1),
                            sizeof(choices) / sizeof(wxString), choices);
    }

    virtual void LoadConfig(wxChoice * ctrl, const ConfigLong & cfg)
    {
        long align = cfg.Get();
        if (align & wxALIGN_CENTER_HORIZONTAL)
            ctrl->SetSelection(1);
        else if (align & wxALIGN_RIGHT)
            ctrl->SetSelection(2);
        else // wxALIGN_LEFT
            ctrl->SetSelection(0);
    }
    virtual void SaveConfig(wxChoice * ctrl, ConfigLong & cfg)
    {
        int valign = cfg.Get() & (wxALIGN_CENTER_VERTICAL | wxALIGN_BOTTOM);
        switch (ctrl->GetSelection())
        {
            case 1:
                cfg.Set(valign | wxALIGN_CENTER_HORIZONTAL);
                break;
            case 2:
                cfg.Set(valign | wxALIGN_RIGHT);
                break;
            default:
                cfg.Set(valign | wxALIGN_LEFT);
        }
    }
};


struct FontDesc : public ControlDesc<FontPickerPanel, wxFont>
{
    FontDesc(int flag = FP_DEFAULT) : m_flag(flag) {}
    virtual FontPickerPanel * NewCtrl(wxWindow * parent)
    {
        return new FontPickerPanel(parent, wxID_ANY, wxNullFont, m_flag);
    }
    int m_flag;
};

struct MetadataFormatDesc : public ControlDesc<wxTextCtrl, wxString>
{
    virtual wxTextCtrl * NewCtrl(wxWindow * parent)
    {
        wxTextCtrl * format = new wxTextCtrl(parent, wxID_ANY, wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize,
                                           wxTE_MULTILINE | wxTE_DONTWRAP);
        wxFont font = format->GetFont();
        font.SetFamily(wxFONTFAMILY_TELETYPE);
        font.SetFaceName("consolas");
        format->SetFont(font);
        return format;
    }
};


// ----------------------------------------------------------------------------
// DefaultControl<TYPE>::type -> the default control for this type
// ----------------------------------------------------------------------------
template <typename T> struct DefaultControl;
template <> struct DefaultControl<bool> { typedef wxCheckBox type; };
template <> struct DefaultControl<wxColour> { typedef ColorChoice type; };
template <> struct DefaultControl<wxFont> { typedef FontPickerPanel type; };


// ----------------------------------------------------------------------------
// ConfigControl<TYPE>
// A template class that stores info for attribute panels
// ----------------------------------------------------------------------------
// Classes for describing required configuration values
class ConfigControlBase
{
public:
    ConfigControlBase() {}
    virtual ~ConfigControlBase() {}
    virtual wxWindow * NewCtrl(wxWindow * parent)=0;
    virtual wxWindow * GetCtrl()=0;
    virtual void LoadConfig()=0;
    virtual void SaveConfig()=0;
    virtual void ResetConfig()=0;
    virtual wxEventType GetEventType() const=0;
};


template <
    typename T,
    typename DESC = ControlDesc<typename DefaultControl<T>::type, T>
>
class ConfigControl : public ConfigControlBase
{
public:
    ConfigControl(ConfigValue<T> & configValue)
        : m_config(configValue)
    {}

    ConfigControl(ConfigValue<T> & configValue, const DESC & desc)
        : m_config(configValue),
          m_desc(desc)
    {}

    wxWindow * NewCtrl(wxWindow * parent)
        { m_ctrl = m_desc.NewCtrl(parent); return m_ctrl; }
    wxWindow * GetCtrl()
        { return m_ctrl; }
    void LoadConfig()
        { m_desc.LoadConfig(m_ctrl, m_config); }
    void SaveConfig()
        { m_desc.SaveConfig(m_ctrl, m_config); }
    void ResetConfig()
        { m_config.Reset(); }
    wxEventType GetEventType() const
        { return ControlEventTraits<typename DESC::ctrlType>::type(); }

    typedef typename DESC::ctrlType ctrlType;
protected:
    DESC m_desc;
    typename ctrlType * m_ctrl;
    ConfigValue<T> & m_config;
};

// Undef macros
#undef DEFINE_CONTROL_DESC
#undef DEFINE_CONTROL_EVENT_TRAITS


#endif // CONFIG_INFO_H