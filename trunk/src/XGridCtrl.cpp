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
#include <wx/dcbuffer.h>
#include <list>
#include <algorithm>
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
// for rebus entries.  It must be created on the heap, and it
// will destroy itself when it is done.
class GridRebusHandler : wxEvtHandler
{
public:
    GridRebusHandler(XGridCtrl & grid);
    void EndEventHandling();

    DECLARE_EVENT_TABLE()

private:
    XGridCtrl & m_grid;

    // The event handling
    void OnKeyDown(wxKeyEvent & evt);
    void OnChar   (wxKeyEvent & evt);
};


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
BEGIN_EVENT_TABLE(XGridCtrl, wxScrolledWindow)
    EVT_PAINT           (XGridCtrl::OnPaint)
    EVT_SIZE            (XGridCtrl::OnSize)
    EVT_CONTEXT_MENU    (XGridCtrl::OnContextMenu)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(XGridCtrl, wxScrolledWindow)

const int UNDEFINED_BOX_SIZE = -1;

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

    m_incorrectSquares = 0;
    m_blankSquares = 0;

    m_wantsRebus = false;

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
    if (! wxScrolledWindow::Create(parent, id, pos, size, style, name))
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

void XGridCtrl::UpdateConfig(ConfigManager & config)
{
    /*
    SetGridStyle(config.ReadLong(_T("style")) );

    SetLetterFont(config.ReadFont(_T("letterFont")) );
    SetNumberFont(config.ReadFont(_T("numberFont")) );

    SetBorderSize(config.ReadLong(_T("lineThickness")));

    SetFocusedLetterColor(config.ReadColor(_T("focusedLetterColor")));
    SetFocusedWordColor(config.ReadColor(_T("focusedWordColor")));
    SetBackgroundColour(config.ReadColor(_T("backgroundColor")));
    SetWhiteSquareColor(config.ReadColor(_T("whiteSquareColor")));
    SetBlackSquareColor(config.ReadColor(_T("blackSquareColor")));
    SetSelectionColor(config.ReadColor(_T("selectionColor")));
    SetPenColor(config.ReadColor(_T("penColor")));
    SetPencilColor(config.ReadColor(_T("pencilColor")));
    SetNumberScale(config.ReadLong(_T("numberScale")) / 100.);
    SetLetterScale(config.ReadLong(_T("letterScale")) / 100.);
    */
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
        m_incorrectSquares = 0;
        m_blankSquares = 0;
        m_focusedSquare = NULL;
        m_focusedWord = NULL;
        m_ownsFocusedWord = false;

        DisconnectEvents();
    }
    else
    {
        m_grid = &puz->GetGrid();

        RecheckGrid();
        m_focusedSquare = FirstWhite();
        m_focusedWord = NULL;
        m_focusedDirection = puz::ACROSS;
        m_ownsFocusedWord = m_grid->IsDiagramless();

        ConnectEvents();
    }
    m_drawer.SetPuzzle(puz);
    Scale();
}


void
XGridCtrl::RecheckGrid()
{
    wxASSERT(! IsEmpty());
    std::vector<puz::Square *> incorrect;
    m_grid->CheckGrid(&incorrect, puz::NO_CHECK_BLANK, HasStyle(STRICT_REBUS));

    std::vector<puz::Square *> wrongOrBlank;
    m_grid->CheckGrid(&wrongOrBlank, puz::CHECK_BLANK, HasStyle(STRICT_REBUS));

    m_incorrectSquares = incorrect.size();
    m_blankSquares     = wrongOrBlank.size() - incorrect.size();
}

bool
XGridCtrl::UnscrambleSolution(unsigned short key)
{
    const bool success = m_grid->UnscrambleSolution(key);

    // We need to update the internal incorrect / blank square counts.
    if (success)
    {
        RecheckGrid();
        // If we should have been checking while typing, go ahead and
        // check the whole grid now that it is possible.
        if (HasStyle(CHECK_WHILE_TYPING))
            CheckGrid(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);
    }

    return success;
}

