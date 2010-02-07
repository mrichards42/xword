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


// This class will take over the XGridCtrl's event processing
// when needed.  It must be created on the heap, and it will
// destroy itself when it is done.
class XGridRebusHandler : wxEvtHandler
{
public:
    XGridRebusHandler(XGridCtrl & grid)
        : m_grid(grid)
    {
        m_grid.PushEventHandler(this);
        // Kill the grid's own event handling
        m_grid.Disconnect(wxEVT_KEY_DOWN, wxKeyEventHandler(XGridCtrl::OnKeyDown));
        m_grid.Disconnect(wxEVT_CHAR,     wxKeyEventHandler(XGridCtrl::OnChar));
    }

    void EndEventHandling()
    {
        m_grid.RemoveEventHandler(this);
        // Restore the grid's own key handling
        m_grid.Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(XGridCtrl::OnKeyDown));
        m_grid.Connect(wxEVT_CHAR,     wxKeyEventHandler(XGridCtrl::OnChar));
    }

    DECLARE_EVENT_TABLE()

private:
    XGridCtrl & m_grid;

    // The event handling
    void OnKeyDown(wxKeyEvent & evt);
    void OnChar   (wxKeyEvent & evt);
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
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(XGridCtrl, wxScrolledWindow)

const int MAX_POINT_SIZE = 150;
const int MIN_POINT_SIZE = 2;

const int UNDEFINED_BOX_SIZE = -1;

// Helper functions for all of the "lookup" functions that return NULL
inline void
SetIfExists(XSquare * &current, XSquare * test)
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
    m_grid = NULL;

    m_focusedSquare = NULL;
    m_focusedStart = NULL;
    m_focusedEnd = NULL;

    m_direction = DIR_ACROSS;

    m_fit = true;
    m_boxSize = 20;
    m_borderSize = 1;
    m_lastBoxSize = UNDEFINED_BOX_SIZE;

    // This is a good default scale.
    // Allowing the square number to take up 5/12 of the square and the text
    // to take up 9/12 (=3/4) should introduce some overlap, but the character
    // height of a font seems to be larger than the actual text size (e.g.
    // GetCharHeight may report a height of 28 px when the letter itself only
    // takes up 24 px).
    m_numberScale = 42 / 100.;
    m_letterScale = 75 / 100.;

    m_incorrectSquares = 0;
    m_blankSquares = 0;

    m_wantsRebus = false;

    // m_rect is already equal to wxRect(0,0,0,0) from its constructor
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

    SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    SetFocusedLetterColor(*wxGREEN);
    SetFocusedWordColor(*wxLIGHT_GREY);
    SetWhiteSquareColor(*wxWHITE);
    SetPenColor(*wxBLACK);
    SetPencilColor(wxColor(200,200,200));

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

        DisconnectEvents();
    }
    else
    {
        m_grid = grid;

        RecheckGrid();

        m_direction     = DIR_ACROSS;
        m_focusedSquare = m_grid->FirstWhite();
        m_focusedStart  = m_focusedSquare->GetWordStart(m_direction);
        m_focusedEnd    = m_focusedSquare->GetWordEnd  (m_direction);

        ConnectEvents();
    }

    Scale();
}


