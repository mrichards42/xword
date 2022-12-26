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


#include "XGridCtrl.hpp"
#include "XGridRebus.hpp"
#include <wx/dcbuffer.h>
#include <wx/tooltip.h>
#include <list>
#include <algorithm>
#include <cfloat>
#include "PuzEvent.hpp"
#include "XGridDrawer.hpp"
#include "messages.hpp"
#include "SelectionEvent.hpp"
#include "App.hpp"
#include "config.hpp"
#include "utils/string.hpp"
#include "utils/timeit.hpp"
#include "puz/Puzzle.hpp"

// This class will take over the XGridCtrl's event processing
// for selections.  It must be created on the heap, and it
// will destroy itself when it is done.
class GridSelectionHandler : wxEvtHandler
{
public:
    GridSelectionHandler (XGridCtrl & grid);
    void EndEventHandling();

    DECLARE_EVENT_TABLE()

private:
    XGridCtrl & m_grid;
    puz::Square * m_start;
    puz::Square * m_end;
    wxRect m_selection;
    wxRect m_lastSelection;

    // The event handling
    void OnLeftDown(wxMouseEvent & evt);
    void OnLeftUp(wxMouseEvent & evt);
    void OnMouseMove(wxMouseEvent & evt);
    void OnKeyDown(wxKeyEvent & evt);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent & WXUNUSED(evt));
};



//------------------------------------------------------------------------------
// Functions / functors for FindSquare
//------------------------------------------------------------------------------
using puz::FIND_WHITE_SQUARE;
using puz::FIND_BLANK_SQUARE;

// This looks for square->number, so in order to use it we must give
//    this struct and Find*Next*Square a square at the start of a word
struct FIND_NEW_CLUE
{
    FIND_NEW_CLUE(puz::GridDirection direction, const puz::Square * square)
        : m_direction(direction),
          m_number(square->GetNumber())
    {
    }

    bool operator() (const puz::Square * square)
    {
        if (! square->IsWhite())
            return false;
        // The standard idea of where to stop:
        // The square has a clue and number that does not match the
        // given number
        if (square->HasNumber())
        {
            return square->GetNumber() != m_number;
        }
        // The square wants a clue in this direction
        else if (square->WantsClue(m_direction))
        {
            return true;
        }
        return false;
    }

    puz::GridDirection m_direction;
    puz::string_t m_number;
};


puz::Square * XGridCtrl::FirstWhite()
{
    return m_grid->FindSquare(FIND_WHITE_SQUARE);
}

const puz::Square * XGridCtrl::FirstWhite() const
{
    return m_grid->FindSquare(FIND_WHITE_SQUARE);
}

puz::Square * XGridCtrl::LastWhite()
{
    return m_grid->FindSquare(m_grid->Last(), FIND_WHITE_SQUARE, puz::LEFT);
}

const puz::Square * XGridCtrl::LastWhite() const
{
    return m_grid->FindSquare(m_grid->Last(), FIND_WHITE_SQUARE, puz::LEFT);
}

//------------------------------------------------------------------------------
// XGridCtrl implementation
//------------------------------------------------------------------------------

// The static events
// The following events will be connected or disconnected depending on whether
// there is actually a grid to draw (see ConnectEvents and DisconnectEvents):
//     EVT_KEY_DOWN
//     EVT_CHAR
//     EVT_LEFT_DOWN
//     EVT_RIGHT_DOWN
BEGIN_EVENT_TABLE(XGridCtrl, wxScrolledCanvas)
    EVT_PAINT           (XGridCtrl::OnPaint)
    EVT_SIZE            (XGridCtrl::OnSize)
    EVT_CONTEXT_MENU    (XGridCtrl::OnContextMenu)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(XGridCtrl, wxScrolledCanvas)

const int UNDEFINED_BOX_SIZE = -1;

const char * MSG_NO_INCORRECT = "No Incorrect Letters!";

wxColour EraseColor; // Sentinel used to erase a square's background

// Helper functions for all of the "lookup" functions that return NULL
inline void
SetIfExists(puz::Square * &current, puz::Square * test)
{
    if (test != NULL)
        current = test;
}


puz::Square *
EitherOr(puz::Square * current, puz::Square * test)
{
    if (test != NULL)
        return test;
    return current;
}


const wxChar * XGridCtrlNameStr = _T("XGridCtrl");



void XGridCtrl::Init()
{
    m_grid = NULL;

    m_focusedSquare = NULL;
    m_focusedWord = NULL;
    m_ownsFocusedWord = false;
    m_focusedDirection = puz::ACROSS;

    m_selectionStart = NULL;
    m_selectionEnd = NULL;
    m_isSelecting = false;

    m_isPaused = false;
    m_fit = true;

    m_lastBoxSize = UNDEFINED_BOX_SIZE;

    m_rebusCtrl = NULL;

    m_areEventsConnected = false;

    // m_rect is already equal to wxRect(0,0,0,0) from its constructor

    // Config
    ConfigManager::Grid_t & grid = wxGetApp().GetConfigManager().Grid;

    grid.style.AddCallback(this, &XGridCtrl::SetGridStyle);
    grid.letterFont.AddCallback(this, &XGridCtrl::SetLetterFont);
    grid.numberFont.AddCallback(this, &XGridCtrl::SetNumberFont);
    grid.lineThickness.AddCallback(this, &XGridCtrl::SetBorderSize);
    grid.focusedLetterColor.AddCallback(this, &XGridCtrl::SetFocusedLetterColor);
    grid.focusedWordColor.AddCallback(this, &XGridCtrl::SetFocusedWordColor);
    grid.backgroundColor.AddCallback(this, &XGridCtrl::SetBackgroundColour);
    grid.whiteSquareColor.AddCallback(this, &XGridCtrl::SetWhiteSquareColor);
    grid.blackSquareColor.AddCallback(this, &XGridCtrl::SetBlackSquareColor);
    grid.highlightTheme.AddCallback(this, &XGridCtrl::ShowThemeHighlight);
    grid.themeSquareColor.AddCallback(this, &XGridCtrl::SetThemeSquareColor);
    grid.selectionColor.AddCallback(this, &XGridCtrl::SetSelectionColor);
    grid.penColor.AddCallback(this, &XGridCtrl::SetPenColor);
    grid.pencilColor.AddCallback(this, &XGridCtrl::SetPencilColor);
    grid.numberScale.AddCallback(this, &XGridCtrl::SetNumberScale);
    grid.letterScale.AddCallback(this, &XGridCtrl::SetLetterScale);
}


bool
XGridCtrl::Create(wxWindow * parent,
                  wxWindowID id,
                  const wxPoint & pos,
                  const wxSize & size,
                  puz::Puzzle * puz,
                  long style,
                  const wxString & name)
{
    style |= wxWANTS_CHARS;
    if (! wxScrolledCanvas::Create(parent, id, pos, size, style, name))
        return false;

    // Have to do this here, or wxMac will break
    // (XGridDrawer would use GetTextExtent on a not yet created window)
    m_drawer.SetWindow(this);

    SetPuzzle(puz);

    SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    SetFocusedLetterColor(*wxGREEN);
    SetFocusedWordColor(*wxLIGHT_GREY);
    SetWhiteSquareColor(*wxWHITE);
    SetPenColor(*wxBLACK);
    SetPencilColor(wxColor(200,200,200));
    SetSelectionColor(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

    // Custom background style needed for wxAutoBufferedPaintDC
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW) );

    SetScrollRate(20, 20);
    return true;
}


XGridCtrl::~XGridCtrl()
{
    if (m_ownsFocusedWord)
        delete m_focusedWord;
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}


