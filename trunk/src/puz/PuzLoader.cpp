/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


// TODO:
//   - Make this work better between unicode and ASCII
//        use wxString::From8BitData and wxString::To8BitData
//        for the checksums instead of weird unicode translation
//        functions
//        Or maybe that's wxString::mb_str ?
//        I need to look into this (another program?)

#include "PuzLoader.hpp"
#include "PuzCk.hpp"     // Puzzle header struct
#include "XPuzzle.hpp"
#include <wx/file.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include <wx/log.h> // for reporting errors

// Typedefs for load/save functions
typedef bool (*PuzSaveFunction)(XPuzzle *, const wxChar * filename);
typedef bool (*PuzLoadFunction)(XPuzzle *, const wxChar * filename);


// Struct to hold the necessary functions for a particular file type
struct TypeFunctions
{
    wxChar * ext;
    wxChar * description;
    PuzLoadFunction load;
    PuzSaveFunction save;

    // Implicit conversion to PuzLoader::TypeDesc
    operator PuzLoader::TypeDesc() const
    {
        PuzLoader::TypeDesc desc;
        desc.ext = ext;
        desc.description = description;
        return desc;
    }
};



// Save and load functions for all file types
//------------------------------------------

bool LoadPuz (XPuzzle * puz, const wxChar * filename);
bool SavePuz (XPuzzle * puz, const wxChar * filename);
bool LoadPyz (XPuzzle * puz, const wxChar * filename);
// bool SavePyz() doesn't exist
bool LoadText(XPuzzle * puz, const wxChar * filename);
bool SaveText(XPuzzle * puz, const wxChar * filename);


// The master list of supported file types
//------------------------------------------

static const TypeFunctions functionList[] = {
    { _T("puz"), _T("Across format"),     LoadPuz,  SavePuz },
    { _T("txt"), _T("ASCII text format"), LoadText, SaveText },
    { _T("pyz"), _T("Pyz format"),        LoadPyz,  NULL },
    { NULL, NULL, NULL, NULL }
};


// Utility for using TypeFunctions
TypeFunctions
GetFunctions(wxString ext)
{
    ext.MakeLower();
    int i;
    for (i = 0; functionList[i].ext != NULL; ++i)
        if (functionList[i].ext == ext)
            break;
    // This will return the blank entry at the end of the table
    // if no match is found.
    return functionList[i];
}


// Exposed save and load functions
//    - Searches the list of functions for the extension
//      and calls the appropriate function.
bool
PuzLoader::Save(XPuzzle * puz, const wxString & filename, wxString ext)
{
    if (ext == wxEmptyString) {
        wxFileName fn(filename);
        if (! fn.HasExt())
            return false;
        ext = fn.GetExt();
    }

    PuzSaveFunction func = GetFunctions(ext).save;
    if (func == NULL)
        return false;
    return func(puz, filename);
}



bool
PuzLoader::Load(XPuzzle * puz, const wxString & filename, wxString ext)
{
    if (ext == wxEmptyString) {
        wxFileName fn(filename);
        if (! fn.HasExt())
            return false;
        ext = fn.GetExt();
    }

    PuzLoadFunction func = GetFunctions(ext).load;
    if (func == NULL)
        return false;
    return func(puz, filename);
}



// Tests to see if the particular extension is listed in the master list
bool
PuzLoader::CanSave(const wxString & ext)
{
    return GetFunctions(ext).save != NULL;
}


bool
PuzLoader::CanLoad(const wxString & ext)
{
    return GetFunctions(ext).load != NULL;
}


// Gets a list of supported file types to load and save
std::vector<PuzLoader::TypeDesc>
PuzLoader::GetLoadTypes()
{
    std::vector<PuzLoader::TypeDesc> types;
    int i;
    for (i = 0; functionList[i].ext != NULL; ++i) {
        if (functionList[i].load == NULL || functionList[i].description == NULL)
            continue;
        types.push_back( functionList[i] );
    }
    return types;
}

std::vector<PuzLoader::TypeDesc>
PuzLoader::GetSaveTypes()
{
    std::vector<PuzLoader::TypeDesc> types;
    int i;
    for (i = 0; functionList[i].ext != NULL; ++i) {
        if (functionList[i].save == NULL || functionList[i].description == NULL)
            continue;
        types.push_back( functionList[i] );
    }
    return types;
}