bool
XGridCtrl::IsCorrect()
{
    // We *can* test scrambled puzzles!  Not sure why I didn't think of
    // this before.  (Inspired by Alex Boisvert:
    // http://alexboisvert.com/software.html#check)
    return m_blankSquares == 0 &&
        (m_incorrectSquares == 0 ||
            (m_grid->IsScrambled() && m_grid->CheckScrambledGrid()));
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
        DrawGrid(dc, GetUpdateRegion());
    else
        DrawPauseMessage(dc);

    //wxScrolledWindow::SetFocus();
}


void
XGridCtrl::DrawPauseMessage(wxDC & dc)
{
    dc.DrawLabel(_T("(Paused)"), wxRect(wxPoint(0,0), dc.GetSize()), wxALIGN_CENTER);
}


void
XGridCtrl::DrawGrid(wxDC & dc, const wxRegion & updateRegion)
{
    if (IsEmpty())
        return;

#ifdef __WXDEBUG__
        int _scrollX, _scrollY;
        GetScrollPixelsPerUnit(&_scrollX, &_scrollY);
        wxASSERT(_scrollX == GetSquareSize() && _scrollY == GetSquareSize());
#endif

    if (m_drawer.GetBoxSize() == 0)
    {
        wxLogDebug(_T("Size is too small"));
        //return;
    }

    puz::Square * square;

    // If we don't have an update region, redraw all squares
    if (updateRegion.IsEmpty())
        for (square = m_grid->First(); square != NULL; square = square->Next())
            DrawSquare(dc, *square);

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
XGridCtrl::DrawSquare(wxDC & dc, const puz::Square & square, const wxColour & color)
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
                                (m_wantsRebus || square.IsBlack());
    if (drawOutline)
    {
        m_drawer.AddFlag(XGridDrawer::DRAW_OUTLINE);
        m_drawer.RemoveFlag(XGridDrawer::DRAW_FLAG | XGridDrawer::DRAW_NUMBER);
    }

    if (color == wxNullColour)
        m_drawer.DrawSquare(dc, square);
    else
        m_drawer.DrawSquare(dc, square, color, GetPenColor());

    // Reset the drawing mode
    if (drawOutline)
    {
        m_drawer.RemoveFlag(XGridDrawer::DRAW_OUTLINE);
        m_drawer.AddFlag(XGridDrawer::DRAW_FLAG | XGridDrawer::DRAW_NUMBER);
    }
}










//-------------------------------------------------------
// Focus functions
//-------------------------------------------------------
puz::Square *
XGridCtrl::ChangeFocusedSquare(puz::Square * square,
                               puz::Word * word,
                               short direction)
{
    wxASSERT(m_focusedSquare != NULL || square != NULL);
    if (square == NULL)
        square = m_focusedSquare;

    // If there is no word in the given direction and there is one
    // in the opposite direction, switch the direction.
    if (! word
        && ! square->HasWord(static_cast<puz::GridDirection>(direction))
        && ! m_puz->FindWord(square, direction))
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
    // Save old state
    //-------------------
    puz::Square * oldSquare = m_focusedSquare;
    puz::Word * oldWord;
    if (oldSquare == NULL)
        oldWord = NULL;
    else
        oldWord = m_focusedWord;

    wxClientDC dc(this); DoPrepareDC(dc);

    // If this was an invented focused word, it will be deleted before
    // we have a chance to redraw.  We need to redraw it now.
    if (m_ownsFocusedWord && oldWord)
    {
        puz::square_iterator it;
        for (it = oldWord->begin(); it != oldWord->end(); ++it)
            DrawSquare(dc, *it, wxNullColour);
        oldWord = NULL;
    }

    // Set new state
    //--------------
    m_focusedSquare = square;
    DoSetFocusedWord(word, direction);
    wxASSERT(m_focusedWord != NULL);

    MakeVisible(*m_focusedSquare);

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
                DrawSquare(dc, *it, wxNullColour);
        }
        else if (oldSquare)
            DrawSquare(dc, *oldSquare, wxNullColour);

        // Draw the new focused word
        if (m_focusedWord)
        {
            puz::square_iterator it;
            for (it = m_focusedWord->begin(); it != m_focusedWord->end(); ++it)
                DrawSquare(dc, *it);
        }
    }

    RecalcDirection();
    return m_focusedSquare;
}


