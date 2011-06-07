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

namespace unzip {

#include "minizip/unzip.h"

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

    bool Open()
    {
        assert(! m_open);
        m_open = true;
        return unzOpenCurrentFile(m_archive) == UNZ_OK;

    }

    bool Close()
    {
        if (m_open)
        {
            m_open = false;
            return unzCloseCurrentFile(m_archive) == UNZ_OK;
        }
        return false;
    }

    std::string GetName()
    {
        unz_file_info info;
        char name[1024];
        unzGetCurrentFileInfo(m_archive, &info, name, 1024, NULL, 0, NULL, 0);
        return std::string(name);
    }

    bool First()
    {
        Close();
        m_isOk = unzGoToFirstFile(m_archive) == UNZ_OK;
        return m_isOk;
    }

    bool Next()
    {
        Close();
        m_isOk = unzGoToNextFile(m_archive) == UNZ_OK;
        return m_isOk;
    }

    int Read(void * buf, unsigned int len)
    {
        assert(m_open);
        return unzReadCurrentFile(m_archive, buf, len);
    }

    operator void * () const { return m_isOk ? m_archive : NULL; }

protected:
    unzFile m_archive;
    bool m_open;
    bool m_isOk;

    void SetArchive(unzFile archive)
    {
        Close();
        m_archive = archive;
        m_isOk = false;
        m_open = false;
    }
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

    bool Open(const std::string & filename)
    {
        Close();
        m_handle = unzOpen(filename.c_str());
        if (m_handle)
        {
            m_currentFile.SetArchive(m_handle);
            m_isOk = true;
            return true;
        }
        m_currentFile.SetArchive(NULL);
        m_isOk = false;
        return false;
    }

    int GetFileCount()
    {
        unz_global_info info;
        if (unzGetGlobalInfo(m_handle, &info) == UNZ_OK)
            return info.number_entry;
        return -1;
    }

    bool Close()
    {
        m_currentFile.SetArchive(NULL);
        m_isOk = false;
        if (m_handle)
        {
            int result = unzClose(m_handle);
            m_handle = NULL;
            return result == UNZ_OK;
        }
        return false;
    }

    File & First()
    {
        m_currentFile.First();
        assert(m_currentFile);
        return m_currentFile;
    }

    operator void * () const { return m_isOk ? m_handle : NULL; }

protected:
    unzFile m_handle;
    File m_currentFile;
    bool m_isOk;
};

} // namespace unzip

#endif // PUZ_MINIZIP_WRAPPER_H

