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

#ifdef XWORD_USE_LUA
#    include "xwordlua.hpp"
#endif // XWORD_USE_LUA

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
    { wxCMD_LINE_SWITCH, "p", "portable", "portable mode" },
#ifdef XWORD_USE_LUA
    { wxCMD_LINE_OPTION, "e", "execute", "lua statement to execute" },
    { wxCMD_LINE_OPTION, "s", "script", "lua script to execute" },
#endif // XWORD_USE_LUA
    { wxCMD_LINE_PARAM, NULL, NULL,
#ifdef XWORD_USE_LUA
      "crossword puzzle or arguments for the script",
#else
      "crossword puzzle",
#endif // XWORD_USE_LUA
      wxCMD_LINE_VAL_STRING,
      wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

    { wxCMD_LINE_NONE }
};

bool MyApp::OnInit()
{
    //_CrtSetBreakAlloc(37630);
    m_frame = NULL;
    m_config = new ConfigManager();
    m_firstRun = false;
    m_isTimerRunning = false;

    wxLogDebug(_T("Starting App"));

    // Parse the command line
    wxCmdLineParser cmd(cmdLineDesc, argc, argv);
    cmd.Parse(false); // don't show usage

    // Portable mode is enabled if there is a file named
    // "portable_mode_enabled" in the executable directory or with the
    // command line switch -p
    wxFileName file_test(argv[0]);
    file_test.SetFullName("portable_mode_enabled");
    m_isPortable = file_test.FileExists() || cmd.Found("portable");

    srand( time(NULL) ); // random numbers for grid scrambling
    wxImage::AddHandler(new wxPNGHandler());
    SetupConfig();
    SetupPrinting();

#ifdef XWORD_USE_LUA
    m_isscript = false;
    m_luaLog = NULL;
    m_luaMessages = 0;

    // Connect lua events
    Connect(wxEVT_LUA_PRINT, wxLuaEventHandler(MyApp::OnLuaPrint));
    Connect(wxEVT_LUA_ERROR, wxLuaEventHandler(MyApp::OnLuaError));
    // Initialze wxLua.
    XWORD_LUA_IMPLEMENT_BIND_ALL
    m_lua.Create(this, wxID_ANY);
    xword_setup_lua(m_lua);

    // Make sure that lua isn't allowed to call wx.wxGetApp:MainLoop()
    // return true from this function will handle MainLoop
    wxLuaState::sm_wxAppMainLoop_will_run = true;

    // Look for lua command line options
    wxString statement, script;
    const bool has_statement = cmd.Found("execute", &statement);
    const bool has_script = cmd.Found("script", &script);
    m_isscript = has_statement || has_script;

    // Execute statement and script
    if (m_isscript)
    {
    #ifdef __WXMSW__
        // Attach to the console
        if (! AttachConsole(ATTACH_PARENT_PROCESS))
            AllocConsole();
        freopen( "CON", "w", stdout );
    #endif // __WXMSW__
        // Don't initialize the xword package here
        if (has_statement)
            m_lua.RunString(statement);
        if (has_script)
        {
            lua_State * L = m_lua.GetLuaState();
            if (! FindLuaScript(script, &script))
                return false;
            if (! luaL_loadfile(L, wx2lua(script))) // Load the file
                return false;
            size_t count = cmd.GetParamCount();
            // Set the "arg" global for the script
            lua_newtable(L);
            // XWord path
            lua_pushstring(L, wx2lua(argv[0]));
            lua_rawseti(L, -2, -1);
            // script path
            lua_pushstring(L, wx2lua(script));
            lua_rawseti(L, -2, 0);
            // parameters
            for (size_t i = 0; i < count; ++i)
            {
                lua_pushstring(L, wx2lua(cmd.GetParam(i)));
                lua_rawseti(L, -2, i + 1);
            }
            lua_setglobal(L, "arg");
            // arguments to pass to the script (...)
            for (size_t i = 0; i < count; ++i)
            {
                lua_pushstring(L, wx2lua(cmd.GetParam(i)));
            }
            // Call with <count> arguments, func already on the stack
            lua_pcall(L, count, 0, 0);
        }
        // Run MainLoop if this is a GUI script
        return wxTopLevelWindows.GetFirst() != NULL;
    }
#endif // XWORD_USE_LUA

    m_frame = new MyFrame();
#ifdef XWORD_USE_LUA
    // Initialize xword package
    wxGetApp().GetwxLuaState().RunFile(GetScriptsDir() + "/xword/init.lua");
#endif // XWORD_USE_LUA
    // Open the first valid puzzle
    for (size_t i = 0; i < cmd.GetParamCount(); ++i)
    {
        wxFileName fn(cmd.GetParam(i));
        if (fn.FileExists() && puz::Puzzle::CanLoad(wx2file(fn.GetFullPath())))
            if (m_frame->LoadPuzzle(fn.GetFullPath()))
                break;
    }

#ifdef XWORD_USE_LUA
    // Show errors from lua init
    MyStatusBar * status = dynamic_cast<MyStatusBar *>(m_frame->GetStatusBar());
    if (status)
        status->SetLuaErrors(m_luaMessages);
#endif // XWORD_USE_LUA

#ifdef __WXMSW__
    // Setup the windows help file
    m_help.Initialize(exedir() + sep() + _T("xword.chm"));
#endif // __WXMSW__

    m_frame->Show();

    return true;
}


