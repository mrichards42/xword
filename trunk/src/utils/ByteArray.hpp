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


#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include <vector>
#include <wx/string.h>

class ByteArray
    : public std::vector<unsigned char>
{
    typedef std::vector<unsigned char> _base;

public:
    // Conversion from string
    ByteArray(const wxString & str)
        : _base()
    {
        push_string(str);
    }

    // Standard std::vector constructors
    //----------------------------------
    ByteArray()
        : _base()
    {}

    explicit
    ByteArray(_base::size_type num, const unsigned char val = 0)
        : _base(num, val)
    {}

    template<class input_iterator>
    ByteArray(input_iterator start, input_iterator end)
        : _base(start, end)
    {}

    // Copy constructor
    ByteArray(const ByteArray & bytes)
        : _base(bytes)
    {}

    // General use functions
    //----------------------
    // Is the vector full of zeros?
    bool has_data() const
    {
        for (const_iterator it = begin(); it != end(); ++it)
            if (*it != 0)
                return true;
        return false;
    }

    // wxString helpers
    //-----------------
    void     push_string(const wxString & str, size_t nItems = wxString::npos);
    wxString to_string(size_t nItems = wxString::npos) const;
    void     append_to(wxString & str, size_t nItems = wxString::npos) const;
};


#endif // BYTE_ARRAY_H