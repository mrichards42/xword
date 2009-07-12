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
#include <cstdlib>
#include <ctime>

IMPLEMENT_APP(MyApp)

// Command line table
static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, _T("c"), _T("convert"), _T("convert multiple files") },

    { wxCMD_LINE_PARAM,  NULL, NULL, _T("files"), wxCMD_LINE_VAL_STRING,
                       wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL },

    { wxCMD_LINE_NONE }
};



bool
MyApp::OnInit()
{
    wxLogDebug(_T("Starting App"));
    SetReturnCode(0);

    // Command line parsing
    // ------------------------------------
    wxCmdLineParser cmd(argc, argv);
    cmd.SetDesc(cmdLineDesc);
    cmd.Parse(false); // don't show usage

    wxString filename;

    const size_t param_count = cmd.GetParamCount();
    wxLogDebug(_T("Command count: %d"), param_count);

    // Convert files (specify one after the other; unlimited)
    if (cmd.Found(_T("c")))
    {
        size_t nFailed = 0;
        size_t i;
        for (i = 0; i + 1 < param_count; i += 2)
        {
            wxLogDebug( _T("Converting %s to %s"),
                        cmd.GetParam(i),
                        cmd.GetParam(i + 1));

            XPuzzle puz(cmd.GetParam(i));
            if (! puz.IsOk() || ! puz.Save(cmd.GetParam(i + 1)))
                ++nFailed;
        }
        if (i < param_count)
            ++nFailed;
        wxLogDebug(_T("Failed: %d"), nFailed);
        SetReturnCode(nFailed);
        return false; // Don't start the app
    }
    // Open a file
    else if (param_count > 0)
        filename = cmd.GetParam(0);

    // Seed random number generator (for making keys)
    srand( time(NULL) );

    // Create the frame
    // ------------------------------------
    m_frame = new MyFrame();
    if (! filename.IsEmpty())
        m_frame->LoadPuzzle(filename);
    m_frame->Show();
    return true;
}