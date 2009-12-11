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

#ifndef TXT_HANDLER_H
#define TXT_HANDLER_H

#include "HandlerBase.hpp"
#include <wx/txtstrm.h>
#include <stack> // For holding extra lines.

// Rebus stuff
#include <set>
#include <map>
// ( ( long_entry, short_entry), circle? )
typedef std::pair< std::pair<wxString, char>, bool > RebusPair;
typedef std::set<RebusPair> RebusSet;
typedef std::map<RebusPair, char> RebusMap;

class TxtHandler : public HandlerBase
{
protected:
    void DoLoad();
    void DoSave();

    void LoadVersion1Grid();
    void LoadVersion2Grid();

    void WriteVersion1Grid();
    void GetRebusEntries(RebusSet & rebusEntries);
    void WriteVersion2Grid(const RebusSet & rebusEntries);

    wxString GetExtension()   const { return _T("txt"); }
    wxString GetDescription() const { return _T("Plain text format"); }
    bool CanLoad() const { return true; }
    bool CanSave() const { return true; }

private:
    wxTextInputStream * m_inText;

    enum
    {
        NO_STRIP_WHITESPACE,
        STRIP_WHITESPACE
    };

    wxString ReadLine(bool strip = STRIP_WHITESPACE);

    // Throw an exception if the next line does not equal name.
    void CheckSection(const wxString & name);

    // Return value indicates if the next line equals test.
    // If it does, the function eats the line.
    // Otherwise, it does not eat the line.
    bool TestLine(const wxString & test, bool strip = STRIP_WHITESPACE);

    void UngetLine(const wxString & line);

    // Store extra lines from TestLine
    std::stack<wxString> m_lastLines;

    size_t m_line;
};



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------

inline
wxString
TxtHandler::ReadLine(bool strip)
{
    wxString ret;

    // See if we have any lines on the stack already
    if (! m_lastLines.empty())
    {
        ret = m_lastLines.top();
        m_lastLines.pop();
    }
    else // Noting in the stack, read a line from the stream
        ret = m_inText->ReadLine();

    ++m_line;

    if (strip)
        return ret.Trim(true).Trim(false); // Left and right sides

    return ret;
}


inline
void
TxtHandler::CheckSection(const wxString & name)
{
    if (ReadLine() != name)
        throw PuzTypeError(_T("Missing %s header"), name.c_str());
}

inline
bool
TxtHandler::TestLine(const wxString & test, bool strip)
{
    // Only strip this line for the test.  In case whitespace characters are
    // meaningful, we should preserve them if we have to UngetLine().
    wxString line = ReadLine(NO_STRIP_WHITESPACE);

    if (strip == true)
    {
        if (line.Trim(true).Trim(false) == test)
            return true;
    }
    else
    {
        if (line == test)
            return true;
    }

    UngetLine(line);
    return false;
}


inline
void
TxtHandler::UngetLine(const wxString & line)
{
    wxASSERT(m_line != 0);
    --m_line;
    m_lastLines.push(line);
}

#endif // TXT_HANDLER_H