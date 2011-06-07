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


#include "XGridCtrl.hpp"
#include <wx/dcbuffer.h>
#include <list>
#include <algorithm>
#include "PuzEvent.hpp"
#include "XGridDrawer.hpp"
#include "messages.hpp"
#include "SelectionEvent.hpp"
#include "utils/string.hpp"
#include "utils/timeit.hpp"

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


inline puz::Square *
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
    m_focusedStart = NULL;
    m_focusedEnd = NULL;

    m_selectionStart = NULL;
    m_selectionEnd = NULL;
    m_isSelecting = false;

    m_direction = puz::ACROSS;

    m_isPaused = false;
    m_fit = true;

    m_lastBoxSize = UNDEFINED_BOX_SIZE;

    m_incorrectSquares = 0;
    m_blankSquares = 0;

    m_wantsRebus = false;

    m_areEventsConnected = false;

    // m_rect is already equal to wxRect(0,0,0,0) from its constructor
}


bool
XGridCtrl::Create(wxWindow * parent,
                  wxWindowID id,
                  const wxPoint & pos,
                  const wxSize & size,
                  puz::Grid * grid,
                  long style,
                  const wxString & name)
{
    style |= wxWANTS_CHARS;
    if (! wxScrolledWindow::Create(parent, id, pos, size, style, name))
        return false;

    // Have to do this here, or wxMac will break
    // (XGridDrawer would use GetTextExtent on a not yet created window)
    m_drawer.SetWindow(this);

    SetGrid(grid);

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
}



