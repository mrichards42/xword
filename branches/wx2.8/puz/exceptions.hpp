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

#ifndef PUZ_EXCEPTIONS_H
#define PUZ_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Exceptions work with 8-bit strings

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

	virtual ~Exception() throw() {}

    std::string message;

    virtual const char * what() const throw() { return message.c_str(); }
};

// String exceptions
PUZ_EXCEPTION(InvalidString, Exception, "Invalid string");
PUZ_EXCEPTION(LongString, InvalidString, "Overlong string");
PUZ_EXCEPTION(NoSymbol, Exception, "Square does not have a symbol");
PUZ_EXCEPTION(InvalidEncoding, InvalidString, "Invalid encoding");

// Grid exceptions
PUZ_EXCEPTION(InvalidDirection, Exception, "Direction expected to be Across or Down");
PUZ_EXCEPTION(InvalidGrid, Exception, "Grid must be larger than 0 x 0");
PUZ_EXCEPTION(InvalidGridCell, Exception, "Unknown error parsing grid cell.");
PUZ_EXCEPTION(NoWord, Exception, "Start and end squares are not aligned");

// Word exceptions
PUZ_EXCEPTION(InvalidWord, Exception, "Words must be vertical, horizontal, or diagonal");

// Clue exceptions
PUZ_EXCEPTION(InvalidClues, Exception, "Clues do not match the grid");
PUZ_EXCEPTION(NoClues,      Exception, "Unknown clue direction");

// Loading / saving errors
// This sucks . . . the lua import package relies on the specific wording here.
class FileTypeError : public Exception
{
public:
    FileTypeError() : Exception("Wrong file type") {}

    explicit FileTypeError (const std::string & type)
        : Exception(std::string("Not a valid ") + type + " file")
    {}
};

class FileError : public Exception
{
public:
    explicit FileError (const std::string & filename)
        : Exception(std::string("Unable to open file ") + filename)
    {}
};

PUZ_EXCEPTION(LoadError, Exception, "Error loading puzzle");
PUZ_EXCEPTION(ConversionError, Exception, "Puzzle cannot be fully represented in this format");
PUZ_EXCEPTION(MissingHandler, Exception, "Unknown file type");

} // namespace puz


#endif // PUZ_EXCEPTIONS_H