puz::Square *
XGridCtrl::SetFocusedSquare(puz::Square * square,
                            puz::Word * word,
                            short direction)
{
    ChangeFocusedSquare(square, word, direction);
    SendEvent();
    return m_focusedSquare;
}

void
XGridCtrl::SendEvent()
{
    wxPuzEvent evt(wxEVT_PUZ_GRID_FOCUS, GetId());
    evt.SetSquare(m_focusedSquare);
    evt.SetWord(m_focusedWord);
    GetEventHandler()->ProcessEvent(evt);
}


void
XGridCtrl::ChangeFocusedWord(puz::Word * word, short direction)
{
    wxASSERT(word != NULL);
    puz::Square * square = word->front();
    if (HasStyle(BLANK_ON_NEW_WORD))
        SetIfExists(square, word->FindSquare(FIND_BLANK_SQUARE));
    ChangeFocusedSquare(square, word, direction);
}

void
XGridCtrl::DoSetFocusedWord(puz::Word * word, short direction)
{
    if (m_grid->IsDiagramless())
    {
        assert(m_ownsFocusedWord);
        delete m_focusedWord;
        if (m_focusedSquare->IsBlack())
        {
            m_focusedWord = new puz::StraightWord(
                m_focusedSquare, m_focusedSquare
            );
        }
        else
        {
            puz::GridDirection dir = puz::IsHorizontal(direction)
                                        ? puz::ACROSS : puz::DOWN;
            m_focusedWord = new puz::StraightWord(
                m_focusedSquare->GetWordStart(dir),
                m_focusedSquare->GetWordEnd(dir)
            );
        }
        return;
    }

    // If these words are going to be the same, return now
    if (m_focusedWord != NULL
        && (word == m_focusedWord
            || (word == NULL
                && puz::AreInLine(m_focusedDirection, direction)
                && m_focusedWord->Contains(m_focusedSquare))))
    {
        return;
    }
    if (m_ownsFocusedWord)
    {
        delete m_focusedWord;
        m_ownsFocusedWord = false;
    }
    if (word == NULL)
    {
        word = m_puz->FindWord(m_focusedSquare, direction);
        if (word == NULL)
        {
            if (puz::IsDiagonal(direction))
            {
                direction = m_focusedDirection;
                word = m_puz->FindWord(m_focusedSquare, m_focusedDirection);
            }
            if (word == NULL)
            {
                // Create a word
                puz::GridDirection dir = puz::IsHorizontal(direction)
                                            ? puz::ACROSS : puz::DOWN;
                word = new puz::StraightWord(
                    m_focusedSquare->GetWordStart(dir),
                    m_focusedSquare->GetWordEnd(dir)
                );
                m_ownsFocusedWord = true;
            }
        }
    }
    m_focusedWord = word;
}

void
XGridCtrl::MakeVisible(const puz::Square & square)
{
    if (m_fit || IsEmpty())
        return;

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

    Scroll(startX, startY);
}