// -------------------------------------------------------------
// Implementation
// -------------------------------------------------------------


// Interesting thing about converting between char and wchar_t:
//   characters beyond the range of normal letters [a-zA-z0-9]
//   are converted as ch & 0xffff (i.e. ch + 0xff00)
// We need to fix that so ASCII characters are converted
//   properly for wxChar and wxString in unicode builds
// This won't affect characters in the range [a-zA-z0-9] as 
//   for them, (ch & 0x00ff == ch)
#ifdef UNICODE
#    define TO_WIDE_CHAR(ch) ( wchar_t( (ch) & 0x00ff ) )
#else
#    define TO_WIDE_CHAR(ch) ch
#endif

//------------------------------------------------------------------------------
// Checksumming
//------------------------------------------------------------------------------
unsigned short cksum_region   (const unsigned char *base,
                               int len,
                               unsigned short cksum);

// Note that we need a different checksumming routine for wxStrings
// because of unicode issues
unsigned short cksum_string   (const wxString & str, unsigned short cksum);

unsigned short cib_cksum      (XPuzzle * puz);

unsigned short primary_cksum  (XPuzzle * puz,
                               unsigned short c_cib);

void           masked_cksum   (XPuzzle * puz,
                               unsigned short c_cib,
                               unsigned char cksums []);


//------------------------------------------------------------------------------
// Read/Write and utility functions
//------------------------------------------------------------------------------
inline wxString ReadTextLine(wxTextInputStream & f);
       void     WriteString (wxFile & f, const wxString & str);
       void     WriteSection(wxFile & f, const wxString & title,
                             const wxString & data);
       void     ReadString  (wxFile & f, wxString * str);
inline bool     LoadError   (const wxString & msg);
inline bool     ReadGridString(wxTextInputStream & f,
                               size_t width, size_t height,
                               wxString * str);

bool SetupClues(XPuzzle * puz);

wxString GetGridSolution(XPuzzle * puz) { return puz->m_grid.GetSolution(); }
wxString GetGridText(XPuzzle * puz)     { return puz->m_grid.GetGrid(); }
wxString GetGext(XPuzzle * puz)         { return puz->m_grid.GetGext(); }
void SetGridSolution (XPuzzle * puz, const char * solution )
    { puz->m_grid.SetSolution(solution); }

