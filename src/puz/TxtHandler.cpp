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


#include "TxtHandler.hpp"
#include <wx/tokenzr.h>

// Characters allowed in the grid
static const wxString allowedCharsV1 = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
                                       _T("abcdefghijklmnopqrstuvwxyz")
                                       _T(".:");

static const wxString allowedCharsV2 = allowedCharsV1 + 
                                       _T("0123456789")
                                       _T("@#$%&+?");


// Checks
void CheckRebusMarker(const wxArrayString & marker);

void CheckGridLine(const wxString & line,
                   const wxString allowedChars,
                   size_t width);



//------------------------------------------------------------------------------
// DoLoad implementation
//------------------------------------------------------------------------------

void
TxtHandler::DoLoad()
{
    // This try . . . catch block is only here so we can append line information
    // to any exceptions.
    try
    {
        m_line = 0;

        wxTextInputStream stream(*m_inStream, _T("\t"), wxConvISO8859_1);
        m_inText = &stream;

        wxString versionStr = ReadLine();
        int version;
        if      (versionStr == _T("<ACROSS PUZZLE>"))
            version = 1;
        else if (versionStr == _T("<ACROSS PUZZLE V2>"))
            version = 2;
        else
            throw PuzLoadError(
                _T("Missing <ACROSS PUZZLE> or <ACROSS PUZZLE V2> header.") );


        // Everything is the same between the two versions except the grid.
        CheckSection(_T("<TITLE>"));
        m_puz->m_title = ReadLine();

        CheckSection(_T("<AUTHOR>"));
        m_puz->m_author = ReadLine();

        CheckSection(_T("<COPYRIGHT>"));
        m_puz->m_copyright = ReadLine();
        // Prepend a (c) to the copyright string
        if (! m_puz->m_copyright.empty())
            m_puz->m_copyright.Prepend(_T("© "));

        CheckSection(_T("<SIZE>"));
        { // Enter scope
            wxString size = ReadLine();
            size_t x_index = size.find(_T("x"));
            if (x_index == wxString::npos)
                throw PuzLoadError(_T("Missing 'x' in size specification"));
            long width, height;
            if (! (size.Mid(0, x_index).ToLong(&width)
                   && size.Mid(x_index+1).ToLong(&height)) )
                throw PuzLoadError(_T("Improper size specification."));

            m_puz->m_grid.SetSize(width, height);
        } // Exit scope

        // Grid is different between versions
        if      (version == 1)
            LoadVersion1Grid();
        else if (version == 2)
            LoadVersion2Grid();

        SetupGrid();


        // Count across and down clues
        size_t nAcross, nDown;
        m_puz->m_grid.CountClues(&nAcross, &nDown);

        std::vector<wxString> across;
        std::vector<wxString> down;

        across.reserve(nAcross);
        across.reserve(nDown);

        CheckSection(_T("<ACROSS>"));
        for (size_t i = 0; i < nAcross; ++i)
        {
            across.push_back(ReadLine());
            if (across.back() == _T("<DOWN>")
                || across.back().empty())
                throw PuzLoadError(_T("Missing some across clues"));
        }

        CheckSection(_T("<DOWN>"));
        for (size_t i = 0; i < nDown; ++i)
        {
            down.push_back(ReadLine());
            if (down.back().empty())
                throw PuzLoadError(_T("Missing some down clues"));
        }

        // Read across and down into m_clues
        std::vector<wxString>::const_iterator across_it = across.begin();
        std::vector<wxString>::const_iterator down_it   = down.begin();

        for (XSquare * square = m_puz->m_grid.First();
             square != NULL;
             square = square->Next())
        {
            if ( (square->GetClueFlag() & ACROSS_CLUE) != 0)
                m_puz->m_clues.push_back(*across_it++);
            if ( (square->GetClueFlag() & DOWN_CLUE) != 0)
                m_puz->m_clues.push_back(*down_it++);
        }

        SetupClues();


        wxString header = ReadLine();
        if (header == _T("<NOTEPAD>"))
        {
            // Read the rest of the file into the notepad section
            for (;;)
            {
                // Test the end of the file
                m_inStream->Peek();
                if (m_inStream->GetLastError() == wxSTREAM_EOF)
                    break;

                m_puz->m_notes.append( ReadLine(NO_STRIP_WHITESPACE) );
            }
        }
        else if (! header.empty())
            throw PuzLoadError(_T("Unrecognized line(s) at end of file."));
    }
    catch (PuzLoadError & error)
    {
        // Add line information to the exception
        error.message.append(wxString::Format(_T("\nAt line: %d"), m_line));
        throw;
    }
}


