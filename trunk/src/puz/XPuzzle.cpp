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
// You should have received a copy of ther GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "XPuzzle.hpp"
#include <wx/filename.h>
#include <wx/wfstream.h>  // file streams
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/choicdlg.h>
#include <wx/module.h>


// File type handlers
#include "HandlerBase.hpp"
#include "PuzHandler.hpp"
#include "TxtHandler.hpp"


bool
XPuzzle::Load(const wxString & filename, wxString ext)
{
    wxFileName fn(filename);
    fn.MakeAbsolute();
    if (ext == wxEmptyString && fn.HasExt())
        ext = fn.GetExt();

    if (! fn.FileExists())
    {
        wxLogError(_T("File \"%s\" does not exist"), fn.GetFullPath().c_str());
        return false;
    }

    return Load(fn.GetFullPath(), GetHandler(ext));
}


bool
XPuzzle::Load(const wxString & filename, HandlerBase * handler)
{
    wxFileName fn(filename);
    fn.MakeAbsolute();

    m_modified = false;
    m_isOk = false;

    if (handler == NULL)
    {
        handler = PromptForLoadHandler(_T("Unknown file type."));
        if (handler == NULL)
            return false;
    }

    wxASSERT(handler->CanLoad());

    Clear();
    m_filename = fn.GetFullPath();

    wxFileInputStream stream(m_filename);
    if (! stream.IsOk())
        throw PuzFileError(_T("Cannot open file %s"), m_filename.c_str());
    handler->Load(this, stream);
    m_isOk = true;
    return true;
}


bool
XPuzzle::Save(const wxString & filename, wxString ext)
{
    wxFileName fn(filename);
    fn.MakeAbsolute();
    if (ext == wxEmptyString && fn.HasExt())
        ext = fn.GetExt();

    return Save(fn.GetFullPath(), GetHandler(ext));
}


bool
XPuzzle::Save(const wxString & filename, HandlerBase * handler)
{
    wxFileName fn(filename);
    fn.MakeAbsolute();

    m_modified = false;

    if (handler == NULL)
    {
        handler = PromptForLoadHandler(_T("Unknown file type."));
        if (handler == NULL)
            return false;
    }

    wxASSERT(handler->CanLoad());

    wxLogDebug(_T("Saving puzzle: %s"), m_filename.c_str());
    try
    {
        wxFileOutputStream stream(fn.GetFullPath());
        if (! stream.IsOk())
            throw PuzFileError(_T("Cannot open file %s"), m_filename.c_str());
        handler->Save(this, stream);
        m_filename = fn.GetFullPath();
        return true;
    }
    catch (...)
    {
        // An exception thrown while saving means that the resulting
        // file is unusable, so just delete it here.
        wxRemoveFile(fn.GetFullPath());
        throw;
    }

}


//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

HandlerBase *
XPuzzle::PromptForLoadHandler(const wxString & message)
{
    wxArrayString choices;
    for (std::vector<HandlerBase *>::const_iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        const HandlerBase * handler = *it;
        if (handler->CanLoad())
            choices.Add(handler->GetDescription());
    }
    int ret = wxGetSingleChoiceIndex(message, _T("Select file type"), choices);
    if (ret == -1)
        return NULL;
    else
        return sm_handlers.at(ret);
}


HandlerBase *
XPuzzle::PromptForSaveHandler(const wxString & message)
{
    wxArrayString choices;
    for (std::vector<HandlerBase *>::const_iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        const HandlerBase * handler = *it;
        if (handler->CanSave())
            choices.Add(handler->GetDescription());
    }
    int ret = wxGetSingleChoiceIndex(message, _T("Select file type"), choices);
    if (ret == -1)
        return NULL;
    else
        return sm_handlers.at(ret);
}



// Get a string to pass to wxWidgets open and save file dialogs
//---------------------------------
wxString
XPuzzle::GetLoadTypeString()
{
    wxString typeStr;
    wxString allTypes;
    for (std::vector<HandlerBase *>::const_iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        const HandlerBase * handler = *it;
        if (! handler->CanLoad())
            continue;

        typeStr << wxString::Format(_T("%s (*.%s)|*.%s|"),
                                    handler->GetDescription().c_str(),
                                    handler->GetExtension().c_str(),
                                    handler->GetExtension().c_str());

        allTypes << wxString::Format(_T("*.%s;"), handler->GetExtension().c_str());
    }

    typeStr << wxString::Format(_T("Supported types (%s)|%s"),
                                allTypes.c_str(),
                                allTypes.c_str());
    typeStr.RemoveLast();
    return typeStr;
}


wxString
XPuzzle::GetSaveTypeString()
{
    // Don't use "Supported Types" if we're saving

    wxString typeStr;
    //wxString allTypes;
    for (std::vector<HandlerBase *>::const_iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        const HandlerBase * handler = *it;
        if (! handler->CanSave())
            continue;

        typeStr << wxString::Format(_T("%s (*.%s)|*.%s|"),
                                    handler->GetDescription().c_str(),
                                    handler->GetExtension().c_str(),
                                    handler->GetExtension().c_str());
    }
    typeStr.RemoveLast();
    return typeStr;
}




bool
XPuzzle::CanLoad(const wxString & ext)
{
    HandlerBase * handler = GetHandler(ext);
    return handler != NULL && handler->CanLoad();
}


bool
XPuzzle::CanSave(const wxString & ext)
{
    HandlerBase * handler = GetHandler(ext);
    return handler != NULL && handler->CanSave();
}



void
XPuzzle::AddHandler(HandlerBase * handler)
{
    HandlerBase * test = GetHandler(handler->GetExtension());
    if (test == NULL)
    {
        sm_handlers.push_back(handler);
    }
    else
    {
        wxLogDebug(_T("Attempting to add an already existing handler"));
        delete handler;
    }
}



HandlerBase *
XPuzzle::GetHandler(const wxString & ext)
{
    for (std::vector<HandlerBase *>::iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        if ((*it)->GetExtension() == ext.Lower())
            return *it;
    }
    return NULL;
}



void
XPuzzle::InitHandlers()
{
    AddHandler(new PuzHandler);
    AddHandler(new TxtHandler);
}

void
XPuzzle::CleanUpHandlers()
{
    for (std::vector<HandlerBase *>::iterator it = sm_handlers.begin();
         it != sm_handlers.end();
         ++it)
    {
        delete *it;
    }
}


std::vector<HandlerBase *> XPuzzle::sm_handlers;


//------------------------------------------------------------------------------
// XPuzzleModule
//------------------------------------------------------------------------------

class XPuzzleModule : public wxModule
{
public:
    XPuzzleModule() {}
    bool OnInit() { XPuzzle::InitHandlers(); return true; }
    void OnExit() { XPuzzle::CleanUpHandlers(); }

DECLARE_DYNAMIC_CLASS(XPuzzleModule)
};

IMPLEMENT_DYNAMIC_CLASS(XPuzzleModule, wxModule)