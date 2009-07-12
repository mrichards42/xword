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

#ifndef TXT_HANDLER_H
#define TXT_HANDLER_H

bool LoadTxt(XPuzzle * puz, const wxString & filename);
bool SaveTxt(XPuzzle * puz, const wxString & filename);


bool
LoadTxt(XPuzzle * puz, const wxChar * filename)
{
    try
    {
        // Reset all puzzle contents
        puz->Clear();

        wxLogDebug(_T("Loading Text file %s"), filename);
        wxFileInputStream input(filename);
        // wxConvLibc is locale dependent: it doesn't kill (c)
        wxTextInputStream f(input, _T("\t"), wxConvLibc);

        wxString version = ReadTextLine(f);
        // We'll get to <ACROSS PUZZLE V2> sometime
        if (! (version == _T("<ACROSS PUZZLE>")) )
            return LoadError(_T("Expected <ACROSS PUZZLE>"));

        wxLogDebug(_T("Has Header"));

        if (ReadTextLine(f) != _T("<TITLE>"))
            return LoadError(_T("Expected <TITLE>"));
        puz->m_title = ReadTextLine(f);

        wxLogDebug(_T("Has Title"));

        if (ReadTextLine(f) != _T("<AUTHOR>"))
            return LoadError(_T("Expected <AUTHOR>"));
        puz->m_author = ReadTextLine(f);

        wxLogDebug(_T("Has Author"));

        if (ReadTextLine(f) != _T("<COPYRIGHT>"))
            return LoadError(_T("Expected <COPYRIGHT>"));
        puz->m_copyright = ReadTextLine(f);
        if (! puz->m_copyright.empty())
            puz->m_copyright.Prepend(_T("© "));

        wxLogDebug(_T("Has Copyright"));

        if (ReadTextLine(f) != _T("<SIZE>"))
            return LoadError(_T("Expected <SIZE>"));
        wxStringTokenizer tok(ReadTextLine(f), _T(" x"), wxTOKEN_STRTOK);
        long width, height;
        // This looks a bit messy, but achieves all steps at once
        if (! (tok.HasMoreTokens() && tok.GetNextToken().ToLong(&width)
               && tok.HasMoreTokens() && tok.GetNextToken().ToLong(&height)) )
        {
            return LoadError(_T("Incorrect size spec"));
        }
        puz->m_grid.SetSize(width, height);
        wxLogDebug(_T("width: %d, height: %d"), width, height);

        if (ReadTextLine(f) != _T("<GRID>"))
            return LoadError(_T("Expected <GRID>"));

        wxString gridString;
        if (! ReadGridString(f, width, height, &gridString))
            return false;
        SetGridSolution(puz, gridString.mb_str());

        puz->m_grid.ClearGrid(); // Set up grid black squares


        // Count across and down clues
        size_t acrossClues, downClues;
        puz->m_grid.CountClues(&acrossClues, &downClues);

        // Read across and down clues
        std::vector<wxString> across;
        std::vector<wxString> down;

        if (ReadTextLine(f) != _T("<ACROSS>"))
            return LoadError(_T("Expected <ACROSS>"));

        for (size_t i = 0; i < acrossClues; ++i)
        {
            across.push_back(ReadTextLine(f));
            if (across.back() == _T("<DOWN>") || across.back() == _T(""))
                return LoadError(_T("Not enough across clues"));
        }

        if (ReadTextLine(f) != _T("<DOWN>"))
            return LoadError(_T("Expected <DOWN>"));
        for (size_t i = 0; i < downClues; ++i)
        {
            down.push_back(ReadTextLine(f));
            if (down.back() == _T(""))
                return LoadError(_T("Not enough down clues"));
        }

        // Read <NOTEPAD> section


        // Read m_across and m_down into m_clues
        // Iterators should always be valid due to some handiwork from earlier
        std::vector<wxString>::const_iterator across_it = across.begin();
        std::vector<wxString>::const_iterator down_it   = down.begin();

        for (size_t row = 0; row < puz->m_grid.GetHeight(); ++row)
        {
            for (size_t col = 0; col < puz->m_grid.GetWidth(); ++col)
            {
                const int clue = puz->m_grid.HasClue(col, row);
                if (clue & ACROSS_CLUE)
                    puz->m_clues.push_back(*across_it++);
                if (clue & DOWN_CLUE)
                    puz->m_clues.push_back(*down_it++);
            }
        }

        SetupClues(puz);

        return true;
    }
    catch (PuzLoadError exception)
    {
        wxMessageBox(exception.message);
    }
    catch (...)
    {
        wxMessageBox(_T("Error loading file"));
    }
    return false;
}


bool
SaveTxt(XPuzzle * puz, const wxChar * filename)
{
    wxFileOutputStream f(filename);
    if (! f.IsOk())
        return false;

    wxTextOutputStream out(f);

    out << _T("<ACROSS PUZZLE>") << endl
        << _T("<TITLE>") << endl
        << puz->m_title << endl
        << _T("<AUTHOR>") << endl
        << puz->m_author << endl
        << _T("<COPYRIGHT>") << endl
        << puz->m_copyright << endl
        << _T("<SIZE>") << endl
        << wxString::Format(_T("%dx%d"), puz->m_grid.GetWidth(), puz->m_grid.GetHeight()) << endl
        << _T("<GRID>") << endl;

    int width = puz->m_grid.GetWidth();
    wxString sol = puz->m_grid.GetSolution();
    size_t pos;
    for (pos = 0; pos < sol.length(); pos += width)
        out << sol.Mid(pos, width) << endl;

    XPuzzle::ClueList::iterator it;
    out << _T("<ACROSS>") << endl;
    for (it = puz->m_across.begin(); it != puz->m_across.end(); ++it)
        out << it->Text() << endl;
    out << _T("<DOWN>") << endl;
    for (it = puz->m_down.begin(); it != puz->m_down.end(); ++it)
        out << it->Text() << endl;

    return true;
}

#endif // TXT_HANDLER_H
