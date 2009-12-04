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


// Across lite claims that it will accept any of the characters in the grid
// as a valid rebus key, but it only takes numbers and lower case letters.
static const wxString allowedCharsRebus = _T("1234567890")
                                          _T("abcdefghijklmnopqrstuvwxyz");
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
            throw PuzHeaderError(
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
            m_puz->m_copyright.Prepend(_T("\xa9 "));

        CheckSection(_T("<SIZE>"));
        { // Enter scope
            wxString size = ReadLine();
            size_t x_index = size.find(_T("x"));
            if (x_index == wxString::npos)
                throw PuzDataError(_T("Missing 'x' in size specification"));
            long width, height;
            if (! (size.Mid(0, x_index).ToLong(&width)
                   && size.Mid(x_index+1).ToLong(&height)) )
                throw PuzDataError(_T("Improper size specification."));

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
                throw PuzDataError(_T("Missing some across clues"));
        }

        CheckSection(_T("<DOWN>"));
        for (size_t i = 0; i < nDown; ++i)
        {
            down.push_back(ReadLine());
            if (down.back().empty())
                throw PuzDataError(_T("Missing some down clues"));
        }

        // Read across and down into m_clues
        std::vector<wxString>::const_iterator across_it = across.begin();
        std::vector<wxString>::const_iterator down_it   = down.begin();

        for (XSquare * square = m_puz->m_grid.First();
             square != NULL;
             square = square->Next())
        {
            if (square->HasClue(DIR_ACROSS))
                m_puz->m_clues.push_back(*across_it++);
            if (square->HasClue(DIR_DOWN))
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
            throw PuzDataError(_T("Unrecognized line(s) at end of file."));
    }
    catch (BasePuzError & error)
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

    // Read the solution into a string
    wxString solution;
    solution.reserve(m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    for (size_t i = 0; i < m_puz->m_grid.GetHeight(); ++i)
    {
        wxString line = ReadLine();

        CheckGridLine(line, allowedCharsV1, m_puz->m_grid.GetWidth());

        solution.append(line);
    }

    wxASSERT(solution.length() ==
                m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    // Set the grid's solution
    wxString::const_iterator it = solution.begin();
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->SetSolution(*it++);
    }

    wxASSERT(it == solution.end());
}



void
TxtHandler::LoadVersion2Grid()
{
    CheckSection(_T("<GRID>"));

    // Read the grid the same way as in V1 (with different allowedChars)
    wxString solution;
    solution.reserve(m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());

    for (size_t i = 0; i < m_puz->m_grid.GetHeight(); ++i)
    {
        wxString line = ReadLine();

        CheckGridLine(line, allowedCharsV1, m_puz->m_grid.GetWidth());

        solution.append(line);
    }

    wxASSERT(solution.length() ==
                m_puz->m_grid.GetWidth() * m_puz->m_grid.GetHeight());


    typedef std::map<unsigned char, std::pair<wxString, wxChar> > rebusTable_t;
    rebusTable_t rebusTable;

    wxArrayString flags;
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

            flags = wxStringTokenize(line, _T("; "), wxTOKEN_STRTOK );
        }
        else if (line.find(_T(":")) == wxString::npos)
            throw PuzDataError(_T("No entries in <REBUS> section."));
        else // This is a marker line
            UngetLine(line);


        // Read rebus markers
        //-------------------
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
            const wxString & long_solution = marker.Item(1);
            const wxChar &   short_solution = marker.Item(2).at(0);

            rebusTable[index] = std::make_pair(long_solution, short_solution);
        }
    }

    // Process flags
    //--------------

    // MARK: circle all lowercase letters
    const bool use_mark = flags.Index(_T("MARK")) != wxNOT_FOUND;

    if (use_mark)
    {
        wxString::iterator str_it = solution.begin();
        for (XSquare * square = m_puz->m_grid.First();
             square != NULL;
             square = square->Next())
        {
            // We need to preserve the original case because lower case letters
            // are valid rebus indexes
            if (wxIslower(*str_it))
                square->SetFlag(XFLAG_CIRCLE);
            ++str_it;
        }
        wxASSERT(str_it == solution.end());
    }

    // Set the grid's solution (including rebus)
    //------------------------------------------
    wxString::const_iterator str_it = solution.begin();

    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        // Look for the letter in the rebus table
        rebusTable_t::const_iterator table_it;
        table_it = rebusTable.find(*str_it);

        if (table_it != rebusTable.end())
        {
            square->SetSolution(table_it->second.first,   // Long solution
                                table_it->second.second); // Short solution
        }
        else // No rebus
            square->SetSolution(*str_it);

        ++str_it;
    }
    wxASSERT(str_it == solution.end());
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
        throw PuzDataError(_T("Missing letters in grid."));

    if (line.length() > width)
        throw PuzDataError(_T("Extra letters in grid."));

    // Make sure all the characters are allowed to be in the grid
    size_t index = line.find_first_not_of(allowedCharsV2);
    if (index != wxString::npos)
        throw PuzDataError(_T("Invalid character in grid: \"%c\"."),
                           line.at(index));
}



