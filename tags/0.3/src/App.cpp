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
#include "dialogs/Convert.hpp"

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
      _T("c"), _T("convert"),
      _T("convert files") },

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
    SetupPrinting();

    return ReadCommandLine();
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
        wxString defaultDir;
        const bool use_directory = cmd.Found(_T("d"), &defaultDir);
        wxString logfile;
        cmd.Found(_T("l"), &logfile);


        wxArrayString input_list;
        wxArrayString output_list;
        for (size_t i = 0; i < param_count; ++i)
        {
            // If output files are specified, we need to make
            // sure there is at least one more parameter.
            if (output_files && i + 1 >= param_count)
                break;

            // Get the input and output files
            wxFileName in_file(cmd.GetParam(i));
            in_file.MakeAbsolute();
            input_list.push_back(in_file.GetFullPath());

            if (output_files)
            {
                ++i;
                wxFileName out_file(cmd.GetParam(i));
                if (use_directory)
                    out_file.MakeAbsolute(defaultDir);
                else
                    out_file.MakeAbsolute(in_file.GetPath());
                output_list.push_back(out_file.GetFullPath());
            }
        }

        // Show a conversion dialog, then end the application.
        ConvertDialog * dlg = new ConvertDialog(NULL,
                                                input_list,
                                                output_list,
                                                defaultDir,
                                                logfile,
                                                overwrite_files);
        dlg->ShowModal();
        // Destroy is necessary to end the application.
        dlg->Destroy();
        return true;
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

void
MyApp::SetupConfig()
{
    wxFileName configFile(GetConfigFile());
    wxLogDebug(_T("Config file: %s"), configFile.GetFullPath().c_str());

    /*
    // Create a blank file it it doesn't exist
    if (! configFile.FileExists())
        wxFile(configFile.GetFullPath(), wxFile::write);
    */

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
    m_config.AddColor(_T("focusedLetterColor"),     *wxGREEN);
    m_config.AddColor(_T("focusedWordColor"),       *wxLIGHT_GREY);
    m_config.AddColor(_T("whiteSquareColor"),       *wxWHITE);
    m_config.AddColor(_T("blackSquareColor"),       *wxBLACK);
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
        // The frame will handle starting the timer
        if (! evt.GetActive())
        {
            wxLogDebug(_T("App Deactivate"));
            wxLogDebug(_T("Stopping timer."));
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
