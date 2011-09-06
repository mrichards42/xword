// This file is part of XWord
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
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
#include <iostream>

// Format handlers
//#include "formats/jpz/jpz.hpp"
#include "formats/ipuz/ipuz.hpp"
#include "formats/xpf/xpf.hpp"
#include "formats/puz/puz.hpp"
#include "formats/txt/txt.hpp"

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
Puzzle::DoLoad(const std::string & filename, const FileHandlerDesc * desc)
{
    // The loading happens here, to prevent excessive recursion in Puzzle::Load
    Clear();
    try {
        desc->handler(this, filename.c_str(), desc->data);
        if (! m_clues.HasWords())
            GenerateWords();
        TestOk();
    }
    catch (...) {
        // If the load function allocated a new FormatData, delete it.
        m_formatData.reset();
        HandleExceptions(this);
    }
}

void
Puzzle::Load(const std::string & filename, const FileHandlerDesc * desc)
{
    m_formatData.reset();
    if (desc)
    {
        DoLoad(filename, desc);
    }
    else // ! desc, Try all the handlers
    {
        // Prefer the handler with the correct extension
        const FileHandlerDesc * expected
            = FindLoadHandler(GetExtension(filename));
        // Save the error from the expected type
        Exception error;
        if (expected)
        {
            try {
                DoLoad(filename, expected);
                return;
            }
            catch (Exception & e) {
                error = e;
            }
        }
        // Try all the handlers
        for (desc = &sm_loadHandlers[0]; desc->handler != NULL; ++desc)
        {
            if (desc == expected) // We've already tried the expected handler
                continue;
            try {
                DoLoad(filename, desc);
                return;
            }
            catch (Exception &) {
                // Do nothing
            }
        }
        if (expected)
            throw error;
        else
            throw MissingHandler();
    }
}


// -----------------------------------------------------------------------
// Save functions
// -----------------------------------------------------------------------
void
Puzzle::Save(const std::string & filename, const FileHandlerDesc * desc)
{
    if (! desc)
        desc = FindSaveHandler(GetExtension(filename));

    if (! desc)
        throw MissingHandler();

    TestOk();

    desc->handler(this, filename, desc->data);
}


// -----------------------------------------------------------------------
// Grid numbering
// -----------------------------------------------------------------------

void Puzzle::NumberClues()
{
    ClueList & across_clues = GetClueList(puzT("Across"));
    ClueList & down_clues = GetClueList(puzT("Down"));

    ClueList::iterator across = across_clues.begin();
    ClueList::iterator down   = down_clues.begin();

    ClueList::iterator across_end = across_clues.end();
    ClueList::iterator down_end   = down_clues.end();

    // Number the clues based on the grid solution
    int clueNumber = 1;
    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->IsSolutionWhite())
            continue;

        const bool wantsAcross = square->SolutionWantsClue(ACROSS);
        const bool wantsDown   = square->SolutionWantsClue(DOWN);

        if (wantsAcross)
        {
            if (across == across_end)
                throw InvalidClues();
            across->SetNumber(clueNumber);
            ++across;
        }

        if (wantsDown)
        {
            if (down == down_end)
                throw InvalidClues();
            down->SetNumber(clueNumber);
            ++down;
        }

        if (wantsAcross || wantsDown)
            ++clueNumber;
    }
    if (across != across_end || down != down_end)
        throw InvalidClues();

}

template <typename T> int sign(T val)
{
    return (val > T(0)) - (val < T(0));
}


