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

#include "App.hpp"
#include <wx/cmdline.h>
#include "MyFrame.hpp"
#include "paths.hpp"
#include <wx/wfstream.h> // wxFileInputStream / wxFileOutputStream for config
#include "messages.hpp"
#include "utils/string.hpp"

#include <wx/log.h>

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

    wxLogDebug(_T("Starting App"));

    m_isTimerRunning = false;

    // Seed random number generator (for grid scrambling)
    srand( time(NULL) );

    // Portable mode is enabled if there is a file named
    // "portable_mode_enabled" in the executable directory.
    // Portable mode can also be enabled via command line switches:
    // --portable OR -p
    wxFileName portable(argv[0]);
    portable.SetFullName(_T("portable_mode_enabled"));
    m_isPortable = portable.FileExists();

    wxCmdLineParser cmd(cmdLineDesc, argc, argv);

#ifdef XWORD_USE_LUA
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
    m_frame->Show();

    return true;
}


int
MyApp::OnExit()
{
    // Save config file
    wxFileName configFile(GetConfigFile());
    if (! configFile.DirExists())
        configFile.Mkdir(0777, wxPATH_MKDIR_FULL);

    wxFileConfig * config = dynamic_cast<wxFileConfig*>(wxFileConfig::Get());
    wxASSERT(config != NULL);

    wxFileOutputStream fileStream(configFile.GetFullPath());
    config->Save(fileStream);

    // Clean up printing stuff.
    delete g_printData;
    delete g_pageSetupData;

    return wxApp::OnExit();
}


#if XWORD_USE_LUA

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
        for (int i = 0; i < param_count; ++i)
        {
            wxFileName fn(cmd.GetParam(i));
            if (fn.FileExists() && puz::Puzzle::CanLoad(wx2puz(fn.GetFullPath())))
            {
                m_frame->LoadPuzzle(fn.GetFullPath());
            }
            else
            {
                wxString result;
                if (FindLuaScript(fn.GetFullPath(), &result))
                    m_frame->RunLuaScript(result);
                else
                    XWordErrorMessage(_T("%s"), result.c_str());
            }
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

    // Setup our config manager
    //-------------------------
    m_config.SetConfig(wxFileConfig::Get());

    // Window size/position defaults
    m_config.SetPath(_T("/Window"));
    m_config.AddLong(_T("top"),       20);
    m_config.AddLong(_T("left"),      20);
    m_config.AddLong(_T("width"),     500);
    m_config.AddLong(_T("height"),    500);
    m_config.AddBool(_T("maximized"), false);

    // Grid style, fonts, and colors
    m_config.SetPath(_T("/Grid"));
    m_config.AddBool(_T("fit"),       false);
    m_config.AddLong(_T("style"),     DEFAULT_GRID_STYLE);
    m_config.AddFont(_T("letterFont"),      *wxSWISS_FONT);
    m_config.AddFont(_T("numberFont"),      *wxSWISS_FONT);
    m_config.AddLong(_T("lineThickness"), 1);
    m_config.AddColor(_T("backgroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_config.AddColor(_T("focusedLetterColor"),     *wxGREEN);
    m_config.AddColor(_T("focusedWordColor"),       *wxLIGHT_GREY);
    m_config.AddColor(_T("whiteSquareColor"),       *wxWHITE);
    m_config.AddColor(_T("blackSquareColor"),       *wxBLACK);
    m_config.AddColor(_T("selectionColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
    m_config.AddColor(_T("penColor"),               *wxBLACK);
    m_config.AddColor(_T("pencilColor"),            wxColor(200,200,200));
    m_config.AddLong(_T("numberScale"),             42);
    m_config.AddLong(_T("letterScale"),             75);

    // Clue box
    m_config.SetPath(_T("/Clue"));
    m_config.AddFont(_T("font"),
                     wxFont(12, wxFONTFAMILY_SWISS,
                            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_config.AddPoint(_T("spacing"),    wxPoint(5, 5));

    m_config.AddColor(_T("listForegroundColor"),        *wxBLACK);
    m_config.AddColor(_T("listBackgroundColor"),        *wxWHITE);
    m_config.AddColor(_T("selectedForegroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
    m_config.AddColor(_T("selectedBackgroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
    m_config.AddColor(_T("crossingForegroundColor"),
        m_config.GetDefaultColor(_T("selectedBackgroundColor")));
    // Using the selected foreground color here can make the list look
    // really ugly.  If, for example, the default selected text color
    // is black, this would make the crossing clue's background color
    // to default to black, which draws too much attention to that clue.
    m_config.AddColor(_T("crossingBackgroundColor"),    *wxWHITE);

    m_config.AddFont(_T("headingFont"),
            wxFont(14, wxFONTFAMILY_SWISS,
                   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_config.AddColor(_T("headingForegroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT ) );
    m_config.AddColor(_T("headingBackgroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE ) );


    // Clue prompt
    m_config.SetPath(_T("/Clue Prompt"));
    m_config.AddFont(_T("font"),      *wxSWISS_FONT);
    m_config.AddColor(_T("foregroundColor"), *wxBLACK);
    m_config.AddColor(_T("backgroundColor"), *wxWHITE);
    m_config.AddString(_T("displayFormat"), _T("%N. %T"));


    // Printing
    m_config.SetPath(_T("/Printing"));
    m_config.AddLong(_T("blackSquareBrightness"), 0);
    m_config.AddLong(_T("gridAlignment"), wxALIGN_TOP | wxALIGN_RIGHT);
    m_config.AddLong(_T("paperID"), wxPAPER_LETTER);
    m_config.AddLong(_T("orientation"), wxPORTRAIT);
    m_config.SetPath(_T("/Printing/Margins"));
    m_config.AddLong(_T("left"), 15);
    m_config.AddLong(_T("right"), 15);
    m_config.AddLong(_T("top"), 15);
    m_config.AddLong(_T("bottom"), 15);
    m_config.SetPath(_T("/Printing/Fonts"));
    m_config.AddBool(_T("useCustomFonts"), false);
    m_config.AddFont(_T("gridLetterFont"), *wxSWISS_FONT);
    m_config.AddFont(_T("gridNumberFont"), *wxSWISS_FONT);
    m_config.AddFont(_T("clueFont"),       *wxSWISS_FONT);
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
        XWordErrorMessage(_T("%s"), result.c_str());
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
    // Escape % to %% for printing
    wxString msg = evt.GetString();
    msg.Replace(_T("%"), _T("%%"));
    wxLogDebug(msg);
}

void
MyApp::OnLuaError(wxLuaEvent & evt)
{
    // Escape % to %% for printing
    wxString msg = evt.GetString();
    msg.Replace(_T("%"), _T("%%"));
    XWordErrorMessage(msg);
}

#endif // XWORD_USE_LUA
