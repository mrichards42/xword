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


BEGIN_EVENT_TABLE(XGridCtrl, wxScrolledWindow)
    EVT_PAINT           (XGridCtrl::OnPaint)
    EVT_SIZE            (XGridCtrl::OnSize)
    EVT_KEY_DOWN        (XGridCtrl::OnKeyDown)
    EVT_LEFT_DOWN       (XGridCtrl::OnLeftDown)
    EVT_RIGHT_DOWN      (XGridCtrl::OnRightDown)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(XGridCtrl, wxScrolledWindow)

const int MAX_POINT_SIZE = 150;
const int MIN_POINT_SIZE = 2;

const int UNDEFINED_BOX_SIZE = -1;

// Helper functions for all of the "lookup" functions that return NULL
inline void
SetIfExists(XSquare *& current, XSquare * test)
{
    if (test != NULL)
        current = test;
}


inline XSquare *
EitherOr(XSquare * current, XSquare * test)
{
    if (test != NULL)
        return test;
    return current;
}



const wxChar * XGridCtrlNameStr = _T("XGridCtrl");



void XGridCtrl::Init()
{
    m_fit = true;
    m_boxSize = 20;
    m_lastBoxSize = UNDEFINED_BOX_SIZE;
    m_borderSize = 1;
    SetXGrid(NULL);
}


