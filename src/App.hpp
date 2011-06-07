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
#define XWORD_VERSION_STRING _T("0.4 development snapshot")
#define XWORD_COPYRIGHT_STRING _T("(C) 2010 Mike Richards <mrichards42@gmx.com>")

class MyApp : public wxApp
{
    friend class MyFrame; // So we can set m_frame to NULL when it is deleted
public:
    bool OnInit();
    // expects an already parsed command line
    void ReadCommandLine(wxCmdLineParser & cmd);

    int OnExit();

    ConfigManager & GetConfigManager() { return m_config; }

    // Portable mode
    bool IsPortable() { return m_isPortable; }

    // Lua logging
#ifdef XWORD_USE_LUA
    bool LogLuaMessage(const wxString & msg);
    bool HasLuaLog() const;
#endif // XWORD_USE_LUA

private:
    void OnActivate(wxActivateEvent & evt);

    void SetupConfig();
    void SetupPrinting();

    MyFrame * m_frame;
    ConfigManager m_config;
    bool m_isTimerRunning;

    // Portable mode
    bool m_isPortable;

#ifdef XWORD_USE_LUA
    enum CmdLineScriptValue {
        NO_SCRIPT,
        GUI_SCRIPT,
        CONSOLE_SCRIPT
    };
    CmdLineScriptValue CheckCommandLineForLua();
    // Lua
    void RunLuaScript(const wxString & filename, int lastarg = -1);
    void OnLuaPrint(wxLuaEvent & evt);
    void OnLuaError(wxLuaEvent & evt);
    wxFile * m_luaLog;
    int m_luaMessages;
#endif // XWORD_USE_LUA

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(MyApp)

#endif // MY_APP_H
