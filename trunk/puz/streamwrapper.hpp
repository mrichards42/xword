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

#ifndef PUZ_STREAM_WRAPPER_H
#define PUZ_STREAM_WRAPPER_H

#include <iostream>
#include <string>

namespace puz {

class istream_wrapper
{
public:
    istream_wrapper(std::istream & stream)
        : m_stream(stream)
    {
        // Throw an exception on errors
        m_stream.exceptions(std::ios::badbit |
                            std::ios::failbit |
                            std::ios::eofbit);
    }

    std::istream & m_stream;

    void Skip(size_t bytes)
    {
        m_stream.seekg(bytes, std::ios::cur);
    }

    unsigned char ReadChar()
    {
        return static_cast<unsigned char>(m_stream.get());
    }

    void ReadCharArray(unsigned char * chars, size_t size)
    {
        m_stream.read(reinterpret_cast<char*>(chars), size);
    }

    unsigned short ReadShort()
    {
        // Always little-endian
        const unsigned short lo_byte = ReadChar();
        const unsigned short hi_byte = ReadChar();
        return (hi_byte << 8) + lo_byte;
    }

    // Read length or -1 for the rest
    std::string ReadString(int length)
    {
        std::string str;
        if (length != -1)
        {
            str.reserve(length);
            for (size_t i = 0; i < length; ++i)
                str.push_back(static_cast<char>(m_stream.get()));
        }
        else // length == -1
        {
             // Read the rest
            for (;;)
            {
                try
                {
                    str.push_back(static_cast<char>(m_stream.get()));
                }
                catch (std::ios::failure &)
                {
                    if (m_stream.eof())
                        break;
                    else
                        throw;
                }
            }
        }
        return str;
    }

    std::string ReadString(char delim = '\0')
    {
        std::string str;
        std::getline(m_stream, str, delim);
        return str;
    }

    std::string ReadLine(char delim = '\n')
    {
        return ReadString(delim);
    }

    bool CheckEof()
    {
        try
        {
            m_stream.putback(m_stream.get());
            return false;
        }
        catch (std::ios::failure &)
        {
            return true;
        }
    }
};


class ostream_wrapper
{
public:
    ostream_wrapper(std::ostream & stream)
        : m_stream(stream)
    {
        // Throw an exception on errors
        m_stream.exceptions(std::ios::badbit |
                            std::ios::failbit |
                            std::ios::eofbit);
    }

    std::ostream & m_stream;

    void Skip(size_t bytes)
    {
        for (size_t i = 0; i < bytes; ++i)
            m_stream.put(0);
    }

    void Put(unsigned char ch)
    {
        m_stream.put(static_cast<char>(ch));
    }

    void Write(const char * chars, size_t size)
    {
        m_stream.write(chars, size);
    }

    void Write(const unsigned char * chars, size_t size)
    {
        m_stream.write(reinterpret_cast<const char*>(chars), size);
    }

    void Write(unsigned short num)
    {
        // Always little-endian
        const unsigned char lo_byte = num & 0x00ff;
        const unsigned char hi_byte = (num & 0xff00) >> 8;
        m_stream.put(lo_byte);
        m_stream.put(hi_byte);
    }

    void Write(const std::string & str)
    {
        m_stream.write(str.c_str(), str.size());
    }

    void WriteNulTerminated(const std::string & str)
    {
        Write(str);
        m_stream.put(0);
    }
};


} // namespace puz

#endif // PUZ_STREAM_HELPER_H