bool
XGridCtrl::Create(wxWindow * parent,
                 wxWindowID id,
                 const wxPoint & pos,
                 const wxSize & size,
                 XGrid * grid,
                 long style,
                 const wxString & name)
{
    style |= wxWANTS_CHARS;
    if (! wxScrolledWindow::Create(parent, id, pos, size, style, name))
        return false;

    SetXGrid(grid);

    SetFont(GetFont());
    SetColors(*wxWHITE, *wxGREEN, *wxLIGHT_GREY);

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
XGridCtrl::SetXGrid(XGrid * grid)
{
    if (grid == NULL)
    {
        m_grid = NULL;
        m_incorrectSquares = 0;
        m_blankSquares = 0;
        m_direction = DIR_ACROSS;
        m_focusedSquare = NULL;
        m_focusedStart = NULL;
        m_focusedEnd = NULL;
    }
    else
    {
        m_grid = grid;

        RecheckGrid();

        m_direction     = DIR_ACROSS;
        m_focusedSquare = m_grid->FirstWhite();
        m_focusedStart  = m_focusedSquare->GetWordStart(m_direction);
        m_focusedEnd    = m_focusedSquare->GetWordEnd  (m_direction);
    }

    Scale();
}


void
XGridCtrl::RecheckGrid()
{
    wxASSERT(m_grid != NULL);
    std::vector<XSquare *> incorrect    = m_grid->CheckGrid();
    std::vector<XSquare *> wrongOrBlank = m_grid->CheckGrid(true);

    m_incorrectSquares = incorrect.size();
    m_blankSquares     = wrongOrBlank.size() - incorrect.size();
}

//-------------------------------------------------------
// Drawing functions
//-------------------------------------------------------
void
XGridCtrl::OnPaint(wxPaintEvent & evt)
{
    // This OnPaint handler is needed so we can use an auto-buffered wxPaintDC
    // The default OnPaint handler is identical, but uses wxPaintDC instead.
    wxAutoBufferedPaintDC dc(this);
    DoPrepareDC(dc);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    DrawGrid(dc, GetUpdateRegion());

    //wxScrolledWindow::SetFocus();
}



void
XGridCtrl::DrawGrid(wxDC & dc, const wxRegion & updateRegion)
{
    if (m_grid == NULL || m_grid->IsEmpty())
    {
        wxLogDebug(_T("Grid is empty"));
        return;
    }

#ifdef __WXDEBUG__
        int _scrollX, _scrollY;
        GetScrollPixelsPerUnit(&_scrollX, &_scrollY);
        wxASSERT(_scrollX == GetSquareSize() && _scrollY == GetSquareSize());
#endif

    if (m_rect.IsEmpty() || m_boxSize == 0)
    {
        wxLogDebug(_T("Size is too small"));
        //return;
    }

    // Draw black as crossword background
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen  (*wxBLACK_PEN);

    dc.DrawRectangle(m_rect);

    XSquare * square;

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
        rect.Offset(- m_rect.GetLeft(), - m_rect.GetTop());

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
XGridCtrl::DrawSquare(wxDC & dc, const XSquare & square, const wxColour & color)
{
    // Don't draw black squares
    if (square.IsBlack())
        return;

    // Draw the square
    dc.SetBrush(wxBrush(color));
    dc.SetPen  (wxPen(color));
    wxPoint pt = TopLeft(square);
    int x = pt.x;
    int y = pt.y;
    dc.DrawRectangle(x, y, m_boxSize, m_boxSize);

    // Draw square's flag if any (top right)
    if (square.HasFlag(XFLAG_RED | XFLAG_BLACK))
    {
        if (square.HasFlag(XFLAG_RED))
        {
            dc.SetBrush(*wxRED_BRUSH);
            dc.SetPen(*wxRED_PEN);
        }
        else
        {
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
        }
        wxPoint pts[3];
        pts[0] = wxPoint(x + 2./3.*m_boxSize, y);
        pts[1] = wxPoint(x + m_boxSize - 1, y);
        pts[2] = wxPoint(x + m_boxSize - 1, y + 1./3.*m_boxSize);
        dc.DrawPolygon(3, pts);
    }

    if (square.HasFlag(XFLAG_CIRCLE))
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxBLACK, 1));
        dc.DrawCircle(x + m_boxSize/2, y + m_boxSize/2, m_boxSize/2);
    }

    dc.SetTextForeground(*wxBLACK);

    // Draw square's number if applicable (top left).
    if (square.GetNumber() != 0)
    {
        // Set a solid text background so it will draw over any circles.
        dc.SetTextBackground(color);
        dc.SetBackgroundMode(wxSOLID);

        dc.SetFont(m_numberFont);
        dc.DrawText(wxString::Format(_T("%d"), square.GetNumber()), x+1, y+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
    }

    // Draw square's text (bottom and center to avoid conflicts with numbers)
    if (! square.IsBlank())
    {
        if (square.GetText().length() > 1)
        {
            wxFont font = m_letterFont;
            font.SetPointSize(font.GetPointSize() / 2);
            dc.SetFont(font);
        }
        else
            dc.SetFont(m_letterFont);

        wxString text = square.GetText();
        int width, height;

        dc.GetTextExtent(text, &width, &height);
        if (width > m_boxSize)
        {
            while (width > m_boxSize && text.length() > 1)
            {
                text = text.substr(0, text.length() - 1);
                dc.GetTextExtent(text + _T("..."), &width, &height);
            }
            text.append(_T("..."));
        }

        if (square.HasFlag(XFLAG_PENCIL))
            dc.SetTextForeground(*wxLIGHT_GREY);
        else
            dc.SetTextForeground(*wxBLACK);

        dc.DrawText(text,
                    x + (m_boxSize - width)/2,
                    y + (m_boxSize - height));
    }

    // Draw an X across the square
    if (square.HasFlag(XFLAG_X))
    {
        dc.SetPen(wxPen(*wxRED, 2));
        // Funky math here because of the way that DCs draw lines
        dc.DrawLine(x + 1, y + 1, x + m_boxSize - 2, y + m_boxSize - 2);
        dc.DrawLine(x + m_boxSize - 2, y + 1, x + 1, y + m_boxSize - 2);
    }
}