//------------------------------------------------------------------------------
// Saving and loading functions
//------------------------------------------------------------------------------
bool
LoadPuz(XPuzzle * puz, const wxChar * filename)
{
    wxLogDebug(_T("Loading puzzle: %s"), filename);
    // Reset all puzzle contents
    puz->Clear();

    wxFile f(filename, wxFile::read);
    if (! f.IsOpened())
        return false;

    PuzCk h;
    f.Read(&h.c_primary, 2);
    f.Read(h.formatstr, 12);

    f.Read(&h.c_cib, 2);
    f.Read(h.c_masked, 8);

    f.Read(h.version, 4);
    f.Read(&h.noise1c, 2);
    f.Read(&h.xunk1e,  2);
    f.Read(&h.noise20, 2);
    f.Read(&h.noise22, 2);
    f.Read(&h.noise24, 2);
    f.Read(&h.noise26, 2);
    f.Read(&h.noise28, 2);
    f.Read(&h.noise2a, 2);

    f.Read(&h.width,  1);
    f.Read(&h.height, 1);
    f.Read(&h.num_clues, 2);
    f.Read(&h.xunk30, 2);
    f.Read(&h.xunk32, 2);

    puz->m_grid.SetSize(h.width, h.height);

    // Careful with malloc and free
    // (needed because of the dynamically sized string)
    char * temp = (char*)malloc(h.width*h.height);
    if (! temp) return false;

    f.Read(temp, h.width * h.height);
    puz->m_grid.SetSolution(temp);

    f.Read(temp, h.width * h.height);
    puz->m_grid.SetGrid(temp);
    
    free(temp); temp = NULL;

    ReadString(f, &puz->m_title);
    ReadString(f, &puz->m_author);
    ReadString(f, &puz->m_copyright);

    int i;
    for (i = 0; i < h.num_clues; ++i)
    {
        puz->m_clues.push_back(wxString());
        ReadString(f, &puz->m_clues.back());
    }
    ReadString(f, &puz->m_notes);

    // Find additional sections (i.e. GEXT, LTIM, etc)
    while (! f.Eof())
    {
        char section[4];     // titles are always 4 characters
        f.Read(section, 4);

        unsigned short section_length;
        unsigned short c_section;
        f.Read(&section_length, 2);
        f.Read(&c_section, 2);

        unsigned char * section_data = (unsigned char*)malloc(section_length);
        f.Read(section_data, section_length);

        // Test the section's checksum
        wxLogDebug(_T("section    %10d  %10d"),
                   c_section,
                   cksum_region(section_data, section_length, 0x00));

        if (c_section != cksum_region(section_data, section_length, 0x00))
        {
            free(section_data); section_data = NULL;
            return LoadError(
                       wxString::Format(
                           _T("Checksum does not match for %s region"),
                           section )
                   );
        }

        unsigned char null_char;
        f.Read(&null_char, 1);
        if (null_char != 0x00)
        {
            free(section_data); section_data = NULL;
            return LoadError(
                       wxString::Format(
                           _T("Missing null terminator for %s region"),
                           section)
                   );
        }

        // GEXT section (grid extension?)
        if (strncmp(section, "GEXT", 4) == 0)
        {
            wxLogDebug(_T("GEXT section"));
            puz->m_grid.SetGext(section_data);
        }
        // LTIM section (lit time?)
        else if (strncmp(section, "LTIM", 4) == 0)
        {
            wxLogDebug(_T("LTIM section"));
            puz->m_time = atoi((char*)section_data);
        }
        // Otherwise we don't know what to do with the section,
        //  so add it to the list of extra sections.
        // THIS NEEDS TO BE IMPLEMENTED
        free(section_data); section_data = NULL;
    }

    // Verify checksums
    unsigned short c_cib = cib_cksum(puz);
    unsigned short c_primary = primary_cksum(puz, c_cib);
    unsigned char c_masked[8];
    masked_cksum(puz, c_cib, c_masked);

    // Test the checksums
    bool isOk = c_cib == h.c_cib
                && c_primary   == h.c_primary
                && c_masked[0] == h.c_masked[0]
                && c_masked[1] == h.c_masked[1]
                && c_masked[2] == h.c_masked[2]
                && c_masked[3] == h.c_masked[3]
                && c_masked[4] == h.c_masked[4]
                && c_masked[5] == h.c_masked[5]
                && c_masked[6] == h.c_masked[6]
                && c_masked[7] == h.c_masked[7];

    // Print the checksums
    wxLogDebug(_T("Checksums      actual      calc'd"));
    wxLogDebug(_T("--------------------------------"));
    wxLogDebug(_T("CIB        %10d  %10d"), h.c_cib, c_cib);
    wxLogDebug(_T("Primary    %10d  %10d"), h.c_primary, c_primary);
    wxLogDebug(_T("Masked[0]  %10d  %10d"), h.c_masked[0], c_masked[0]);
    wxLogDebug(_T("Masked[1]  %10d  %10d"), h.c_masked[1], c_masked[1]);
    wxLogDebug(_T("Masked[2]  %10d  %10d"), h.c_masked[2], c_masked[2]);
    wxLogDebug(_T("Masked[3]  %10d  %10d"), h.c_masked[3], c_masked[3]);
    wxLogDebug(_T("Masked[4]  %10d  %10d"), h.c_masked[4], c_masked[4]);
    wxLogDebug(_T("Masked[5]  %10d  %10d"), h.c_masked[5], c_masked[5]);
    wxLogDebug(_T("Masked[6]  %10d  %10d"), h.c_masked[6], c_masked[6]);
    wxLogDebug(_T("Masked[7]  %10d  %10d"), h.c_masked[7], c_masked[7]);

    if (isOk)
        return SetupClues(puz);
    else
        return false;
}