void
XGridCtrl::SetPuzzle(puz::Puzzle * puz)
{
    m_puz = puz;
    if (m_ownsFocusedWord)
        delete m_focusedWord;
    if (puz == NULL)
    {
        m_grid = NULL;
        m_focusedSquare = NULL;
        m_focusedWord = NULL;
        m_ownsFocusedWord = false;

        DisconnectEvents();
    }
    else
    {
        m_grid = &puz->GetGrid();

        // Attempt to find a canonical "first" clue list.
        puz::Clues* clues = &puz->GetClues();
        puz::ClueList* firstClueList = NULL;
        if (!m_grid->IsDiagramless()) {
            if (clues->find(puzT("Across")) != clues->end()) {
                // If there's an "Across" list, use that.
                firstClueList = &clues->GetAcross();
            }
            else if (!clues->empty()) {
                // Select the first clue list - assumes that the file is likely in the intended
                // clue list order (and that this order is retained when reading the file).
                firstClueList = &clues->begin()->second;
            }
        }

        m_focusedWord = NULL;
        m_ownsFocusedWord = false;
        puz::Word * firstWord = NULL;
        if (firstClueList != NULL && !firstClueList->empty()) {
            // If we found a canonical "first" clue list, focus on the first word in that list.
            firstWord = &firstClueList->front().GetWord();
            m_focusedSquare = firstWord->front();
            m_focusedDirection = firstWord->GetDirection();
        } else {
            // Otherwise, focus on a word containing the first white square.
            m_focusedSquare = FirstWhite();
            m_focusedDirection = puz::ACROSS;
        }
        DoSetFocusedWord(m_focusedSquare, firstWord, m_focusedDirection);

        if (m_grid->IsDiagramless())
            GetGrid()->NumberGrid();

        ConnectEvents();
    }
    m_drawer.SetPuzzle(puz);
    if (puz != NULL) {
        SetFocusedSquare(m_focusedSquare);
    }
    Scale();
}

bool
XGridCtrl::UnscrambleSolution(unsigned short key)
{
    const bool success = m_grid->UnscrambleSolution(key);

    if (success)
    {
        // If we should have been checking while typing, go ahead and
        // check the whole grid now that it is possible.
        if (HasStyle(CHECK_WHILE_TYPING))
            CheckGrid(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);
    }

    return success;
}


// Helper for IsCorrect and GetStats
CorrectStatus GetCorrectStatus(const puz::Grid * grid, bool correct)
{
    // We *can* test scrambled puzzles!  Not sure why I didn't think of
    // this before.  (Inspired by Alex Boisvert:
    // http://alexboisvert.com/software.html#check)
    if (correct)
    {
        return CORRECT_PUZZLE;
    }
    else if (grid->IsScrambled())
    {
        if (grid->CheckScrambledGrid())
            return CORRECT_PUZZLE;
        else
            return INCORRECT_PUZZLE;
    }
    else if (! grid->HasSolution())
    {
        return UNCHECKABLE_PUZZLE;
    }
    else
    {
        return INCORRECT_PUZZLE;
    }
}

CorrectStatus
XGridCtrl::IsCorrect() const
{
    bool strictRebus = HasStyle(STRICT_REBUS);
    bool correct = true;
    puz::Square * square;
    for (square = m_grid->First(); square != NULL; square = square->Next())
    {
        if (square->IsBlank())
            return INCOMPLETE_PUZZLE;
        else if (! square->Check(puz::NO_CHECK_BLANK, strictRebus))
            correct = false;
    }
    return GetCorrectStatus(m_grid, correct);
}

void
XGridCtrl::GetStats(GridStats * stats) const
{
    stats->blank_correct = 0;
    stats->blank = 0;
    stats->black = 0;
    stats->white = 0;

    bool strictRebus = HasStyle(STRICT_REBUS);
    bool correct = true;
    puz::Square * square;
    for (square = m_grid->First(); square != NULL; square = square->Next())
    {
        if (square->IsBlack())
        {
            ++stats->black;
        }
        else if (!square->IsMissing())
        {
            ++stats->white;
            if (square->IsBlank())
            {
                ++stats->blank;
                if (square->IsSolutionBlank())
                    ++stats->blank_correct;
            }
            // If the puzzle is correct so far, and without blanks (that do
            // not also have a blank solution), check this square
            else if (stats->blank == stats->blank_correct
                     && correct
                     && ! square->Check(puz::NO_CHECK_BLANK, strictRebus))
            {
                correct = false;
            }
        }
    }
    // Set the correct flag
    // If we have a puzzle with a partially blank solution, don't alert the
    // user if there are incorrect letters since that would give away the
    // blank entries
    if (stats->blank == 0)
        stats->correct = GetCorrectStatus(m_grid, correct);
    else if (stats->blank == stats->blank_correct && correct)
        stats->correct = CORRECT_PUZZLE;
    else
        stats->correct = INCOMPLETE_PUZZLE;
}

//-------------------------------------------------------
// Drawing functions
//-------------------------------------------------------
void
XGridCtrl::OnPaint(wxPaintEvent & WXUNUSED(evt))
{
    //TimeIt(_T("XGridCtrl::OnPaint"));
    // This OnPaint handler is needed so we can use an auto-buffered wxPaintDC
    // The default OnPaint handler is identical, but uses wxPaintDC instead.
    wxAutoBufferedPaintDC dc(this);
    DoPrepareDC(dc);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    if (! m_isPaused)
    {
        if (m_rebusCtrl && ! m_rebusCtrl->IsShown())
            m_rebusCtrl->Show();
        DrawGrid(dc, GetUpdateRegion());
        if (!IsEmpty()) {
            ConnectEvents();
        }
    }
    else
    {
        if (m_rebusCtrl && m_rebusCtrl->IsShown())
            m_rebusCtrl->Hide();
        DrawPauseMessage(dc);
        DisconnectEvents();
    }

    //wxScrolledWindow::SetFocus();
}


void
XGridCtrl::DrawPauseMessage(wxDC & dc)
{
    wxString msg = _T("(Paused)");
    // Scale the font
    wxFont font = GetFont();
    int max_width, max_height;
    GetClientSize(&max_width, &max_height);
    int w = 0, h = 0;
    while (w < max_width && h < max_height && font.GetPointSize() < 50)
    {
        font.SetPointSize(font.GetPointSize() + 2);
        dc.SetFont(font);
        dc.GetTextExtent(msg, &w, &h);
    }
    while ((w > max_width || h > max_height) && font.GetPointSize() > 6)
    {
        font.SetPointSize(font.GetPointSize() - 2);
        dc.SetFont(font);
        dc.GetTextExtent(msg, &w, &h);
    }
    // Draw the label
    dc.DrawLabel(msg, wxRect(wxPoint(0,0), GetClientSize()), wxALIGN_CENTER);
}


void
XGridCtrl::DrawGrid(wxDC & dc, const wxRegion & updateRegion)
{
    if (IsEmpty())
        return;

#ifdef _DEBUG
        int _scrollX, _scrollY;
        GetScrollPixelsPerUnit(&_scrollX, &_scrollY);
        wxASSERT(_scrollX == GetSquareSize() && _scrollY == GetSquareSize());
#endif // _DEBUG

    if (m_drawer.GetBoxSize() == 0)
    {
        wxLogDebug(_T("Size is too small"));
        //return;
    }

    puz::Square * square;

    // If we don't have an update region, redraw all squares
    if (updateRegion.IsEmpty())
        for (square = m_grid->First(); square != NULL; square = square->Next())
            DrawSquare(dc, *square, GetSquareColor(*square), /* propagate= */ false);

    // If we do have an update region, redraw all squares within the region
    else
    {
        // Just deal with the update rect, not region
        // it's easier and faster.
        wxRect rect = updateRegion.GetBox();
        if (rect.IsEmpty())
            return;

        // Adjust update rect based on position of grid rect
        rect.Offset(- m_drawer.GetLeft(), - m_drawer.GetTop());

        // Adjust udpate rect based on scroll position
        int scrollX, scrollY;
        int scrollPix = GetSquareSize();
        GetViewStart(&scrollX, &scrollY);
        rect.Offset(scrollX * scrollPix, scrollY * scrollPix);

        int firstCol = rect.GetLeft()   / GetSquareSize();
        int firstRow = rect.GetTop()    / GetSquareSize();
        int lastCol  = rect.GetRight()  / GetSquareSize();
        int lastRow  = rect.GetBottom() / GetSquareSize();

        // Fit everything within the bounds of 0 - last
        firstCol = std::min(std::max(firstCol, 0), m_grid->LastCol());
        lastCol  = std::min(std::max(lastCol,  0), m_grid->LastCol());
        firstRow = std::min(std::max(firstRow, 0), m_grid->LastRow());
        lastRow  = std::min(std::max(lastRow,  0), m_grid->LastRow());

        square = &At(firstCol, firstRow);
        while(square != NULL)
        {
            DrawSquare(dc, *square);

            // If we're at the end of a row, loop to the next row
            if (square->GetCol() == lastCol)
            {
                // If this is the last square, stop drawing
                if (square->GetRow() == lastRow)
                    break;
                square = &At(firstCol, square->GetRow() + 1);
            }
            else
                square = square->Next();
        }
    }
}



