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


#include "PuzLoader.hpp"
#include <wx/filename.h>
#include <wx/wfstream.h>  // file streams
#include <wx/msgdlg.h>

#include "XPuzzle.hpp"

// File type handlers
#include "HandlerCommon.hpp"
#include "PuzHandler.hpp"
//#include "TxtHandler.hpp"

/*
#include <wx/file.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>

#include <wx/log.h> // for reporting errors
*/


namespace PuzLoader
{

// Typedefs for load/save functions
typedef void (*SaveFunction)(XPuzzle *, wxOutputStream & stream);
typedef void (*LoadFunction)(XPuzzle *, wxInputStream &  stream);


struct TypeDesc
{
    wxString ext;         // Extension for this type  (e.g. "puz")
    wxString description; // Descriptive string       (e.g. "Across format")
    LoadFunction load;
    SaveFunction save;
};




// The master list of supported file types
//------------------------------------------
const TypeDesc functionList[] = {
    { _T("puz"), _T("Across format"),     LoadPuz,  SavePuz },
    //{ _T("txt"), _T("ASCII text format"), LoadTxt, NULL },

    // Always end with this item
    { wxEmptyString, wxEmptyString, NULL, NULL }
};


TypeDesc GetType(wxString ext);


// Exposed load and save functions
//---------------------------------
bool
Load(XPuzzle * puz, const wxString & filename, wxString ext)
{
    try
    {
        wxFileName fn(filename);
        fn.MakeAbsolute();
        if (ext == wxEmptyString)
        {
            if (! fn.HasExt())
                return false;
            ext = fn.GetExt();
        }

        LoadFunction func = GetType(ext).load;
        if (func == NULL)
            return false;

        puz->Clear();
        puz->m_filename = fn.GetFullPath();

        wxLogDebug(_T("Loading puzzle: %s"), puz->m_filename);
        wxFileInputStream stream(puz->m_filename);
        if (! stream.IsOk())
            throw PuzLoadError(_T("Cannot open file %s"), puz->m_filename);
        func(puz, stream);
        return true;
    }
    catch (PuzLoadError & error)
    {
        if (! error.isProcessed)
        {
            error.isProcessed = true;

            wxMessageBox(error.message,
                        wxMessageBoxCaptionStr,
                        wxOK | wxICON_ERROR);
        }
    }
    catch (...)
    {
        wxMessageBox(_T("Error loading file"),
                     wxMessageBoxCaptionStr,
                     wxOK | wxICON_ERROR);
    }
    return false;
}


bool
Save(XPuzzle * puz, const wxString & filename, wxString ext)
{
    try
    {
        wxFileName fn(filename);
        fn.MakeAbsolute();
        if (ext == wxEmptyString)
        {
            if (! fn.HasExt())
                return false;
            ext = fn.GetExt();
        }

        SaveFunction func = GetType(ext).save;
        if (func == NULL)
            return false;

        puz->m_filename = fn.GetFullPath();
        wxLogDebug(_T("Saving puzzle: %s"), puz->m_filename);
        wxFileOutputStream stream(puz->m_filename);
        if (! stream.IsOk())
            throw PuzLoadError(_T("Cannot open file %s"), puz->m_filename);
        func(puz, stream);
        return true;
    }
    catch (PuzLoadError & error)
    {
        if (! error.isProcessed)
        {
            error.isProcessed = true;

            wxMessageBox(error.message,
                        wxMessageBoxCaptionStr,
                        wxOK | wxICON_ERROR);
        }
    }
    catch (...)
    {
        wxMessageBox(_T("Error saving file"),
                     wxMessageBoxCaptionStr,
                     wxOK | wxICON_ERROR);
    }
    return false;
}


// Test to see if we can load or save an extension
//---------------------------------
bool
CanSave(const wxString & ext)
{
    return GetType(ext).save != NULL;
}


bool
CanLoad(const wxString & ext)
{
    return GetType(ext).load != NULL;
}



// Get a list of supported file types to load and save
//---------------------------------
std::vector<wxString>
GetLoadTypes()
{
    std::vector<wxString> types;
    for (size_t i = 0; functionList[i].ext != wxEmptyString; ++i)
    {
        const TypeDesc & type = functionList[i];
        if (type.load == NULL || type.description == wxEmptyString)
            continue;
        types.push_back( functionList[i].ext );
    }
    return types;
}


std::vector<wxString>
GetSaveTypes()
{
    std::vector<wxString> types;
    for (size_t i = 0; functionList[i].ext != wxEmptyString; ++i)
    {
        const TypeDesc & type = functionList[i];
        if (type.save == NULL || type.description == wxEmptyString)
            continue;
        types.push_back( functionList[i].ext );
    }
    return types;
}


// Get a string to pass to wxWidgets open and save file dialogs
//---------------------------------
wxString
GetLoadTypeString()
{
    wxString typeStr;
    wxString allTypes;
    for (size_t i = 0; functionList[i].ext != wxEmptyString; ++i)
    {
        const TypeDesc & type = functionList[i];
        if (type.load == NULL || type.description == wxEmptyString)
            continue;

        typeStr << wxString::Format(_T("%s (*.%s)|*.%s|"),
                                    type.description,
                                    type.ext,
                                    type.ext);

        allTypes << wxString::Format(_T("*.%s;"), type.ext);
    }
    allTypes.RemoveLast();

    typeStr << wxString::Format(_T("Supported types (%s)|%s"),
                                allTypes,
                                allTypes);
    return typeStr;
}


wxString
GetSaveTypeString()
{
    wxString typeStr;
    wxString allTypes;
    for (size_t i = 0; functionList[i].ext != wxEmptyString; ++i)
    {
        const TypeDesc & type = functionList[i];
        if (type.save == NULL || type.description == wxEmptyString)
            continue;

        typeStr << wxString::Format(_T("%s (*.%s)|*.%s|"),
                                    type.description,
                                    type.ext,
                                    type.ext);

        allTypes << wxString::Format(_T("*.%s;"), type.ext);
    }
    allTypes.RemoveLast();

    typeStr << wxString::Format(_T("Supported types (%s)|%s"),
                                allTypes,
                                allTypes);
    return typeStr;
}




// Utility for using TypeDesc
TypeDesc
GetType(wxString ext)
{
    ext.MakeLower();
    size_t i;
    for (i = 0; functionList[i].ext != wxEmptyString; ++i)
        if (functionList[i].ext == ext)
            break;
    // This will return the blank entry at the end of the table
    // if no match is found.
    return functionList[i];
}

} // namespace PuzLoader





//------------------------------------------------------------------------------
// Read/Write and utility functions
//------------------------------------------------------------------------------
inline wxString ReadTextLine  (wxTextInputStream & f);
inline bool     ReadGridString(wxTextInputStream & f,
                               size_t width, size_t height,
                               wxString * str);




/*
//------------------------------------------------------------------------------
// Read/Write utility functions
//------------------------------------------------------------------------------
inline wxString
ReadTextLine(wxTextInputStream & f)
{
    return f.ReadLine().Trim(true).Trim(false);
}

inline bool
ReadGridString(wxTextInputStream & f, size_t width, size_t height, wxString * str)
{
    str->Empty();
    size_t i;
    for (i = 0; i < height; ++i) {
        wxString line = ReadTextLine(f);
        if (line.length() != width) {
            return LoadError(_T("Incorrect grid width"));
        }
        str->Append(line);
    }
    if (str->length() != width * height)
        return LoadError(_T("Grid improperly sized"));
    return true;
}

*/