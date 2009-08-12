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

#include <wx/log.h>

#include <cstdlib> // srand()
#include <ctime>   // time()

IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(MyApp, wxApp)
    EVT_ACTIVATE_APP(MyApp::OnActivate)
END_EVENT_TABLE()

// Command line table
const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH,
      _T("c"), _T("convert"),
      _T("convert multiple files without a gui") },

    { wxCMD_LINE_SWITCH,
      _T("o"), _T("output-files"),
      _T("specify output file after each input file") },

    { wxCMD_LINE_SWITCH,
      _T("w"), _T("overwrite"),
      _T("overwrite existing files") },

    { wxCMD_LINE_OPTION,
      _T("d"), _T("directory"),
      _T("parent directory for output of converted files") },

    { wxCMD_LINE_OPTION,
      _T("l"), _T("log"),
      _T("log file for conversion") },

    { wxCMD_LINE_PARAM,
      NULL, NULL,
      _T("crossword puzzle"),
      wxCMD_LINE_VAL_STRING,
      wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

    { wxCMD_LINE_NONE }
};


bool
MyApp::OnInit()
{
    m_frame = NULL;

    wxLogDebug(_T("Starting App"));
    SetReturnCode(0);

    m_isTimerRunning = false;

    // Seed random number generator (for grid scrambling)
    srand( time(NULL) );

    SetupConfig();

    return ReadCommandLine();
}


int
MyApp::OnExit()
{    
    // Save config file
    wxFileOutputStream file(GetConfigPath());
    wxFileConfig * config = dynamic_cast<wxFileConfig*>(wxFileConfig::Get());
    wxASSERT(config != NULL);
    config->Save( file );

    return wxApp::OnExit();
}


bool
MyApp::ReadCommandLine()
{
    wxCmdLineParser cmd(argc, argv);
    cmd.SetDesc(cmdLineDesc);
    cmd.Parse(false); // don't show usage

    const size_t param_count = cmd.GetParamCount();
    wxLogDebug(_T("Command count: %d"), param_count);

    // Convert files
    if (cmd.Found(_T("c")))
    {
        // Gather other options
        const bool output_files = cmd.Found(_T("o"));
        const bool overwrite_files = cmd.Found(_T("w"));
        wxString directory;
        const bool use_directory = cmd.Found(_T("d"), &directory);
        wxString log_file;
        const bool use_log_file = cmd.Found(_T("l"), &log_file);


        size_t nFailed = 0;
        size_t i;
        for (i = 0; i < param_count; ++i)
        {
            // Make sure we have an output file if requested.
            if (output_files && i + 1 >= param_count)
            {
                ++nFailed;
                break;
            }

            // Find the input and output files
            wxFileName in_file(cmd.GetParam(i));
            in_file.MakeAbsolute();

            wxFileName out_file;
            if (output_files)
            {
                ++i;
                out_file.Assign(cmd.GetParam(i));
            }
            else
            {
                out_file.Assign(in_file.GetName());
            }

            if (use_directory)
                out_file.MakeAbsolute(directory);
            else
                out_file.MakeAbsolute(in_file.GetPath());

            out_file.SetExt(_T("puz"));
            if (! overwrite_files && out_file.FileExists())
            {
                wxString orig_name = out_file.GetName();
                int i = 1;
                while (out_file.FileExists() && i < 100)
                {
                    out_file.SetName(orig_name +
                                     wxString::Format(_T("%03d"), i));
                    ++i;
                }
                if (i == 100)
                {
                    ++nFailed;
                    continue;
                }
            }


            wxLogDebug(_T("Converting %s to %s"),
                         in_file.GetFullPath().c_str(),
                         out_file.GetFullPath().c_str());

            // Make sure we have a directory
            out_file.Mkdir(0777, wxPATH_MKDIR_FULL);

            XPuzzle puz;
            if (XPuzzle::CanLoad(in_file.GetExt()))
                puz.Load(in_file.GetFullPath());
            else
                puz.Load(in_file.GetFullPath(), _T("txt"));

            if (! puz.IsOk() || ! puz.Save(out_file.GetFullPath()))
                ++nFailed;
        }
        wxLogDebug(_T("Failed: %d"), nFailed);
        SetReturnCode(nFailed);
        return false; // Don't start the app
    }

    // End of non-GUI parameters
    //--------------------------

    m_frame = new MyFrame();

    // Log to a window
#if defined(__WXDEBUG__ ) && ! defined(__VISUALC__)
    wxLogWindow *w = new wxLogWindow(m_frame, _T("Logger"));
    w->Show();
#endif


    // XWord puzzle to open
    if (param_count > 0)
        m_frame->LoadPuzzle( cmd.GetParam(0) );
    m_frame->Show();

    return true;
}





