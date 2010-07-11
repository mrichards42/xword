// This file is part of XWord
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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

// Time a function (or more literally, a scope)
// Time is logged in the destructor
#ifdef __WXDEBUG__
#include <wx/stopwatch.h>
#include <wx/log.h>

class _TimeIt
{
public:
    _TimeIt(const wxString & name = _T("Function"))
        : m_name(name),
          m_sw()
    {}

    ~_TimeIt()
    {
        wxLogDebug(_T("%s took %d milliseconds"), m_name.c_str(), m_sw.Time());
    }

    wxStopWatch m_sw;
    wxString m_name;
};

// For debugging purposes
#define TimeIt(name) _TimeIt ___the_timer(name)

#else
#define TimeIt
#endif