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


#include "PreferencesDialog.hpp"
#include "PreferencesPanel.hpp"

#include "../MyFrame.hpp"
#include "../CluePanel.hpp"
#include "../CluePrompt.hpp"
#include "../XGridCtrl.hpp"
#include "../App.hpp" // For the ConfigManager
#include "fontface.hpp" // FontFaceCtrl::Init/ClearFacenames()
#if XWORD_USE_LUA
#   include "../xwordlua.hpp"
#endif // XWORD_USE_LUA

extern int wxluatype_wxBookCtrlBase;

PreferencesDialog::PreferencesDialog(wxWindow * parent)
    : wxPropertySheetDialog(),
      m_config(NULL),
      m_oldConfig(CONFIG_DUMMY)
{
#if XWORD_PREFERENCES_SHRINK
    SetSheetStyle(wxPROPSHEET_DEFAULT | wxPROPSHEET_SHRINKTOFIT);
#endif
    Create(parent, wxID_ANY, "Preferences", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    // Start enumerating font faces
    FontFaceCtrl::InitFacenames();

    // Setup our ConfigManager
    ConfigManager & appConfig = wxGetApp().GetConfigManager();
    m_oldConfig.Copy(appConfig);
#if XWORD_PREFERENCES_LIVE_PREVIEW
    m_config = &appConfig;
#else
    m_config = new ConfigManager(CONFIG_DUMMY);
#endif

    // Add the pages
    wxBookCtrlBase * book = GetBookCtrl();
    GetBookCtrl()->AddPage(new SolvePanel(book, *m_config), "General");
    GetBookCtrl()->AddPage(new AppearancePanel(book, *m_config), "Appearance");
    GetBookCtrl()->AddPage(new PrintPanel(book, *m_config), "Printing");

    LoadConfig();

    // Setup buttons
#ifndef __WXOSX__
#   if XWORD_PREFERENCES_LIVE_PREVIEW
    CreateButtons(wxOK | wxCANCEL);
#   else
    CreateButtons(wxOK | wxCANCEL | wxAPPLY);
    Bind(wxEVT_BUTTON, &PreferencesDialog::OnApply, this, wxID_APPLY);
#   endif // XWORD_PREFERENCES_LIVE_PREVIEW
    Bind(wxEVT_BUTTON, &PreferencesDialog::OnOK, this, wxID_OK);
    Bind(wxEVT_BUTTON, &PreferencesDialog::OnCancel, this, wxID_CANCEL);
#endif // __WXOSX__

#if XWORD_USE_LUA
    // Add preferences pages from lua
    MyFrame * frame = wxDynamicCast(GetParent(), MyFrame);
    wxASSERT(frame);
    if (frame)
    {
        wxLuaState & lua = frame->GetwxLuaState();
        lua_State * L = lua.GetLuaState();
        // Find the function xword.OnInitPreferencesDialog
        lua_getglobal(L, "xword");
        lua_getfield(L, -1, "OnInitPreferencesDialog");
        if (lua_isfunction(L, -1))
        {
            // Call the function with our notebook argument
            if (wxluaT_pushuserdatatype(L, GetBookCtrl(), wxluatype_wxBookCtrlBase))
                lua_pcall(L, 1, 0, 0);
        }
        lua_pop(L, 1); // remove the xword table from the stack
    }
#endif // XWORD_USE_LUA
    InvalidateBestSize();
    LayoutDialog();
}


PreferencesDialog::~PreferencesDialog()
{
#if ! XWORD_PREFERENCES_LIVE_PREVIEW
    delete m_config;
#endif
    FontFaceCtrl::ClearFacenames();
    if (this == s_dialog)
        s_dialog = NULL;
}

PreferencesDialog * PreferencesDialog::s_dialog = NULL;
void PreferencesDialog::ShowDialog(wxWindow * parent)
{
    if (! s_dialog)
        s_dialog = new PreferencesDialog(parent);
    s_dialog->Raise();
    s_dialog->Show();
}

void PreferencesDialog::LoadConfig()
{
#if ! XWORD_PREFERENCES_LIVE_PREVIEW
    m_config->Copy(wxGetApp().GetConfigManager());
#endif
    wxBookCtrlBase * book = GetBookCtrl();
    for (size_t i = 0; i < book->GetPageCount(); ++i)
    {
        PreferencesPanelBase * panel = dynamic_cast<PreferencesPanelBase *>(book->GetPage(i));
        if (panel)
            panel->LoadConfig();
    }
}


void PreferencesDialog::SaveConfig()
{
    wxBookCtrlBase * book = GetBookCtrl();
    for (size_t i = 0; i < book->GetPageCount(); ++i)
    {
        PreferencesPanelBase * panel = dynamic_cast<PreferencesPanelBase *>(book->GetPage(i));
        if (panel)
            panel->SaveConfig();
    }
#if XWORD_USE_LUA
    // Save preferences from lua
    MyFrame * frame = wxDynamicCast(GetParent(), MyFrame);
    wxASSERT(frame);
    if (frame)
    {
        wxLuaState & lua = frame->GetwxLuaState();
        lua_State * L = lua.GetLuaState();
        // Find the function xword.OnSavePreferences
        lua_getglobal(L, "xword");
        lua_getfield(L, -1, "OnSavePreferences");
        if (lua_isfunction(L, -1))
        {
            // Call the function with our notebook argument
            lua_pcall(L, 0, 0, 0);
        }
        lua_pop(L, 1); // remove the xword table from the stack
    }
#endif
#if ! XWORD_PREFERENCES_LIVE_PREVIEW
    wxGetApp().GetConfigManager().Copy(*m_config);
#endif
}

void PreferencesDialog::ResetConfig()
{
    wxGetApp().GetConfigManager().Copy(m_oldConfig);
}

// Events -- NB: Don't call Close() here because that will send an extra
// event with wxID_CANCEL.  Use evt.Skip() instead
void PreferencesDialog::OnOK(wxCommandEvent & evt)
{
    SaveConfig();
    GetParent()->Refresh();
    evt.Skip();
}

void PreferencesDialog::OnApply(wxCommandEvent & evt)
{
    SaveConfig();
    GetParent()->Refresh();
    evt.Skip();
}

void PreferencesDialog::OnCancel(wxCommandEvent & evt)
{
    ResetConfig();
    evt.Skip();
}