void
XGridCtrl::DrawSquare(wxDC & dc, const puz::Square & square, const wxColour & color, bool propagate)
{
    // Don't draw missing squares
    if (square.IsMissing())
        return;

    // If the user has pressed <insert> we are in rebus mode.  The focused square
    // will be outlined in the selected color (and have a white background).
    // Certain features will not be drawn in this case:
    //    - The number
    //    - The incorrect/revealed indicator
    //    - The X.
    const bool drawOutline = IsFocusedLetter(square) &&
                                (IsRebusEntry() || square.IsBlack());
    if (drawOutline)
    {
        m_drawer.AddFlag(XGridDrawer::DRAW_OUTLINE);
        m_drawer.RemoveFlag(XGridDrawer::DRAW_FLAG | XGridDrawer::DRAW_NUMBER);
    }

    if (color == wxNullColour || &color == &EraseColor)
        m_drawer.DrawSquare(dc, square);
    else
        m_drawer.DrawSquare(dc, square, color, GetPenColor(), /* blendBackground= */ true);

    // Reset the drawing mode
    if (drawOutline)
    {
        m_drawer.RemoveFlag(XGridDrawer::DRAW_OUTLINE);
        m_drawer.AddFlag(XGridDrawer::DRAW_FLAG | XGridDrawer::DRAW_NUMBER);
    }

    if (propagate && !square.GetPartnerSquares().empty()) {
        std::vector<puz::Square*> partners = square.GetPartnerSquares();
        for (std::vector<puz::Square*>::iterator it = partners.begin(); it != partners.end(); ++it) {
            puz::Square* partner = *it;
            if (&color == &EraseColor)
                DrawSquare(dc, *partner, EraseColor, false);
            else
                DrawSquare(dc, *partner, GetSquareColor(*partner), false);
        }
    }
}










//-------------------------------------------------------
// Focus functions
//-------------------------------------------------------
puz::Square *
XGridCtrl::SetFocusedSquare(puz::Square * square,
                            puz::Word * word,
                            short direction)
{
    wxASSERT(m_focusedSquare != NULL || square != NULL);
    if (square == NULL)
        square = m_focusedSquare;

    wxASSERT(square->IsWhite() || m_grid->IsDiagramless());

    if (! word)
    {
        // If there is no word supplied, see if the current word works
        if (m_focusedWord && m_focusedWord->Contains(square)
            && m_focusedWord->GetDirection() == direction)
        {
            word = m_focusedWord;
        }
        // If there is no word in the given direction and there is one
        // in the opposite direction, switch the direction.
        else if (! square->HasWord(static_cast<puz::GridDirection>(direction))
            && ! m_puz->FindWord(square, direction))
        {
            if (GetGrid()->IsAcrostic())
            {
                direction = puz::ACROSS;
            }
            else
            {
                puz::GridDirection newdir
                    = puz::IsVertical(direction) ? puz::ACROSS : puz::DOWN;
                if (square->HasWord(newdir))
                    direction = newdir;
                else
                {
                    word = m_puz->FindWord(square, newdir);
                    if (word)
                        direction = newdir;
                }
            }
        }
    }
    // Save old state
    //-------------------
    puz::Square * oldSquare = m_focusedSquare;
    puz::Word * oldWord;
    if (oldSquare == NULL)
        oldWord = NULL;
    else
        oldWord = m_focusedWord;

    const bool isrebus = IsRebusEntry();
    if (isrebus)
        EndRebusEntry();

    wxClientDC dc(this); DoPrepareDC(dc);

    // If this was an invented focused word, it will be deleted before
    // we have a chance to redraw.  We need to redraw it now.
    if (m_ownsFocusedWord && oldWord)
    {
        puz::square_iterator it;
        for (it = oldWord->begin(); it != oldWord->end(); ++it)
            DrawSquare(dc, *it, EraseColor);
        oldWord = NULL;
    }

    // Set new state
    //--------------
    m_focusedSquare = square;
    if (isrebus)
        StartRebusEntry();
    DoSetFocusedWord(square, word, direction);
    wxASSERT(m_focusedWord != NULL);

    if (! MakeVisible(*m_focusedSquare))
    { // We didn't refresh
        if (oldWord == m_focusedWord)
        {
            if (oldSquare != m_focusedSquare && oldSquare)
                DrawSquare(dc, *oldSquare, GetFocusedWordColor());
            // Always redraw the newly focused square.
            DrawSquare(dc, *m_focusedSquare, GetFocusedLetterColor());
        }
        else
        {
            // Draw the old word
            if (oldWord)
            {
                puz::square_iterator it;
                for (it = oldWord->begin(); it != oldWord->end(); ++it)
                    DrawSquare(dc, *it, EraseColor);
            }
            else if (oldSquare)
                DrawSquare(dc, *oldSquare, EraseColor);

            // Draw the new focused word
            if (m_focusedWord)
            {
                puz::square_iterator it;
                for (it = m_focusedWord->begin(); it != m_focusedWord->end(); ++it)
                    DrawSquare(dc, *it);
            }
        }
    }

    RecalcDirection();
    SendEvent(wxEVT_PUZ_GRID_FOCUS);
    return m_focusedSquare;
}

puz::Square *
XGridCtrl::MoveFocusedSquare(puz::Square * square,
                             puz::Word * word,
                             short direction)
{
    if (! HasStyle(BLANK_ON_NEW_WORD))
        return SetFocusedSquare(square, word, direction);
    if (square == NULL)
        square = m_focusedSquare;
    // Find the new word
    word = CalcFocusedWord(square, word, direction);
    // If this is not a new word, we're done
    if (m_focusedWord != NULL && m_focusedWord == word)
        return SetFocusedSquare(square, word, direction);
    if (word)
    {
        puz::Square * blank = word->FindSquare(FIND_BLANK_SQUARE);
        if (blank)
            square = blank;
        return SetFocusedSquare(square, word, direction);
    }
    else // If this word doesn't exist, we need to create it
    {
        if (puz::IsDiagonal(direction)) // Don't make up diagonal words.
            direction = m_focusedDirection;
        puz::GridDirection dir = puz::IsHorizontal(direction)
                                    ? puz::ACROSS : puz::DOWN;
        word = new puz::Word(
            square->GetWordStart(dir),
            square->GetWordEnd(dir)
        );
        // Check that this is a new word.
        if (! (m_focusedWord->front() == word->front() &&
               m_focusedWord->back() == word->back()))
        {
            // Find the next blank square
            puz::Square * blank = word->FindSquare(FIND_BLANK_SQUARE);
            if (blank)
                square = blank;
        }
        delete word;
        return SetFocusedSquare(square, NULL, direction);
    }
}



void
XGridCtrl::SendEvent(int type)
{
    wxPuzEvent evt(type, GetId());
    evt.SetSquare(m_focusedSquare);
    evt.SetWord(m_focusedWord);
    GetEventHandler()->ProcessEvent(evt);
}


