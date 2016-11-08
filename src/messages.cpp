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

#include "App.hpp"
#include "messages.hpp"
#include <wx/msgdlg.h>


//-----------------------------------------------------------------------------
// Message / Prompt / Error functions
//-----------------------------------------------------------------------------

// Make message formatting a little cleaner
#define _FORMAT_MESSAGE(lastparam, format, strname)                  \
        va_list argptr;                                              \
        va_start(argptr, lastparam);                                 \
        const wxString strname = wxString::FormatV(format, argptr);  \
        va_end(argptr);

namespace messagesPrivate
{

void DoXWordMessage(wxWindow * parent, const wxChar * fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    wxMessageBox(message, XWORD_APP_NAME _T(" Message"), wxOK | wxICON_INFORMATION, parent);
}

bool DoXWordPrompt(wxWindow * parent, const wxChar * fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    const int ret = wxMessageBox(message, XWORD_APP_NAME _T(" Message"), wxYES_NO | wxICON_QUESTION,
                                 parent);
    return ret == wxYES || ret == wxOK;
}

int DoXWordCancelablePrompt(wxWindow * parent, const wxChar * fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    return wxMessageBox(message, XWORD_APP_NAME _T(" Message"), wxYES_NO | wxICON_QUESTION | wxCANCEL,
                        parent);
}

void DoXWordErrorMessage(wxWindow * parent, const wxChar * fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    wxMessageBox(message, XWORD_APP_NAME _T(" Error"), wxOK | wxICON_ERROR, parent);
}

} // namespace messagesPrivate

#undef _FORMAT_MESSAGE
