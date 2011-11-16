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

#include "App.hpp"
#include <wx/cmdline.h>
#include "MyFrame.hpp"
#include "MyStatusBar.hpp"
#include "paths.hpp"
#include <wx/wfstream.h> // wxFileInputStream / wxFileOutputStream for config
#include "messages.hpp"
#include "utils/string.hpp"
#include <wx/filename.h>
#include <wx/stdpaths.h> // wxStandardPaths

#include <wx/log.h>
#include <wx/file.h>

#include <cstdlib> // srand()
#include <ctime>   // time()

// Initialize the global printing variables
wxPrintData * g_printData = (wxPrintData*)NULL;
wxPageSetupDialogData * g_pageSetupData = (wxPageSetupDialogData*)NULL;


IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(MyApp, wxApp)
    EVT_ACTIVATE_APP(MyApp::OnActivate)
END_EVENT_TABLE()

// Command line table
const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH,
      _T("p"), _T("portable"),
      _T("portable mode") },

    { wxCMD_LINE_OPTION,
      _T("s"), _T("script"),
      _T("lua script to execute") },

    { wxCMD_LINE_PARAM,
      NULL, NULL,
#ifdef XWORD_USE_LUA
      _T("crossword puzzle or arguments for the script"),
#else
      _T("crossword puzzle"),
#endif // XWORD_USE_LUA
      wxCMD_LINE_VAL_STRING,
      wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

    { wxCMD_LINE_NONE }
};

bool
MyApp::OnInit()
{
    //_CrtSetBreakAlloc(37630);
    m_frame = NULL;
    m_config = new ConfigManager();

    wxLogDebug(_T("Starting App"));

    m_isTimerRunning = false;

    // Seed random number generator (for grid scrambling)
    srand( time(NULL) );

    // Portable mode is enabled if there is a file named
    // "portable_mode_enabled" in the executable directory.
    // Portable mode can also be enabled via command line switches:
    // --portable OR -p
    wxFileName file_test(argv[0]);
    file_test.SetFullName(_T("portable_mode_enabled"));
    m_isPortable = file_test.FileExists();

    // This will be set in SetupConfig()
    m_firstRun = false;

    wxCmdLineParser cmd(cmdLineDesc, argc, argv);

#ifdef XWORD_USE_LUA
    m_luaLog = NULL;
    m_luaMessages = 0;

    // Check to see if we are just running a lua script

    // Make sure that lua isn't allowed to call 
    // wx.wxGetApp:MainLoop()
    wxLuaState::sm_wxAppMainLoop_will_run = true;

    switch (CheckCommandLineForLua())
    {
        case NO_SCRIPT:
            break; // Continue as usual
        case GUI_SCRIPT:
            return true; // Run MainLoop()
        case CONSOLE_SCRIPT:
            return false; // Script is finished; exit immediately
    }
#endif // XWORD_USE_LUA

    // Check to see if "portable" was specified on the command line
    // before SetupConfig because it affects the location of the
    // config file
    cmd.Parse(false); // don't show usage
    m_isPortable = m_isPortable || cmd.Found(_T("portable"));

    // SetupConfig before creating the frame because the frame
    // uses config stuff for initialization.
    SetupConfig();
    SetupPrinting();

    // Create the frame before completely parsing the command line
    // because we might have puzzles to load.
    m_frame = new MyFrame();

    ReadCommandLine(cmd);

    // Show the errors
#ifdef XWORD_USE_LUA
    if (m_frame)
    {
        MyStatusBar * status = dynamic_cast<MyStatusBar *>(m_frame->GetStatusBar());
        if (status)
            status->SetLuaErrors(m_luaMessages);
    }
#endif // XWORD_USE_LUA

#ifdef __WXMSW__
    m_help.Initialize(exedir() + sep() + _T("xword.chm"));
#endif // __WXMSW__

    m_frame->Show();

    return true;
}


