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


#include "Puzzle.hpp"
#include "util.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

// Load Handlers
#include "LoadPuz.hpp"
// Save Handlers
#include "SavePuz.hpp"


namespace puz {

// Functors for find_if used with load and save handlers
struct find_ext
{
    find_ext(const std::string & ext)
        : m_ext(ext)
    {
        MakeLower(m_ext);
    }

    bool operator()(const Puzzle::FileHandlerDesc & desc)
    {
        return desc.ext == m_ext;
    }

private:
    std::string m_ext;
};

struct find_handler
{
    find_handler(Puzzle::FileHandler handler, void * data)
        : m_handler(handler),
          m_data(data)
    {
    }

    bool operator()(const Puzzle::FileHandlerDesc & desc)
    {
        return desc.handler == m_handler
            && desc.data == m_data;
    }

private:
    Puzzle::FileHandler m_handler;
    void * m_data;
};



void
Puzzle::GetClueList(std::vector<std::string> * clues)
{
    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();
    DoGetClueList(clues);
}

void
Puzzle::GetClueList(std::vector<std::string> * clues) const
{
    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();
    DoGetClueList(clues);
}


void
Puzzle::DoGetClueList(std::vector<std::string> * clues) const
{
    // Assemble the clues list from across and down
    Puzzle::ClueList::const_iterator across_it = m_across.begin();
    Puzzle::ClueList::const_iterator down_it   = m_down.begin();

    clues->clear();
    for (const Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(ACROSS))
        {
            clues->push_back(across_it->Text());
            ++across_it;
        }
        if (square->HasClue(DOWN))
        {
            clues->push_back(down_it->Text());
            ++down_it;
        }
    }
    assert(across_it == m_across.end() && down_it == m_down.end());
}


void
Puzzle::SetClueList(const std::vector<std::string> & clues)
{
    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();

    // Write across and down clues from the clue list
    m_across.clear();
    m_down.clear();
    std::vector<std::string>::const_iterator it = clues.begin();

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(ACROSS))
        {
            if (it == clues.end())
                throw InvalidClues();
            m_across.push_back(Clue(square->GetNumber(), *it++));
        }
        if (square->HasClue(DOWN))
        {
            if (it == clues.end())
                throw InvalidClues();
            m_down.push_back(Clue(square->GetNumber(), *it++));
        }
    }
    if (it != clues.end())
        throw InvalidClues();
}


void
Puzzle::RenumberClues()
{
    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();

    Puzzle::ClueList::iterator across_it = m_across.begin();
    Puzzle::ClueList::iterator down_it   = m_down.begin();

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasClue(ACROSS))
        {
            across_it->m_num = square->GetNumber();
            ++across_it;
        }
        if (square->HasClue(DOWN))
        {
            down_it->m_num = square->GetNumber();
            ++down_it;
        }
    }
}



void
Puzzle::Load(const std::string & filename)
{
    Load(filename, GetExtension(filename));
}


void
Puzzle::Load(const std::string & filename, const std::string & ext)
{
    // Try to load the puzzle with all known handlers for this extension.
    std::vector<FileHandlerDesc>::iterator it = sm_loadHandlers.begin();
    std::vector<FileHandlerDesc>::iterator end = sm_loadHandlers.end();

    it = std::find_if(it, end, find_ext(ext));
    if (it == end)
        throw MissingHandler();

    for (;;)
    {
        try
        {
            Load(filename, &*it);
            return;
        }
        catch (...)
        {
            ++it;
            it = std::find_if(it, end, find_ext(ext));
            // We've already tried all the handlers, so rethrow
            if (it == end)
                throw;
        }
    }
}


void
Puzzle::Load(const std::string & filename, FileHandlerDesc * desc)
{
    ClearError();
    if (! desc)
        throw MissingHandler();

    Clear();
    desc->handler(this, filename, desc->data);
}


void
Puzzle::Save(const std::string & filename)
{
    Save(filename, GetExtension(filename));
}

void
Puzzle::Save(const std::string & filename, const std::string & ext)
{
    // Try to save the puzzle with all known handlers for this extension.
    std::vector<FileHandlerDesc>::iterator it = sm_saveHandlers.begin();
    std::vector<FileHandlerDesc>::iterator end = sm_saveHandlers.end();

    it = std::find_if(it, end, find_ext(ext));
    if (it == end)
        throw MissingHandler();

    for (;;)
    {
        try
        {
            Save(filename, &*it);
            return;
        }
        catch (...)
        {
            it = std::find_if(it, end, find_ext(ext));
            // We've already tried all the handlers, so rethrow
            if (it == end)
                throw;
        }
    }
}



