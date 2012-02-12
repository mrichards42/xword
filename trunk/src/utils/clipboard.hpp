// This file is part of XWord
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#ifndef MY_CLIPBOARD_H
#define MY_CLIPBOARD_H

#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <vector>
#include "puz/word.hpp"
#include "utils/string.hpp"

// A class for copying the text from a word of squares.
// Data is stored as a series of tab-delimited strings and converted
// to a series of tokens on the fly.

class WordDataObject : public wxTextDataObject
{
public:
    WordDataObject(const puz::Word * word = NULL)
    {
        SetFormat(wxDataFormat(_T("XWord 0.5 word")));
        if (word)
        {
            wxString str;
            puz::square_iterator square;
            for (square = word->begin(); square != word->end(); ++square)
                str << puz2wx(square->GetText()) << _T('\t');
            SetText(str);
        }
    }

    WordDataObject(const puz::Square * square)
        : wxTextDataObject(square->GetText() + _T('\t'))
    {
        SetFormat(wxDataFormat(_T("XWord 0.5 word")));
    }

    wxStringTokenizer GetTokens() const
    {
        return wxStringTokenizer(GetText(), _T("\t"));
    }
};


class XWordTextDataObject : public wxDataObjectComposite
{
public:
    XWordTextDataObject(const puz::Word * word = NULL)
    {
        if (word)
        {
            Add(new WordDataObject(word), true);
            wxString str;
            puz::square_iterator square;
            for (square = word->begin(); square != word->end(); ++square)
                str << puz2wx(square->GetText());
            Add(new wxTextDataObject(str));
        }
    }

    XWordTextDataObject(const puz::Square * square)
    {
        Add(new WordDataObject(square), true);
        Add(new wxTextDataObject(square->GetText()));
    }
};

#endif // MY_CLIPBOARD_H