bool
XGridCtrl::SetSquareText(puz::Square & square, const wxString & text)
{
    // Not allowed to overwrite revealed letters
    if (square.HasFlag(puz::FLAG_REVEALED))
        return false;

    // Adjust blank and incorrect counts each time a letter is changed
    // The logic is a little confusing at first, but it's correct
    const int correctBefore = square.Check(puz::NO_CHECK_BLANK, HasStyle(STRICT_REBUS));
    const int blankBefore   = square.IsBlank() && ! square.IsSolutionBlank();
    // renumber the grid if the square is changing from black to white or vice-versa
    const bool numberGrid = GetGrid()->IsDiagramless() &&
                            (text == puz2wx(puz::Square::Black) != (square.IsBlack()));

    // This could throw an exception if text is longer than
    // 8 characeters, so try this first.
    square.SetText(wx2puz(text));

    if (square.HasFlag(puz::FLAG_X))
    {
        square.AddFlag(puz::FLAG_BLACK);
        square.RemoveFlag(puz::FLAG_X);
    }

    const int correctAfter = square.Check(puz::NO_CHECK_BLANK, HasStyle(STRICT_REBUS));
    const int blankAfter   = square.IsBlank() && ! square.IsSolutionBlank();

    m_blankSquares     += blankAfter   - blankBefore;
    m_incorrectSquares -= correctAfter - correctBefore;

    // Don't send letter updated events if the user is in the middle of entering
    // a rebus entry.
    if (! m_wantsRebus)
    {
        if (HasStyle(CHECK_WHILE_TYPING) && ! m_grid->IsScrambled())
            CheckLetter(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);

        wxPuzEvent evt(wxEVT_PUZ_LETTER, GetId());
        evt.SetString(text);
        GetEventHandler()->ProcessEvent(evt);
    }

    if (numberGrid)
    {
        GetGrid()->NumberGrid();
        Refresh();
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
    GetEventHandler()->ProcessEvent(
        GridSelectionEvent(GetId(), wxEVT_Grid_SELECTION,
                            this,
                            m_selectionStart, m_selectionEnd)
    );
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
    if (m_focusedSquare != NULL)
        MakeVisible(*m_focusedSquare);
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
}




//-------------------------------------------------------
// Check/Reveal functions
//-------------------------------------------------------
void
XGridCtrl::CheckGrid(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    const bool checkBlank = (options & CHECK_ALL) != 0;
    std::vector<puz::Square *> incorrect;
    m_grid->CheckGrid(&incorrect, checkBlank, HasStyle(STRICT_REBUS));

    PostCheck(incorrect, options);
}


void
XGridCtrl::CheckWord(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    const bool checkBlank = (options & CHECK_ALL) != 0;
    std::vector<puz::Square *> incorrect;
    m_grid->CheckWord( &incorrect, m_focusedWord,
                       checkBlank,
                       HasStyle(STRICT_REBUS));

    PostCheck(incorrect, options);
}



void
XGridCtrl::CheckLetter(int options)
{
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

    puz::Square * square = GetFocusedSquare();

    const bool checkBlank = (options & CHECK_ALL) != 0;

    std::vector<puz::Square *> incorrect;
    if (! square->Check(checkBlank, HasStyle(STRICT_REBUS)))
        incorrect.push_back(square);

    PostCheck(incorrect, options);
}


void
XGridCtrl::PostCheck(std::vector<puz::Square *> & incorrect, int options)
{
    if (incorrect.empty() && (options & NO_MESSAGE_BOX) == 0)
    {
        XWordMessage(this, MSG_NO_INCORRECT);
        return;
    }

    wxClientDC dc(this); DoPrepareDC(dc);
    for (std::vector<puz::Square *>::iterator it = incorrect.begin();
         it != incorrect.end();
         ++it)
    {
        puz::Square * square = *it;

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
    }
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
// These class names are an absolte mess.


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

    const bool checkBlank = (options & CHECK_ALL) != 0;

    std::vector<puz::Square *> incorrect;

    for (int col = start->GetCol(); col <= end->GetCol(); ++col)
        for (int row = start->GetRow(); row <= end->GetRow(); ++row)
        {
            puz::Square * square = &At(col, row);
            if (square->IsWhite() && ! square->Check(checkBlank, HasStyle(STRICT_REBUS)))
                incorrect.push_back(square);
        }

    PostCheck(incorrect, options);
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
        Connect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
        Connect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
        Connect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
        Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    }
    m_areEventsConnected = true;
}


void
XGridCtrl::DisconnectEvents()
{
    if (m_areEventsConnected)
    {
        Disconnect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
        Disconnect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
        Disconnect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
        Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    }
    m_areEventsConnected = false;
}



