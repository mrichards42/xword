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

#ifndef HANDLER_COMMON_H
#define HANDLER_COMMON_H

// Define some common file and XPuzzle operations

#include <wx/string.h>
#include <map>
#include "XPuzzle.hpp"


// This needs to come before #include XPuzzle.hpp
// An array to use instead of unsigned char *
#include <vector>
typedef std::vector<unsigned char> ByteArray;


void BytesToString(const ByteArray & bytes,
                   wxString * str,
                   size_t nItems = wxString::npos);

void StringToBytes(const wxString & str,
                   ByteArray * bytes,
                   size_t nItems = wxString::npos);



class wxInputStream;
class wxOutputStream;


// A load/save exception
class PuzLoadError
{
public:
    PuzLoadError(const wxChar *format, ...)
        : isProcessed(false)
    {
        va_list argptr;
        va_start(argptr, format);

        message.PrintfV(format, argptr);

        va_end(argptr);
    }

    wxString message;
    bool isProcessed;
};


// Read/Write functions (for exceptions)
//---------------------------------------
void Read(wxInputStream & stream, void * buffer, size_t count)    throw(...);
wxString ReadString(wxInputStream & stream)                       throw(...);
wxString ReadString(wxInputStream & stream, size_t len)           throw(...);

void Write(wxOutputStream & stream,
           const void * buffer, size_t count)                     throw(...);
void WriteString(wxOutputStream & stream, const wxString & str)   throw(...);


// XPuzzle functions
//------------------

// Load functions
void SetGridSolution(XPuzzle * puz, const ByteArray & solution);
void SetGridText    (XPuzzle * puz, const ByteArray & text);
void SetGext        (XPuzzle * puz, const ByteArray & gext);

inline void SetupGrid      (XPuzzle * puz) { puz->m_grid.SetupGrid(); }
void SetupClues            (XPuzzle * puz);

// Rebus
void SetRebusSolution(XPuzzle * puz,
                      const ByteArray & table,
                      const ByteArray & grid);

void SetRebusUserGrid(XPuzzle * puz, const ByteArray & rebus);



// Save functions
wxString GetGridSolution(XPuzzle * puz);
wxString GetGridText    (XPuzzle * puz);
// Return true means that there is data (besides 0) in gext
bool     GetGext        (XPuzzle * puz, ByteArray * gext);


// Checksumming (template for string and vector)
//----------------------------------------------
template <typename T>
unsigned short
cksum_region(const T & base, unsigned short cksum)
{
    const size_t len = base.size();

    for (size_t i = 0; i < len; ++i)
    {
        if ((cksum & 1) != 0)
            cksum = (cksum >> 1) + 0x8000;
        else
            cksum = cksum >> 1;
        cksum += base.at(i);
    }

    return cksum;
}

inline
unsigned short
cksum_string(const wxString & str, unsigned short cksum)
{
    return cksum_region(str, cksum);
}

#endif // HANDLER_COMMON_H
