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

#ifndef PUZ_EXCEPTIONS_H
#define PUZ_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace puz {

#define PUZ_EXCEPTION(name, subclass, default)            \
    class name : public subclass                          \
    {                                                     \
    public:                                               \
        explicit name(const std::string & msg = default)  \
            : subclass(msg)                               \
        {}                                                \
    }

// Base class
class Exception : public std::exception
{
public:
    explicit Exception(const std::string & a_message = "")
        : message(a_message)
    {}

    std::string message;

    virtual const char * what() const { return message.c_str(); }
};

// Square text exceptions
PUZ_EXCEPTION(InvalidString, Exception, "Invalid string");
PUZ_EXCEPTION(LongString, InvalidString, "Overlong string");
PUZ_EXCEPTION(NoSymbol, Exception, "Square does not have a symbol");

// Grid exceptions
PUZ_EXCEPTION(InvalidGrid, Exception, "Grid must be larger than 0 x 0");
PUZ_EXCEPTION(NoWord, Exception, "Start and end squares are not aligned");

// Clue exceptions
PUZ_EXCEPTION(InvalidClues, Exception, "Clues do not match the grid");

// Loading / saving errors
PUZ_EXCEPTION(FileError, Exception, "Unknown file error");
PUZ_EXCEPTION(FatalFileError, FileError, "Unknown fatal file error");

PUZ_EXCEPTION(HeaderError, FatalFileError, "Unknown file header error");
PUZ_EXCEPTION(VersionError, HeaderError, "Unreadable file version");
PUZ_EXCEPTION(FileMagicError, HeaderError, "Not a valid AcrossLite puz file");

PUZ_EXCEPTION(DataError, FileError, "Unknown non-fatal file data error");
PUZ_EXCEPTION(SectionError, DataError, "Puz section error");
PUZ_EXCEPTION(ChecksumError, DataError, "Puz checksum mismatch error");

PUZ_EXCEPTION(MissingHandler, Exception, "Unknown file type");

} // namespace puz


#endif // PUZ_EXCEPTIONS_H
