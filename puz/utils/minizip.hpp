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

#ifndef PUZ_MINIZIP_WRAPPER_H
#define PUZ_MINIZIP_WRAPPER_H

// Wrapper for minizip


// ---------------------------------------------------------------------------
// ZIP
// ---------------------------------------------------------------------------

#include "minizip/zip.h"
#include <string>
#include <cassert>

namespace zip {

class Archive;

class File
{
    friend class Archive;
public:
    File(zipFile archive = NULL)
        : m_open(false)
    {
        SetArchive(archive);
    }

    ~File() { Close(); }

    bool Open(const std::string & filename);
    bool Close();

    int Write(const void * buf, unsigned int len);

    operator void * () const { return m_isOk ? m_archive : NULL; }

protected:
    zipFile m_archive;
    bool m_open;
    bool m_isOk;

    void SetArchive(zipFile archive);
};


class Archive
{
    friend class File;
public:
    Archive()
        : m_handle(NULL),
          m_isOk(false)
    {}

    explicit Archive(const std::string & filename)
        : m_handle(NULL),
          m_isOk(false)
    {
        Open(filename);
    }

    ~Archive() { Close(); }

    bool Open(const std::string & filename);

    // Open a file for writing in the zip archive
    File & OpenFile(const std::string & filename);

    bool Close();

    operator void * () const { return m_isOk ? m_handle : NULL; }

protected:
    zipFile m_handle;
    File m_currentFile;
    bool m_isOk;
};

} // namespace zip

// ---------------------------------------------------------------------------
// UNZIP
// ---------------------------------------------------------------------------

#include "minizip/unzip.h"

namespace unzip {

class Archive;

class File
{
    friend class Archive;
public:
    File(unzFile archive = NULL)
        : m_open(false)
    {
        SetArchive(archive);
    }

    ~File() { Close(); }

    bool Open();
    bool Close();
    std::string GetName();

    bool First();
    bool Next();

    int Read(void * buf, unsigned int len);

    operator void * () const { return m_isOk ? m_archive : NULL; }

protected:
    unzFile m_archive;
    bool m_open;
    bool m_isOk;

    void SetArchive(unzFile archive);
};


class Archive
{
    friend class File;
public:
    Archive()
        : m_handle(NULL),
          m_isOk(false)
    {}

    explicit Archive(const std::string & filename)
        : m_handle(NULL),
          m_isOk(false)
    {
        Open(filename);
    }

    ~Archive() { Close(); }

    bool Open(const std::string & filename);
    int GetFileCount();
    bool Close();

    File & First();

    operator void * () const { return m_isOk ? m_handle : NULL; }

protected:
    unzFile m_handle;
    File m_currentFile;
    bool m_isOk;
};

} // namespace unzip


#endif // PUZ_MINIZIP_WRAPPER_H