void
XGridCtrl::SetGrid(puz::Grid * grid)
{
    if (grid == NULL)
    {
        m_grid = NULL;
        m_incorrectSquares = 0;
        m_blankSquares = 0;
        m_direction = puz::ACROSS;
        m_focusedSquare = NULL;
        m_focusedStart = NULL;
        m_focusedEnd = NULL;

        DisconnectEvents();
    }
    else
    {
        m_grid = grid;

        RecheckGrid();

        m_direction     = puz::ACROSS;
        m_focusedSquare = m_grid->FirstWhite();
        m_focusedStart  = m_focusedSquare->GetWordStart(m_direction);
        m_focusedEnd    = m_focusedSquare->GetWordEnd  (m_direction);

        ConnectEvents();
    }
    m_drawer.SetGrid(grid);
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

    // Draw black as crossword background
    dc.SetBrush(wxBrush(GetBlackSquareColor()));
    dc.SetPen  (wxPen(GetBlackSquareColor()));

    dc.DrawRectangle(m_drawer.GetRect());

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
    // Don't draw black squares
    if (square.IsBlack())
        return;

    // If the user has pressed <insert> we are in rebus mode.  The focused square
    // will be outlined in the selected color (and have a white background).
    // Certain features will not be drawn in this case:
    //    - The number
    //    - The incorrect/revealed indicator
    //    - The X.
    const bool drawOutline = m_wantsRebus && IsFocusedLetter(square);
    if (drawOutline)
    {
        m_drawer.AddFlag(XGridDrawer::DRAW_OUTLINE);
        m_drawer.RemoveFlag(XGridDrawer::DRAW_FLAG | XGridDrawer::DRAW_NUMBER);
    }

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
XGridCtrl::ChangeSquareFocus(puz::Square * square, puz::GridDirection direction)
{
    wxASSERT(m_focusedSquare != NULL || square != NULL);

    // Save old state
    //-------------------
    puz::Square * oldFocused = m_focusedSquare;
    puz::Square * oldStart;
    puz::Square * oldEnd;
    if (oldFocused == NULL)
    {
        oldStart = NULL;
        oldEnd   = NULL;
    }
    else
    {
        oldStart = oldFocused->GetWordStart(m_direction);
        oldEnd   = oldFocused->GetWordEnd(m_direction);
    }

    puz::GridDirection oldDirection = m_direction;

    // Set new state
    //-------------------
    m_direction = direction;
    if (square != NULL)
        m_focusedSquare = square;
    // else m_focusedSquare stays the same

    // If there is no word for this direction, try the other direction
    if (m_focusedSquare->GetWordStart(m_direction) == NULL)
        m_direction = puz::SwapDirection(m_direction);

    m_focusedStart  = m_focusedSquare->GetWordStart(m_direction);
    m_focusedEnd    = m_focusedSquare->GetWordEnd  (m_direction);

    wxASSERT(m_focusedStart != NULL && m_focusedEnd != NULL);

    MakeVisible(*m_focusedSquare);

    // Redraw
    wxClientDC dc(this); DoPrepareDC(dc);

    // If the old and new focused words are the same, only redraw two squares
    if (m_focusedStart == oldStart && m_focusedEnd == oldEnd)
    {
        DrawSquare(dc, *oldFocused, GetFocusedWordColor());
    }
    // Otherwise redraw both words
    else
    {
        // Only draw old squares if there was a focused word to begin with
        if (oldFocused != NULL)
            for (square = oldStart;
                 square != NULL;
                 square = square->Next(oldDirection))
            {
                DrawSquare(dc, *square, GetWhiteSquareColor());
                if (square == oldEnd)
                    break;
            }
        // Draw new word
        for (square = m_focusedStart;
             square != NULL;
             square = square->Next(m_direction))
        {
            DrawSquare(dc, *square, GetFocusedWordColor());
            if (square == m_focusedEnd)
                break;
        }
    }

    // Draw the square with focus
    DrawSquare(dc, *m_focusedSquare, GetFocusedLetterColor());

    // Always keep focus here
    //wxScrolledWindow::SetFocus();

    // return whatever square now has focus
    return m_focusedSquare;
}


puz::Square *
XGridCtrl::SetSquareFocus(puz::Square * square, puz::GridDirection direction)
{
    square = ChangeSquareFocus(square, direction);

    wxPuzEvent evt(wxEVT_PUZ_GRID_FOCUS, GetId());

    // Check for across clue
    puz::Square * start = square->GetWordStart(puz::ACROSS);
    if (start == NULL)
        evt.SetAcrossClue(0);
    else
        evt.SetAcrossClue(start->GetNumber());

    // Check for down clue
    start = square->GetWordStart(puz::DOWN);
    if (start == NULL)
        evt.SetDownClue(0);
    else
        evt.SetDownClue(start->GetNumber());

    // One of the two needs to be set
    wxASSERT(evt.GetAcrossClue() != 0 || evt.GetDownClue() != 0);

    evt.SetDirection(m_direction);
    evt.SetString(puz2wx(square->GetText()));
    GetEventHandler()->ProcessEvent(evt);

    return square;
}


void
XGridCtrl::ChangeFocusedClue(int cluenum, puz::GridDirection direction)
{
    puz::Square * square = GetClueNumber(cluenum);
    if (square == NULL)
        return;

    if (HasStyle(BLANK_ON_NEW_WORD))
        SetIfExists(square,
                    FindSquare(square->GetWordStart(direction),
                               std::mem_fun(&puz::Square::IsBlank),
                               direction) );

    ChangeSquareFocus(square, direction);
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
    if (square.HasFlag(puz::FLAG_RED))
        return false;

    // Adjust blank and incorrect counts each time a letter is changed
    // The logic is a little confusing at first, but it's correct
    const int correctBefore = square.Check(puz::NO_CHECK_BLANK, HasStyle(STRICT_REBUS));
    const int blankBefore   = square.IsBlank();

    // This could throw an exception if text is longer than
    // 8 characeters, so try this first.
    square.SetText(wx2puz(text));

    if (square.HasFlag(puz::FLAG_X))
        square.ReplaceFlag(puz::FLAG_X, puz::FLAG_BLACK);

    const int correctAfter = square.Check();
    const int blankAfter   = square.IsBlank();

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
    m_grid->CheckWord( &incorrect,
                       m_focusedSquare->GetWordStart(m_direction),
                       m_focusedSquare->GetWordEnd  (m_direction),
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
        XWordMessage( MSG_NO_INCORRECT );
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
            square->AddFlag(puz::FLAG_RED);
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
    if (square != NULL && square->IsWhite())
        SetSquareFocus(square, m_direction);

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
    if (square != NULL && square->IsWhite())
        SetSquareFocus(square, puz::SwapDirection(m_direction));

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

    if      (key == WXK_LEFT  || key == WXK_NUMPAD_LEFT)     /* Left      */
        OnArrow(puz::ACROSS, puz::PREV, mod);
    else if (key == WXK_RIGHT || key == WXK_NUMPAD_RIGHT)    /* Right     */
        OnArrow(puz::ACROSS, puz::NEXT, mod);
    else if (key == WXK_UP    || key == WXK_NUMPAD_UP)       /* Up        */
        OnArrow(puz::DOWN,   puz::PREV, mod);
    else if (key == WXK_DOWN  || key == WXK_NUMPAD_DOWN)     /* Down      */
        OnArrow(puz::DOWN,   puz::NEXT, mod);
    else if (key == WXK_HOME  || key == WXK_NUMPAD_HOME)     /* Home      */
        OnHome(mod);
    else if (key == WXK_END   || key == WXK_NUMPAD_END)      /* End       */
        OnEnd(mod);
    else if (key == WXK_TAB   || key == WXK_NUMPAD_TAB ||    /* Tab       */
             key == WXK_RETURN || key == WXK_NUMPAD_ENTER)   /* Enter     */
        OnTab(mod);
    else if (key == WXK_BACK)                                /* Backspace */
        OnBackspace(mod);
    else if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)  /* Delete    */
        OnDelete(mod);
    else if (key == WXK_INSERT || key == WXK_NUMPAD_INSERT)  /* Insert    */
    {
        OnInsert(mod);
        return; // Don't skip this event.
    }
    else
    {
        evt.Skip();
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
    wxASSERT(m_focusedSquare->GetWordStart(m_direction)->GetNumber() != 0);

    if (! (mod == wxMOD_NONE || mod == wxMOD_SHIFT))
        return;

    puz::Square & square = *GetFocusedSquare();
    wxASSERT(! m_wantsRebus);

    if (static_cast<int>(key) == WXK_SPACE)
        SetSquareText(square, _T(""));
    else
        SetSquareText(square, key);

    // Space bar always moves forward one square
    if (static_cast<int>(key) == WXK_SPACE)
        SetSquareFocus(FindNextSquare(m_focusedSquare, std::mem_fun(&puz::Square::IsWhite), m_direction));
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
            newSquare = FindNextSquare(m_focusedSquare,
                                      std::mem_fun(&puz::Square::IsBlank),
                                      m_direction,
                                      puz::NEXT);
            // Last blank in the word: jump to the first blank if it exists
            if (newSquare == NULL)
            {
                newSquare = FindSquare(m_focusedSquare->GetWordStart(m_direction),
                                       std::mem_fun(&puz::Square::IsBlank),
                                       m_direction,
                                       puz::NEXT);
            }
        }
        // Either the user wants "move to next letter", or there are no more blanks in the
        // word.  Move to the next letter if it exists
        if (newSquare == NULL)
        {
            newSquare = FindNextSquare(m_focusedSquare,
                                       std::mem_fun(&puz::Square::IsWhite),
                                       m_direction,
                                       puz::NEXT);
            // if newSquare == NULL (possibly again), it's the last square in the word,
            // and the focus won't change.
        }
    }

    SetSquareFocus(newSquare, m_direction);
}