void
XGridCtrl::SetFocusedWord(puz::Word * word, short direction)
{
    wxASSERT(word != NULL);
    puz::Square * square = word->front();
    if (HasStyle(BLANK_ON_NEW_WORD))
        SetIfExists(square, word->FindSquare(FIND_BLANK_SQUARE));
    SetFocusedSquare(square, word, direction);
}

// Figure out the focused word.
puz::Word *
XGridCtrl::CalcFocusedWord(puz::Square * square,
                           puz::Word * word,
                           short direction)
{
    // If we were supplied with a word, we're done.
    if (word != NULL)
        return word;
    // Diagramless puzzles always have to invent a word.
    if (m_grid->IsDiagramless())
        return NULL;
    // If the current word contains this square and it is in the
    // supplied direction, use the current word.
    if (m_focusedWord != NULL
        && puz::AreInLine(m_focusedDirection, direction)
        && m_focusedWord->Contains(square))
    {
        return m_focusedWord;
    }
    // Look for a word in the supplied direction
    word = m_puz->FindWord(square, direction);
    if (word)
        return word;
    // If we moved diagonally, try using the focused direction
    if (puz::IsDiagonal(direction))
    {
        word = m_puz->FindWord(square, m_focusedDirection);
        if (word)
            return word;
    }
    // Look for a word that contains the focused square that is neither across
    // nor down (diagonal or arbitrary words).
    puz::Clues & clues = m_puz->GetClues();
    puz::Clues::iterator it;
    for (it = clues.begin(); it != clues.end(); ++it)
    {
        puz::ClueList & cluelist = it->second;
        puz::ClueList::iterator clue;
        for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
        {
            word = &clue->GetWord();
            if (word->Contains(square)
                && word->GetDirection() % 90 != 0)
            {
                return word;
            }
        }
    }
    // Couldn't find any words . . . we must invent a word.
    return NULL;
}


void
XGridCtrl::DoSetFocusedWord(puz::Square * square,
                            puz::Word * word,
                            short direction)
{
    if (! m_grid->IsDiagramless())
    {
       // Find the new word
        word = CalcFocusedWord(square, word, direction);
        // If these words are the same, return now
        if (m_focusedWord != NULL && word == m_focusedWord)
            return;
        // Delete the old word if necessary
        if (m_ownsFocusedWord)
        {
            delete m_focusedWord;
            m_ownsFocusedWord = false;
        }
        // If we got a new word, use that
        if (word != NULL)
        {
            m_focusedWord = word;
            return;
        }
    }
    else // diagramless
    {
        assert(m_ownsFocusedWord || m_focusedWord == NULL);
        if (m_focusedWord != NULL && m_focusedWord == word)
            return;
        if (m_ownsFocusedWord && m_focusedWord != NULL)
            delete m_focusedWord;
        if (square->IsBlack())
        {
            m_focusedWord = new puz::Word(square, square);
            m_ownsFocusedWord = true;
            return;
        }
    }
    // As a last resort, make up a word
    if (puz::IsDiagonal(direction)) // Don't make up diagonal words.
        direction = m_focusedDirection;
    puz::GridDirection dir = puz::IsHorizontal(direction)
                                ? puz::ACROSS : puz::DOWN;
    m_focusedWord = new puz::Word(
        square->GetWordStart(dir),
        square->GetWordEnd(dir)
    );
    m_ownsFocusedWord = true;
}


bool
XGridCtrl::MakeVisible(const puz::Square & square)
{
    if (m_fit || IsEmpty())
        return false;

    const int col = square.GetCol();
    const int row = square.GetRow();

    int startX = -1;
    int startY = -1;

    int firstX, firstY;
    GetViewStart(&firstX, &firstY);
    wxSize client = GetClientSize();

    // Subtract 1 here because the squares are zero-based, not one-based
    const int lengthX = int(client.x / GetSquareSize()) - 1;
    const int lengthY = int(client.y / GetSquareSize()) - 1;

    // X Scrolling
    if (col <= firstX || col >= firstX + lengthX)
    {
        startX = col - lengthX / 2;

        // Fit witihin constraints
        if (startX < 0)
            startX = 0;
        if (startX > m_grid->LastCol() - lengthX)
            startX = m_grid->LastCol() - lengthX;
    }

    // Y Scrolling
    if (row <= firstY || row >= firstY + lengthY)
    {
        startY = row - lengthY/ 2;

        // Fit witihin constraints
        if (startY < 0)
            startY = 0;
        if (startY > m_grid->LastRow() - lengthY)
            startY = m_grid->LastRow() - lengthY;
    }

    if (startX != -1 || startY != -1)
    {
        Scroll(startX, startY);
        Refresh();
        return true;
    }
    return false;
}



bool
XGridCtrl::SetSquareText(puz::Square & square, const wxString & text)
{
    // Are we allowed to enter this text?
    if (text == _T("."))
    {
        if (! GetGrid()->IsDiagramless())
            return false;
    }
    else
    {
        for (wxString::const_iterator it = text.begin(); it != text.end(); ++it)
            if (! IsValidChar(*it))
                return false;
    }

    // Not allowed to overwrite revealed letters or checked letters
    if (square.HasFlag(puz::FLAG_REVEALED | puz::FLAG_CORRECT))
        return false;

    // renumber the grid if the square is changing from black to white or vice-versa
    const bool numberGrid = GetGrid()->IsDiagramless() &&
                            (text == puz2wx(puz::Square::Black) != (square.IsBlack()));

    square.SetText(wx2puz(text));

    if (square.HasFlag(puz::FLAG_X))
    {
        square.AddFlag(puz::FLAG_BLACK);
        square.RemoveFlag(puz::FLAG_X);
    }

    // Don't send letter updated events if the user is in the middle of entering
    // a rebus entry.
    if (! IsRebusEntry())
    {
        if (HasStyle(CHECK_WHILE_TYPING) && ! m_grid->IsScrambled())
            CheckLetter(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);

        SendEvent(wxEVT_PUZ_LETTER);
    }

    if (numberGrid)
    {
        GetGrid()->NumberGrid();
        Refresh();
    }
    else {
        RefreshSquare(square);
    }

    return true;
}



void
XGridCtrl::StartSelection(wxObjectEventFunction func, wxEvtHandler * evtSink)
{
    Disconnect(wxEVT_Grid_SELECTION);
    Connect(wxEVT_Grid_SELECTION, func, NULL, evtSink);
    // Don't create another selection handler if we already have one.
    if (! m_isSelecting)
    {
        new GridSelectionHandler(*this);
        m_isSelecting = true;
    }
}

void
XGridCtrl::EndSelection(bool success)
{
    // Process this event instead of Posting it so that the function
    // is called immediately.

    // The attached function will always be called.  Let the function
    // know if the selection failed here.
    if (! success)
    {
        m_selectionStart = NULL;
        m_selectionEnd   = NULL;
    }
	GridSelectionEvent evt(GetId(), wxEVT_Grid_SELECTION, this,
						   m_selectionStart, m_selectionEnd);
    GetEventHandler()->ProcessEvent(evt);

    Disconnect(wxEVT_Grid_SELECTION);
    m_selectionStart = NULL;
    m_selectionEnd   = NULL;
    m_isSelecting = false;
    Refresh();
    // GridSelectionHandler will destroy itself
}

std::vector<puz::Square *>
XGridCtrl::GetSelection()
{
    std::vector<puz::Square *> selection;

    puz::Square * square;
    for (square = m_grid->First(); square != NULL; square = square->Next())
        if (IsSelected(*square))
            selection.push_back(square);

    return selection;
}




//-------------------------------------------------------
// Scaling
//-------------------------------------------------------
void
XGridCtrl::OnSize(wxSizeEvent & WXUNUSED(evt))
{
    Freeze();
    Scale();
    bool refresh = false;
    if (m_focusedSquare != NULL)
        refresh = ! MakeVisible(*m_focusedSquare);
    Thaw();
    Refresh();
}

