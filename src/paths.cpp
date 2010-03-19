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

#include "paths.hpp"
#include <wx/string.h>
#include <wx/filename.h> // wxFileName
#include <wx/stdpaths.h> // wxStandardPaths
#include "App.hpp" // wxGetApp().IsPortable()

#ifdef __UNIX__
const wxChar * configFileName = _T(".xword");
#else // ! __UNIX__
const wxChar * configFileName = _T("config.ini");
#endif // __UNIX__ / ! __UNIX__

wxChar sep()
{
    return wxFileName::GetPathSeparator();
}

wxString exedir()
{
    return wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
}


wxString GetConfigDirectory()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("config");
    else
        return wxStandardPaths::Get().GetUserDataDir() + sep() + _T("config");
}

wxString GetConfigFile()
{
    return GetConfigDirectory() + sep() + configFileName;
}

// If wxWidgets is compiled in debug mode under windows, *and* the exe is located
// in a folder that begins with "debug" certain wxStandardPaths functions return
// the parent directory instead of the exe directory.  This is clearly intended
// as a feature, but I find it to be a pain.
// The following are affected:
//     wxStandardPaths::GetDataDir
//     wxStandardPaths::GetPluginsDir
//     wxStandardPaths::GetResourcesDir
//     wxStandardPaths::GetAppDir
// Fortunately these should always return the exe directory.
#if ! defined(__WXDEBUG__) || ! defined(__WXMSW__)

wxString GetImagesDirectory()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("images");
    else
        return wxStandardPaths::Get().GetResourcesDir() + sep() + _T("images");
}


wxString GetScriptsDirectory()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("scripts");
    else
        return wxStandardPaths::Get().GetResourcesDir() + sep() + _T("scripts");
}

#else

wxString GetImagesDirectory()
{
    return exedir() + sep() + _T("images");
}


wxString GetScriptsDirectory()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("scripts");
    else
        return wxStandardPaths::Get().GetUserDataDir() + sep() + _T("scripts");
}

#endif // ! defined(__WXDEBUG__) || ! defined(__WXMSW__)



// Compatibility with old versions of XWord

// Find the config file in places that XWord 0.3 expected.
wxString GetConfigFile_XWord_3()
{
    wxString filename;

    // Look in several locations for the config file (order is preserved)
    wxPathList paths;
    paths.Add(exedir());

    // Look for config.ini in the executable directory
    filename = paths.FindAbsoluteValidPath(_T("config.ini"));
    if (! filename.empty())
        return filename;

    // It's not there . . . check other directories
    paths.Add(wxStandardPaths::Get().GetUserConfigDir());
    paths.Add(wxStandardPaths::Get().GetUserDataDir());
    paths.Add(wxStandardPaths::Get().GetDataDir());

    return paths.FindAbsoluteValidPath(configFileName);
}