void
XGridCtrl::OnArrow(puz::GridDirection arrowDirection, puz::FindDirection increment, int mod)
{
    wxASSERT(! IsEmpty());
    puz::Square * newSquare = NULL;

    // If there is no word in the arrowDirection, treat it as if shift
    // were pressed.
    if (m_focusedSquare->GetWordStart(arrowDirection) == NULL)
        mod = wxMOD_SHIFT;

    if (mod == wxMOD_SHIFT)
    {
        if (m_direction == arrowDirection)
        {
            newSquare =
                FindNextSquare(m_focusedSquare,
                               puz::FIND_WORD(arrowDirection, m_focusedSquare),
                               arrowDirection,
                               increment,
                               puz::SKIP_BLACK_SQUARES,
                               puz::WRAP_LINES);

            if (newSquare != NULL)
                newSquare = newSquare->GetWordStart(arrowDirection);
        }
        else
        {
            newSquare =
                FindNextSquare(m_focusedSquare,
                               std::mem_fun(&puz::Square::IsWhite),
                               arrowDirection,
                               increment,
                               puz::SKIP_BLACK_SQUARES);
        }

        // Test BLANK_ON_NEW_WORD
        const bool isNewWord = newSquare != NULL &&
               newSquare->      GetWordStart(m_direction) !=
               m_focusedSquare->GetWordStart(m_direction);

        if (HasStyle(BLANK_ON_NEW_WORD) && isNewWord)
        {
            SetIfExists(newSquare,
                FindSquare(newSquare->GetWordStart(m_direction),
                           std::mem_fun(&puz::Square::IsBlank),
                           m_direction,
                           puz::NEXT));
        }

        SetSquareFocus(newSquare, m_direction);
    }
    else // Shift is not pressed
    {
        if (m_direction != arrowDirection
            && HasStyle(PAUSE_ON_SWITCH)
            && m_focusedSquare->IsBlank())
        {
            SetSquareFocus(m_focusedSquare, puz::SwapDirection(m_direction));
            return;
        }
        else
        {
            newSquare =
                FindNextSquare(m_focusedSquare,
                               std::mem_fun(&puz::Square::IsWhite),
                               arrowDirection,
                               increment,
                               puz::SKIP_BLACK_SQUARES);
        }

        SetSquareFocus(newSquare, arrowDirection);
    }
}



