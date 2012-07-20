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

enum XWordMessageId
{
    // Loading / saving messages
    MSG_CORRUPT_PUZ,
    MSG_CORRUPT_SECTION,
    MSG_PUZ_ERROR,
    MSG_STD_EXCEPTION,
    MSG_UNKNOWN_ERROR,
    MSG_SAVE_PUZ,
    MSG_DELETE_PUZ,

    // Check / reveal messages
    MSG_REVEAL_ALL,
    MSG_NO_INCORRECT,

    // Scrambling / unscrambling messages
    MSG_UNSCRAMBLE,
    MSG_SCRAMBLE,
    MSG_WRONG_KEY,

    // Internal use: the total number of messages
    MSG_TOTAL_MESSAGES
};


// Return the raw return value of the wxMessagebox
// (wxYES / wxNO / wxCANCEL / wxOK)
int XWordMessage(wxWindow * parent, XWordMessageId id, ...);
int XWordMessage(wxWindow * parent, const wxChar * fmt, ...);

// Return true if the wxMessageBox returns either wxYES or wxOK
bool XWordPrompt(wxWindow * parent, XWordMessageId id, ...);
bool XWordPrompt(wxWindow * parent, const wxChar * fmt, ...);

// Return the raw return value of the wxMessagebox
// The return value is pretty much meaningless, since an error dialog
// should only have an OK button anyways.
int XWordErrorMessage(wxWindow * parent, const wxChar * fmt, ...);

#endif // MY_MESSAGES_H
