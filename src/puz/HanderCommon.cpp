#include "HandlerCommon.hpp"
#include <wx/stream.h>
#include <wx/string.h>

void
Read(wxInputStream & stream, void * buffer, size_t count)
{
    stream.Read(buffer, count);

    // Check for an error
    const wxStreamError error = stream.GetLastError();

    if (error == wxSTREAM_NO_ERROR)
        return;

    if      (error == wxSTREAM_EOF)
        throw PuzLoadError(_T("Unexpected end of file"));
    else if (error == wxSTREAM_READ_ERROR)
        throw PuzLoadError(_T("Read error"));
}


// Read a nul-terminate string from a stream
wxString
ReadString(wxInputStream & stream)
{
    wxString str;
    wxByte ch;
    Read(stream, &ch, 1);
    while (ch != 0)
    {
       str.Append(ch);
       Read(stream, &ch, 1);
    }
    return str;
}


// Read a string from the stream of len length
wxString
ReadString(wxInputStream & stream, size_t len)
{
    char * ch = new char [len];
    try
    {
        Read(stream, ch, len);
        wxString str = wxString::From8BitData(ch, len);
        delete [] ch;
        return str;
    }
    catch (...)
    {
        delete [] ch;
        throw;
    }
}










void
Write(wxOutputStream & stream, const void * buffer, size_t count)
{
    stream.Write(buffer, count);

    // Check for an error
    const wxStreamError error = stream.GetLastError();

    if (error == wxSTREAM_NO_ERROR)
        return;

    else if (error == wxSTREAM_WRITE_ERROR)
        throw PuzLoadError(_T("Write error"));
}



// Writes a string (not null-terminated)
void
WriteString(wxOutputStream & stream, const wxString & str)
{
    Write(stream, (const char *)str.To8BitData(), str.length());
}




// ByteArray functions

void
BytesToString(const ByteArray & bytes, wxString * str, size_t nItems)
{
    ByteArray::const_iterator it = bytes.begin();
    ByteArray::const_iterator end;
    if (nItems == wxString::npos)
        end = bytes.end();
    else
        end = it + nItems;

    for (; it != end; ++it)
        str->Append((wxChar)*it);
}



void
StringToBytes(const wxString & str, ByteArray * bytes, size_t nItems)
{
    wxString::const_iterator it = str.begin();
    wxString::const_iterator end;
    if (nItems == wxString::npos)
        end = str.end();
    else
        end = it + nItems;

    for (; it != end; ++it)
        bytes->push_back((wxByte)*it);
}




























void
SetGridSolution(XPuzzle * puz, const ByteArray & solution)
{
    wxASSERT(puz != NULL);
    wxASSERT(solution.size() == puz->m_grid.GetWidth() * puz->m_grid.GetHeight());

    ByteArray::const_iterator it = solution.begin();
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->solution = *it++;
    }
}



void
SetGridText(XPuzzle * puz, const ByteArray & text)
{
    wxASSERT(puz != NULL);
    wxASSERT(text.size() == puz->m_grid.GetWidth() * puz->m_grid.GetHeight());

    ByteArray::const_iterator it = text.begin();
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->text = *it++;
    }
}


void
SetGext(XPuzzle * puz, const ByteArray & gext)
{
    wxASSERT(puz != NULL);

    wxASSERT(gext.size() == puz->m_grid.GetWidth() * puz->m_grid.GetHeight());
    ByteArray::const_iterator it = gext.begin();

    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        square->flag = *it++;
    }
}





void
SetRebusUserGrid(XPuzzle * puz, const ByteArray & rebus)
{
    wxASSERT(puz != NULL);

    ByteArray::const_iterator it = rebus.begin();

    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (*it == '[')
        {
            ++it;
            if (*it == 0)
                throw PuzLoadError(_T("Missing closing ']' in RUSR section"));
            square->rebusSym = *it;
            ++it;
            if (*it != ']')
                throw PuzLoadError(_T("Missing closing ']' in RUSR section"));
            ++it;
        }
        else
        {
            while (*it != 0)
            {
                square->rebus.Append((wxChar)*it);
                ++it;
            }
        }

        // We've read one square (until the first \0 byte)
        ++it;
    }
}





void
SetRebusSolution(XPuzzle * puz, const ByteArray & table, const ByteArray & rebus)
{
    // An important note here:
    // In the grid rebus section, the index is actually 1 greater than the real
    // number.
    // Note sure why.

    wxASSERT(puz != NULL);

    wxASSERT(rebus.size() == puz->m_grid.GetWidth() * puz->m_grid.GetHeight());

    // For the time being, we're not supporting the webdings symbols
    // (They seem silly in the first place . . .)


    // Create the rebus table
    // Format: ' ' index ':' string ';'
    // Important note: index can be multiple characters!
    std::map<wxByte, wxString> rebusTable;

    ByteArray::const_iterator table_it = table.begin();

    while (table_it != table.end())
    {
        wxString value;
        wxString index_str;

        ++table_it;         // Throw away ' '

        // Read the index
        while (*table_it != ':')
        {
            index_str.Append((wxChar)*table_it);
            ++table_it;
        }

        long index;
        if (! index_str.ToLong(&index))
            throw PuzLoadError(_T("Invalid rebus table key"));


        ++table_it;         // Throw away ':'


        // Read the string value
        while (*table_it != ';')
        {
            value.Append((wxChar)*table_it);
            ++table_it;
        }

        ++table_it;         // Throw away ';' . . . done with this entry

        rebusTable[(wxByte)index] = value;
    }

    // Load the values from the grid rebus into the solution grid
    // Each byte corresponds with a sqare (like GEXT), and is the index
    // to rebusTable.

    // Note that the index value in the grid-rebus section is 1 greater than the
    // actual index in the rebus-table . . .
    // Hence [ (*rebus_it) - 1 ] is used as the index, not just [*rebus_it]
    ByteArray::const_iterator rebus_it = rebus.begin();

    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        // If the key doesn't exist, the default is an empty string.
        if (*rebus_it > 1)
            square->rebusSol = rebusTable[ *rebus_it - 1 ];
        ++rebus_it;
    }
}







wxString
GetGridSolution(XPuzzle * puz)
{
    wxASSERT(puz != NULL);

    wxString str;
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        str.Append(square->solution);
    }
    return str;
}



wxString
GetGridText(XPuzzle * puz)
{
    wxASSERT(puz != NULL);

    wxString str;
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        str.Append(square->text);
    }
    return str;
}


bool
GetGext(XPuzzle * puz, ByteArray * gext)
{
    wxASSERT(puz != NULL);

    bool isEmpty = true;
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        gext->push_back(square->flag);
        if (isEmpty && square->flag != XFLAG_CLEAR)
            isEmpty = false;
    }
    return ! isEmpty;
}



void
SetupClues(XPuzzle * puz)
{
    std::vector<wxString>::iterator clue_it = puz->m_clues.begin();
    for (XSquare * square = puz->m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if ( (square->clueFlag & ACROSS_CLUE) != 0 )
        {
            wxASSERT(clue_it != puz->m_clues.end());
            puz->m_across.push_back( XPuzzle::Clue(square->number,
                                                   *clue_it++) );
        }

        if ( (square->clueFlag & DOWN_CLUE) != 0 )
        {
            wxASSERT(clue_it != puz->m_clues.end());
            puz->m_down.push_back( XPuzzle::Clue(square->number,
                                                 *clue_it++) );
        }
    }
    wxASSERT(clue_it == puz->m_clues.end());

}

