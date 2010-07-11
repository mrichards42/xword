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

// -----------------------------------------------------------------------
// Load functions
// -----------------------------------------------------------------------
void
Puzzle::Load(const std::string & filename, const FileHandlerDesc * desc)
{
    if (! desc)
        desc = FindLoadHandler(GetExtension(filename));

    ClearError();
    if (! desc)
        throw MissingHandler();

    Clear();
    desc->handler(this, filename, desc->data);

#ifndef NDEBUG
    TestOk();
#endif NDEBUG
}


// -----------------------------------------------------------------------
// Save functions
// -----------------------------------------------------------------------
void
Puzzle::Save(const std::string & filename, const FileHandlerDesc * desc)
{
    if (! desc)
        desc = FindSaveHandler(GetExtension(filename));

    ClearError();
    if (! desc)
        throw MissingHandler();

    // Make sure the grid has clue numbers assigned, etc.
    m_grid.SetupGrid();

#ifndef NDEBUG
    TestOk();
#endif

    desc->handler(this, filename, desc->data);
}



// -----------------------------------------------------------------------
// Clue functions
// -----------------------------------------------------------------------
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


//------------------------------------------------------------------------------
// Error checking
//------------------------------------------------------------------------------
void
Puzzle::TestOk() const
{
    // Make sure all squares have a solution
    for (const Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->GetSolution().empty() || square->GetPlainSolution() == 0)
            throw InvalidGrid("All squares must have a solution");
    }

    // Make sure we have the correct number of clues
    size_t expected_across, expected_down;
    m_grid.CountClues(&expected_across, &expected_down);

    size_t across_clues = m_across.size();
    if (across_clues < expected_across)
        throw puz::InvalidClues("Missing across clues");
    if (across_clues > expected_across)
        throw puz::InvalidClues("Extra across clues");

    size_t down_clues = m_down.size();
    if (down_clues < expected_down)
        throw puz::InvalidClues("Missing down clues");
    if (down_clues > expected_down)
        throw puz::InvalidClues("Extra down clues");
}

//------------------------------------------------------------------------------
// Static functions (load/save handlers)
//------------------------------------------------------------------------------

bool
Puzzle::CanLoad(const std::string & filename)
{
    return FindLoadHandler(GetExtension(filename)) != NULL;
}


bool
Puzzle::CanSave(const std::string & filename)
{
    return FindSaveHandler(GetExtension(filename)) != NULL;
}



// Helper for find functions
const Puzzle::FileHandlerDesc *
FindHandler(const Puzzle::FileHandlerDesc * start, const std::string & ext)
{
    for (const Puzzle::FileHandlerDesc * d = start; d->ext != NULL; ++d)
        if (ext == d->ext)
            return d;
    return NULL;
}

const Puzzle::FileHandlerDesc *
Puzzle::FindLoadHandler(const std::string & ext)
{
    return FindHandler(sm_loadHandlers, ext);
}


const Puzzle::FileHandlerDesc *
Puzzle::FindSaveHandler(const std::string & ext)
{
    return FindHandler(sm_saveHandlers, ext);
}


// -----------------------------------------------------------------------
// C++ load and save handlers
// -----------------------------------------------------------------------

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
    std::ifstream stream(filename.c_str(), std::ios::in | std::ios::binary);
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
    std::ofstream stream(filename.c_str(), std::ios::out | std::ios::binary);
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


const Puzzle::FileHandlerDesc Puzzle::sm_loadHandlers[] = {
    { CppLoad, "puz", "Across Lite", LoadPuz },
    { NULL, NULL, NULL }
};

const Puzzle::FileHandlerDesc Puzzle::sm_saveHandlers[] = {
    { CppSave, "puz", "Across Lite", SavePuz },
    { NULL, NULL, NULL }
};

} // namespace puz