int
MyApp::OnExit()
{
#ifdef XWORD_USE_LUA
    // Clean up lua log file
    if (m_luaLog)
    {
        m_luaLog->Close();
        delete m_luaLog;
    }
#endif

    // Save config file
    wxFileName configFile(GetConfigFile());
    if (! configFile.DirExists())
        configFile.Mkdir(0777, wxPATH_MKDIR_FULL);

    wxFileConfig * config = dynamic_cast<wxFileConfig*>(wxFileConfig::Get());
    wxASSERT(config != NULL);

    wxFileOutputStream fileStream(configFile.GetFullPath());
    config->Save(fileStream);

    delete m_config;

    // Clean up printing stuff.
    delete g_printData;
    delete g_pageSetupData;

    return wxApp::OnExit();
}


#ifdef XWORD_USE_LUA

MyApp::CmdLineScriptValue
MyApp::CheckCommandLineForLua()
{
    wxString scriptFilename;
    // argv[0] is always the executable name
    for (int i = 1; i < argc; ++i)
    {
        wxString arg(argv[i]);
        // Script file can be specified using any of 
        // -s
        // /s
        // --script
        if (arg.StartsWith(_T("-s"), &scriptFilename) ||
            arg.StartsWith(_T("/s"), &scriptFilename) ||
            arg.StartsWith(_T("--script"), &scriptFilename))
        {
            int previousArg = i-1;
            if (scriptFilename.empty())
            {
                // The filename is separated by a space.
                if (i < argc)
                    scriptFilename = argv[++i];
                else
                    return NO_SCRIPT; // Poorly formed command line
            }
            else if (scriptFilename.StartsWith(_T(":")))
            {
                // The filename is separated by a colon.
                scriptFilename = scriptFilename.substr(1);
            }

            // Read the command line up until the script switch
            wxCmdLineParser cmd(cmdLineDesc, previousArg, argv);
            cmd.Parse(false); // don't show usage
            ReadCommandLine(cmd);

            RunLuaScript(scriptFilename, i);

            // Start the main loop if there are any windows.
            if (wxTopLevelWindows.GetFirst() != NULL)
                return GUI_SCRIPT;
            else
                return CONSOLE_SCRIPT;
        }
    }
    return NO_SCRIPT;
}


bool MyApp::LogLuaMessage(const wxString & msg)
{
    if (! m_luaLog)
        m_luaLog = new wxFile();
    if (! m_luaLog->IsOpened())
    {
        if (! m_luaLog->Open(GetLuaLogFilename(), wxFile::write))
            return false;
        else
            m_luaLog->Write(_T("Lua log for ") + wxDateTime::Now().FormatDate() + _T("\n") +
                            _T("==========================================================\n"));
        ++m_luaMessages;
        if (m_frame)
        {
            MyStatusBar * status = dynamic_cast<MyStatusBar *>(m_frame->GetStatusBar());
            if (status)
                status->SetLuaErrors(m_luaMessages);
        }
    }
    m_luaLog->Write(wxDateTime::Now().FormatTime());
    m_luaLog->Write(_T(": "));
    m_luaLog->Write(msg);
    m_luaLog->Write(_T("\n"));
    return true;
}

bool MyApp::HasLuaLog() const
{
    return m_luaLog != NULL && m_luaLog->IsOpened();
}
#endif // XWORD_USE_LUA


void
MyApp::ReadCommandLine(wxCmdLineParser & cmd)
{
    // Portable mode?
    m_isPortable = m_isPortable || cmd.Found(_T("portable"));

    // Load file(s)
    if (m_frame)
    {
        const size_t param_count = cmd.GetParamCount();
        wxLogDebug(_T("Command count: %d"), param_count);

        // Open files
        for (unsigned int i = 0; i < param_count; ++i)
        {
            wxFileName fn(cmd.GetParam(i));
            if (fn.FileExists() && puz::Puzzle::CanLoad(wx2file(fn.GetFullPath())))
            {
                m_frame->LoadPuzzle(fn.GetFullPath());
            }
#ifdef XWORD_USE_LUA
            else
            {
                wxString result;
                if (FindLuaScript(fn.GetFullPath(), &result))
                    m_frame->RunLuaScript(result);
                else
                    XWordErrorMessage(NULL, _T("%s"), result.c_str());
            }
#endif // XWORD_USE_LUA
        }
    }
}