void
XGridCtrl::Scale(double factor)
{
    if (IsEmpty())
        return;

    // Recalculate box size
    if (m_fit)
    {
        m_drawer.SetMaxSize(GetClientSize());
        m_drawer.SetAlign(wxALIGN_CENTER);
    }
    else
    {
        // factor should only equal 1 when the user has decided to "un-fit"
        // the grid without explicitly zooming in or out.  In this case, set the
        // zoom factor to whatever the last zoom factor was.
        if (factor == 1.0 && m_lastBoxSize != UNDEFINED_BOX_SIZE)
            m_drawer.SetBoxSize(m_lastBoxSize);
        // Otherwise, change the zoom by the specified factor.
        else
            m_drawer.SetBoxSize(m_drawer.GetBoxSize() * factor);

        m_lastBoxSize = m_drawer.GetBoxSize();

        // Change the alignment.
        // If a given dimension fits in the window, it should be centered.
        // Otherwise, it will be aligned to the top / left.
        const wxSize clientSize = GetClientSize();
        long align = wxALIGN_LEFT | wxALIGN_TOP;
        if (m_drawer.GetWidth() < clientSize.GetWidth())
            align |= wxALIGN_CENTER_HORIZONTAL;
        if (m_drawer.GetHeight() < clientSize.GetHeight())
            align |= wxALIGN_CENTER_VERTICAL;
        m_drawer.SetAlign(align);
    }

    // Virtual size does not include the bottom and right grid border.
    // If it did include the border, there would often be an extra scroll unit
    // because the scroll unit is equal to one square plus _one_ border, not
    // both borders.
    SetVirtualSize(m_drawer.GetWidth()  - m_drawer.GetBorderSize(),
                   m_drawer.GetHeight() - m_drawer.GetBorderSize());
    SetScrollRate(m_drawer.GetSquareSize(), m_drawer.GetSquareSize());
    if (m_rebusCtrl)
        m_rebusCtrl->UpdateSize();
}




//-------------------------------------------------------
// Check/Reveal functions
//-------------------------------------------------------
void
XGridCtrl::CheckGrid(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    puz::square_iterator begin(m_grid->First());
    puz::square_iterator end(m_grid->Last()->Next());
    Check(begin, end, options);
}


void
XGridCtrl::CheckWord(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    Check(m_focusedWord->begin(), m_focusedWord->end(), options);
}



void
XGridCtrl::CheckLetter(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    puz::Square * square = GetFocusedSquare();
    puz::square_iterator begin(square);
    puz::square_iterator end(square->Next());

    Check(begin, end, options);
}


void
XGridCtrl::Check(puz::square_iterator begin, puz::square_iterator end,
                 int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    wxClientDC dc(this); DoPrepareDC(dc);
    bool incorrect = false;
    for (puz::square_iterator square = begin; square != end; ++square)
        if (! CheckSquare(&*square, options, dc))
            incorrect = true;
    if (! incorrect && (options & NO_MESSAGE_BOX) == 0)
        XWordMessage(this, MSG_NO_INCORRECT);
}

// Check an individual square, set flags, refresh the square
bool
XGridCtrl::CheckSquare(puz::Square * square, int options, wxDC & dc)
{
    if (! square->IsWhite() && ! m_puz->IsDiagramless())
        return true;
    square->RemoveFlag(puz::FLAG_CORRECT);
    if (! square->Check((options & CHECK_ALL) != 0, HasStyle(STRICT_REBUS)))
    {
        if ( (options & REVEAL_ANSWER) != 0)
        {
            SetSquareText(*square, puz2wx(square->GetSolution()));
            square->RemoveFlag(puz::FLAG_BLACK | puz::FLAG_X);
            square->AddFlag(puz::FLAG_REVEALED);
        }
        else
        {
            square->RemoveFlag(puz::FLAG_BLACK);
            square->AddFlag(puz::FLAG_X);
        }
        RefreshSquare(dc, *square);
        return false;
    }
    else if (! square->IsBlank() && ! square->HasFlag(puz::FLAG_REVEALED))
    {
        square->AddFlag(puz::FLAG_CORRECT);
        RefreshSquare(dc, *square);
        return true;
    }
    return true;
}


// Check selection functions
//--------------------------

// This is a bit messy:
//    (1) StartSelection() creates a new instance of GridSelectionHandler
//        which takes over event handling for the grid.
//    (2) The user makes a selection.
//    (3) GridSelectionHandler destroys itself, returns event handling
//        control to the XGridCtrl, and calls EndSelection().
//    (4) EndSelection() dispatches an GridSelectionEvent which was
//        bound in the call to StartSelection(function, sink), then
//        Unconnects the event.
//
// Unfortunately, in order to pass a function to StartSelection() as
// an event handler [StartSelection() calls wxEvtHandler::Connect()],
// the function must be a member function of a wxEvtHandler - derived
// class which takes wxEvent & as its only argument.
// i.e. wxEvtHandler::Function(wxEvent & evt)
// In order to pass real arguments to DoCheckSelection, there is an
// itermediate class (GridCheckSelectionClass) which is used like a
// functor -- that is, it is passed arguments in the constructor
// which are subsequently used in the only function call.
//
// These class names are an absolute mess.


// A dummy class that is used to call DoCheckSelection with the
// correct variables.
class GridCheckSelectionClass : public wxEvtHandler
{
public:
    GridCheckSelectionClass(XGridCtrl * a_grid, int a_options)
        : m_grid(a_grid),
          m_options(a_options)
    {}

    // We have to make sure this function gets called so that this
    // heap-allocated class gets destroyed.
    void CheckSelection(GridSelectionEvent & evt)
    {
        if (evt.HasSelection())
            m_grid->DoCheckSelection(evt.GetSelectionStart(),
                                     evt.GetSelectionEnd(),
                                     m_options);
        delete this;
    }

private:
    XGridCtrl * m_grid;
    int m_options;
};


// The real action takes place here
void
XGridCtrl::DoCheckSelection(puz::Square * start, puz::Square * end, int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    wxClientDC dc(this); DoPrepareDC(dc);
    bool incorrect = false;
    for (int col = start->GetCol(); col <= end->GetCol(); ++col)
        for (int row = start->GetRow(); row <= end->GetRow(); ++row)
            if (! CheckSquare(&At(col, row), options, dc))
                incorrect = true;
    if (! incorrect && (options & NO_MESSAGE_BOX) == 0)
        XWordMessage(this, MSG_NO_INCORRECT);
}

// Start the selection process and defer checking the squares until later.
void
XGridCtrl::CheckSelection(int options)
{
    GridCheckSelectionClass * handler = new GridCheckSelectionClass(this, options);
    StartSelection(GridSelectionEventHandler(GridCheckSelectionClass::CheckSelection), handler);
}



//-------------------------------------------------------------
// Events
//-------------------------------------------------------------

void
XGridCtrl::ConnectEvents()
{
    // Prevent double-connecting events
    if (! m_areEventsConnected)
    {
        Bind(wxEVT_KEY_DOWN, &XGridCtrl::OnKeyDown, this);
        Bind(wxEVT_CHAR, &XGridCtrl::OnChar, this);
        Bind(wxEVT_LEFT_DOWN, &XGridCtrl::OnLeftDown, this);
        Bind(wxEVT_LEFT_DCLICK, &XGridCtrl::OnLeftDClick, this);
        Bind(wxEVT_RIGHT_DOWN, &XGridCtrl::OnRightDown, this);
        Bind(wxEVT_MOTION, &XGridCtrl::OnMouseMove, this);
    }
    m_areEventsConnected = true;
}


void
XGridCtrl::DisconnectEvents()
{
    if (m_areEventsConnected)
    {
        Unbind(wxEVT_KEY_DOWN, &XGridCtrl::OnKeyDown, this);
        Unbind(wxEVT_CHAR, &XGridCtrl::OnChar, this);
        Unbind(wxEVT_LEFT_DOWN, &XGridCtrl::OnLeftDown, this);
        Unbind(wxEVT_LEFT_DCLICK, &XGridCtrl::OnLeftDClick, this);
        Unbind(wxEVT_RIGHT_DOWN, &XGridCtrl::OnRightDown, this);
        Unbind(wxEVT_MOTION, &XGridCtrl::OnMouseMove, this);

    }
    m_areEventsConnected = false;
}