// Number the grid, set the clues, and create words
void Puzzle::SetAllClues(const std::vector<string_t> & clues)
{
    NumberGrid();

    ClueList & across = SetClueList(puzT("Across"), ClueList());
    ClueList & down   = SetClueList(puzT("Down"), ClueList());

    std::vector<string_t>::const_iterator clue_it = clues.begin();
    std::vector<string_t>::const_iterator clue_end = clues.end();

    // Number the clues based on the grid solution
    int clueNumber = 1;
    for (Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->IsSolutionWhite())
            continue;

        const bool wantsAcross = square->SolutionWantsClue(ACROSS);
        const bool wantsDown   = square->SolutionWantsClue(DOWN);

        if (wantsAcross)
        {
            if (clue_it == clue_end)
                throw InvalidClues();
            across.push_back(Clue(clueNumber, *clue_it));
            ++clue_it;
        }

        if (wantsDown)
        {
            if (clue_it == clue_end)
                throw InvalidClues();
            down.push_back(Clue(clueNumber, *clue_it));
            ++clue_it;
        }

        if (wantsAcross || wantsDown)
            ++clueNumber;
    }

    if (clue_it != clue_end)
        throw InvalidClues();
    GenerateWords();
}

void Puzzle::NumberGrid()
{
    m_grid.NumberGrid();
}

void Puzzle::GenerateWords()
{
    Clues::iterator cluelist_it;
    GridDirection dir;
    for (cluelist_it = m_clues.begin(); cluelist_it != m_clues.end(); ++cluelist_it)
    {
        ClueList & cluelist = cluelist_it->second;
        if (cluelist_it->first == puzT("Across"))
            dir = ACROSS;
        else if (cluelist_it->first == puzT("Down"))
            dir = DOWN;
        else if (cluelist_it->first == puzT("Diagonal"))
            dir = DIAGONAL_SE;
        else
            throw InvalidClues();
        ClueList::iterator it;
        for (it = cluelist.begin(); it != cluelist.end(); ++it)
        {
            // Find the square with this clue number.
            Square * start = m_grid.FindSquare(FIND_CLUE_NUMBER(it->GetNumber()));
            if (! start)
                throw InvalidClues("All clues must have a word");
            Square * end = start->GetSolutionWordEnd(dir);
            if (! end)
                throw InvalidClues("All clues must have a word");
            it->SetWord(new StraightWord(start, end));
        }
    }
}


bool Puzzle::UsesNumberAlgorithm() const
{
    if (! (HasClueList(puzT("Across")) && HasClueList(puzT("Down"))))
        return false;
    if (GetClues().size() != 2)
        return false;

    const ClueList & across_clues = GetClueList(puzT("Across"));
    const ClueList & down_clues = GetClueList(puzT("Down"));

    ClueList::const_iterator across = across_clues.begin();
    ClueList::const_iterator down   = down_clues.begin();

    ClueList::const_iterator across_end = across_clues.end();
    ClueList::const_iterator down_end   = down_clues.end();

    // Try to number the clues based on the grid solution
    int clueNumber = 1;
    for (const Square * square = m_grid.First();
         square != NULL;
         square = square->Next())
    {
        if (! square->IsSolutionWhite())
            continue;

        const bool wantsAcross = square->SolutionWantsClue(ACROSS);
        const bool wantsDown   = square->SolutionWantsClue(DOWN);

        if (wantsAcross)
        {
            if (across == across_end || 
                ToInt(across->GetNumber()) != clueNumber)
                return false;
            ++across;
        }

        if (wantsDown)
        {
            if (down == down_end || 
                ToInt(down->GetNumber()) != clueNumber)
                return false;
            ++down;
        }

        if (wantsAcross || wantsDown)
            ++clueNumber;
    }
    if (across != across_end || down != down_end)
        return false;
    return true;
}


//------------------------------------------------------------------------------
// Find functions
//------------------------------------------------------------------------------
const Word *
Puzzle::FindWord(const puz::Square * square) const
{
    const Clue * clue = FindClue(square);
    if (! clue)
        return NULL;
    return clue->GetWord();
}

