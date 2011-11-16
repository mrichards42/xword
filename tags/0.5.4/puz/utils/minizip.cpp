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

#include "minizip.hpp"

// ---------------------------------------------------------------------------
// UNZIP
// ---------------------------------------------------------------------------
namespace unzip {

bool File::Open()
{
    assert(! m_open);
    m_open = true;
    return unzOpenCurrentFile(m_archive) == UNZ_OK;
}

bool File::Close()
{
    if (m_open)
    {
        m_open = false;
        return unzCloseCurrentFile(m_archive) == UNZ_OK;
    }
    return false;
}

std::string File::GetName()
{
    unz_file_info info;
    char name[1024];
    unzGetCurrentFileInfo(m_archive, &info, name, 1024, NULL, 0, NULL, 0);
    return std::string(name);
}

bool File::First()
{
    Close();
    m_isOk = unzGoToFirstFile(m_archive) == UNZ_OK;
    return m_isOk;
}

bool File::Next()
{
    Close();
    m_isOk = unzGoToNextFile(m_archive) == UNZ_OK;
    return m_isOk;
}

int File::Read(void * buf, unsigned int len)
{
    assert(m_open);
    return unzReadCurrentFile(m_archive, buf, len);
}


void File::SetArchive(unzFile archive)
{
    Close();
    m_archive = archive;
    m_isOk = false;
    m_open = false;
}


bool Archive::Open(const std::string & filename)
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

int Archive::GetFileCount()
{
    unz_global_info info;
    if (unzGetGlobalInfo(m_handle, &info) == UNZ_OK)
        return info.number_entry;
    return -1;
}

bool Archive::Close()
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

File & Archive::First()
{
    m_currentFile.First();
    assert(m_currentFile);
    return m_currentFile;
}

} // namespace unzip


// ---------------------------------------------------------------------------
// ZIP
// ---------------------------------------------------------------------------
namespace zip {

bool File::Open(const std::string & filename)
{
    assert(! m_open);
    m_open = true;
    zip_fileinfo info;
    m_isOk = zipOpenNewFileInZip(m_archive, filename.c_str(),
                               &info, NULL, 0, NULL, 0,
                               NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION)
        == UNZ_OK;
    return m_isOk;
}

bool File::Close()
{
    if (m_open)
    {
        m_open = false;
        return zipCloseFileInZip(m_archive) == UNZ_OK;
    }
    return false;
}

int File::Write(const void * buf, unsigned int len)
{
    assert(m_open);
    return zipWriteInFileInZip(m_archive, buf, len);
}

void File::SetArchive(zipFile archive)
{
    Close();
    m_archive = archive;
    m_isOk = false;
    m_open = false;
}


bool Archive::Open(const std::string & filename)
{
    Close();
    m_handle = zipOpen(filename.c_str(), APPEND_STATUS_CREATE);
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

// Open a file for writing in the zip archive
File & Archive::OpenFile(const std::string & filename)
{
    m_currentFile.Close();
    m_currentFile.Open(filename);
    return m_currentFile;
}

bool Archive::Close()
{
    m_currentFile.SetArchive(NULL);
    m_isOk = false;
    if (m_handle)
    {
        int result = zipClose(m_handle, "");
        m_handle = NULL;
        return result == UNZ_OK;
    }
    return false;
}

} // namespace zip