void
XGridCtrl::OnLeftDown(wxMouseEvent & evt)
{
    wxASSERT(! IsEmpty());

    wxPoint pt = evt.GetPosition();
    puz::Square * square = HitTest(pt.x, pt.y);
    if (square != NULL && (square->IsWhite() || GetGrid()->IsDiagramless())) {
        if (GetFocusedSquare() == square && HasStyle(SWAP_ON_DCLICK)) {
            // swap direction
            SetFocusedSquare(square, puz::IsVertical(m_focusedDirection) ? puz::ACROSS : puz::DOWN);
        } else {
            // move to square
            SetFocusedSquare(square);
        }
    }

    // Make sure to skip this event or we don't get keyboard focus!
    evt.Skip();
}

void
XGridCtrl::OnLeftDClick(wxMouseEvent & evt)
{
    // The goal for double-click is to just run the down handler twice, but
    // LEFT_DOWN will always be sent before LEFT_DCLICK, so all we need to do
    // here is call the LeftDown handler _once_.
    OnLeftDown(evt);
}

void
XGridCtrl::OnRightDown(wxMouseEvent & evt)
{
    wxASSERT(! IsEmpty());

    puz::Square * square = GetFocusedSquare();
    if (HasStyle(MOVE_ON_RIGHT_CLICK))
    {
        wxPoint pt = evt.GetPosition();
        square = HitTest(pt.x, pt.y);
    }
    if (square != NULL && (square->IsWhite() || GetGrid()->IsDiagramless()))
    {
        // Swap the direction (across/down)
        SetFocusedSquare(square, puz::IsVertical(m_focusedDirection) ? puz::ACROSS : puz::DOWN);
    }
    // Kill the event.
}


void
XGridCtrl::OnMouseMove(wxMouseEvent & evt)
{
    wxASSERT(! IsEmpty());

    wxPoint pt = evt.GetPosition();
    puz::Square * square = HitTest(pt.x, pt.y);
    if (square != NULL && (square->IsWhite() || GetGrid()->IsDiagramless())
        && square->GetText().length() > 1)
    {
        wxString text = puz2wx(square->GetText());
        wxToolTip * tip = GetToolTip();
        if (tip && tip->GetTip() != text)
            SetToolTip(text);
    }
    else
        SetToolTip(wxEmptyString);

    evt.Skip();
}

void
XGridCtrl::OnContextMenu(wxContextMenuEvent & evt)
{
    if (HasStyle(CONTEXT_MENU))
    {
        // CONTEXT_MENU style is not yet implemented.
    }
    // Kill the event.
}


void
XGridCtrl::OnKeyDown(wxKeyEvent & evt)
{
    wxASSERT(! IsEmpty());

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();

    switch(key)
    {
    // Directional keys
    case WXK_LEFT:
    case WXK_NUMPAD_LEFT:
        OnArrow(puz::LEFT, mod);
        break;
    case WXK_RIGHT:
    case WXK_NUMPAD_RIGHT:
        OnArrow(puz::RIGHT, mod);
        break;
    case WXK_UP:
    case WXK_NUMPAD_UP:
        OnArrow(puz::UP,    mod);
        break;
    case WXK_DOWN:
    case WXK_NUMPAD_DOWN:
        OnArrow(puz::DOWN,  mod);
        break;

    // Diagonals
    case WXK_NUMPAD_HOME:
        OnArrow(puz::DIAGONAL_NW,  mod);
        break;
    case WXK_NUMPAD_END:
        OnArrow(puz::DIAGONAL_SW,  mod);
        break;
    case WXK_NUMPAD_PAGEUP:
        OnArrow(puz::DIAGONAL_NE,  mod);
        break;
    case WXK_NUMPAD_PAGEDOWN:
        OnArrow(puz::DIAGONAL_SE,  mod);
        break;

    // Other keys
    case WXK_HOME:
        OnHome(mod);
        break;
    case WXK_END:
        OnEnd(mod);
        break;
    case WXK_TAB:
    case WXK_NUMPAD_TAB:
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
        OnTab(mod);
        break;
    case WXK_BACK:
        OnBackspace(mod);
        break;
    case WXK_DELETE:
    case WXK_NUMPAD_DELETE:
        OnDelete(mod);
        break;
    case WXK_INSERT:
    case WXK_NUMPAD_INSERT:
    case WXK_ESCAPE:
        OnRebusKey(mod);
        break;
    default:
        evt.Skip();
        break;
    }
}


// This is for letters only
void
XGridCtrl::OnChar(wxKeyEvent & evt)
{
    wxASSERT(! IsEmpty());

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();

    if (key == _T('*'))
    {
        GetFocusedSquare()->ToggleFlag(puz::FLAG_CIRCLE);
        RefreshSquare();
    }
    else if (GetGrid()->IsDiagramless() && key == _T('.'))
    {
        if (GetFocusedSquare()->IsBlack())
            SetSquareText(*GetFocusedSquare(), _T(""));
        else
        {
            SetSquareText(*GetFocusedSquare(), puz2wx(puz::Square::Black));
            MoveAfterLetter();
            // Invalidate the current focused word
            if (m_ownsFocusedWord)
                delete m_focusedWord;
            m_focusedWord = NULL;
            // Make a new word
            SetFocusedSquare();
        }
    }
    else if (IsValidChar(key) && key < WXK_START)
    {
        OnLetter(key, mod);
    }
}



void
XGridCtrl::OnLetter(wxChar key, int mod)
{
    wxASSERT(IsValidChar(key));
    wxASSERT(! IsEmpty());

    if (! (mod == wxMOD_NONE || mod == wxMOD_SHIFT))
        return;

    wxASSERT(! IsRebusEntry());

    if (static_cast<int>(key) == WXK_SPACE)
        SetSquareText(*m_focusedSquare, _T(""));
    else
        SetSquareText(*m_focusedSquare, key);

    // Space bar always moves forward one square
    if (static_cast<int>(key) == WXK_SPACE)
        SetFocusedSquare(m_focusedWord->FindNextSquare(m_focusedSquare, FIND_WHITE_SQUARE), m_focusedWord);
    else
        MoveAfterLetter();
}


void
XGridCtrl::MoveAfterLetter()
{
    puz::Square * newSquare = NULL;

    if (HasStyle(MOVE_AFTER_LETTER))
    {
        if (HasStyle(MOVE_TO_NEXT_BLANK))
        {
            newSquare = m_focusedWord->FindNextSquare(
                m_focusedSquare, FIND_BLANK_SQUARE
            );
            // Last blank in the word: jump to the first blank if it exists
            if (newSquare == NULL)
            {
                newSquare = m_focusedWord->FindSquare(FIND_BLANK_SQUARE);
            }
        }
        // Either the user wants "move to next letter", or there are no more blanks in the
        // word.  Move to the next letter if it exists
        if (newSquare == NULL)
        {
            newSquare = m_focusedWord->FindNextSquare(m_focusedSquare, FIND_WHITE_SQUARE);
            // if newSquare == NULL (possibly again), it's the last square in the word,
            // and the focus won't change.
        }
    }

    SetFocusedSquare(newSquare, m_focusedWord);
}


