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


#include "Square.hpp"
#include "Grid.hpp"

//#define PUZ_CHECK_STRINGS

namespace puz {

//------------------------------------------------------------------------------
// Constructors and operator=()
//------------------------------------------------------------------------------

// Default constructor
// Square is white and blank.
Square::Square()
    : m_col(-1),
      m_row(-1),
      m_flag(FLAG_CLEAR),
      m_number(),
      m_red(255),
      m_green(255),
      m_blue(255)
{
    SetText(puzT(""));
    SetSolution(puzT(""));

    // These will be filled in by the grid
    m_next[ACROSS][PREV] = NULL;
    m_next[ACROSS][NEXT] = NULL;
    m_next[DOWN]  [PREV] = NULL;
    m_next[DOWN]  [NEXT] = NULL;

    m_isLast[ACROSS][PREV] = false;
    m_isLast[ACROSS][NEXT] = false;
    m_isLast[DOWN]  [PREV] = false;
    m_isLast[DOWN]  [NEXT] = false;

    m_wordStart[ACROSS] = NULL;
    m_wordEnd  [ACROSS] = NULL;
    m_wordStart[DOWN]   = NULL;
    m_wordEnd  [DOWN]   = NULL;

    m_hasClue[ACROSS] = false;
    m_hasClue[DOWN]   = false;
}

// Copy constructor
// Square is copied because it is in a std::vector and needs to
// be relocated (Grid::SetSize() was caled).  Spatial information
// must be reset.  Linked-list information will be filled in
// by the grid.
Square::Square(const Square & other)
    : m_col(other.m_col),
      m_row(other.m_row),
      m_asciiSolution(other.m_asciiSolution),
      m_solution(other.m_solution),
      m_text(other.m_text),
      m_flag(other.m_flag),
      m_number(other.m_number),
      m_red(other.m_red),
      m_green(other.m_green),
      m_blue(other.m_blue)
{
    m_next[ACROSS][PREV] = NULL;
    m_next[ACROSS][NEXT] = NULL;
    m_next[DOWN]  [PREV] = NULL;
    m_next[DOWN]  [NEXT] = NULL;

    m_isLast[ACROSS][PREV] = false;
    m_isLast[ACROSS][NEXT] = false;
    m_isLast[DOWN]  [PREV] = false;
    m_isLast[DOWN]  [NEXT] = false;

    m_wordStart[ACROSS] = NULL;
    m_wordEnd  [ACROSS] = NULL;
    m_wordStart[DOWN]   = NULL;
    m_wordEnd  [DOWN]   = NULL;

    m_hasClue[ACROSS] = other.m_hasClue[ACROSS];
    m_hasClue[DOWN]   = other.m_hasClue[DOWN];
}

// Since all constructors are private, only a grid can create squares.
// Thus, this function will only be called as follows:
//     grid1.At(col, row) = grid2.At(col, row);
// In order to preserve iteration, we need to prevent grid information
// from being copied.
//
// Word start and end can be preserved if we are copying white square
// to white square or black square to black square.
// Otherwise we need reset word start/end (for every square in the word,
// start to end).
// This could be a somewhat expensive operation.
Square & Square::operator=(const Square & other)
{
    m_asciiSolution = other.m_asciiSolution;
    m_solution = other.m_solution;
    m_text = other.m_text;
    m_flag = other.m_flag;

    m_red = other.m_red;
    m_green = other.m_green;
    m_blue = other.m_blue;

    m_number = other.m_number;
    m_hasClue[ACROSS] = other.m_hasClue[ACROSS];
    m_hasClue[DOWN] = other.m_hasClue[DOWN];

    // Don't copy grid information

    // Reset word and clue info if needed (for all squares in the word).
    if (IsWhite() != other.IsWhite())
    {
        // Words ACROSS and DOWN (a little convoluted, yes)
        for (GridDirection dir = ACROSS; ; dir = DOWN)
        {
            Square * end = GetWordEnd(dir);
            for (Square * square = GetWordStart(dir);
                 square != end; square = square->Next())
            {
                square->m_wordStart[dir] =
                square->m_wordEnd[dir] = NULL;
            }
            if (dir == DOWN)
                break;
        }
    }

    return *this;
}


// Character tables and the definition of Square::Blank
#include "char_tables.hpp"

//------------------------------------------------------------------------------
// Character table functions (static)
//------------------------------------------------------------------------------


char Square::ToPlain(int ch)
{
    if (ch > 0 && ch < (sizeof(ascii) / sizeof(char)))
        return ascii[ch];
    return 0;
}

char Square::ToPlain(const string_t & str)
{
    if (str.empty())
        return ToPlain(Blank);
    return ToPlain(to_unicode(str).at(0));
}

char_t Square::ToGrid(int ch)
{
    if (ch == Black[0] || ch == puzT('['))
        return 0;

#if PUZ_UNICODE
    return iswprint(ch) ? towupper(ch) : 0;
#else
    return isprint(ch) ? toupper(ch) : 0;
#endif // PUZ_UNICODE
}

string_t Square::ToGrid(const string_t & str)
{
#ifdef PUZ_CHECK_STRINGS
    if (str.length() > REBUS_ENTRY_LENGTH)
        throw LongString();
#endif 
    string_t ret = str;
    string_t::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it)
    {
        *it = ToGrid(*it);
#ifdef PUZ_CHECK_STRINGS
        if (*it == 0)
            throw InvalidString();
#endif
    }
    return ret;
}