//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------

wxString
MyApp::GetConfigPath()
{
    // Look in several locations for the config file (order is preserved)
    wxPathList paths;

    paths.EnsureFileAccessible(wxString(wxTheApp->argv[0]));
    paths.Add(wxStandardPaths::Get().GetUserConfigDir());
    paths.Add(wxStandardPaths::Get().GetUserDataDir());
    paths.Add(wxStandardPaths::Get().GetDataDir());

    wxString filename = paths.FindAbsoluteValidPath(_T("config.ini"));

    // If there is no config file, put it in the same directory as the executable
    if (filename.empty())
        filename = wxPathOnly(wxTheApp->argv[0]) +
                   wxFileName::GetPathSeparator() +
                   _T("config.ini");

    return filename;
}



void
MyApp::SetupConfig()
{
    const wxString configPath = GetConfigPath();
    wxLogDebug(_T("Config file: %s"), configPath.c_str());

    // Create a blank file it it doesn't exist
    if (! wxFileName::FileExists(configPath))
        wxFile(configPath, wxFile::write);

    // Setup wxFileConfig
    wxFileInputStream file(configPath);
    wxFileConfig::Set( new wxFileConfig(file) );

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
    m_config.AddFont(_T("font"),      *wxSWISS_FONT);
    m_config.AddColor(_T("focusedLetterColor"),     *wxGREEN);
    m_config.AddColor(_T("focusedWordColor"),       *wxLIGHT_GREY);
    m_config.AddColor(_T("whiteSquareColor"),       *wxWHITE);
    m_config.AddColor(_T("blackSquareColor"),       *wxBLACK);
    m_config.AddColor(_T("penColor"),               *wxBLACK);
    m_config.AddColor(_T("pencilColor"),            wxColor(200,200,200));
    m_config.AddLong(_T("numberScale"),             42);
    m_config.AddLong(_T("letterScale"),             75);

    // Clue box
    //---------
    m_config.SetPath(_T("/Clue"));
    m_config.AddFont(_T("font"),      *wxSWISS_FONT);
    m_config.AddColor(_T("headingForegroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
    m_config.AddColor(_T("headingBackgroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
    m_config.AddColor(_T("listForegroundColor"),        *wxBLACK);
    m_config.AddColor(_T("listBackgroundColor"),        *wxWHITE);
    m_config.AddColor(_T("selectedForegroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
    m_config.AddColor(_T("selectedBackgroundColor"),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
    m_config.AddColor(_T("crossingForegroundColor"),
        m_config.GetDefaultColor(_T("selectedBackgroundColor")));
    m_config.AddColor(_T("crossingBackgroundColor"),
        m_config.GetDefaultColor(_T("selectedForegroundColor")));
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
            if (! m_frame->IsIconized() && m_isTimerRunning)
                m_frame->StartTimer();
        }
        else
        {
            wxLogDebug(_T("App Deactivate"));
            m_isTimerRunning = m_frame->IsTimerRunning();
            m_frame->StopTimer();
        }
    }

    evt.Skip();
}