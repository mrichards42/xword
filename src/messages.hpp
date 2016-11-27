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

#ifndef MY_MESSAGES_H
#define MY_MESSAGES_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

// XWordMessage(parent, message):
// Show a wxMessageBox with the given informational message.
void XWordMessage(wxWindow * parent, const wxString & message);

// XWordPrompt(parent, message):
// Show a wxMessageBox prompting the user with the given question.
// Returns whether the user said yes.
bool XWordPrompt(wxWindow * parent, const wxString & message);

// XWordCancelablePrompt(parent, message):
// Show a wxMessageBox prompting the user with the given question for a cancelable action.
// Return the raw return value of the wxMessageBox (wxYES / wxNO / wxCANCEL).
int XWordCancelablePrompt(wxWindow * parent, const wxString & message);

// XWordErrorMessage(parent, message):
// Show a wxMessageBox with the given error message.
void XWordErrorMessage(wxWindow * parent, const wxString & message);

#endif // MY_MESSAGES_H