//-------------------------------------------------------
// Focus functions
//-------------------------------------------------------
XSquare *
XGridCtrl::ChangeSquareFocus(XSquare * square, bool direction)
{
    wxASSERT(m_focusedSquare != NULL || square != NULL);

    // Save old state
    //-------------------
    XSquare * oldFocused = m_focusedSquare;
    XSquare * oldStart;
    XSquare * oldEnd;
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

    bool oldDirection = m_direction;

    // Set new state
    //-------------------
    m_direction = direction;
    if (square != NULL)
        m_focusedSquare = square;
    // else m_focusedSquare stays the same

    // If there is no word for this direction, try the other direction
    if (m_focusedSquare->GetWordStart(m_direction) == NULL)
        m_direction = ! m_direction;

    m_focusedStart  = m_focusedSquare->GetWordStart(m_direction);
    m_focusedEnd    = m_focusedSquare->GetWordEnd  (m_direction);

    wxASSERT(m_focusedStart != NULL && m_focusedEnd != NULL);

    MakeVisible(*m_focusedSquare);

    // Redraw
    wxClientDC dc(this); DoPrepareDC(dc);

    // If the old and new focused words are the same, only redraw two squares
    if (m_focusedStart == oldStart && m_focusedEnd == oldEnd)
    {
        DrawSquare(dc, *oldFocused, m_colors[WORD_SQUARE]);
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
                DrawSquare(dc, *square, m_colors[WHITE_SQUARE]);
                if (square == oldEnd)
                    break;
            }
        // Draw new word
        for (square = m_focusedStart;
             square != NULL;
             square = square->Next(m_direction))
        {
            DrawSquare(dc, *square, m_colors[WORD_SQUARE]);
            if (square == m_focusedEnd)
                break;
        }
    }

    // Draw the square with focuse
    DrawSquare(dc, *m_focusedSquare, m_colors[LETTER_SQUARE]);

    // Always keep focus here
    //wxScrolledWindow::SetFocus();

    // return whatever square now has focus
    return m_focusedSquare;
}


XSquare *
XGridCtrl::SetSquareFocus(XSquare * square, bool direction)
{
    square = ChangeSquareFocus(square, direction);

    wxPuzEvent evt(wxEVT_PUZ_GRID_FOCUS, GetId());

    // Check for across clue
    XSquare * start = square->GetWordStart(DIR_ACROSS);
    if (start == NULL)
        evt.SetAcrossClue(0);
    else
        evt.SetAcrossClue(start->GetNumber());

    // Check for down clue
    start = square->GetWordStart(DIR_DOWN);
    if (start == NULL)
        evt.SetDownClue(0);
    else
        evt.SetDownClue(start->GetNumber());

    // One of the two needs to be set
    wxASSERT(evt.GetAcrossClue() != 0 || evt.GetDownClue() != 0);

    evt.SetDirection(direction);
    evt.SetString(square->GetText());
    GetEventHandler()->ProcessEvent(evt);

    return square;
}


void
XGridCtrl::ChangeFocusedClue(int cluenum, bool direction)
{
    XSquare * square = GetClueNumber(cluenum);
    if (square == NULL)
        return;

    if (HasStyle(BLANK_ON_NEW_WORD))
        SetIfExists(square,
                    FindSquare(square->GetWordStart(direction),
                               FIND_BLANK_SQUARE,
                               direction) );

    ChangeSquareFocus(square, direction);
}


