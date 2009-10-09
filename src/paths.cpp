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
#include <wx/filefn.h>   // wxPathList
#include <wx/filename.h> // wxFileName
#include <wx/stdpaths.h> // wxStandardPaths
#include <wx/app.h>      // wxTheApp
#include <wx/log.h>

#ifdef __UNIX__
const wxChar * configFileName = _T(".xword");
#else // ! __UNIX__
const wxChar * configFileName = _T("config.ini");
#endif // __UNIX__ / ! __UNIX__


wxString GetConfigFile()
{
    wxString filename;

    // Look in several locations for the config file (order is preserved)
    wxPathList paths;

    paths.EnsureFileAccessible(wxString(wxTheApp->argv[0]));

    // Check to see if config.ini is present in the application's directory. If
    // so, use that file (portable mode).
    filename = paths.FindAbsoluteValidPath(_T("config.ini"));
    if (! filename.empty())
        return filename;

    paths.Add(wxStandardPaths::Get().GetUserConfigDir());
    paths.Add(wxStandardPaths::Get().GetUserDataDir());
    paths.Add(wxStandardPaths::Get().GetDataDir());

    filename = paths.FindAbsoluteValidPath(configFileName);

    // If there is no config file, put it in the same directory as the executable
    if (filename.empty())
        filename = wxPathOnly(wxTheApp->argv[0]) +
                   wxFileName::GetPathSeparator() +
                   configFileName;

    return filename;
}



wxString GetImagesDirectory()
{
    // Look in several locations for the images file (order is preserved)
    // We can't use wxPathList to search for a directory (only a file), but we
    // can use it to make sure that directories aren't added twice.  Since it
    // inherits from wxArrayString, we can just iterate over the elements
    // directory to search for the directory.
    wxPathList directories;
    directories.Add(wxPathOnly(wxString(wxTheApp->argv[0])));
    directories.Add(wxStandardPaths::Get().GetUserDataDir());
    directories.Add(wxStandardPaths::Get().GetLocalDataDir());
    directories.Add(wxStandardPaths::Get().GetDataDir());
    directories.Add(wxStandardPaths::Get().GetResourcesDir());

    for (wxArrayString::const_iterator it = directories.begin();
                 it != directories.end();
                 ++it)
    {
        wxFileName dir; dir.AssignDir(*it);
        dir.AppendDir(_T("images"));
        if (dir.DirExists())
            return dir.GetFullPath();
    }

    // If there is no images directory, alert the user
    wxString message = _T("Could not find \"images\" folder.  ")
                       _T("Folders searched:\n");
    for (wxArrayString::const_iterator it = directories.begin();
         it != directories.end();
         ++it)
    {
        message.append(*it + _T("\n"));
    }
    wxLogError(message);

    return wxEmptyString;
}