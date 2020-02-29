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


#ifndef MY_APP_H
#define MY_APP_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/app.h>
#endif

#include "config.hpp"

#ifdef __WXMSW__
#    include "wx/msw/helpchm.h"
#endif // __WXMSW__

#ifdef XWORD_USE_LUA
#    include "wxlua/wxlua.h"
#endif // XWORD_USE_LUA

class MyFrame;
class wxPrintData;
class wxPageSetupDialogData;

#ifdef XWORD_USE_LUA
    class wxLuaEvent;
    class wxFile;
#endif

// Global printing variables
extern wxPrintData * g_printData;
extern wxPageSetupDialogData* g_pageSetupData;

// Global misc
#define XWORD_APP_NAME       _T("XWord")
#define XWORD_COPYRIGHT_STRING _T("(C) 2013 Mike Richards <mrichards42@gmx.com>")

class MyApp : public wxApp
{
    friend class MyFrame; // So we can set m_frame to NULL when it is deleted
public:
    bool OnInit();
    int OnExit();
    void CleanUp();

    ConfigManager & GetConfigManager() { return *m_config; }
    bool IsPortable() { return m_isPortable; }
    bool FirstRun() { return m_firstRun; }
    MyFrame * GetFrame() { return m_frame; }

    // Public lua stuff
#ifdef XWORD_USE_LUA
    wxLuaState & GetwxLuaState() { return m_lua; }
    bool LogLuaMessage(const wxString & msg);
    bool HasLuaLog() const;
    void SetLuaMessageCount(int count);
    int GetLuaMessageCount() const { return m_luaMessages; }
#endif // XWORD_USE_LUA

    void ShowHelp();

private:
    void OnActivate(wxActivateEvent & evt);

    void SetupConfig();
    void SetupPrinting();

    MyFrame * m_frame;
    ConfigManager * m_config;
    bool m_isTimerRunning;

#ifdef __WXMSW__
    wxCHMHelpController m_help;
#endif // __WXMSW__

#ifdef __WXMAC__
    virtual void MacOpenFile(const wxString & fileName);
#endif

    // Portable mode
    bool m_isPortable;
    // First run
    bool m_firstRun;

#ifdef XWORD_USE_LUA
    // Lua
    wxLuaState m_lua;
    bool m_isscript;
    void OnLuaPrint(wxLuaEvent & evt);
    void OnLuaError(wxLuaEvent & evt);
    wxFile * m_luaLog;
    int m_luaMessages;
#endif // XWORD_USE_LUA

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(MyApp)

#endif // MY_APP_H
