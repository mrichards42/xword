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

// Internal helper methods for WX_DEFINE_VARARG_FUNC declarations.
namespace messagesPrivate {
    void DoXWordMessage(wxWindow * parent, const wxChar * fmt, ...);
    bool DoXWordPrompt(wxWindow * parent, const wxChar * fmt, ...);
    int DoXWordCancelablePrompt(wxWindow * parent, const wxChar * fmt, ...);
    void DoXWordErrorMessage(wxWindow * parent, const wxChar * fmt, ...);
}

// NOTE: WX_DEFINE_VARARG_FUNC takes two method implementations; the one we provide, which is used
// when wxWidgets uses the native wchar_t, and an alternative used if wxUSE_UNICODE_UTF8 == 1. Since
// wxWidgets 3.0+ uses the native representation by default on all platforms, we leave out the
// implementation for UTF-8, so this code will fail to compile if wxWidgets is compiled with
// wxUSE_UNICODE_UTF8 == 1.

// XWordMessage(parent, format, ...):
// Show a wxMessageBox with the given informational message.
WX_DEFINE_VARARG_FUNC_VOID(XWordMessage, 2, (wxWindow*, const wxFormatString&),
                           messagesPrivate::DoXWordMessage, UnusedUtf8Implementation);

// XWordPrompt(parent, format, ...):
// Show a wxMessageBox prompting the user with the given question.
// Returns whether the user said yes.
WX_DEFINE_VARARG_FUNC(bool, XWordPrompt, 2, (wxWindow*, const wxFormatString&),
                      messagesPrivate::DoXWordPrompt, UnusedUtf8Implementation);

// XWordCancelablePrompt(parent, format, ...):
// Show a wxMessageBox prompting the user with the given question for a cancelable action.
// Return the raw return value of the wxMessageBox (wxYES / wxNO / wxCANCEL).
WX_DEFINE_VARARG_FUNC(int, XWordCancelablePrompt, 2, (wxWindow*, const wxFormatString&),
                      messagesPrivate::DoXWordCancelablePrompt, UnusedUtf8Implementation);

// XWordErrorMessage(parent, format, ...):
// Show a wxMessageBox with the given error message.
WX_DEFINE_VARARG_FUNC_VOID(XWordErrorMessage, 2, (wxWindow*, const wxFormatString&),
                           messagesPrivate::DoXWordErrorMessage, UnusedUtf8Implementation);

#endif // MY_MESSAGES_H
