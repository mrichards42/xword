// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#include <wx/wxprec.h>
#include "fontface.hpp"
#include <wx/fontenum.h>
#include <wx/textctrl.h>

#include <wx/thread.h>

// A critical section for our font thread
static wxCriticalSection s_fontCS;

// ----------------------------------------------------------------------------
// The ComboCtrl class
// ----------------------------------------------------------------------------

FontFaceCtrl::FontFaceCtrl(wxWindow * parent, wxWindowID id,
                           const wxString & faceName)
    : VirtualComboBox(parent, id, wxEmptyString)
{
    // Make sure we have a list of font faces
    if (! s_facenamesComplete)
        InitFaceNames();

    SetPopupMinWidth(200);

    // Set the first value
    if (faceName == wxEmptyString)
        SetValue(wxSWISS_FONT->GetFaceName());
    else
        SetValue(faceName);
    // Connect events
    GetTextCtrl()->Connect(
        wxEVT_KILL_FOCUS,
        wxFocusEventHandler(FontFaceCtrl::OnKillFocus),
        NULL, this);
}


void FontFaceCtrl::SetValue(const wxString & value)
{
    wxCriticalSectionLocker lock(s_fontCS);
    wxString facename = value.Lower(); // case-insensitive
    // Look for the closest facename in the list.
    bool hasMatch = false;
    for (size_t i = 0; i < s_facenames.size(); ++i)
    {
        const wxString & item = s_facenames[i].Lower();
        if (item.StartsWith(facename))
        {
            m_lastFaceName = s_facenames[i];
            break;
        }
        else if (facename.StartsWith(item))
        {
            m_lastFaceName = s_facenames[i];
        }
    }
    VirtualComboBox::SetValue(m_lastFaceName);
}


// ---------------------------------------------------------------------------
// wxFontEnumerator thread
// ---------------------------------------------------------------------------

int SortNoCase(const wxString & a, const wxString & b)
{
    return a.CmpNoCase(b);
}

class FontEnumeratorThread : public wxThread, public wxFontEnumerator
{
public:
    FontEnumeratorThread()
        : wxThread(wxTHREAD_DETACHED),
          wxFontEnumerator()
    {}

    // The thread entry point
    virtual ExitCode Entry()
    {
        {
            wxCriticalSectionLocker lock(s_fontCS);
            FontFaceCtrl::s_facenames.clear();
            FontFaceCtrl::s_facenamesComplete = false;
        }
        EnumerateFacenames(); // The real work is done in OnFacename
        {
            wxCriticalSectionLocker lock(s_fontCS);
            FontFaceCtrl::s_facenames.Sort(SortNoCase);
            if (TestDestroy())
                FontFaceCtrl::s_facenamesComplete = false;
            else
                FontFaceCtrl::s_facenamesComplete = true;
            FontFaceCtrl::s_thread = NULL;
        }
        return (ExitCode)0;
    }

    // The font enumerator callback
    virtual bool OnFacename(const wxString & font)
    {
        if (TestDestroy()) // We've been asked to exit
            return false;
        if (! font.StartsWith(_T("@")))
        {
            wxCriticalSectionLocker lock(s_fontCS);
            FontFaceCtrl::s_facenames.push_back(font);
        }
        return true;
    }
};

// The facenames array
//--------------------

wxArrayString FontFaceCtrl::s_facenames;
bool FontFaceCtrl::s_facenamesComplete = false;
FontEnumeratorThread * FontFaceCtrl::s_thread = NULL;

void FontFaceCtrl::InitFaceNames()
{
    {
        wxCriticalSectionLocker lock(s_fontCS);
        if (s_thread != NULL)
            return;
        s_facenames.clear();
        s_thread = new FontEnumeratorThread;
        s_thread->Create();
        s_thread->Run();
    }
}


void FontFaceCtrl::ClearFaceNames()
{
    wxCriticalSectionLocker lock(s_fontCS);
    s_facenames.Clear();
    s_facenamesComplete = false;
    if (s_thread)
    {
        s_thread->Delete();
        s_thread = NULL;
    }
}


// Find items
//-----------
int FontFaceCtrl::FindItem(const wxString & s) const
{
    wxCriticalSectionLocker lock(s_fontCS);
    return s_facenames.Index(s);
}

wxString FontFaceCtrl::GetItem(size_t n) const
{
    wxCriticalSectionLocker lock(s_fontCS);
    return s_facenames[n];
}

size_t FontFaceCtrl::GetCount() const
{
    wxCriticalSectionLocker lock(s_fontCS);
    return s_facenames.size();
}