void
TxtHandler::LoadVersion1Grid()
{
    CheckSection(_T("<GRID>"));

    wxString solution;
    for (size_t i = 0; i < m_puz->m_grid.GetHeight(); ++i)
    {
        wxString line = ReadLine();

        CheckGridLine(line, allowedCharsV1, m_puz->m_grid.GetWidth());

        solution.append(line);
    }
    SetGridSolution(solution);
}



void
TxtHandler::LoadVersion2Grid()
{
    // Read the grid the same way as before (but with different allowedChars)
    CheckSection(_T("<GRID>"));

    wxString solution;
    for (size_t i = 0; i < m_puz->m_grid.GetHeight(); ++i)
    {
        wxString line = ReadLine();

        CheckGridLine(line, allowedCharsV2, m_puz->m_grid.GetWidth());

        solution.append(line);
    }


    // Read the <REBUS> section (optional)
    //------------------------------------
    if ( TestLine(_T("<REBUS>")) )
    {
        wxString line = ReadLine();

        // Read flag lines
        //----------------
        if (line.find(_T(";")) != wxString::npos)
        {
            // Across lite only supports a MARK flag for now.
            // If the line doesn't start with "MARK", Across Lite won't read
            // the file.
            // We'll be a little more lenient here, and split the line by ';'
            // so that more flags can be used in the future if needed.

            wxArrayString flags =
                wxStringTokenize(line, _T("; "), wxTOKEN_STRTOK );

            for (wxArrayString::iterator it = flags.begin();
                 it != flags.end();
                 ++it)
            {
                // MARK: circle all lowercase letters
                if (*it == _T("MARK"))
                {
                    ByteArray gext;
                    for (wxString::iterator str_it = solution.begin();
                         str_it != solution.end();
                         ++str_it)
                    {
                        if (wxIslower(*str_it))
                            gext.push_back(XFLAG_CIRCLE);
                        else
                            gext.push_back(XFLAG_CLEAR);
                    }
                    if (gext.has_data())
                        SetGext(gext);
                }
            }
        }
        else if (line.find(_T(":")) == wxString::npos)
            throw PuzLoadError(_T("No entries in <REBUS> section."));
        else // This is a marker line
            UngetLine(line);


        // Read rebus markers
        //-------------------
        std::map<unsigned char, wxString> table;
        std::map<unsigned char, wxChar> asciiTable;

        for (;;)
        {
            line = ReadLine();
            if (line.find(_T(":")) == wxString::npos)
            {
                UngetLine(line);
                break;
            }

            // Add the marker to the rebus table.
            wxArrayString marker =
                wxStringTokenize(line, _T(": "), wxTOKEN_STRTOK );

            CheckRebusMarker(marker);

            const unsigned char index =
                static_cast<unsigned char>( marker.Item(0).at(0) );

            table.insert(      std::make_pair(index, marker.Item(1)) );
            asciiTable.insert( std::make_pair(index, marker.Item(2).at(0)) );
        }

        // Replace entries in the solution grid with the markers
        //------------------------------------------------------
        ByteArray grid;
        for (wxString::iterator it = solution.begin();
             it != solution.end();
             ++it)
        {
            // Look for the letter in the rebus table
            std::map<unsigned char, wxString>::iterator table_it;
            table_it = table.find(*it);

            // This square has a rebus
            if (table_it != table.end())
            {
                grid.push_back(table_it->first);
                *it = asciiTable[table_it->first];
            }
            else
                grid.push_back(0);
        }
        SetGridSolution(solution);
        SetSolutionRebus(table, grid);
    }
    else // No rebus section
        SetGridSolution(solution);
}



//------------------------------------------------------------------------------
// Checks
//------------------------------------------------------------------------------

void
CheckGridLine(const wxString & line,
              const wxString allowedChars,
              size_t width)
{
    if (line.length() < width)
        throw PuzLoadError(_T("Missing letters in grid."));

    if (line.length() > width)
        throw PuzLoadError(_T("Extra letters in grid."));

    // Make sure all the characters are allowed to be in the grid
    size_t index = line.find_first_not_of(allowedCharsV2);
    if (index != wxString::npos)
        throw PuzLoadError(_T("Invalid character in grid: \"%c\"."),
                           line.at(index));
}



void
CheckRebusMarker(const wxArrayString & marker)
{
    if (marker.size() < 3)
        throw PuzLoadError(_T("Missing marker entries in <REBUS>."));
    if (marker.size() > 3)
        throw PuzLoadError(_T("Extra marker entries in <REBUS>."));

    if (marker.Item(0).length() > 1)
        throw PuzLoadError(_T("<REBUS> marker can only be one character."));

    if (allowedCharsV2.find(marker.Item(0)) == wxString::npos)
        throw PuzLoadError(_T("Invalid marker character in <REBUS>."));

    if (! wxIsupper(marker.Item(2).at(0)))
        throw PuzLoadError(_T("<REBUS> short solution must be upper case."));
}
