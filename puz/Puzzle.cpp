// This file is part of XWord
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
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
#include <iostream>

namespace puz {

void HandleExceptions(Puzzle * puz)
{
    try
    {
        throw;
    }
    catch (std::ios::failure &)
    {
        throw FatalFileError("Unexpected end of file");
    }
    // Everything else falls through
}

// -----------------------------------------------------------------------
// Load functions
// -----------------------------------------------------------------------
void
Puzzle::Load(const string_t & filename, const FileHandlerDesc * desc)
{
    if (! desc)
        desc = FindLoadHandler(GetExtension(filename));

    ClearError();
    if (! desc)
        throw MissingHandler();

    Clear();
    try {
        desc->handler(this, filename.c_str(), desc->data);
        TestOk();
    }
    catch (...) {
        HandleExceptions(this);
    }
}


// -----------------------------------------------------------------------
// Save functions
// -----------------------------------------------------------------------
void
Puzzle::Save(const string_t & filename, const FileHandlerDesc * desc)
{
    if (! desc)
        desc = FindSaveHandler(GetExtension(filename));

    ClearError();
    if (! desc)
        throw MissingHandler();

    TestOk();

    desc->handler(this, filename, desc->data);
}


// -----------------------------------------------------------------------
// Grid numbering
// -----------------------------------------------------------------------

// Number the grid and clues
void Puzzle::NumberClues()
{
    NumberGrid();

    ClueList::iterator across = GetAcross().begin();
    ClueList::iterator down   = GetDown().begin();

    ClueList::iterator across_end = GetAcross().end();
    ClueList::iterator down_end   = GetDown().end();

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->IsWhite())
            continue;

        if (square->HasClue(ACROSS))
        {
            if (across == across_end)
                throw InvalidClues();
            across->SetNumber(square->GetNumber());
            ++across;
        }

        if (square->HasClue(DOWN))
        {
            if (down == down_end)
                throw InvalidClues();
            down->SetNumber(square->GetNumber());
            ++down;
        }
    }
    if (across != across_end || down != down_end)
        throw InvalidClues();

}

void Puzzle::SetAllClues(const std::vector<string_t> & clues)
{
    NumberGrid();

    ClueList & across = GetAcross();
    ClueList & down = GetDown();
    across.clear();
    down.clear();

    std::vector<string_t>::const_iterator clue_it = clues.begin();
    std::vector<string_t>::const_iterator clue_end = clues.end();

    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->IsWhite())
            continue;

        if (square->HasClue(ACROSS))
        {
            if (clue_it == clue_end)
                throw InvalidClues();
            across.push_back(Clue(square->GetNumber(), *clue_it));
            ++clue_it;
        }

        if (square->HasClue(DOWN))
        {
            if (clue_it == clue_end)
                throw InvalidClues();
            down.push_back(Clue(square->GetNumber(), *clue_it));
            ++clue_it;
        }
    }
    if (clue_it != clue_end)
        throw InvalidClues();
}

void Puzzle::NumberGrid()
{
    m_grid.NumberGrid();
}

//------------------------------------------------------------------------------
// Error checking
//------------------------------------------------------------------------------

// Helper for FindSquare() in TestClueList
struct FIND_CLUE
{
    FIND_CLUE(string_t num_, GridDirection dir_)
        : num(num_), dir(dir_)
    {
        assert(! num.empty());
    }
    string_t num;
    GridDirection dir;

    bool operator()(const Square * square)
    {
        return square->GetNumber() == num &&
               square->HasClue(dir);
    }
};

void Puzzle::TestClueList(const string_t & direction)
{
    const ClueList & clues = GetClues(direction);
    GridDirection dir;
    bool has_dir = true;
    if (&clues == &GetAcross())
        dir = ACROSS;
    else if (&clues == &GetDown())
        dir = DOWN;
    else
        has_dir = false;

    ClueList::const_iterator it;
    for (it = clues.begin(); it != clues.end(); ++it)
    {
        if (it->GetNumber().empty())
            throw InvalidClues("All clues must have a number");
        if (! has_dir)
            continue;
        Square * square = m_grid.FindSquare(FIND_CLUE(it->GetNumber(), dir));
        if (! square)
            throw InvalidClues("All clues must have a square");
    }
}

void
Puzzle::TestOk()
{
    m_isOk = false;
    // Make sure there is a grid
    if (m_grid.GetWidth() == 0 || m_grid.GetHeight() == 0)
        throw InvalidGrid();

    // All squares do not need to have a solution

    // Make sure that all clues can find a square.
    TestClueList(puzT("Across"));
    TestClueList(puzT("Down"));
    Clues::cluemap_t::const_iterator it;
    for (it = m_clues.m_otherClues.begin();
         it != m_clues.m_otherClues.begin();
         ++it)
    {
        TestClueList(it->first);
    }

    // Make sure that all numbered squares have a clue
    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (square->HasNumber())
        {
            if (square->HasClue(puz::ACROSS) &&
                GetAcross().Find(square->GetNumber()) == GetAcross().end())
                    throw InvalidGrid("All numbered squares must have a clue");
            if (square->HasClue(puz::DOWN) &&
                GetDown().Find(square->GetNumber()) == GetDown().end())
                    throw InvalidGrid("All numbered squares must have a clue");
        }
    }
    m_isOk = true;
}

//------------------------------------------------------------------------------
// Static functions (load/save handlers)
//------------------------------------------------------------------------------

bool
Puzzle::CanLoad(const string_t & filename)
{
    return FindLoadHandler(GetExtension(filename)) != NULL;
}


bool
Puzzle::CanSave(const string_t & filename)
{
    return FindSaveHandler(GetExtension(filename)) != NULL;
}



// Helper for find functions
const Puzzle::FileHandlerDesc *
FindHandler(const Puzzle::FileHandlerDesc * start, const string_t & ext)
{
    for (const Puzzle::FileHandlerDesc * d = start; d->ext != NULL; ++d)
        if (ext == d->ext)
            return d;
    return NULL;
}

const Puzzle::FileHandlerDesc *
Puzzle::FindLoadHandler(const string_t & ext)
{
    return FindHandler(sm_loadHandlers, ext);
}


const Puzzle::FileHandlerDesc *
Puzzle::FindSaveHandler(const string_t & ext)
{
    return FindHandler(sm_saveHandlers, ext);
}


// -----------------------------------------------------------------------
// Load and save handlers
// -----------------------------------------------------------------------

// Load Handlers
extern void LoadPuz(Puzzle * puz, const string_t & filename, void *);
extern void LoadXPF(Puzzle * puz, const string_t & filename, void *);

const Puzzle::FileHandlerDesc Puzzle::sm_loadHandlers[] = {
    { LoadPuz, puzT("puz"), puzT("Across Lite"), NULL },
    { LoadXPF, puzT("xml"), puzT("XPF"), NULL },
    { NULL, NULL, NULL }
};


// Save Handlers
extern void SavePuz(Puzzle * puz, const string_t & filename, void *);

const Puzzle::FileHandlerDesc Puzzle::sm_saveHandlers[] = {
    { SavePuz, puzT("puz"), puzT("Across Lite"), NULL },
    { NULL, NULL, NULL }
};

} // namespace puz
