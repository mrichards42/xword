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

#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include "XPuzzle.hpp"
#include "../utils/ByteArray.hpp"
#include <vector>
#include <wx/string.h>
#include <wx/stream.h>


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


//==============================================================================
// The base class for all XWord file handlers
//
// Overload the following:
//   - wxString GetExtension() const   =0
//   - wxString GetDescription() const =0
//
// Overload each pair if applicable:
//   - bool CanLoad() const
//   - void DoLoad()
//
//   - bool CanSave() const
//   - void DoSave()
//==============================================================================

// This is a friend of XPuzzle, XGrid, and XSquare
class HandlerBase
{
public:
    HandlerBase()
        : m_puz(NULL),
          m_inStream(NULL),
          m_outStream(NULL)
    {}

    virtual ~HandlerBase() {}

    virtual bool CanLoad() const { return false; }
    virtual bool CanSave() const { return false; }
    virtual wxString GetExtension()   const = 0;
    virtual wxString GetDescription() const = 0;

    void Load(XPuzzle * puz, wxInputStream & stream)
    {
        wxASSERT(puz != NULL);
        m_puz = puz;
        m_inStream = &stream;
        DoLoad();
        m_inStream = NULL;
        m_puz = NULL;
    }

    void Save(XPuzzle * puz, wxOutputStream & stream)
    {
        wxASSERT(puz != NULL);
        m_puz = puz;
        m_outStream = &stream;
        DoSave();
        m_outStream = NULL;
        m_puz = NULL;
    }

    // Checksummming (static)
    //-----------------------
    static
    unsigned short cksum_region(const ByteArray & base, unsigned short cksum);


protected:
    // The real loading and saving functions
    //--------------------------------------
    virtual void DoLoad() { wxFAIL_MSG(_T("Not implemented.")); }
    virtual void DoSave() { wxFAIL_MSG(_T("Not implemented.")); }


    // Read functions
    //-------------------------------------
    void Read(void * buffer, size_t count);
    wxString ReadString();


    // Write functions
    //-------------------------------------
    void Write(const void * buffer, size_t count);
    void Write(const ByteArray & bytes) { Write(&bytes[0], bytes.size()); }

    // Internal use
    void CheckError(wxStreamBase & stream);

    // Members
    XPuzzle * m_puz;
    wxInputStream  * m_inStream;
    wxOutputStream * m_outStream;


    // XGrid functions
    //----------------
    void SetGridFlag (unsigned short flag)  { m_puz->m_grid.m_flag  = flag; }
    void SetGridType (unsigned short type)  { m_puz->m_grid.m_type  = type; }
    void SetGridCksum(unsigned short cksum) { m_puz->m_grid.m_cksum = cksum; }

    void SetupGrid() { m_puz->m_grid.SetupGrid(); }
    void SetupClues();
};



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------

inline
void
HandlerBase::Read(void * buffer, size_t count)
{
    m_inStream->Read(buffer, count);
    CheckError(*m_inStream);
}


inline
wxString
HandlerBase::ReadString()
{
    wxString str;
    unsigned char ch;
    Read(&ch, 1);
    while (ch != 0)
    {
       str.Append(static_cast<wxChar>(ch));
       Read(&ch, 1);
    }
    return str;
}


inline
void
HandlerBase::Write(const void * buffer, size_t count)
{
    m_outStream->Write(buffer, count);
    CheckError(*m_outStream);
}


// Check a stream for an error
inline
void
HandlerBase::CheckError(wxStreamBase & stream)
{
    const wxStreamError error = stream.GetLastError();

    if (error == wxSTREAM_NO_ERROR)
        return;

    if      (error == wxSTREAM_EOF)
        throw PuzLoadError(_T("Unexpected end of file"));
    else if (error == wxSTREAM_READ_ERROR)
        throw PuzLoadError(_T("Read error"));
    else if (error == wxSTREAM_WRITE_ERROR)
        throw PuzLoadError(_T("Write error"));
}

#endif // HANDLER_BASE_H