bool
SavePuz(XPuzzle * puz, const wxChar * filename)
{
    wxLogDebug(_T("Saving puzzle: %s"), filename);
    wxFile f(filename, wxFile::write);
    if (! f.IsOpened())
        return false;

    // Get checksums
    unsigned short c_cib = cib_cksum(puz);
    unsigned short c_primary = primary_cksum(puz, c_cib);
    unsigned char c_masked[8];
    masked_cksum(puz, c_cib, c_masked);

    // Write File
    f.Write(&c_primary, 2);
    f.Write("ACROSS&DOWN\0", 12);

    f.Write(&c_cib, 2);
    f.Write(c_masked, 8);

    f.Write("1.2\0", 4);
    // 16 blank bytes
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    f.Write("\0\0", 2);
    char temp = puz->m_grid.GetWidth();
    f.Write(&temp,  1);
    temp = puz->m_grid.GetHeight();
    f.Write(&temp, 1);
    short num_clues = puz->m_clues.size();
    f.Write(&num_clues,     2);
    f.Write("\1\0", 2);
    f.Write("\0\0", 2);

    WriteString(f, GetGridSolution(puz));
    WriteString(f, GetGridText(puz));
    WriteString(f, puz->m_title + _T('\0'));
    WriteString(f, puz->m_author + _T('\0'));
    WriteString(f, puz->m_copyright + _T('\0'));

    std::vector<wxString>::iterator it;
    for (it = puz->m_clues.begin(); it != puz->m_clues.end(); ++it) {
        WriteString(f, *it + _T('\0'));
    }
    WriteString(f, puz->m_notes + _T('\0'));

    WriteSection(f, _T("LTIM"), wxString::Format(_T("%d,%d"), puz->m_time, 0));
    WriteSection(f, _T("GEXT"), GetGext(puz));

    return true;
}


bool
LoadText(XPuzzle * puz, const wxChar * filename)
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
    int i;
    for (i = 0; i < acrossClues; ++i) {
        across.push_back(ReadTextLine(f));
        if (across.back() == _T("<DOWN>") || across.back() == _T(""))
            return LoadError(_T("Not enough across clues"));
    }

    if (ReadTextLine(f) != _T("<DOWN>"))
        return LoadError(_T("Expected <DOWN>"));
    for (i = 0; i < downClues; ++i) {
        down.push_back(ReadTextLine(f));
        if (down.back() == _T(""))
            return LoadError(_T("Not enough down clues"));
    }

    // Read <NOTEPAD> section


    // Read m_across and m_down into m_clues
    // Iterators should always be valid due to some handiwork from earlier
    std::vector<wxString>::const_iterator across_it = across.begin();
    std::vector<wxString>::const_iterator down_it   = down.begin();

    int row, col;
    for (row = 0; row < puz->m_grid.GetHeight(); ++row) {
        for (col = 0; col < puz->m_grid.GetWidth(); ++col) {
            int clue = puz->m_grid.HasClue(col, row);
            if (clue & ACROSS_CLUE)
                puz->m_clues.push_back(*across_it++);
            if (clue & DOWN_CLUE)
                puz->m_clues.push_back(*down_it++);
        }
    }

    return SetupClues(puz);
}


bool
SaveText(XPuzzle * puz, const wxChar * filename)
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



