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

#include "messages.hpp"
#include <wx/msgdlg.h>

//-----------------------------------------------------------------------------
// Message definition
//-----------------------------------------------------------------------------

// Data for each message
struct MessageDesc
{
    const wxChar * message;
    const wxChar * title;
    int flags;
};

// A generic error
const wxString errorTitle = _T("XWord Error");
const int errorFlags = wxOK | wxICON_ERROR;

// A generic message
const wxString messageTitle = _T("XWord Message");
const int messageFlags = wxOK | wxICON_INFORMATION;

// A generic question
const wxString questionTitle = _T("XWord Message");
const int questionFlags = wxYES_NO | wxICON_QUESTION;


// Messages *must* be in the same order they are defined in messages.hpp
const MessageDesc messageTable [] = {

    // MSG_CORRUPT_PUZ
    {
        _T("This puzzle seems to be corrupt.\n")
        _T("Load it anyway?"),
        errorTitle,
        wxYES_NO | wxICON_ERROR
    },

    // MSG_CORRUPT_SECTION
    {
        _T("Some parts of this puzzle are corrupt, but the basic puzzle information is intact.\n")
        _T("Load it anyway?"),
        errorTitle,
        wxYES_NO | wxICON_ERROR
    },

    // MSG_PUZ_ERROR
    {
        _T("%s"),
        errorTitle,
        errorFlags
    },

    // MSG_STD_EXCEPTION
    {
        _T("%s"),
        errorTitle,
        errorFlags
    },

    // MSG_UNKNOWN_ERROR
    {
        _T("Unknown error."),
        errorTitle,
        errorFlags
    },

    // MSG_SAVE_PUZ
    {
        _T("Current Puzzle not saved.  Save before closing?"),
        questionTitle,
        wxYES_NO | wxCANCEL | wxICON_QUESTION
    },

    // MSG_REVEAL_ALL
    {
        _T("This will reveal the entire grid.  Continue?"),
        questionTitle,
        questionFlags
    },

    // MSG_NO_INCORRECT,
    {
        
        _T("No Incorrect Letters!"),
        messageTitle,
        messageFlags
    },

    // MSG_UNSCRAMBLE
    {
        _T("Solution unscrambled!"),
        messageTitle,
        messageFlags
    },

    // MSG_SCRAMBLE
    {
        _T("Solution scrambled.  Key is %d"),
        messageTitle,
        messageFlags
    },

    // MSG_WRONG_KEY
    {
        _T("Wrong Key!"),
        messageTitle,
        messageFlags
    }
};

wxCOMPILE_TIME_ASSERT(sizeof(messageTable) / sizeof(MessageDesc) == MSG_TOTAL_MESSAGES,
                      Bad_messageTable);


//-----------------------------------------------------------------------------
// Message / Prompt functions
//-----------------------------------------------------------------------------

// Make message formatting a little cleaner
#define _FORMAT_MESSAGE(lastparam, format, strname)                  \
        va_list argptr;                                              \
        va_start(argptr, lastparam);                                 \
        const wxString strname = wxString::FormatV(format, argptr);  \
        va_end(argptr);

int XWordMessage(XWordMessageId id, ...)
{
    const MessageDesc & desc = messageTable[id];
    _FORMAT_MESSAGE(id, desc.message, message);
    return wxMessageBox(message, desc.title, desc.flags);
}

int XWordMessage(const wxString & fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    return wxMessageBox(message, messageTitle, messageFlags);
}


bool XWordPrompt(XWordMessageId id, ...)
{
    const MessageDesc & desc = messageTable[id];
    // I think this function should work with the variable arguments
    // as is . . . not entirely sure though.
    _FORMAT_MESSAGE(id, desc.message, message);
    const int ret = wxMessageBox(message, desc.title, desc.flags);
    return (ret == wxYES || ret == wxOK);
}

bool XWordPrompt(const wxString & fmt, ...)
{
    _FORMAT_MESSAGE(fmt, fmt, message);
    const int ret = wxMessageBox(message, questionTitle, questionFlags);
    return (ret == wxYES || ret == wxOK);
}

#undef _FORMAT_MESSAGE
