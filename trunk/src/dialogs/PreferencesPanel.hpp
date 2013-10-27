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

#ifndef PREF_PANEL_H
#define PREF_PANEL_H

#ifdef __WXOSX__
#   include "wxFB_PreferencesPanelsOSX.h"
#else
#   include "wxFB_PreferencesPanels.h"
#endif
#include "PreferencesDialog.hpp"
#include "TreeCtrls.hpp"

class ConfigManager;

// Base class for all preferences panels
class PreferencesPanelBase
{
public:
    PreferencesPanelBase(ConfigManager & cfg) : m_config(cfg), m_disableEvent(false) {}
    virtual ~PreferencesPanelBase() {}

    void LoadConfig()
    {
        m_disableEvent = true;
        DoLoadConfig();
        m_disableEvent = false;
    }
    void SaveConfig() { DoSaveConfig(); }

protected:
    virtual void DoLoadConfig()=0;
    virtual void DoSaveConfig()=0;

    // Automatic save when a control value is changed (if live preview is used)
    void OnChanged(wxEvent & evt)
    {
        evt.Skip();
        if (! m_disableEvent)
            SaveConfig();
    }

    template <typename T>
    void BindChangedEvent(T handler)
    {
#if XWORD_PREFERENCES_LIVE_PREVIEW
        ::BindChangedEvent(handler, &PreferencesPanelBase::OnChanged, this);
#endif
    }

    ConfigManager & m_config;
    bool m_disableEvent;
};


// Classes for each preferences panel
class SolvePanel : public wxFB_SolvePanel, public PreferencesPanelBase
{
public:
    SolvePanel(wxWindow * parent, ConfigManager & cfg)
        : wxFB_SolvePanel(parent),
          PreferencesPanelBase(cfg)
    {
        ConnectChangedEvents();
    }
protected:
    void DoLoadConfig();
    void DoSaveConfig();
    void ConnectChangedEvents();
    void OnMoveAfterLetter(wxCommandEvent & evt);
    void OnUseAutoSave(wxCommandEvent & evt);
    void OnSaveFileHistory( wxCommandEvent& event );
};


class wxTreebook;
class wxBookCtrlEvent;

class AppearancePanel : public wxFB_AppearancePanel, public PreferencesPanelBase
{
public:
    AppearancePanel(wxWindow * parent, ConfigManager & cfg)
        : wxFB_AppearancePanel(parent),
          PreferencesPanelBase(cfg)
    {
        SetupTree();
    }
protected:
    void DoLoadConfig();
    void DoSaveConfig();
    void SetupTree();
    void UpdateLayout();
    void OnAdvancedChoice(wxCommandEvent& event);
    void OnResetDefaults(wxCommandEvent& WXUNUSED(event));
    void OnPageChanged(wxBookCtrlEvent& WXUNUSED(event));
    wxTreebook * m_treebook;
};


class PrintPanel : public wxFB_PrintPanel, public PreferencesPanelBase
{
public:
    PrintPanel(wxWindow * parent, ConfigManager & cfg)
        : wxFB_PrintPanel(parent),
          PreferencesPanelBase(cfg)
    {
        ConnectChangedEvents();
    }
protected:
    void DoLoadConfig();
    void DoSaveConfig();
    void ConnectChangedEvents();
    void OnPrintCustomFonts(wxCommandEvent& event);
    void OnBlackSquareBrightness(wxScrollEvent& event);
};

#endif // PREF_PANEL_H