bool
LoadPyz(XPuzzle * puz, const wxChar * filename)
{
    // Reset all puzzle contents
    puz->Clear();

    wxLogDebug(_T("Loading Pyz file %s"), filename);
    wxFileInputStream input(filename);
    wxTextInputStream f(input, _T("\t"), wxConvLibc); // Locale dependent, doesn't kill (c)

    puz->m_title = ReadTextLine(f);
    puz->m_author = ReadTextLine(f);
    puz->m_copyright = ReadTextLine(f);
    long time;
    if (! ReadTextLine(f).ToLong(&time))
        return LoadError(_T("No Time Value"));
    puz->m_time = time;
    long width, height;
    ReadTextLine(f).ToLong(&width);
    ReadTextLine(f).ToLong(&height);
    puz->m_grid.SetSize(width, height);

    // Solution grid
    wxString gridString;
    int i;
    for (i = 0; i < height; ++i) {
        wxString line = ReadTextLine(f);
        if (line.length() != width) {
            return LoadError(_T("Incorrect solution grid width"));
        }
        gridString.Append(line);
    }
    if (gridString.length() != width*height)
        return LoadError(_T("Solution grid improperly sized"));
    puz->m_grid.SetSolution(gridString.mb_str());

    // User grid
    gridString = wxEmptyString;
    for (i = 0; i < height; ++i) {
        wxString line = ReadTextLine(f);
        if (line.length() != width) {
            return LoadError(_T("Incorrect user grid width"));
        }
        gridString.Append(line);
    }
    if (gridString.length() != width*height)
        return LoadError(_T("User grid improperly sized"));
    puz->m_grid.SetGrid(gridString.mb_str());

    // Gext
    gridString = wxEmptyString;
    for (i = 0; i < height; ++i) {
        long val;
        wxString line = ReadTextLine(f);
        if (line.length() != width) {
            return LoadError(_T("Incorrect GEXT grid width"));
        }
        size_t j;
        for (j = 0; j < width; ++j) {
            if (! line.Mid(j,1).ToLong(&val))
                return LoadError(_T("Incorrect value in GEXT"));
            gridString += wxChar(val);
        }
    }
    if (gridString.length() != width*height)
        return LoadError(_T("GEXT improperly sized"));
    puz->m_grid.SetGext((const unsigned char *)(const char *)gridString.mb_str());


    // Count across and down clues
    size_t acrossClues, downClues;
    puz->m_grid.CountClues(&acrossClues, &downClues);

    std::vector<wxString> across;
    std::vector<wxString> down;

    // Read across and down clues 
    for (i = 0; i < acrossClues; ++i)
        across.push_back(ReadTextLine(f));

    for (i = 0; i < downClues; ++i)
        down.push_back(ReadTextLine(f));

    // Read m_across and m_down into m_clues
    // Iterators should always be valid due to some handiwork from earlier
    std::vector<wxString>::const_iterator across_it = across.begin();
    std::vector<wxString>::const_iterator down_it   = down.begin();

    int row, col;
    for (row = 0; row < puz->m_grid.GetHeight(); ++row) {
        for (col = 0; col < puz->m_grid.GetWidth(); ++col) {
            int clue = puz->m_grid.HasClue(col, row);
            if (clue & ACROSS_CLUE)
                puz->m_clues.push_back(*across_it++);
            if (clue & DOWN_CLUE)
                puz->m_clues.push_back(*down_it++);
        }
    }

    return SetupClues(puz);
}





// Utility functions
bool
SetupClues(XPuzzle * puz)
{
    // Note a lack of error checking here:
    // This should work out if we have already tested the checksums
    //   and found the puzzle to be in good working order

    std::vector<wxString>::iterator it = puz->m_clues.begin();
    size_t clueNumber;
    for (size_t row = 0; row < puz->m_grid.GetHeight(); ++row)
    {
        for (size_t col = 0; col < puz->m_grid.GetWidth(); ++col)
        {
            size_t clue = puz->m_grid.HasClue(col, row, &clueNumber);
            if (clue != NO_CLUE && it == puz->m_clues.end())
                return LoadError(_T("Not enough clues!"));
            if (clue & ACROSS_CLUE)
                puz->m_across.push_back( XPuzzle::Clue(clueNumber, *it++) );
            if (clue & DOWN_CLUE)
                puz->m_down.push_back( XPuzzle::Clue(clueNumber, *it++) );
        }
    }
    return true;
}


//-------------------------------------------------------------------------------
// Checksumming
//-------------------------------------------------------------------------------
unsigned short
cksum_region(const unsigned char *base, int len, unsigned short cksum)
{
    int i;

    for (i = 0; i < len; ++i) {
        if (cksum & 0x0001)
            cksum = (cksum >> 1) + 0x8000;
        else
            cksum = cksum >> 1;
        cksum += *(base+i);
    }

    return cksum;
}


// Workaround for wxString in unicode builds
// Otherwise, funny things happen when we try to convert from
//    wxString to char *
unsigned short
cksum_string(const wxString & str, unsigned short cksum)
{
    // Don't checksum empty strings
    if (str == _T('\0'))
        return cksum;
    wxString::const_iterator it;
    for (it = str.begin(); it != str.end(); ++it) {
        if (cksum & 0x0001)
            cksum = (cksum >> 1) + 0x8000;
        else
            cksum = cksum >> 1;
        cksum += int(*it);
    }
    return cksum;
}



unsigned short
cib_cksum(XPuzzle * puz)
{
    // numbers to initialize the checksum with
    unsigned char cib_region[8];
    cib_region[0] = puz->m_grid.GetWidth();
    cib_region[1] = puz->m_grid.GetHeight();
    cib_region[2] = (puz->m_clues.size() & 0x00ff);      // retrieve the first byte of a little endian short
    cib_region[3] = (puz->m_clues.size() & 0xff00) >> 8; // retrieve the second byte of a little endian short
    cib_region[4] = 0x01;
    cib_region[5] = 0x00;
    cib_region[6] = 0x00;
    cib_region[7] = 0x00;

    return cksum_region(cib_region, 8, 0x0000);
}