void
XGridCtrl::OnLeftDown(wxMouseEvent & evt)
{
    wxASSERT(! IsEmpty());

    wxPoint pt = evt.GetPosition();
    puz::Square * square = HitTest(pt.x, pt.y);
    if (square != NULL && (square->IsWhite() || GetGrid()->IsDiagramless()))
        SetFocusedSquare(square);

    // Make sure to skip this event or we don't get keyboard focus!
    evt.Skip();
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
        OnInsert(mod);
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
            SetSquareText(*GetFocusedSquare(), puz2wx(puz::Square::Black));
        MoveAfterLetter();
    }
    else if (IsValidChar(key))
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

    wxASSERT(! m_wantsRebus);

    if (static_cast<int>(key) == WXK_SPACE)
        SetSquareText(*m_focusedSquare, _T(""));
    else
        SetSquareText(*m_focusedSquare, key);

    // Space bar always moves forward one square
    if (static_cast<int>(key) == WXK_SPACE)
        SetFocusedSquare(m_focusedWord->FindNextSquare(m_focusedSquare, FIND_WHITE_SQUARE));
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

    SetFocusedSquare(newSquare);
}


void
XGridCtrl::OnArrow(puz::GridDirection arrowDirection, int mod)
{
    wxASSERT(! IsEmpty());
    if (mod != wxMOD_SHIFT || IsDiagonal(arrowDirection))
    {
        if (! GetGrid()->IsDiagramless())
        {
            if (HasStyle(PAUSE_ON_SWITCH) && m_focusedSquare->IsBlank())
            {
                // Check to see if there *is* a word in arrowDirection.
                puz::Word * word = m_puz->FindWord(m_focusedSquare, arrowDirection);
                if (word && word != m_focusedWord)
                {
                    SetFocusedSquare(m_focusedSquare, word, arrowDirection);
                    return;
                }
                // Check to see if there *should be* a (non-diagonal) word
                // in arrowDirection.
                if (! AreInLine(arrowDirection, m_focusedDirection)
                    && ! IsDiagonal(arrowDirection)
                    && m_focusedSquare->HasWord(arrowDirection))
                {
                    SetFocusedSquare(m_focusedSquare, NULL, arrowDirection);
                    return;
                }
            }
            // Find the next white square in the arrow direction
            SetFocusedSquare(
                m_grid->FindNextSquare(
                    m_focusedSquare, FIND_WHITE_SQUARE,
                    arrowDirection, puz::NO_WRAP
                ),
                NULL, arrowDirection
            );
        }
        else // diagramless
        {
            if (! m_focusedSquare->IsLast(arrowDirection))
                SetFocusedSquare(m_focusedSquare->Next(arrowDirection));
        }
    }
    else // Shift
    {
        puz::GridDirection focusedDirection = 
            static_cast<puz::GridDirection>(m_focusedDirection);
        puz::Square * newSquare = NULL;
        puz::Word * word = NULL;
        if (AreInLine(m_focusedDirection, arrowDirection))
        {
            // Move to the next word in the arrow direction
            for (newSquare = m_focusedSquare;
                 newSquare;
                 newSquare = newSquare->Next(arrowDirection))
            {
                word = m_puz->FindWord(newSquare, m_focusedDirection);
                if (word)
                {
                    if (word != m_focusedWord)
                        break;
                    else
                        word = NULL;
                }
                else if (newSquare->HasWord(focusedDirection)
                          && newSquare->GetSolutionWordStart(focusedDirection)
                                != m_focusedWord->front())
                {
                    break;
                }
            }
            // Find the first square in the word
            if (newSquare)
            {
                if (word)
                    newSquare = word->front();
                else
                    newSquare = newSquare->GetSolutionWordStart(focusedDirection);
            }
        }
        else
        {
            // Move to the next white square in the arrow direction
            newSquare = m_grid->FindNextSquare(
                m_focusedSquare,
                FIND_WHITE_SQUARE,
                arrowDirection,
                puz::NO_WRAP
            );
        }
        if (newSquare && HasStyle(BLANK_ON_NEW_WORD))
        {
            if (word)
                SetIfExists(newSquare, word->FindSquare(FIND_BLANK_SQUARE));
            else
                SetIfExists(newSquare,
                    m_grid->FindSquare(
                        newSquare->GetSolutionWordStart(focusedDirection),
                        FIND_BLANK_SQUARE,
                        focusedDirection,
                        puz::FIND_IN_WORD
                    )
                );
        }
        SetFocusedSquare(newSquare, word);
    }
}