void
XGridCtrl::MakeVisible(const XSquare & square)
{
    if (m_fit || m_grid == NULL || m_grid->IsEmpty())
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



void
XGridCtrl::SetSquareText(XSquare & square, const wxString & text)
{
    // Adjust blank and incorrect counts each time a letter is changed
    // The logic is a little confusing at first, but it's correct
    const int correctBefore = square.Check();
    const int blankBefore   = square.IsBlank();

    square.SetText(text);

    const int correctAfter = square.Check();
    const int blankAfter   = square.IsBlank();

    m_blankSquares     += blankAfter   - blankBefore;
    m_incorrectSquares -= correctAfter - correctBefore;

    square.SetText(text);

    wxPuzEvent evt(wxEVT_PUZ_LETTER, GetId());
    evt.SetString(text);
    GetEventHandler()->ProcessEvent(evt);
}




//-------------------------------------------------------
// Scaling
//-------------------------------------------------------
void
XGridCtrl::ScaleFont(wxFont * font, double desiredHeight)
{
    int height;
    GetTextExtent(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"), NULL, &height, NULL, NULL, font);
    // First "guess" should speed things up
    font->SetPointSize(font->GetPointSize() * desiredHeight / height);

    GetTextExtent(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"), NULL, &height, NULL, NULL, font);
    while (height < desiredHeight && font->GetPointSize() < MAX_POINT_SIZE) {
        font->SetPointSize(font->GetPointSize()+1);
        GetTextExtent(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"), NULL, &height, NULL, NULL, font);
    }
    while (height > desiredHeight && font->GetPointSize() > MIN_POINT_SIZE) {
        font->SetPointSize(font->GetPointSize()-1);
        GetTextExtent(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"), NULL, &height, NULL, NULL, font);
    }

    // This happens if desiredHeight is very small or zero
    if (font->GetPointSize() < MIN_POINT_SIZE)
        font->SetPointSize(MIN_POINT_SIZE);
}



wxSize
XGridCtrl::GetBestSize() const
{
    if (! m_grid || m_grid->IsEmpty())
        return wxDefaultSize;

    return wxSize( m_grid->GetWidth()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize,

                   m_grid->GetHeight()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize );
}


void
XGridCtrl::OnSize(wxSizeEvent & evt)
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
    if (m_grid == NULL || m_grid->IsEmpty())
    {
        m_rect.SetWidth(0);
        m_rect.SetHeight(0);
        return;
    }

    int max_width, max_height;
    GetClientSize(&max_width, &max_height);

    // Recalculate box size
    if (! m_fit)
    {
        // This should only occur when FitGrid(false) is called
        if (factor == 1.0 && m_lastBoxSize != UNDEFINED_BOX_SIZE)
            m_boxSize = m_lastBoxSize;
        else
            m_boxSize *= factor;

        m_lastBoxSize = m_boxSize;
    }
    else // fit == true
    {
        // If the window is too small to fit the grid, catch it here
        if (max_width  < (m_grid->GetWidth()  + 1)  * m_borderSize
         || max_height < (m_grid->GetHeight() + 1) * m_borderSize)
        {
            m_boxSize = 0;
        }
        else
        {
            const size_t width =
                (max_width  - (m_grid->GetWidth() + 1)  * m_borderSize)
                / m_grid->GetWidth();

            const size_t height =
                (max_height - (m_grid->GetHeight() + 1) * m_borderSize)
                / m_grid->GetHeight();

            m_boxSize = std::min(width, height);
        }
    }

    // Set grid rect size based on m_boxSize
    m_rect.SetWidth ( m_grid->GetWidth()
                      * (m_boxSize + m_borderSize)
                      + m_borderSize );

    m_rect.SetHeight( m_grid->GetHeight()
                      * (m_boxSize + m_borderSize)
                      + m_borderSize );

    wxASSERT(! m_fit
             || m_boxSize == 0
             || (m_rect.width <= max_width && m_rect.height <= max_height) );

    // Set left side of grid
    if (m_fit || m_rect.width <= max_width)
        m_rect.SetX( (max_width  - m_rect.width)  / 2 );
    else
        m_rect.SetX(0);

    // Set top of grid
    if (m_fit || m_rect.height <= max_height)
        m_rect.SetY( (max_height - m_rect.height) / 2 );
    else
        m_rect.SetY(0);

    // Scale fonts
    ScaleFont(&m_letterFont, m_boxSize * LETTER_SCALE);
    ScaleFont(&m_numberFont, m_boxSize * NUMBER_SCALE);

    // Virtual size does not include the bottom and right grid border.
    // If it did include the border, there would often be an extra scroll unit
    // because the scroll unit is equal to one square plus _one_ border, not
    // both borders.
    SetVirtualSize(m_rect.width  - m_borderSize, m_rect.height - m_borderSize);
    SetScrollRate(GetSquareSize(), GetSquareSize());
}




//-------------------------------------------------------
// Check/Reveal functions
//-------------------------------------------------------
void
XGridCtrl::CheckGrid(int options)
{
    wxASSERT(m_grid != NULL && ! m_grid->IsScrambled());

    std::vector<XSquare *> incorrect = \
        m_grid->CheckGrid( (options & CHECK_ALL) != 0 );

    if (incorrect.empty() && (options & NO_MESSAGE_BOX) == 0)
    {
        wxMessageBox(_T("No Incorrect Letters!"), _T("Message"));
        return;
    }

    wxClientDC dc(this); DoPrepareDC(dc);
    for (std::vector<XSquare *>::iterator it = incorrect.begin();
         it != incorrect.end();
         ++it)
    {
        XSquare * square = *it;

        if ( (options & REVEAL_ANSWER) != 0)
        {
            square->SetText(square->GetSolution());
            square->RemoveFlag(XFLAG_BLACK | XFLAG_X);
            square->AddFlag(XFLAG_RED);
        }
        else
        {
            square->RemoveFlag(XFLAG_BLACK);
            square->AddFlag(XFLAG_X);
        }
        RefreshSquare(dc, *square);
    }
}


void
XGridCtrl::CheckWord(int options)
{
    wxASSERT(m_grid != NULL && ! m_grid->IsScrambled());

    std::vector<XSquare *> incorrect =
        m_grid->CheckWord( m_focusedSquare->GetWordStart(m_direction),
                           m_focusedSquare->GetWordEnd  (m_direction) );

    if (incorrect.empty() && (options & NO_MESSAGE_BOX) == 0)
    {
        wxMessageBox(_T("No Incorrect Letters!"), _T("Message"));
        return;
    }

    wxClientDC dc(this); DoPrepareDC(dc);
    for (std::vector<XSquare *>::iterator it = incorrect.begin();
         it != incorrect.end();
         ++it)
    {
        XSquare * square = *it;

        if ( (options & REVEAL_ANSWER) != 0)
        {
            square->SetText(square->GetSolution());
            square->RemoveFlag(XFLAG_BLACK | XFLAG_X);
            square->AddFlag(XFLAG_RED);
        }
        else
        {
            square->RemoveFlag(XFLAG_BLACK);
            square->AddFlag(XFLAG_X);
        }
        RefreshSquare(dc, *square);
    }
}



void
XGridCtrl::CheckLetter(int options)
{
    wxASSERT(m_grid != NULL && ! m_grid->IsScrambled());

    XSquare & square = *GetFocusedSquare();

    if (! m_grid->CheckSquare(square))
    {
        if ( (options & REVEAL_ANSWER) != 0)
        {
            square.SetText(square.GetSolution());
            square.RemoveFlag(XFLAG_BLACK | XFLAG_X);
            square.AddFlag(XFLAG_RED);
        }
        else
        {
            square.RemoveFlag(XFLAG_BLACK);
            square.AddFlag(XFLAG_X);
        }
        RefreshSquare(*m_focusedSquare);
    }
    else if ( (options & NO_MESSAGE_BOX) == 0)
        wxMessageBox(_T("No Incorrect Letters!"), _T("Message"));
}







//-------------------------------------------------------------
// Movement events
//-------------------------------------------------------------
void
XGridCtrl::OnLeftDown(wxMouseEvent & evt)
{
    if (! m_grid || m_grid->IsEmpty())
    {
        evt.Skip();
        return;
    }

    wxPoint pt = evt.GetPosition();
    XSquare * square = HitTest(pt.x, pt.y);
    if (square != NULL && square->IsWhite())
        SetSquareFocus(square, m_direction);

    // MAKE SURE THIS SKIPS OR WE DON'T GET KEYBOARD FOCUS
    evt.Skip();
}

void
XGridCtrl::OnRightDown(wxMouseEvent & evt)
{
    if (! m_grid || m_grid->IsEmpty())
        return;

    if (HasStyle(CONTEXT_MENU))
        return;

    XSquare * square = GetFocusedSquare();
    if (HasStyle(MOVE_ON_RIGHT_CLICK))
    {
        wxPoint pt = evt.GetPosition();
        square = HitTest(pt.x, pt.y);
    }
    if (square != NULL && square->IsWhite())
        SetSquareFocus(square, !m_direction);

    // Kill the event
}


void
XGridCtrl::OnKeyDown(wxKeyEvent & evt)
{
    if (! m_grid || m_grid->IsEmpty())
    {
        evt.Skip();
        return;
    }

    int key = evt.GetKeyCode();
    int mod = evt.GetModifiers();

    if      (key == WXK_LEFT  || key == WXK_NUMPAD_LEFT)     /* Left      */
        OnArrow(DIR_ACROSS, FIND_PREV, mod);
    else if (key == WXK_RIGHT || key == WXK_NUMPAD_RIGHT)    /* Right     */
        OnArrow(DIR_ACROSS, FIND_NEXT, mod);
    else if (key == WXK_UP    || key == WXK_NUMPAD_UP)       /* Up        */
        OnArrow(DIR_DOWN,   FIND_PREV, mod);
    else if (key == WXK_DOWN  || key == WXK_NUMPAD_DOWN)     /* Down      */
        OnArrow(DIR_DOWN,   FIND_NEXT, mod);
    else if (key == WXK_HOME  || key == WXK_NUMPAD_HOME)     /* Home      */
        OnHome(mod);
    else if (key == WXK_END   || key == WXK_NUMPAD_END)      /* End       */
        OnEnd(mod);
    else if (key == WXK_TAB   || key == WXK_NUMPAD_TAB       /* Tab       */
          || key == WXK_RETURN || key == WXK_NUMPAD_ENTER)   /* Enter     */
        OnTab(mod);
    else if (65 <= key && key <= 90 || key == WXK_SPACE)     /* Letter    */
        OnLetter(evt.GetUnicodeKey(), mod);
    else if (key == WXK_BACK)                                /* Backspace */
        OnBackspace(mod);
    else if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)  /* Delete    */
        OnDelete(mod);
}



void
XGridCtrl::OnLetter(wxChar key, int mod)
{
    wxASSERT(m_focusedSquare->GetWordStart(m_direction)->GetNumber() != 0);

    if (! (mod == wxMOD_NONE || mod == wxMOD_SHIFT))
        return;

    XSquare & square = *GetFocusedSquare();

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite revealed letters
    if (! square.HasFlag(XFLAG_RED))
    {
        if (static_cast<int>(key) == WXK_SPACE)
            SetSquareText(square, _T("-"));
        else
            SetSquareText(square, key);
    }

    XSquare * newSquare = NULL;
    if (HasStyle(CHECK_WHILE_TYPING))
        CheckLetter(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);

    if (HasStyle(MOVE_AFTER_LETTER))
    {
        if (HasStyle(MOVE_TO_NEXT_BLANK))
        {
            newSquare = FindSquare(m_focusedSquare,
                                   FIND_BLANK_SQUARE,
                                   m_direction,
                                   FIND_NEXT);
        }
        else
        {
            newSquare = FindNextSquare(m_focusedSquare,
                                       FIND_WHITE_SQUARE,
                                       m_direction,
                                       FIND_NEXT);
        }
    }
    SetSquareFocus(newSquare, m_direction);
}



void
XGridCtrl::OnArrow(bool arrowDirection, bool increment, int mod)
{
    XSquare * newSquare = NULL;

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
                               FIND_WORD(arrowDirection, m_focusedSquare),
                               arrowDirection,
                               increment,
                               SKIP_BLACK_SQUARES,
                               WRAP_LINES);

            if (newSquare != NULL)
                newSquare = newSquare->GetWordStart(arrowDirection);
        }
        else
        {
            newSquare =
                FindNextSquare(m_focusedSquare,
                               FIND_WHITE_SQUARE,
                               arrowDirection,
                               increment,
                               SKIP_BLACK_SQUARES);
        }

        // Test BLANK_ON_NEW_WORD
        const bool isNewWord = newSquare != NULL
            && newSquare->      GetWordStart(m_direction) !=
               m_focusedSquare->GetWordStart(m_direction);

        if (HasStyle(BLANK_ON_NEW_WORD) && isNewWord)
        {
            SetIfExists(newSquare,
                FindSquare(newSquare->GetWordStart(m_direction),
                           FIND_BLANK_SQUARE,
                           m_direction,
                           FIND_NEXT));
        }
    }
    else {  // Shift is not pressed
        if (m_direction != arrowDirection
            && HasStyle(PAUSE_ON_SWITCH)
            && m_focusedSquare->IsBlank())
        {
            SetSquareFocus(m_focusedSquare, ! m_direction);
            return;
        }
        else
        {
            newSquare =
                FindNextSquare(m_focusedSquare,
                               FIND_WHITE_SQUARE,
                               arrowDirection,
                               increment,
                               SKIP_BLACK_SQUARES);
        }
    }

    SetSquareFocus(newSquare,
        (mod == wxMOD_SHIFT ? m_direction : arrowDirection));
}