void
XGridCtrl::OnArrow(puz::GridDirection arrowDirection, int mod)
{
    wxASSERT(! IsEmpty());
    if (mod != wxMOD_SHIFT || IsDiagonal(arrowDirection))
    {
        // Check to see if the next square is part of the current word
        if (m_focusedWord &&
            m_focusedWord->Contains(m_focusedSquare->Next(arrowDirection)) &&
            m_focusedSquare->Next(arrowDirection)->IsWhite())
        {
            SetFocusedSquare(m_focusedSquare->Next(arrowDirection),
                             m_focusedWord);
            return;
        }
        // Switching directions
        if (! AreInLine(arrowDirection, m_focusedDirection)
            && HasStyle(PAUSE_ON_SWITCH)
            && m_focusedSquare->IsBlank())
        {
            // Check to see if there *is* a word in arrowDirection.
            if (! GetGrid()->IsDiagramless())
            {
                puz::Word * word = m_puz->FindWord(m_focusedSquare, arrowDirection);
                if (word && word != m_focusedWord)
                {
                    SetFocusedSquare(m_focusedSquare, word, arrowDirection);
                    return;
                }
            }
            // Check to see if there *should be* a (non-diagonal) word
            // in arrowDirection.
            if (! IsDiagonal(arrowDirection)
                && m_focusedSquare->HasWord(arrowDirection)
                && !GetGrid()->IsAcrostic())
            {
                SetFocusedSquare(m_focusedSquare, NULL, arrowDirection);
                return;
            }
        }
        // Not switching directions
        if (! GetGrid()->IsDiagramless())
        {
            // Find the next white square in the arrow direction
            puz::GridDirection newDirection;
            if (GetGrid()->IsAcrostic())
                newDirection = puz::ACROSS;
            else
                newDirection = arrowDirection;
            SetFocusedSquare(
                m_grid->FindNextSquare(
                    m_focusedSquare, FIND_WHITE_SQUARE,
                    arrowDirection, puz::NO_WRAP
                ),
                NULL, newDirection
            );
        }
        else // Diagramless
        {
            // Move to the next square (so that black squares can be selected)
            SetFocusedSquare(m_focusedSquare->Next(arrowDirection),
                              NULL, arrowDirection);
        }
    }
    else // Shift
    {
        // If the arrow is in the focused direction, wrap around the grid.
        bool sameDirection = AreInLine(m_focusedDirection, arrowDirection);
        unsigned int options = sameDirection ? 0 : puz::NO_WRAP;
        puz::GridDirection focusedDirection = 
            static_cast<puz::GridDirection>(m_focusedDirection);
        // Move to the next white square in the arrow direction that
        // *could* have a word.
        puz::Square* newSquare = NULL;
        puz::Word* newWord = NULL;
        for (newSquare = m_focusedSquare;
            newSquare;
            newSquare = m_grid->FindNextSquare(newSquare, FIND_WHITE_SQUARE, arrowDirection, options))
        {
            puz::Word* word = m_puz->FindWord(newSquare, focusedDirection);
            if ((newSquare->HasWord(focusedDirection) || word != NULL)
                && !m_focusedWord->Contains(newSquare))
            {
                newWord = word;
                break;
            }
        }
        // If we didn't find a square, relax the requirement that the word must be in the same
        // direction as the current word, and instead look for a word in the same clue list.
        if (!newSquare && !m_ownsFocusedWord) {
            puz::Clues& clues = m_puz->GetClues();
            puz::Clues::iterator cluelist_it;
            for (cluelist_it = clues.begin(); cluelist_it != clues.end(); ++cluelist_it)
            {
                puz::ClueList& cluelist = cluelist_it->second;
                puz::ClueList::iterator clue;
                // See if this cluelist contains the current word.
                for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
                {
                    if (&clue->GetWord() == m_focusedWord)
                        break;
                }
                if (clue == cluelist.end())
                    continue;
                // Find the closest square in a different word and the same direction as the arrow.
                puz::Square* closestSquare = NULL;
                puz::Word* closestSquareWord = NULL;
                double closestSquareDistance = DBL_MAX;
                for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
                {
                    puz::Word* word = &clue->GetWord();
                    if (word == m_focusedWord)
                        continue;
                    for (puz::square_iterator square_it = word->begin(); square_it != word->end(); ++square_it) {
                        if (arrowDirection == puz::GetDirection(*m_focusedSquare, *square_it)) {
                            double distance =
                                std::abs(m_focusedSquare->GetRow() - square_it->GetRow()) +
                                std::abs(m_focusedSquare->GetCol() - square_it->GetCol());
                            if (distance < closestSquareDistance) {
                                closestSquareDistance = distance;
                                closestSquare = &*square_it;
                                closestSquareWord = word;
                            }
                        }
                    }
                }
                newSquare = closestSquare;
                newWord = closestSquareWord;
            }
        }
        // If the arrow is in the focused direction, find the first square in the word.
        if (newSquare && sameDirection) {
            if (newWord)
                newSquare = newWord->front();
            else
                newSquare = newSquare->GetWordStart(focusedDirection);
        }
        MoveFocusedSquare(newSquare, focusedDirection);
    }
}



void
XGridCtrl::OnBackspace(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    SetSquareText(*m_focusedSquare, _T(""));
    SetFocusedSquare(m_focusedWord->FindPrevSquare(m_focusedSquare, FIND_WHITE_SQUARE),
                     m_focusedWord);
}



void
XGridCtrl::OnDelete(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    SetSquareText(*m_focusedSquare, _T(""));
    SetFocusedSquare(m_focusedSquare, m_focusedWord);
}



void
XGridCtrl::OnHome(int mod)
{
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite instead of the usual ctrl / command
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
        SetFocusedSquare(FirstWhite());
    else
        SetFocusedSquare(m_focusedWord->front());
}



void
XGridCtrl::OnEnd(int mod)
{
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite instead of the usual ctrl / command
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
        SetFocusedSquare(LastWhite());
    else
        SetFocusedSquare(m_focusedWord->back());
}


void
XGridCtrl::OnTab(int mod)
{
    // Move to the next clue in the current clue list
    wxASSERT(! IsEmpty());
    if (m_ownsFocusedWord) // Implies that there is no clue.
    {
        OnArrow(mod != wxMOD_SHIFT ? puz::ACROSS : puz::LEFT, wxMOD_SHIFT);
        return;
    }
    // There is a clue . . . find it and move to the next one.
    puz::Clues & clues = m_puz->GetClues();
    puz::Clues::iterator cluelist_it;
    for (cluelist_it = clues.begin(); cluelist_it != clues.end(); ++cluelist_it)
    {
        puz::ClueList & cluelist = cluelist_it->second;
        puz::ClueList::iterator clue;
        for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
        {
            if (&clue->GetWord() == m_focusedWord)
                break;
        }
        if (clue != cluelist.end()) // We found our clue
        {
            if (mod != wxMOD_SHIFT) // Forward
            {
                // Move to the next clue
                ++clue;
                if (clue != cluelist.end())
                {
                    MoveFocusedWord(&clue->GetWord());
                }
                else
                {
                    // We're at the end of this clue list: try the next list.
                    ++cluelist_it;
                    if (cluelist_it == clues.end())
                        // We're at the end of the lists: go to the first clue.
                        cluelist_it = clues.begin();
                    MoveFocusedWord(&cluelist_it->second.front().GetWord());
                }
            }
            else // Shift (reverse)
            {
                // Move to the previous clue
                if (clue != cluelist.begin())
                {
                    --clue;
                    MoveFocusedWord(&clue->GetWord());
                }
                else if (cluelist_it == clues.begin())
                {
                    // We're at the beginning: go to the last clue.
                    cluelist_it = clues.end();
                    --cluelist_it;
                    MoveFocusedWord(&cluelist_it->second.back().GetWord());
                }
                else
                {
                    // We're at the beginning of this clue list: try the
                    // previous list.
                    --cluelist_it;
                    MoveFocusedWord(&cluelist_it->second.back().GetWord());
                }
            }
            return;
        }
    }
}



// Allow entering multiple letters
void
XGridCtrl::OnRebusKey(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    if (! m_rebusCtrl)
        StartRebusEntry();
    else
        EndRebusEntry();
}