void
XGridCtrl::OnBackspace(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    SetSquareText(*m_focusedSquare, _T(""));
    SetFocusedSquare(m_focusedWord->FindNextSquare(
        m_focusedSquare, FIND_WHITE_SQUARE, puz::PREV
    ));
}



void
XGridCtrl::OnDelete(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    SetSquareText(*m_focusedSquare, _T(""));
    SetFocusedSquare();
}



void
XGridCtrl::OnHome(int mod)
{
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite instead of the usual ctrl / command
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
    {
        puz::Square * newSquare = FirstWhite();
        puz::Word * word = m_puz->FindWord(newSquare);
        if (HasStyle(BLANK_ON_NEW_WORD))
        {
            SetFocusedSquare( EitherOr(newSquare,
                                       word->FindSquare(newSquare,
                                                        FIND_BLANK_SQUARE)));
        }
        else
        {
            SetFocusedSquare(newSquare);
        }
    }
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


// Allow entering multiple letters
void
XGridCtrl::OnInsert(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    if (! m_wantsRebus)
    {
        m_wantsRebus = true;

        RefreshSquare();

        // GridRebusHandler will capture keyboard until it is destroyed.
        // When GridRebusHandler recieves a key event that indicates that
        // the user is done entering a rebus square, it will destroy itself.
        new GridRebusHandler(*this);
    }
    else
    {
        m_wantsRebus = false;
        // Force checking the square since checking has been prevented by
        // GridRebusHandler.
        SetSquareText(*m_focusedSquare, puz2wx(m_focusedSquare->GetText()));

        if (! m_focusedSquare->IsBlank())
            MoveAfterLetter();
        else
            RefreshSquare();
    }
}


void
XGridCtrl::OnTab(int mod)
{
    wxASSERT(! IsEmpty());
    puz::Square * newSquare = NULL;
    // Move to the next clue in the current clue list
    if (m_ownsFocusedWord) // Implies that there is no clue.
    {
        // Find the next clue
        OnArrow(mod != wxMOD_SHIFT ? puz::ACROSS : puz::LEFT, wxMOD_SHIFT);
        return;
    }
    // Otherwise find the clue in one of the clue lists.
    puz::Clues & clues = m_puz->GetClues();
    puz::Clues::iterator clues_it;
    for (clues_it = clues.begin(); clues_it != clues.end(); ++clues_it)
    {
        puz::ClueList & cluelist = clues_it->second;
        puz::ClueList::iterator it;
        for (it = cluelist.begin(); it != cluelist.end(); ++it)
            if (it->GetWord() == m_focusedWord)
                break;
        if (it != cluelist.end()) // We found our clue
        {
            if (mod != wxMOD_SHIFT)
            {
                // Move to the next clue
                ++it;
                if (it == cluelist.end())
                {
                    // If we're at the end of this list, try the next list
                    ++clues_it;
                     // If we're at the end, go to the first clue
                    if (clues_it == clues.end())
                        clues_it = clues.begin();
                    SetFocusedWord(clues_it->second.front().GetWord());
                }
                else
                {
                    SetFocusedWord(it->GetWord());
                }
            }
            else // Shift
            {
                // Move to the previous clue
                // If we're at the beginning of this list, try the previous
                if (it == cluelist.begin())
                {
                    // If we're at the beginning, go to the last clue
                    if (clues_it == clues.begin())
                        clues_it = clues.end(); // One past the end
                    // Find the previous clue list
                    --clues_it;
                    SetFocusedWord(clues_it->second.back().GetWord());
                }
                else
                {
                    --it;
                    SetFocusedWord(it->GetWord());
                }
            }
        }
    }
    // TODO: Blank on new word
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
    else
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
    const puz::Square * square1 = m_focusedSquare;
    const puz::Square * square2 = m_focusedWord->FindNextSquare(m_focusedSquare, FIND_WHITE_SQUARE);
    if (! square2 || square2 == square1)
    {
        // If this is the end of the word, measure the angle between
        // the previous square and this square
        square2 = m_focusedSquare;
        square1 = m_focusedWord->FindNextSquare(m_focusedSquare, FIND_WHITE_SQUARE, puz::PREV);
    }
    if (square1 && square2)
        m_focusedDirection = puz::GetDirection(*square1, *square2);
    // Else focused direction doesn't change.
}








//------------------------------------------------------------------------------
// GridRebusHandler implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GridRebusHandler, wxEvtHandler)
    EVT_KEY_DOWN    (GridRebusHandler::OnKeyDown)
    EVT_CHAR        (GridRebusHandler::OnChar)
END_EVENT_TABLE()

GridRebusHandler::GridRebusHandler(XGridCtrl & grid)
    : m_grid(grid)
{
    m_grid.PushEventHandler(this);
    // Kill the grid's own event handling
    m_grid.Disconnect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    m_grid.Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    m_grid.Disconnect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    m_grid.Disconnect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
}

void
GridRebusHandler::EndEventHandling()
{
    m_grid.RemoveEventHandler(this);
    // Restore the grid's own key handling
    m_grid.Connect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    m_grid.Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    m_grid.Connect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    m_grid.Connect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
}

void
GridRebusHandler::OnKeyDown(wxKeyEvent & evt)
{
    // Skip this event by default so that we recieve EVT_CHAR events.
    evt.Skip();

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();
    if      (key == WXK_INSERT || key == WXK_NUMPAD_INSERT ||
             key == WXK_RETURN || key == WXK_NUMPAD_ENTER ||
             key == WXK_ESCAPE)
    {
        evt.Skip(false);
        EndEventHandling();
        // Notify the XGridCtrl that we are done capturing events.
        m_grid.OnInsert(wxMOD_NONE);
        delete this;
    }
    else if (key == WXK_BACK)
    {
        puz::Square * square = m_grid.GetFocusedSquare();
        if (square->IsBlank())
            return;
        if ((mod & (wxMOD_CONTROL | wxMOD_SHIFT)) != 0)
            m_grid.SetSquareText(*square, _T(""));
        else
            m_grid.SetSquareText(*square,
                puz2wx(square->GetText().substr(0, square->GetText().length() - 1)));
        m_grid.RefreshSquare();
    }
    else if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        puz::Square * square = m_grid.GetFocusedSquare();
        if (square->IsBlank())
            return;
        if ((mod & (wxMOD_CONTROL | wxMOD_SHIFT)) != 0)
            m_grid.SetSquareText(*square, _T(""));
        else
            m_grid.SetSquareText(*square, puz2wx(square->GetText().substr(1)));
        m_grid.RefreshSquare();
    }

}


void
GridRebusHandler::OnChar(wxKeyEvent & evt)
{
    evt.Skip();

    const wxChar key = static_cast<wxChar>(evt.GetKeyCode());

    if (! XGridCtrl::IsValidChar(key))
        return;

    puz::Square * square = m_grid.GetFocusedSquare();

    if (square->GetText().length() >= puz::REBUS_ENTRY_LENGTH)
        return;
    if (square->IsBlank())
        m_grid.SetSquareText(*square, key);
    else
        m_grid.SetSquareText(*square, puz2wx(square->GetText()) + key);
    m_grid.RefreshSquare();
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
    m_grid.Disconnect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    m_grid.Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    m_grid.Disconnect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    m_grid.Disconnect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));

    m_grid.m_selectionStart = NULL;
    m_grid.m_selectionEnd = NULL;

    m_grid.SetCursor(wxCursor(wxCURSOR_CROSS));
}

void
GridSelectionHandler::EndEventHandling()
{
    m_grid.RemoveEventHandler(this);
    // Restore the grid's own key handling
    m_grid.Connect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    m_grid.Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
    m_grid.Connect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    m_grid.Connect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));

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