void
XGridCtrl::RecheckGrid()
{
    wxASSERT(! IsEmpty());
    std::vector<XSquare *> incorrect    = m_grid->CheckGrid();
    std::vector<XSquare *> wrongOrBlank = m_grid->CheckGrid(true);

    m_incorrectSquares = incorrect.size();
    m_blankSquares     = wrongOrBlank.size() - incorrect.size();
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
    if (IsEmpty())
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
    dc.SetBrush(wxBrush(GetBlackSquareColor()));
    dc.SetPen  (wxPen(GetBlackSquareColor()));

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

    // The order of drawing is important to make sure that we don't draw over
    // the more important parts of a square:
    // 1. The background
    // 2. An incorrect / revealed indicator (the little triangle in the corner)
    // 3. A circle
    // 4. The number (with an opaque background so it draws over the circle)
    // 5. The text
    // 6. An X over everything if necessary

    // If the user has pressed <insert> we are in rebus mode.  The focused square
    // will be outlined in the selected color (and have a white background).
    // Certain features will not be drawn in this case:
    //    - The number
    //    - The incorrect/revealed indicator
    //    - The X (if applicable).


    const bool drawOutline = m_wantsRebus && IsFocusedLetter(square);
    // Use this color for the background.  If we are looking for a rebus
    // entry, and this is the focused square, make the background white
    // and draw a border around the square.
    const wxColor bgcolor =
        (drawOutline ? GetWhiteSquareColor() : color);

    wxPoint pt = TopLeft(square);
    int x = pt.x;
    int y = pt.y;

    // Draw the square background
    if (drawOutline)
    {
        int outlineSize = m_boxSize / 15;
        if (outlineSize < 1)
            outlineSize = 1;

        wxPen outlinePen(color, outlineSize * 2);
        outlinePen.SetJoin(wxJOIN_MITER);

        dc.SetPen(outlinePen);
        dc.SetBrush(wxBrush(bgcolor));
        dc.DrawRectangle(x + outlineSize, y + outlineSize,
                         m_boxSize - outlineSize * 2 + 1, m_boxSize - outlineSize * 2 + 1);

    }
    else
    {
        dc.SetBrush(wxBrush(bgcolor));
        dc.SetPen  (wxPen(bgcolor));

        dc.DrawRectangle(x, y, m_boxSize, m_boxSize);
    }

    // Draw square's flag if any (top right)
    if (square.HasFlag(XFLAG_RED | XFLAG_BLACK) && ! drawOutline)
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

    dc.SetTextForeground(GetPenColor());

    // Draw square's number if applicable (top left).
    if (square.GetNumber() != 0 && ! drawOutline)
    {
        // Set a solid text background so it will draw over any circles.
        dc.SetTextBackground(bgcolor);
        dc.SetBackgroundMode(wxSOLID);

        dc.SetFont(m_numberFont);
        dc.DrawText(wxString::Format(_T("%d"), square.GetNumber()), x+1, y);
        dc.SetBackgroundMode(wxTRANSPARENT);
    }

    // Draw square's text (bottom and center to avoid conflicts with numbers)
    if (! square.IsBlank())
    {
        wxString text = square.GetText();

        if (text.length() > 4)
        {
            const int len = (text.length() + 1) / 2;
            text.insert(len, _T("\n"));

            dc.SetFont(m_letterFont[3]);
        }
        else
            dc.SetFont(m_letterFont[text.length() - 1]);

        if (square.HasFlag(XFLAG_PENCIL))
            dc.SetTextForeground(GetPencilColor());
        else
            dc.SetTextForeground(GetPenColor());

        if (text.length() == 1)
        {
            int width, height;
            dc.GetTextExtent(text, &width, &height);
            dc.DrawText(text,
                        x + (m_boxSize - width)/2,
                        y + (m_boxSize - height));
        }
        else
        {
            dc.DrawLabel(text, wxRect(x, y + m_boxSize - GetLetterHeight(),
                                      m_boxSize, GetLetterHeight()), wxALIGN_CENTER);
        }
    }

    // Draw an X across the square
    if (square.HasFlag(XFLAG_X) && ! drawOutline)
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



void
XGridCtrl::SetSquareText(XSquare & square, const wxString & text)
{
    if (m_wantsRebus)
    {
        square.SetText(text);
        return;
    }
    // Adjust blank and incorrect counts each time a letter is changed
    // The logic is a little confusing at first, but it's correct
    const int correctBefore = square.Check();
    const int blankBefore   = square.IsBlank();

    square.SetText(text);

    const int correctAfter = square.Check();
    const int blankAfter   = square.IsBlank();

    m_blankSquares     += blankAfter   - blankBefore;
    m_incorrectSquares -= correctAfter - correctBefore;

    if (HasStyle(CHECK_WHILE_TYPING))
        CheckLetter(NO_REVEAL_ANSWER | NO_MESSAGE_BOX);

    wxPuzEvent evt(wxEVT_PUZ_LETTER, GetId());
    evt.SetString(text);
    GetEventHandler()->ProcessEvent(evt);
}




//-------------------------------------------------------
// Scaling
//-------------------------------------------------------
double DoScale(int width, int height, int maxWidth, int maxHeight)
{
    if      (maxWidth == -1)
        return static_cast<double>(maxHeight) / static_cast<double>(height);
    else if (maxHeight == -1)
        return static_cast<double>(maxWidth) / static_cast<double>(width);

    return std::min( static_cast<double>(maxWidth)  / static_cast<double>(width),
                     static_cast<double>(maxHeight) / static_cast<double>(height) );
}

void
XGridCtrl::ScaleFont(wxFont * font, int maxWidth, int maxHeight)
{
    // Don't test numbers and symbols because they're probably not as wide.
    const wxString text = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // Adjust width to fit the test string
    if (maxWidth != -1)
        maxWidth *= text.length();

    int h, w;
    GetTextExtent(text, &w, &h, NULL, NULL, font);

    // First "guess" should speed things up
    font->SetPointSize(font->GetPointSize() * DoScale(w, h, maxWidth, maxHeight));

    // Make the font larger if needed.
    GetTextExtent(text, &w, &h, NULL, NULL, font);
    while ( (maxHeight == -1 ? true : h < maxHeight) &&
            (maxWidth  == -1 ? true : w < maxWidth) &&
            font->GetPointSize() < MAX_POINT_SIZE )
    {
        font->SetPointSize(font->GetPointSize()+1);
        GetTextExtent(text, &w, &h, NULL, NULL, font);
    }

    // Make the font smaller if needed.
    while ( (maxHeight == -1 ? true : h > maxHeight) &&
            (maxWidth  == -1 ? true : w > maxWidth) &&
            font->GetPointSize() > MIN_POINT_SIZE )
    {
        font->SetPointSize(font->GetPointSize()-1);
        GetTextExtent(text, &w, &h, NULL, NULL, font);
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
    SetNumberFont(m_numberFont);
    SetLetterFont(m_letterFont[0]);

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
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

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
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

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
    wxASSERT(! IsEmpty() && ! m_grid->IsScrambled());

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
// Events
//-------------------------------------------------------------

void
XGridCtrl::ConnectEvents()
{
    Connect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    Connect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
    Connect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
}


void
XGridCtrl::DisconnectEvents()
{
    Disconnect(wxEVT_KEY_DOWN,   wxKeyEventHandler  (XGridCtrl::OnKeyDown));
    Disconnect(wxEVT_CHAR,       wxKeyEventHandler  (XGridCtrl::OnChar));
    Disconnect(wxEVT_LEFT_DOWN,  wxMouseEventHandler(XGridCtrl::OnLeftDown));
    Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(XGridCtrl::OnRightDown));
}



void
XGridCtrl::OnLeftDown(wxMouseEvent & evt)
{
    wxASSERT(! IsEmpty());

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
    wxASSERT(! IsEmpty());

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
    wxLogDebug(_T("OnKeyDown at XGridCtrl"));
    wxASSERT(! IsEmpty());

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();

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

    evt.Skip();
}


// This is for letters only
void
XGridCtrl::OnChar(wxKeyEvent & evt)
{
    wxASSERT(! IsEmpty());

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();

    if (XSquare::IsValidChar(static_cast<wxChar>(key)))
        OnLetter(wxToupper(key), mod);
    else if (key == WXK_SPACE && ! m_wantsRebus)
        OnLetter(key, mod);
}



void
XGridCtrl::OnLetter(wxChar key, int mod)
{
    wxASSERT(! IsEmpty());
    wxASSERT(m_focusedSquare->GetWordStart(m_direction)->GetNumber() != 0);

    if (! (mod == wxMOD_NONE || mod == wxMOD_SHIFT))
        return;

    XSquare & square = *GetFocusedSquare();
    wxASSERT(! m_wantsRebus);

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite revealed letters
    if (! square.HasFlag(XFLAG_RED))
    {
        if (static_cast<int>(key) == WXK_SPACE)
            SetSquareText(square, _T(""));
        else
            SetSquareText(square, key);
    }

    MoveAfterLetter();
}


void
XGridCtrl::MoveAfterLetter()
{
    XSquare * newSquare = NULL;

    if (HasStyle(MOVE_AFTER_LETTER))
    {
        if (HasStyle(MOVE_TO_NEXT_BLANK))
        {
            newSquare = FindNextSquare(m_focusedSquare,
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
    wxASSERT(! IsEmpty());
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
        const bool isNewWord = newSquare != NULL &&
               newSquare->      GetWordStart(m_direction) !=
               m_focusedSquare->GetWordStart(m_direction);

        if (HasStyle(BLANK_ON_NEW_WORD) && isNewWord)
        {
            SetIfExists(newSquare,
                FindSquare(newSquare->GetWordStart(m_direction),
                           FIND_BLANK_SQUARE,
                           m_direction,
                           FIND_NEXT));
        }

        SetSquareFocus(newSquare, m_direction);
    }
    else // Shift is not pressed
    {
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

        SetSquareFocus(newSquare, arrowDirection);
    }
}



void
XGridCtrl::OnBackspace(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    XSquare & square = *m_focusedSquare;

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite reveal letters
    if (! square.HasFlag(XFLAG_RED))
            SetSquareText(square, _T(""));

    SetSquareFocus(FindNextSquare(m_focusedSquare,
                                  FIND_WHITE_SQUARE,
                                  m_direction,
                                  FIND_PREV),
                   m_direction);
}



void
XGridCtrl::OnDelete(int WXUNUSED(mod))
{
    wxASSERT(! IsEmpty());
    XSquare & square = *m_focusedSquare;

    if (square.HasFlag(XFLAG_X))
        square.ReplaceFlag(XFLAG_X, XFLAG_BLACK);

    // Not allowed to overwrite reveal letters
    if (! square.HasFlag(XFLAG_RED))
        SetSquareText(square, _T(""));

    SetSquareFocus(m_focusedSquare, m_direction);
}



void
XGridCtrl::OnHome(int mod)
{
    wxASSERT(! IsEmpty());
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
    wxASSERT(! IsEmpty());
    // Shift key is used in Across Lite
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

        // This will handle events from now until it is destroyed.
        // It will destroy itself when it is done.
        new XGridRebusHandler(*this);
    }
    else
    {
        m_wantsRebus = false;
        // Force checking the square since this had been prevented previously
        SetSquareText(*m_focusedSquare, m_focusedSquare->GetText());

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
    int col = floor(static_cast<double>(x) / (m_boxSize+m_borderSize));
    int row = floor(static_cast<double>(y) / (m_boxSize+m_borderSize));

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
        return m_colors[LETTER];
    else if (IsFocusedWord(square))
        return m_colors[WORD];
    else
        return m_colors[WHITE];
}



bool
XGridCtrl::IsFocusedWord(const XSquare & square)
{
    return m_grid->IsBetween(&square, m_focusedStart, m_focusedEnd);
}



XSquare *
XGridCtrl::GetClueNumber(int num)
{
    wxASSERT(! IsEmpty());
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










//------------------------------------------------------------------------------
// XGridRebusHandler implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(XGridRebusHandler, wxEvtHandler)
    EVT_KEY_DOWN    (XGridRebusHandler::OnKeyDown)
    EVT_CHAR        (XGridRebusHandler::OnChar)
END_EVENT_TABLE()

void
XGridRebusHandler::OnKeyDown(wxKeyEvent & evt)
{
    evt.Skip();
    wxLogDebug(_T("KeyDown at XGridRebusHandler"));

    const int key = evt.GetKeyCode();
    const int mod = evt.GetModifiers();
    if      (key == WXK_INSERT || key == WXK_NUMPAD_INSERT ||
             key == WXK_RETURN || key == WXK_NUMPAD_ENTER ||
             key == WXK_ESCAPE)
    {
        evt.Skip(false);
        EndEventHandling();
        m_grid.OnInsert(wxMOD_NONE);
        delete this;
    }
    else if (key == WXK_BACK)
    {
        XSquare * square = m_grid.GetFocusedSquare();
        if (square->IsBlank())
            return;
        if ((mod & (wxMOD_CONTROL | wxMOD_SHIFT)) != 0)
            square->SetText(_T(""));
        else
            square->SetText(square->GetText().substr(0, square->GetText().length() - 1));
        m_grid.RefreshSquare();
    }
    else if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        XSquare * square = m_grid.GetFocusedSquare();
        if (square->IsBlank())
            return;
        if ((mod & (wxMOD_CONTROL | wxMOD_SHIFT)) != 0)
            square->SetText(_T(""));
        else
            square->SetText(square->GetText().substr(1));
        m_grid.RefreshSquare();
    }

}


void
XGridRebusHandler::OnChar(wxKeyEvent & evt)
{
    evt.Skip();

    const wxChar key = static_cast<wxChar>(wxToupper(evt.GetKeyCode()));

    if (! XSquare::IsValidChar(key))
        return;

    XSquare * square = m_grid.GetFocusedSquare();
    if (square->GetText().length() == 8) // Max of 8 characters
        return;

    if (square->IsBlank())
        square->SetText(key);
    else
        square->SetText(square->GetText() + key);
    m_grid.RefreshSquare();
}