void
XGridCtrl::StartRebusEntry()
{
    if (m_rebusCtrl
        || ! m_focusedSquare
        || m_focusedSquare->HasFlag(puz::FLAG_REVEALED | puz::FLAG_CORRECT))
        return;

    m_rebusCtrl = new XGridRebusCtrl(this);
    m_rebusCtrl->Show();
    if (! m_focusedSquare->IsBlank())
        m_rebusCtrl->SetValue(m_focusedSquare->GetText());
    m_rebusCtrl->GetTextCtrl()->SelectAll();
    m_rebusCtrl->SetFocus();
}

void
XGridCtrl::EndRebusEntry(bool success)
{
    if (! m_rebusCtrl)
        return;

    wxString text = m_rebusCtrl->GetValue();
    m_rebusCtrl->Destroy();
    m_rebusCtrl = NULL;
    if (success)
    {
        SetSquareText(*m_focusedSquare, text);
        MoveAfterLetter();
    }
    RefreshSquare();
    wxScrolledCanvas::SetFocus();
}

void
XGridCtrl::SetFocus()
{
    if (m_rebusCtrl)
        m_rebusCtrl->SetFocus();
    else
        wxScrolledCanvas::SetFocus();
}


//-------------------------------------------------------
// Information functions
//-------------------------------------------------------

// Find the row and column that (x, y) would be at (does not
// need to be a valid square).
void
XGridCtrl::HitTest(int x, int y, int * col, int * row)
{
    CalcUnscrolledPosition(x, y, &x, &y);
    x -= (m_drawer.GetLeft() + m_drawer.GetBorderSize());
    y -= (m_drawer.GetTop() + m_drawer.GetBorderSize());
    *col = floor(static_cast<double>(x) / (m_drawer.GetSquareSize()));
    *row = floor(static_cast<double>(y) / (m_drawer.GetSquareSize()));
}

// Find the square that exists at (x, y) or NULL
puz::Square *
XGridCtrl::HitTest(int x, int y)
{
    if (IsEmpty())
        return NULL;
    int col, row;
    HitTest(x, y, &col, &row);
    if (   0 <= col && col < m_grid->GetWidth()
        && 0 <= row && row < m_grid->GetHeight())
    {
        return &At(col, row);
    }
    return NULL;
}

// Find the square nearest to (x, y)
puz::Square *
XGridCtrl::HitTestNearest(int x, int y)
{
    int col, row;
    HitTest(x, y, &col, &row);

    return &At(
        std::min(std::max(col, 0), m_grid->LastCol()),
        std::min(std::max(row, 0), m_grid->LastRow())
    );
}



const wxColor &
XGridCtrl::GetSquareColor(const puz::Square & square)
{
    if (HasSelection() && IsSelected(square))
        return GetSelectionColor();
    else if (IsFocusedLetter(square))
        return GetFocusedLetterColor();
    else if (IsFocusedWord(square))
        return GetFocusedWordColor();
    else if (!square.GetPartnerSquares().empty()) {
        std::vector<puz::Square*> partners = square.GetPartnerSquares();
        for (std::vector<puz::Square*>::iterator it = partners.begin(); it != partners.end(); ++it) {
            if (IsFocusedLetter(*(*it)))
                return GetFocusedLetterColor();
        }
    }
    return wxNullColour; // XGridDrawer will decide
}




puz::Square *
XGridCtrl::GetClueNumber(const wxString & num)
{
    wxASSERT(! IsEmpty());
    wxASSERT(FirstWhite() != NULL);
    const puz::string_t number = wx2puz(num);
    for (puz::Square * square = FirstWhite();
         square != NULL;
         square = square->Next())
    {
        if (square->GetNumber() == number)
            return square;
    }
    return NULL;
}


short
XGridCtrl::GetDirection() const
{
    return m_focusedDirection;
}

void
XGridCtrl::RecalcDirection()
{
    if (m_focusedWord->front() != m_focusedWord->back())
        m_focusedDirection = m_focusedWord->GetDirection();
}


//------------------------------------------------------------------------------
// GridSelectionHandler implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GridSelectionHandler, wxEvtHandler)
    EVT_LEFT_DOWN            (GridSelectionHandler::OnLeftDown)
    EVT_MOTION               (GridSelectionHandler::OnMouseMove)
    EVT_LEFT_UP              (GridSelectionHandler::OnLeftUp)
    EVT_KEY_DOWN             (GridSelectionHandler::OnKeyDown)
    EVT_MOUSE_CAPTURE_LOST   (GridSelectionHandler::OnMouseCaptureLost)
END_EVENT_TABLE()

GridSelectionHandler::GridSelectionHandler(XGridCtrl & grid)
    : m_grid(grid),
      m_start(NULL),
      m_end(NULL)
{
    m_grid.PushEventHandler(this);
    // Kill the grid's own event handling
    m_grid.DisconnectEvents();

    m_grid.m_selectionStart = NULL;
    m_grid.m_selectionEnd = NULL;

    m_grid.SetCursor(wxCursor(wxCURSOR_CROSS));
}

void
GridSelectionHandler::EndEventHandling()
{
    m_grid.RemoveEventHandler(this);
    // Restore the grid's own key handling
    m_grid.ConnectEvents();

    m_grid.SetCursor(wxNullCursor);

    if (m_grid.HasCapture())
        m_grid.ReleaseMouse();
}


void
GridSelectionHandler::OnLeftDown(wxMouseEvent & evt)
{
    // Set the first square in the selection
    wxPoint pt = evt.GetPosition();
    m_start = m_grid.HitTestNearest(pt.x, pt.y);
    wxASSERT(m_start != NULL);
    m_selection.x = m_start->GetCol();
    m_selection.y = m_start->GetRow();
    m_selection.SetSize(wxSize(1,1));

    m_grid.CaptureMouse();
}

void
GridSelectionHandler::OnMouseMove(wxMouseEvent & evt)
{
    if (m_start == NULL)
        return;

    // Find the second corner of the selection
    wxPoint pt = evt.GetPosition();
    m_end = m_grid.HitTestNearest(pt.x, pt.y);
    wxASSERT(m_end != NULL);

    // Set the top left and bottom right squares of this selection region.
    m_selection.SetLeft  (std::min(m_start->GetCol(), m_end->GetCol()));
    m_selection.SetTop   (std::min(m_start->GetRow(), m_end->GetRow()));
    m_selection.SetRight (std::max(m_start->GetCol(), m_end->GetCol()));
    m_selection.SetBottom(std::max(m_start->GetRow(), m_end->GetRow()));

    m_grid.m_selectionStart = &m_grid.At(m_selection.GetLeft(),
                                         m_selection.GetTop());
    m_grid.m_selectionEnd   = &m_grid.At(m_selection.GetRight(),
                                         m_selection.GetBottom());

    // Redraw needed squares
    wxRect total = m_selection;
    total.Union(m_lastSelection);

    wxClientDC dc(&m_grid);
    m_grid.DoPrepareDC(dc);
    for (int col = total.GetLeft(); col <= total.GetRight(); ++col)
        for (int row = total.GetTop(); row <= total.GetBottom(); ++row)
            // Draw newly selected squares
            if (m_selection.Contains(col, row))
            {
                if (! m_lastSelection.Contains(col, row))
                    m_grid.DrawSquare(dc, col, row);
            }
            // Draw newly unselected squares
            else if (m_lastSelection.Contains(col, row))
            {
                    m_grid.DrawSquare(dc, col, row);
            }

    m_lastSelection = m_selection;
}

void
GridSelectionHandler::OnLeftUp(wxMouseEvent & evt)
{
    if (m_start == NULL)
        return;

    // The selection should already be set at this point.
    EndEventHandling();
    // Notify the XGridCtrl that we are done capturing events.
    m_grid.EndSelection(true);
    delete this;
}


void
GridSelectionHandler::OnKeyDown(wxKeyEvent & evt)
{
    if (evt.GetKeyCode() == WXK_ESCAPE)
    {
        EndEventHandling();
        m_grid.EndSelection(false); // Abort
        delete this;
    }
}

void
GridSelectionHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent & WXUNUSED(evt))
{
    EndEventHandling();
    m_grid.EndSelection(false); // Abort
    delete this;
}
