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

#ifndef MY_PATHS_H
#define MY_PATHS_H

#include <wx/string.h>
#include <wx/image.h>
#include <wx/bitmap.h>

// On Windows, turn off the wxWidgets default behavior of returning the parent directory instead of
// the exe directory when we're compiled in debug mode and the exe is located in a folder that
// begins with "debug".
#ifdef __WXMSW__
#define InitPaths() wxStandardPaths::Get().DontIgnoreAppSubDir();
#else // ! __WXMSW__
#define InitPaths()
#endif // __WXMSW__ / ! __WXMSW__

// Locations where the application expects to find its files.
// It is up to the caller to determine whether or not the path exists.
wxString GetUserDataDir(); // Directory for user data
wxString GetConfigDir();   // Directory for config files (DataDir/config)
wxString GetConfigFile();  // The main XWord config file (DataDir/config/config.ini)
wxString GetDefaultConfigFile(); // Default config file to use if no config exists
wxString GetImagesDir();   // Images
wxString GetScriptsDir();  // Scripts
wxString exedir();
wxChar sep();


wxImage LoadXWordImage(const wxString & name, int size = -1);
wxBitmap LoadXWordBitmap(const wxString & name, int size = -1);

#ifdef XWORD_USE_LUA
#include <wx/arrstr.h>

wxString GetLuaPath();
const wxArrayString & GetLuaPathList();
wxString GetLuaCPath();
const wxArrayString & GetLuaCPathList();
bool FindLuaScript(const wxString & name, wxString * result);
wxString GetLuaLogFilename();
#endif // XWORD_USE_LUA

// Compatibility with older versions
wxString GetConfigFile_XWord_3();

#endif // MY_PATHS_H