void
CheckRebusMarker(const wxArrayString & marker)
{
    if (marker.size() < 3)
        throw PuzDataError(_T("Missing marker entries in <REBUS>."));
    if (marker.size() > 3)
        throw PuzDataError(_T("Extra marker entries in <REBUS>."));

    if (marker.Item(0).length() > 1)
        throw PuzDataError(_T("<REBUS> marker can only be one character."));

    if (allowedCharsV2.find(marker.Item(0)) == wxString::npos)
        throw PuzDataError(_T("Invalid marker character in <REBUS>."));

    if (! wxIsupper(marker.Item(2).at(0)))
        throw PuzDataError(_T("<REBUS> short solution must be upper case."));
}






//------------------------------------------------------------------------------
// DoSave implementation
//------------------------------------------------------------------------------

void
TxtHandler::DoSave()
{
    // No way we can save a scrambled puzzle to a txt file.
    if (m_puz->IsScrambled())
        throw FatalPuzError(_T("Puzzle is scrambled"));

    // Make the rebus table.  Then we can figure out which text version to
    // save the puzzle as.
    RebusSet rebusEntries;
    GetRebusEntries(rebusEntries);
    const bool use_version_2 = ! rebusEntries.empty();

    if (use_version_2)
        Write("<ACROSS PUZZLE V2>\n");
    else
        Write("<ACROSS PUZZLE>\n");

    Write("<TITLE>\n");
    Write(m_puz->m_title + _T("\n"));

    Write("<AUTHOR>\n");
    Write(m_puz->m_author + _T("\n"));

    Write("<COPYRIGHT>\n");
    // Strip the (c) symbol
    if (m_puz->m_copyright.StartsWith(_T("\xa9 ")))
        Write(m_puz->m_copyright.substr(2) + _T("\n"));
    else
        Write(m_puz->m_copyright + _T("\n"));

    Write("<SIZE>\n");
    Write(wxString::Format(_T("%dx%d"),
                           m_puz->m_grid.GetWidth(),
                           m_puz->m_grid.GetHeight()) + _T("\n"));

    Write("<GRID>\n");
    if (! use_version_2)
        WriteVersion1Grid();
    else
        WriteVersion2Grid(rebusEntries);

    Write("<ACROSS>\n");
    for (XPuzzle::ClueList::iterator it = m_puz->m_across.begin();
         it != m_puz->m_across.end();
         ++it)
    {
        Write(it->Text() + _T("\n"));
    }

    Write("<DOWN>\n");
    for (XPuzzle::ClueList::iterator it = m_puz->m_down.begin();
         it != m_puz->m_down.end();
         ++it)
    {
        Write(it->Text() + _T("\n"));
    }

    if (! m_puz->m_notes.empty())
    {
        Write("<NOTEPAD>\n");
        Write(m_puz->m_notes + _T("\n"));
    }
}