void
XGridCtrl::OnBackspace(int mod)
{
    XSquare & square = *m_focusedSquare;

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite reveal letters
    if (! square.HasFlag(XFLAG_RED))
            SetSquareText(square, _T("-"));

    SetSquareFocus(FindNextSquare(m_focusedSquare,
                                  FIND_WHITE_SQUARE,
                                  m_direction,
                                  FIND_PREV),
                   m_direction);
}



void
XGridCtrl::OnDelete(int mod)
{
    XSquare & square = *m_focusedSquare;

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite reveal letters
    if (! square.HasFlag(XFLAG_RED))
        SetSquareText(square, _T("-"));

    SetSquareFocus(m_focusedSquare, m_direction);
}



void
XGridCtrl::OnHome(int mod)
{
    // Shift key is used in Across Lite
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
    {
        XSquare * newSquare = m_grid->FirstWhite();
        if (HasStyle(BLANK_ON_NEW_WORD))
        {
            SetSquareFocus( EitherOr(newSquare,
                                     FindSquare(newSquare,
                                                FIND_BLANK_SQUARE,
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
    // Shift key is used in Across Lite
    if (mod == wxMOD_CMD || mod == wxMOD_SHIFT)
        SetSquareFocus(m_grid->LastWhite(), m_direction);
    else
        SetSquareFocus(m_focusedSquare->GetWordEnd(m_direction), m_direction);
}




void
XGridCtrl::OnTab(int mod)
{
    XSquare * newSquare = NULL;
    bool newDir = m_direction;
    if (mod == wxMOD_SHIFT)
    {
        newSquare =
            FindNextSquare(m_focusedStart,
                           FIND_CLUE(m_direction, m_focusedStart),
                           DIR_ACROSS,
                           FIND_PREV,
                           SKIP_BLACK_SQUARES,
                           WRAP_LINES);

        if (newSquare == NULL) // We're at the start of the grid
        {
            // Find the last clue (different depending on direction)
            newDir = !newDir;
            newSquare =
                FindNextSquare(m_grid->LastWhite(),
                               FIND_CLUE(newDir, m_grid->FirstWhite()),
                               DIR_ACROSS,
                               FIND_PREV,
                               SKIP_BLACK_SQUARES,
                               WRAP_LINES);
        }
    }
    else
    {
        newSquare =
            FindNextSquare(m_focusedStart,
                           FIND_CLUE(m_direction, m_focusedStart),
                           DIR_ACROSS,
                           FIND_NEXT,
                           SKIP_BLACK_SQUARES,
                           WRAP_LINES);

        if (newSquare == NULL) // We're at the end of the grid
        {
            newDir = !newDir;
            newSquare = m_grid->FirstWhite();
        }
    }
    newSquare = newSquare->GetWordStart(newDir);

    const bool isNewWord =
        newSquare->GetWordStart(newDir)
            != m_focusedSquare->GetWordStart(m_direction);

    if (HasStyle(BLANK_ON_NEW_WORD) &&  isNewWord)
        SetIfExists( newSquare,
                     FindSquare(newSquare,
                                FIND_BLANK_SQUARE,
                                newDir,
                                FIND_NEXT) );

    SetSquareFocus(newSquare, newDir);
}






//-------------------------------------------------------
// Information functions
//-------------------------------------------------------
XSquare *
XGridCtrl::HitTest(int x, int y)
{
    CalcUnscrolledPosition(x, y, &x, &y);
    x -= (m_rect.x + m_borderSize);
    y -= (m_rect.y + m_borderSize);
    int col = floor((double)x / (m_boxSize+m_borderSize));
    int row = floor((double)y / (m_boxSize+m_borderSize));

    if (   0 <= col && col < m_grid->GetWidth()
        && 0 <= row && row < m_grid->GetHeight())
    {
        return &At(col, row);
    }
    return NULL;
}


const wxColor &
XGridCtrl::GetSquareColor(const XSquare & square)
{
    if (IsFocusedLetter(square))
        return m_colors[LETTER_SQUARE];
    else if (IsFocusedWord(square))
        return m_colors[WORD_SQUARE];
    else
        return m_colors[WHITE_SQUARE];
}



bool
XGridCtrl::IsFocusedWord(const XSquare & square)
{
    return m_grid->IsBetween(&square, m_focusedStart, m_focusedEnd);
}



XSquare *
XGridCtrl::GetClueNumber(int num)
{
    wxASSERT(m_grid != NULL);
    wxASSERT(m_grid->FirstWhite() != NULL);
    for (XSquare * square = m_grid->FirstWhite();
         square != NULL;
         square = square->Next())
    {
        if (square->GetNumber() == num)
            return square;
    }
    return NULL;
}
