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
    : wxComboBox(parent, id, wxEmptyString)
{
    SetMinSize(wxSize(150, -1));
    // Make sure we have a list of font faces
    if (! s_threadComplete)
        InitFacenames();

    // Set the first value
    if (faceName == wxEmptyString)
        SetValue(wxSWISS_FONT->GetFaceName());
    else
        SetValue(faceName);
    // Connect events
    Bind(wxEVT_KILL_FOCUS, &FontFaceCtrl::OnKillFocus, this);
    Bind(wxEVT_SET_FOCUS, &FontFaceCtrl::OnSetFocus, this);
    // I can't seem to get wxTE_PROCESS_ENTER and wxEVT_TEXT_ENTER to work,
    // so we'll circumvent that with a CHAR_HOOK event
    // Perhaps wxPropertySheetDialog is catching enter events?
    Bind(wxEVT_CHAR_HOOK, &FontFaceCtrl::OnCharHook, this);
}

void FontFaceCtrl::OnSetFocus(wxFocusEvent & evt)
{
    evt.Skip();
    // Set the combobox list contents
    wxCriticalSectionLocker lock(s_fontCS);
    wxString value = GetValue();
    Set(s_facenames);
    AutoComplete(s_facenames);
    wxComboBox::SetValue(value);
    if (s_threadComplete)
        Unbind(wxEVT_SET_FOCUS, &FontFaceCtrl::OnSetFocus, this);
}

void FontFaceCtrl::OnCharHook(wxKeyEvent & evt)
{
    if (evt.GetKeyCode() == WXK_RETURN || evt.GetKeyCode() == WXK_NUMPAD_ENTER)
        SetValue(GetValue());
    else
        evt.Skip();
}

void FontFaceCtrl::SetValue(const wxString & value)
{
    // Look for the closest facename in the list.
    wxString facename = value.Lower(); // case-insensitive
    bool hasMatch = false;
    int selection = -1;
    for (size_t i = 0; i < s_facenames.size(); ++i)
    {
        const wxString & item = s_facenames[i].Lower();
        if (item.StartsWith(facename))
        {
            m_lastFaceName = s_facenames[i];
            selection = i;
            break;
        }
        else if (facename.StartsWith(item))
        {
            m_lastFaceName = s_facenames[i];
            selection = i;
        }
    }
    // Set Value and send an event
    SetSelection(selection);
    wxComboBox::SetValue(m_lastFaceName);
    SendSelectionChangedEvent(wxEVT_COMBOBOX); // from wxControlWithItems
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
            FontFaceCtrl::s_threadComplete = false;
        }
        EnumerateFacenames(); // The real work is done in OnFacename
        {
            wxCriticalSectionLocker lock(s_fontCS);
            FontFaceCtrl::s_facenames.Sort(SortNoCase);
            if (TestDestroy())
                FontFaceCtrl::s_threadComplete = false;
            else
                FontFaceCtrl::s_threadComplete = true;
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

// Static thread functions
//------------------------

wxArrayString FontFaceCtrl::s_facenames;
bool FontFaceCtrl::s_threadComplete = false;
FontEnumeratorThread * FontFaceCtrl::s_thread = NULL;

void FontFaceCtrl::InitFacenames()
{
    wxCriticalSectionLocker lock(s_fontCS);
    // Only start this thread once
    if (s_thread != NULL)
        return;
    s_facenames.clear();
    s_thread = new FontEnumeratorThread;
    s_thread->Create();
    s_thread->Run();
}


void FontFaceCtrl::ClearFacenames()
{
    wxCriticalSectionLocker lock(s_fontCS);
    s_facenames.Clear();
    s_threadComplete = false;
    if (s_thread)
    {
        s_thread->Delete();
        s_thread = NULL;
    }
}