void
TxtHandler::WriteVersion1Grid()
{
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        Write(square->GetPlainSolution());
        if (square->IsLast(DIR_ACROSS))
            Write("\n");
    }
}

void
TxtHandler::WriteVersion2Grid(const RebusSet & rebusEntries)
{
    // Figure out what characters we can use for the rebus table.
    bool hasMark = false;
    wxString available_rebus = allowedCharsRebus;
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasFlag(XFLAG_CIRCLE))
        {
            hasMark = true;
            if (! square->HasSolutionRebus())
                available_rebus.Replace(
                    wxString(
                        static_cast<wxChar>(
                            wxTolower(square->GetPlainSolution()))),
                    _T(""));
        }
    }

    // Make the rebus table
    //---------------------

    RebusMap rebusTable;

    // We can only make the table if there are enough letters left for all
    // the rebus entries.
    if (rebusEntries.size() <= available_rebus.length())
    {
        // Start from the end (the lower-case letters).  This iterator will be
        // used for all rebus entries that also need a circle.
        wxString::reverse_iterator letter_it = available_rebus.rbegin();
        // Start from the beginning (the numbers letters).  This iterator will
        // be used for all rebus entries do not need a circle.
        wxString::iterator number_it = available_rebus.begin();

        for (RebusSet::const_iterator it = rebusEntries.begin();
             it != rebusEntries.end();
             ++it)
        {
            // If this assert fails, we've run out of characters in
            // available_rebus.  We've already checked to make sure we wouldn't
            // run out of characters, so this is just a sanity check.
            wxASSERT(*number_it != *letter_it);

            // Do we need a circle?
            if (hasMark && it->second == true)
            {
                // If we need to use lower-case letters to indicate circles and
                // we've run out of them, give up on the circles and press on
                // with the rebus.
                if (hasMark && ! wxIsalpha(*number_it))
                    hasMark = false;

                rebusTable[*it] = *letter_it;
                ++letter_it;
            }
            else // No circle
            {
                // If we need to use lower-case letters to indicate circles and
                // we've run out of numbers for the non-circled rebus squares,
                // give up on the circles and press on with the rebus.
                if (hasMark && ! wxIsdigit(*number_it))
                    hasMark = false;

                rebusTable[*it] = *number_it;
                ++number_it;
            }
        }
    }

    // Write the grid
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasSolutionRebus())
        {
            RebusPair entry( std::make_pair(square->GetSolution(),
                                            square->GetPlainSolution()),
                             square->HasFlag(XFLAG_CIRCLE) );
            const char key = rebusTable[entry];
            Write(key);
        }
        else
        {
            if (hasMark && square->HasFlag(XFLAG_CIRCLE))
                Write(wxTolower(square->GetPlainSolution()));
            else
                Write(square->GetPlainSolution());
        }
        if (square->IsLast(DIR_ACROSS))
            Write("\n");
    }

    // Write the rebus section
    if (hasMark || ! rebusTable.empty())
    {
        Write("<REBUS>\n");
        if (hasMark)
            Write("MARK;\n");
        for (RebusMap::iterator it = rebusTable.begin();
             it != rebusTable.end();
             ++it)
        {
            const RebusPair & entry = it->first;
            Write(it->second);
            Write(":");
            Write(entry.first.first);
            Write(":");
            Write(entry.first.second);
            Write("\n");
        }
    }
    // Done!
}



void
TxtHandler::GetRebusEntries(RebusSet & rebusEntries)
{
    for (XSquare * square = m_puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->HasSolutionRebus())
            continue;

        RebusPair entry( std::make_pair(square->GetSolution(),
                                        square->GetPlainSolution()),
                         square->HasFlag(XFLAG_CIRCLE) );

        rebusEntries.insert(entry);
    }
}