void
XGridCtrl::OnBackspace(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    puz::Square & square = *m_focusedSquare;

    SetSquareText(square, _T(""));

    SetSquareFocus(FindNextSquare(m_focusedSquare,
                                  std::mem_fun(&puz::Square::IsWhite),
                                  m_direction,
                                  puz::PREV),
                   m_direction);
}



void
XGridCtrl::OnDelete(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    puz::Square & square = *m_focusedSquare;

    SetSquareText(square, _T(""));

    SetSquareFocus(m_focusedSquare, m_direction);
}



void
XGridCtrl::OnHome(int mod)
{
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite instead of the usual ctrl / command
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
    {
        puz::Square * newSquare = m_grid->FirstWhite();
        if (HasStyle(BLANK_ON_NEW_WORD))
        {
            SetSquareFocus( EitherOr(newSquare,
                                     FindSquare(newSquare,
                                                std::mem_fun(&puz::Square::IsBlank),
                                                m_direction)),
                             m_direction );
        }
        else
        {
            SetSquareFocus(newSquare, m_direction);
        }
    }
    else
        SetSquareFocus(m_focusedSquare->GetWordStart(m_direction), m_direction);
}



void
XGridCtrl::OnEnd(int mod)
{
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite instead of the usual ctrl / command
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
        SetSquareFocus(m_grid->LastWhite(), m_direction);
    else
        SetSquareFocus(m_focusedSquare->GetWordEnd(m_direction), m_direction);
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
    puz::GridDirection newDir = m_direction;
    if (mod == wxMOD_SHIFT)
    {
        newSquare =
            FindNextSquare(m_focusedStart,
                           puz::FIND_CLUE(m_direction, m_focusedStart),
                           puz::ACROSS,
                           puz::PREV,
                           puz::SKIP_BLACK_SQUARES,
                           puz::WRAP_LINES);

        if (newSquare == NULL) // We're at the start of the grid
        {
            // Find the last clue (different depending on direction)
            newDir = puz::SwapDirection(newDir);
            newSquare =
                FindNextSquare(m_grid->LastWhite(),
                               puz::FIND_CLUE(newDir, m_grid->FirstWhite()),
                               puz::ACROSS,
                               puz::PREV,
                               puz::SKIP_BLACK_SQUARES,
                               puz::WRAP_LINES);
        }
    }
    else
    {
        newSquare =
            FindNextSquare(m_focusedStart,
                           puz::FIND_CLUE(m_direction, m_focusedStart),
                           puz::ACROSS,
                           puz::NEXT,
                           puz::SKIP_BLACK_SQUARES,
                           puz::WRAP_LINES);

        if (newSquare == NULL) // We're at the end of the grid
        {
            newDir = puz::SwapDirection(newDir);
            newSquare = FindSquare(m_grid->FirstWhite(),
                                   std::bind2nd(std::mem_fun(&puz::Square::HasClue),
                                                newDir),
                                   puz::ACROSS,
                                   puz::NEXT,
                                   puz::SKIP_BLACK_SQUARES,
                                   puz::WRAP_LINES);
        }
    }
    newSquare = newSquare->GetWordStart(newDir);

    const bool isNewWord =
        newSquare->GetWordStart(newDir)
            != m_focusedSquare->GetWordStart(m_direction);

    if (HasStyle(BLANK_ON_NEW_WORD) &&  isNewWord)
        SetIfExists( newSquare,
                     FindSquare(newSquare,
                                std::mem_fun(&puz::Square::IsBlank),
                                newDir,
                                puz::NEXT) );

    SetSquareFocus(newSquare, newDir);
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
        return GetWhiteSquareColor();
}




puz::Square *
XGridCtrl::GetClueNumber(int num)
{
    wxASSERT(! IsEmpty());
    wxASSERT(m_grid->FirstWhite() != NULL);
    for (puz::Square * square = m_grid->FirstWhite();
         square != NULL;
         square = square->Next())
    {
        if (square->GetNumber() == num)
            return square;
    }
    return NULL;
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