//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------
void
MyApp::SetupConfig()
{
    wxFileName configFile(GetConfigFile());
    wxLogDebug(_T("Config file: %s"), configFile.GetFullPath().c_str());

    // Compatibility:
    // Search for the config file anywhere it could possibly be found from
    // previous versions.
    if (! configFile.FileExists())
        configFile.Assign(GetConfigFile_XWord_3());


    // Setup wxFileConfig
    if (configFile.FileExists())
    {
        wxFileInputStream fileStream(configFile.GetFullPath());
        wxFileConfig::Set( new wxFileConfig(fileStream) );
    }
    else
    {
        wxFileConfig::Set( new wxFileConfig() );
    }

    // First run
    wxConfigBase * config = wxFileConfig::Get();
    if (config->Read(_T("XWordVersion"), _T("None")) != XWORD_VERSION_STRING)
    {
        m_firstRun = true;
        config->Write(_T("XWordVersion"), XWORD_VERSION_STRING);
    }

    // Setup our config manager
    m_config->SetConfig(config);
}



// Handle timer starting and stopping logic
void
MyApp::OnActivate(wxActivateEvent & evt)
{
    // MyFrame will NULL this pointer when it is destroyed
    if (m_frame != NULL)
    {
        if (evt.GetActive())
        {
            wxLogDebug(_T("App Activate"));
            m_frame->OnAppActivate();
        }
        else
        {
            wxLogDebug(_T("App Deactivate"));
            m_frame->OnAppDeactivate();
        }
    }

    evt.Skip();
}

//------------------------------------------------------------------------------
// Printing
//------------------------------------------------------------------------------

void
MyApp::SetupPrinting()
{
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
}



//------------------------------------------------------------------------------
// Lua
//------------------------------------------------------------------------------

#ifdef XWORD_USE_LUA

#include "xwordlua.hpp"

// When calling a script from the command line, there is no frame.
// Shortcut the GetFrame function here.
int xword_GetFrame_nil(lua_State * L)
{
    lua_pushnil(L);
    return 1;
}

// Create a wxLuaState, run a script, and return
void
MyApp::RunLuaScript(const wxString & filename, int lastarg)
{
    // Connect lua events
    Connect(wxEVT_LUA_PRINT, wxLuaEventHandler(MyApp::OnLuaPrint));
    Connect(wxEVT_LUA_ERROR, wxLuaEventHandler(MyApp::OnLuaError));

    // Initialze wxLua.
    XWORD_LUA_IMPLEMENT_BIND_ALL
    wxLuaState lua(this, wxID_ANY);
    xword_setup_lua(lua);

    lua_State * L = lua.GetLuaState();

    // xword.GetFrame = xword_GetFrame_nil
    lua_getglobal(L, "xword");
    lua_pushstring(L, "GetFrame");
    lua_pushcfunction(L, xword_GetFrame_nil);
    lua_settable(L, -3);

    // Initialize the lua additions to the xword package
    lua.RunFile(GetScriptsDir() + _T("/xword/init.lua"));

    wxString result;
    if (! FindLuaScript(filename, &result))
    {
        XWordErrorMessage(NULL, _T("%s"), result.c_str());
        return;
    }

    // Create an arg table for the arguments
    lua_newtable(L);
    if (lastarg != -1)
    {
        // push arguments
        for (int i = 0; i < argc; ++i)
        {
            lua_pushnumber(L, i - lastarg);
            if (i == lastarg)
                lua_pushstring(L, wx2lua(filename));
            else
                lua_pushstring(L, wx2lua(argv[i]));
            lua_settable(L, -3);
        }
    }
    lua_setglobal(L, "arg");

    // Run the script
    lua.RunFile(result);
}

void
MyApp::OnLuaPrint(wxLuaEvent & evt)
{
    wxLogDebug(_T("%s"), evt.GetString().c_str());
}

void
MyApp::OnLuaError(wxLuaEvent & evt)
{
    // Escape % to %% for printing
    LogLuaMessage(_T("(error) ") + evt.GetString());
}

#endif // XWORD_USE_LUA
