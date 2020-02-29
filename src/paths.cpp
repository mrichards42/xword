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

wxString GetUserDataDir()
{
    if (wxGetApp().IsPortable())
        return exedir();
    else
        return wxStandardPaths::Get().GetUserDataDir();
}


wxString GetConfigDir()
{
    return GetUserDataDir() + sep() + _T("config");
}

wxString GetConfigFile()
{
    return GetConfigDir() + sep() + configFileName;
}

wxString GetDefaultConfigFile()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + "default_config.ini";
    else
        return wxStandardPaths::Get().GetResourcesDir() + sep() + "default_config.ini";
}

wxString GetImagesDir()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("images");
    else
        return wxStandardPaths::Get().GetResourcesDir() + sep() + _T("images");
}


wxString GetScriptsDir()
{
    if (wxGetApp().IsPortable())
        return exedir() + sep() + _T("scripts");
    else
        return wxStandardPaths::Get().GetPluginsDir() + sep() + _T("scripts");
}

#define GetPluginsDir() wxStandardPaths::Get().GetPluginsDir()

wxImage LoadXWordImage(const wxString & name, int size)
{
    wxString icon;
    if (size != -1)
        icon.Printf(_T("%s_%d.png"), name, size);
    else
        icon = name;

    wxFileName iconPath(icon);
    iconPath.MakeAbsolute(GetImagesDir());

    return wxImage(iconPath.GetFullPath());
}


wxBitmap LoadXWordBitmap(const wxString & name, int size)
{
    return wxBitmap(LoadXWordImage(name, size));
}


#ifdef XWORD_USE_LUA

#include <luaconf.h>

const wxArrayString & GetLuaPathList()
{
    static wxArrayString pathList;
    if (pathList.empty())
    {
        wxString scripts = GetScriptsDir();
        pathList.push_back(scripts + sep() + _T("?.lua"));
        pathList.push_back(scripts + sep() + _T("?") + sep() + _T("init.lua"));
        pathList.push_back(scripts + sep() + _T("?") + sep() + _T("?.lua"));
        pathList.push_back(scripts + sep() + _T("libs") + sep() + _T("?.lua"));
        pathList.push_back(scripts + sep() + _T("libs") + sep() + _T("?") + sep() + _T("init.lua"));
        pathList.push_back(scripts + sep() + _T("libs") + sep() + _T("?") + sep() + _T("?.lua"));
    }
    return pathList;
}

wxString GetLuaPath()
{
    wxString path;
    const wxArrayString & pathList = GetLuaPathList();
    assert(! pathList.empty());
    wxArrayString::const_iterator it;
    for (it = pathList.begin(); it != pathList.end()-1; ++it)
    {
        path.append(*it);
        path.append(_T(";"));
    }
    path.append(pathList.back());
    return path;
}



// Shared library extensions for different operating systems
#if defined(__WXMSW__)
#   define LUA_DLL_EXT _T(".dll")
#elif defined(__WXMAC__)
#   define LUA_DLL_EXT _T(".dylib")
#else
#   define LUA_DLL_EXT _T(".so")
#endif

const wxArrayString & GetLuaCPathList()
{
    static wxArrayString cpathList;
    if (cpathList.empty())
    {
        wxString scripts = GetScriptsDir();
        wxString plugins = GetPluginsDir();
        cpathList.push_back(plugins + sep() + _T("?") + LUA_DLL_EXT);
        cpathList.push_back(plugins + sep() + _T("?51") + LUA_DLL_EXT);
        cpathList.push_back(scripts + sep() + _T("libs") + sep() + _T("?") + LUA_DLL_EXT);
        cpathList.push_back(scripts + sep() + _T("libs") + sep() + _T("?51") + LUA_DLL_EXT);
#ifdef __WXMAC__
		wxString frameworks = wxPathOnly(exedir()) + sep() + _T("Frameworks");
        cpathList.push_back(frameworks + sep() + _T("?") + LUA_DLL_EXT);
        cpathList.push_back(frameworks + sep() + _T("?51") + LUA_DLL_EXT);

		// lib variants
        cpathList.push_back(plugins + sep() + _T("lib?") + LUA_DLL_EXT);
        cpathList.push_back(plugins + sep() + _T("lib?51") + LUA_DLL_EXT);
        cpathList.push_back(scripts + sep() + _T("libs") + sep() + _T("lib?") + LUA_DLL_EXT);
        cpathList.push_back(scripts + sep() + _T("libs") + sep() + _T("lib?51") + LUA_DLL_EXT);
        cpathList.push_back(frameworks + sep() + _T("lib?") + LUA_DLL_EXT);
        cpathList.push_back(frameworks + sep() + _T("lib?51") + LUA_DLL_EXT);

#endif // __WXMAC__
    }
    return cpathList;
}

wxString GetLuaCPath()
{
    wxString cpath;
    const wxArrayString & cpathList = GetLuaCPathList();
    assert(! cpathList.empty());
    wxArrayString::const_iterator it;
    for (it = cpathList.begin(); it != cpathList.end()-1; ++it)
    {
        cpath.append(*it);
        cpath.append(_T(";"));
    }
    cpath.append(cpathList.back());
    return cpath;
}



bool FindLuaScript(const wxString & name, wxString * result)
{
    if (wxFileName::FileExists(name))
    {
        *result = name;
        return true;
    }

    wxString filename;
    // Chop off the extension
    if (name.EndsWith(_T(".lua")))
        filename = name.substr(0, name.length() - 4);
    else
        filename = name;

    wxString error;
    error << _T("Could not find file: ") << name << _T(". Paths searched:");

    const wxArrayString & pathList = GetLuaPathList();
    wxArrayString::const_iterator it;
    for (it = pathList.begin(); it != pathList.end(); ++it)
    {
        wxString path = *it;
        path.Replace(_T(LUA_PATH_MARK), filename);
        wxFileName fn(path);
        fn.Normalize();
        if (fn.FileExists())
        {
            *result = fn.GetFullPath();
            return true;
        }
        else
            error << _T("\n") << fn.GetFullPath();
    }
    *result = error;
    return false;
}

wxString GetLuaLogFilename()
{
    return GetUserDataDir() +
           wxFileName::GetPathSeparator() +
           _T("lualog.txt");
}

#endif // XWORD_USE_LUA


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