const Word *
Puzzle::FindWord(const puz::Square * square, short direction) const
{
    short inverseDirection = InvertDirection(direction);
    // Prefer words in exactly the specified direction.
    // If we can't find any of those, return a word in the inverse
    // direction.
    const Word * inverseWord = NULL;

    Clues::const_iterator it;
    for (it = m_clues.begin(); it != m_clues.end(); ++it)
    {
        const Clue * clue = it->second.Find(square);
        if (! clue)
            continue;
        const Word * word = clue->GetWord();
        if (word->Contains(square))
        {
            short wordDirection = word->GetDirection();
            if (wordDirection == direction)
                return word;
            else if (wordDirection == inverseDirection)
                inverseWord = word;
        }
    }
    return inverseWord;
}

const Clue *
Puzzle::FindClue(const puz::Square * square) const
{
    Clues::const_iterator it;
    for (it = m_clues.begin(); it != m_clues.end(); ++it)
    {
        const Clue * clue = it->second.Find(square);
        if (clue)
            return clue;
    }
    return NULL;
}

const Clue *
Puzzle::FindClue(const puz::Word * word) const
{
    Clues::const_iterator it;
    for (it = m_clues.begin(); it != m_clues.end(); ++it)
    {
        const Clue * clue = it->second.Find(word);
        if (clue)
            return clue;
    }
    return NULL;
}


Word *
Puzzle::FindWord(const puz::Square * square)
{
    return const_cast<Word *>(const_cast<const Puzzle *>(this)
        ->FindWord(square));
}

Word *
Puzzle::FindWord(const puz::Square * square, short direction)
{
    return const_cast<Word *>(const_cast<const Puzzle *>(this)
            ->FindWord(square, direction));
}

Clue *
Puzzle::FindClue(const puz::Square * square)
{
    return const_cast<Clue *>(const_cast<const Puzzle *>(this)
        ->FindClue(square));
}

Clue *
Puzzle::FindClue(const puz::Word * word)
{
    return const_cast<Clue *>(const_cast<const Puzzle *>(this)
        ->FindClue(word));
}


//------------------------------------------------------------------------------
// Puzzle validation
//------------------------------------------------------------------------------


void Puzzle::TestClueList(const string_t & direction)
{
    const ClueList & clues = GetClueList(direction);
    ClueList::const_iterator it;
    for (it = clues.begin(); it != clues.end(); ++it)
    {
        const Word * word = it->GetWord();
        if (word == NULL)
            throw InvalidClues("All clues must have a word.");
        // Make sure that no words cross missing squares.
        const_square_iterator word_it;
        for (word_it = word->begin(); word_it != word->end(); ++word_it)
            if (word_it->IsMissing())
                throw InvalidWord("Words cannot contain missing squares.");
    }
}

void
Puzzle::TestOk()
{
    m_isOk = false;
    // Make sure there is a grid
    if (m_grid.GetWidth() == 0 || m_grid.GetHeight() == 0)
        throw InvalidGrid();

    // Make sure that all clues have words.
    {
        Clues::const_iterator it;
        for (it = m_clues.begin(); it != m_clues.end(); ++it)
            TestClueList(it->first);
    }

    m_isOk = true;
}

//------------------------------------------------------------------------------
// Static functions (load/save handlers)`
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
// Load and save handlers
// -----------------------------------------------------------------------

// Load handlers
const Puzzle::FileHandlerDesc Puzzle::sm_loadHandlers[] = {
    { LoadPuz, "puz", puzT("Across Lite"), NULL },
    { LoadTxt, "txt", puzT("Across Lite Text"), NULL },
    { LoadXPF, "xml", puzT("XPF"), NULL },
    //{ LoadJpz, "jpz", puzT("jpuz"), NULL },
    { LoadIpuz,"ipuz", puzT("ipuz"), NULL },
    { NULL, NULL, NULL }
};

const Puzzle::FileHandlerDesc Puzzle::sm_saveHandlers[] = {
    { SavePuz, "puz", puzT("Across Lite"), NULL },
    { SaveXPF, "xml", puzT("XPF"), NULL },
    //{ SaveJpz, "jpz", puzT("jpuz"), NULL },
    { NULL, NULL, NULL }
};

} // namespace puz