bool Square::IsValidChar(int ch)
{
    return ToGrid(ch) != 0;
}


bool Square::IsValidString(const string_t & str)
{
    if (str == Black)
        return true;

#if PUZ_UNICODE
    std::wstring::const_iterator begin = str.begin();
    std::wstring::const_iterator end   = str.end();
#else
    std::wstring unicode = to_unicode(str);
    std::wstring::const_iterator begin = unicode.begin();
    std::wstring::const_iterator end   = unicode.end();
#endif // PUZ_UNICODE

    std::wstring::const_iterator it;
    for (it = begin; it != end; ++it)
        if (! IsValidChar(*it))
            return false;
    return true;
}

//------------------------------------------------------------------------------
// Clue
//------------------------------------------------------------------------------
bool Square::WantsClue(GridDirection dir) const
{
    return IsWhite() &&
           GetWordStart(dir) == this &&
           GetWordEnd  (dir) != this;
}

//------------------------------------------------------------------------------
// Word start and end
//------------------------------------------------------------------------------
static Square * FindWord(Square * square, GridDirection dir, FindDirection inc)
{
    // Only white squares have a word
    if (! square->IsWhite())
        return NULL;

    // Iterate until the grid edge or a black/missing square.
    for (;;)
    {
        // Not a white square (one past the end)
        if (! square->IsWhite())
            return square->Prev(dir, inc);
        // Edge of the grid
        if (square->IsLast(dir, inc))
            return square;
        square = square->Next(dir, inc);
    }
}


Square * Square::GetWordStart(GridDirection dir)
{
    if (m_wordStart[dir] == NULL && IsWhite())
        m_wordStart[dir] = FindWord(this, dir, PREV);
    return m_wordStart[dir];
}

const Square * Square::GetWordStart(GridDirection dir) const 
{
    return const_cast<Square *>(this)->GetWordStart(dir);
}


Square * Square::GetWordEnd(GridDirection dir)
{
    if (m_wordEnd[dir] == NULL && IsWhite())
        m_wordEnd[dir] = FindWord(this, dir, NEXT);
    return m_wordEnd[dir];
}

const Square * Square::GetWordEnd(GridDirection dir) const
{
    return const_cast<Square *>(this)->GetWordEnd(dir);
}

bool Square::HasWord(GridDirection dir) const
{
    const Square * start = GetWordStart(dir);
    const Square * end = GetWordEnd(dir);
    return start && end && start != end;
}


//------------------------------------------------------------------------------
// Color
//------------------------------------------------------------------------------
void Square::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    m_red = red;
    m_green = green;
    m_blue = blue;
    if (red == 255 && green == 255 && blue == 255)
        RemoveFlag(FLAG_COLOR);
    else
        AddFlag(FLAG_COLOR);
}

void Square::RemoveColor()
{
    SetColor(255,255,255);
    RemoveFlag(FLAG_COLOR);
}



//------------------------------------------------------------------------------
// Square text and solution
//------------------------------------------------------------------------------

void Square::SetText(const string_t & text)
{
    if (text.empty())
        m_text = Blank;
    else if (text == Black || IsSymbol(text))
        m_text = text;
    else
        m_text = ToGrid(text);
}

void Square::SetSolution(const string_t & solution)
{
    SetSolution(solution, ToPlain(solution));
}

void Square::SetSolution(const string_t & solution, char plain)
{
    SetSolutionRebus(solution);
    if (plain != 0)
        SetPlainSolution(plain);
}

void Square::SetPlainSolution(char solution)
{
    solution = ToPlain(solution);
#ifdef PUZ_CHECK_STRINGS
    if (solution == 0)
        throw InvalidString();
#endif
    // This could break word start and end

    m_asciiSolution = solution;
}

void Square::SetSolutionRebus(const string_t & rebus)
{
    if (rebus.empty())
        m_solution = Blank;
    else if (rebus == Blank ||
             rebus == Black ||
             IsSymbol(rebus))
        m_solution = rebus;
    else
        m_solution = ToGrid(rebus);
}

void Square::SetSolutionSymbol(unsigned char symbol)
{
    m_solution = puzT("[ ]");
    m_solution.at(1) = static_cast<char_t>(symbol);
}

bool Square::HasTextRebus() const
{
    const size_t len = m_text.length();
    return len > 1 || (len > 0 && m_text.at(0) != GetPlainText());
}


bool Square::HasSolutionRebus() const
{
    const size_t len = m_solution.length();
    return len > 1 || (m_solution.at(0) != GetPlainSolution());
}


bool Square::HasTextSymbol() const
{
    return IsSymbol(m_text);
}


bool Square::HasSolutionSymbol() const
{
    return IsSymbol(m_solution);
}

char_t Square::GetTextSymbol() const
{
    if (! HasTextSymbol())
        throw NoSymbol();
    return m_text.at(1);
}


char_t Square::GetSolutionSymbol() const
{
    if (! HasSolutionSymbol())
        throw NoSymbol();
    return m_solution.at(1);
}



bool Square::Check(bool checkBlank, bool strictRebus)  const
{
    if (IsBlack())
        return true;
    if (IsBlank() && ! IsSolutionBlank())
        return ! checkBlank;

    if (strictRebus || HasTextRebus())
        return m_solution == m_text;
    else
        return GetPlainText() == GetPlainSolution();
}

bool Square::IsSymbol(const string_t & str)
{
    return str.length() == 3
        && str[0] == puzT('[')
        && str[2] == puzT(']');
}

} // namespace puz