void
Puzzle::Save(const std::string & filename, FileHandlerDesc * desc)
{
    ClearError();
    if (! desc)
        throw MissingHandler();

    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();

    desc->handler(this, filename, desc->data);
}


//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

bool
Puzzle::CanLoad(const std::string & filename)
{
    return GetLoadHandler(GetExtension(filename)) != NULL;
}


bool
Puzzle::CanSave(const std::string & filename)
{
    return GetSaveHandler(GetExtension(filename)) != NULL;
}


void
Puzzle::AddLoadHandler(FileHandler handler, const char * extension, void * data)
{
    FileHandlerDesc d;
    d.handler = handler;
    d.ext = extension;
    d.data = data;
    sm_loadHandlers.push_back(d);
}


void
Puzzle::AddSaveHandler(FileHandler handler, const char * extension, void * data)
{
    FileHandlerDesc d;
    d.handler = handler;
    d.ext = extension;
    d.data = data;
    sm_saveHandlers.push_back(d);
}

void
Puzzle::RemoveLoadHandler(FileHandler handler, void * data)
{
    std::vector<FileHandlerDesc>::iterator end = sm_loadHandlers.end();
    std::vector<FileHandlerDesc>::iterator it =
        std::find_if(sm_loadHandlers.begin(), end, find_handler(handler, data));
    if (it != end)
        sm_loadHandlers.erase(it);
}

void
Puzzle::RemoveSaveHandler(FileHandler handler, void * data)
{
    std::vector<FileHandlerDesc>::iterator end = sm_loadHandlers.end();
    std::vector<FileHandlerDesc>::iterator it =
        std::find_if(sm_loadHandlers.begin(), end, find_handler(handler, data));
    if (it != end)
        sm_saveHandlers.erase(it);
}

Puzzle::FileHandlerDesc *
Puzzle::GetLoadHandler(const std::string & ext)
{
    std::vector<FileHandlerDesc>::iterator end = sm_loadHandlers.end();
    std::vector<FileHandlerDesc>::iterator it =
        std::find_if(sm_loadHandlers.begin(), end, find_ext(ext));
    if (it == end)
        return NULL;
    return &*it;
}

Puzzle::FileHandlerDesc *
Puzzle::GetSaveHandler(const std::string & ext)
{
    std::vector<FileHandlerDesc>::iterator end = sm_saveHandlers.end();
    std::vector<FileHandlerDesc>::iterator it =
        std::find_if(sm_saveHandlers.begin(), end, find_ext(ext));
    if (it == end)
        return NULL;
    return &*it;
}

void HandleExceptions(Puzzle * puz)
{
    try
    {
        throw;
    }
    catch (DataError & e)
    {
        puz->SetError(e.what());
    }
    catch (std::ios::failure &)
    {
        throw FatalFileError("Unexpected end of file");
    }
    // Everything else falls through
}

void CppLoad(Puzzle * puz, const std::string & filename, void * function)
{
    std::ifstream stream(filename.c_str());
    if (stream.fail())
        throw FatalFileError(std::string("Unable to open file: ") + filename);
    try
    {
        (*(puz::Puzzle::CppLoadHandler)(function))(puz, stream);
    }
    catch (...)
    {
        HandleExceptions(puz);
    }
    puz->SetOk(true);
}


void CppSave(Puzzle * puz, const std::string & filename, void * function)
{
    std::ofstream stream(filename.c_str());
    if (stream.fail())
    {
        remove(filename.c_str());
        throw FatalFileError(std::string("Unable to open file: ") + filename);
    }
    try
    {
        (*(puz::Puzzle::CppSaveHandler)(function))(puz, stream);
    }
    catch (...)
    {
        // An exception thrown while saving means that the resulting
        // file is unusable, so just delete it here.
        remove(filename.c_str());
        HandleExceptions(puz);
    }
}


const Puzzle::FileHandlerDesc g_initLH[] = {
    { CppLoad, "puz", LoadPuz }
};

std::vector<Puzzle::FileHandlerDesc> Puzzle::sm_loadHandlers(
    g_initLH + 0,
    g_initLH + sizeof(Puzzle::FileHandlerDesc) / sizeof(g_initLH)
);


const Puzzle::FileHandlerDesc g_initSH[] = {
    { CppSave, "puz", SavePuz }
};

std::vector<Puzzle::FileHandlerDesc> Puzzle::sm_saveHandlers(
    g_initSH + 0,
    g_initSH + sizeof(Puzzle::FileHandlerDesc) / sizeof(g_initSH)
);

} // namespace puz