unsigned short
primary_cksum(XPuzzle * puz, unsigned short c_cib)
{
    unsigned short c_primary = c_cib;
    c_primary = cksum_string(puz->m_grid.GetSolution(),      c_primary);
    c_primary = cksum_string(puz->m_grid.GetGrid(),          c_primary);
    c_primary = cksum_string(puz->m_title     + _T('\0'),    c_primary);
    c_primary = cksum_string(puz->m_author    + _T('\0'),    c_primary);
    c_primary = cksum_string(puz->m_copyright + _T('\0'),    c_primary);

    std::vector<wxString>::iterator it;
    for (it = puz->m_clues.begin(); it != puz->m_clues.end(); ++it)
        c_primary = cksum_string(*it, c_primary);

    return c_primary;
}

void
masked_cksum(XPuzzle * puz, unsigned short c_cib, unsigned char cksums [])
{
    // Masked sums
    unsigned short c_sol  = cksum_string(puz->m_grid.GetSolution(), 0x0000);
    unsigned short c_grid = cksum_string(puz->m_grid.GetGrid(),     0x0000);

    unsigned short c_part;
    c_part = cksum_string(puz->m_title     + _T('\0'), 0x0000);
    c_part = cksum_string(puz->m_author    + _T('\0'), c_part);
    c_part = cksum_string(puz->m_copyright + _T('\0'), c_part);
    std::vector<wxString>::iterator it;
    for (it = puz->m_clues.begin(); it != puz->m_clues.end(); ++it)
        c_part = cksum_string(*it, c_part);

    // This is the best part
    // le-low bits
    cksums[0] = 'I' ^ (c_cib  & 0xff);
    cksums[1] = 'C' ^ (c_sol  & 0xff);
    cksums[2] = 'H' ^ (c_grid & 0xff);
    cksums[3] = 'E' ^ (c_part & 0xff);
    // le-high bits
    cksums[4] = 'A' ^ ((c_cib  & 0xff00) >> 8);
    cksums[5] = 'T' ^ ((c_sol  & 0xff00) >> 8);
    cksums[6] = 'E' ^ ((c_grid & 0xff00) >> 8);
    cksums[7] = 'D' ^ ((c_part & 0xff00) >> 8);
}




//-------------------------------------------------------------------------------
// Read/Write utility functions
//-------------------------------------------------------------------------------
inline wxString
ReadTextLine(wxTextInputStream & f)
{
    return f.ReadLine().Trim(true).Trim(false);
}

inline bool
ReadGridString(wxTextInputStream & f, size_t width, size_t height, wxString * str)
{
    str->Empty();
    size_t i;
    for (i = 0; i < height; ++i) {
        wxString line = ReadTextLine(f);
        if (line.length() != width) {
            return LoadError(_T("Incorrect grid width"));
        }
        str->Append(line);
    }
    if (str->length() != width * height)
        return LoadError(_T("Grid improperly sized"));
    return true;
}




// Writes a string (not null-terminated)
void
WriteString(wxFile & f, const wxString & str)
{
    char buf;
    wxString::const_iterator it;
    for (it = str.begin(); it != str.end(); ++it) {
        buf = (int)(*it);
        f.Write(&buf, 1);
    }
}


// Writes an extra section (e.g. GEXT, LTIM)
void
WriteSection(wxFile & f, const wxString & title, const wxString & data)
{
    short length = data.length();
    if (length > 0) { // don't write empty sections
        short cksum  = cksum_string(data, 0x00);
        WriteString(f, title);
        f.Write(&length, 2);
        f.Write(&cksum, 2);
        WriteString(f, data);
        f.Write("\0", 1);
    }
}


// Reads a null-terminated string
void
ReadString(wxFile & f, wxString * str)
{
    if (f.Eof())
        return;
    char buf;
    f.Read(&buf, 1);
    while (buf != 0x00) {
        str->Append(TO_WIDE_CHAR(buf));
        if (f.Eof())
            return;
        f.Read(&buf, 1);
    }
}


#include <wx/msgdlg.h>

inline bool
LoadError(const wxString & msg)
{
    wxMessageBox(msg);
    wxTrap();
    return false;
}