int MyApp::OnExit()
{
#ifdef XWORD_USE_LUA
    // Close lua itself
    m_lua.CloseLuaState(true);
    // Clean up lua log file
    if (m_luaLog)
    {
        m_luaLog->Close();
        // Check to see if we have lua messages
        if (m_luaMessages) {
            XWordErrorMessage(NULL, _T("Errors occurred.  See log file: %s"), (const wxChar *)GetLuaLogFilename().c_str());
        #ifdef __WXDEBUG__
            wxShell(wxString::Format(_T("\"%s\""), (const wxChar *)GetLuaLogFilename().c_str()));
        #endif // __WXDEBUG__
        }
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

//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------
void MyApp::SetupConfig()
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
void MyApp::OnActivate(wxActivateEvent & evt)
{
    // MyFrame will NULL this pointer when it is destroyed
    if (m_frame != NULL)
    {
        if (evt.GetActive())
            m_frame->OnAppActivate();
        else
            m_frame->OnAppDeactivate();
    }

    evt.Skip();
}


//------------------------------------------------------------------------------
// Help
//------------------------------------------------------------------------------
void MyApp::ShowHelp()
{
#ifdef __WXMSW__
    if (! m_help.DisplayContents())
#endif // __WXMSW__
        wxLaunchDefaultBrowser(_T("http://wx-xword.sourceforge.net/index.html"));
}

//------------------------------------------------------------------------------
// Printing
//------------------------------------------------------------------------------

void MyApp::SetupPrinting()
{
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
}

//-----------------------------------------------------------------------------
// Lua
//-----------------------------------------------------------------------------

#ifdef XWORD_USE_LUA

// Logging functions
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
        SetLuaMessageCount(m_luaMessages + 1);
    }
    m_luaLog->Write(wxDateTime::Now().FormatTime());
    m_luaLog->Write(_T(": "));
    m_luaLog->Write(msg);
    m_luaLog->Write(_T("\n"));
    return true;
}

void MyApp::SetLuaMessageCount(int count)
{
    ++m_luaMessages = count;
    if (m_frame)
    {
        MyStatusBar * status = dynamic_cast<MyStatusBar *>(m_frame->GetStatusBar());
        if (status)
            status->SetLuaErrors(m_luaMessages);
    }
}

bool MyApp::HasLuaLog() const
{
    return m_luaLog != NULL && m_luaLog->IsOpened();
}


void MyApp::OnLuaPrint(wxLuaEvent & evt)
{
    wxLogDebug(_T("%s"), evt.GetString().c_str());
    if (m_isscript)
        printf("%s\n", (const char *)evt.GetString().c_str());
}

void MyApp::OnLuaError(wxLuaEvent & evt)
{
    wxLogDebug(_T("(error) %s"), evt.GetString().c_str());
    // Write to the log file
    LogLuaMessage(_T("(error) ") + evt.GetString());
    if (m_isscript)
        printf("(error) %s\n", (const char *)evt.GetString().c_str());
}

#endif // XWORD_USE_LUA

//------------------------------------------------------------------------------
// Mac
//------------------------------------------------------------------------------

#ifdef __WXMAC__
void MyApp::MacOpenFile(const wxString & fileName)
{
    if (m_frame)
        m_frame->LoadPuzzle(fileName);
}
#endif
