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

#include "XGridDrawer.hpp"
#include "puz/XGrid.hpp"
#include "puz/XSquare.hpp"

const int MAX_POINT_SIZE = 150;
const int MIN_POINT_SIZE = 2;

XGridDrawer::XGridDrawer(XGrid * grid)
    : m_grid(grid),
      m_dc(NULL),
      m_status(INVALID_MEASURER)
{
    Init();
}

XGridDrawer::XGridDrawer(wxDC * dc, XGrid * grid)
    : m_grid(grid)
{
    SetDC(dc);
    Init();
}

XGridDrawer::XGridDrawer(wxWindow * window, XGrid * grid)
    : m_grid(grid)
{
    SetWindow(window);
    Init();
}

void
XGridDrawer::Init()
{
    m_boxSize = 20;
    m_borderSize = 1;

    // This is a good default scale.
    // Allowing the square number to take up 5/12 of the square and the text
    // to take up 9/12 (=3/4) should introduce some overlap, but the character
    // height of a font seems to be larger than the actual text size (e.g.
    // GetCharHeight may report a height of 28 px when the letter itself only
    // takes up 24 px).
    m_numberScale = 5 / 12.;
    m_letterScale = 9 / 12.;

    SetNumberFont(*wxSWISS_FONT);
    SetLetterFont(*wxSWISS_FONT);

    SetWhiteSquareColor(*wxWHITE);
    SetBlackSquareColor(*wxBLACK);
    SetPenColor(*wxBLACK);

    m_drawOptions = DRAW_ALL;

    UpdateGridSize();
}

//-----------------------------------------------------------------------------
// Measuring
//-----------------------------------------------------------------------------
void
XGridDrawer::SetDC(wxDC * dc)
{
    m_dc = dc;
    if (dc != NULL)
        m_status = HAS_DC;
    else
        m_status = INVALID_MEASURER;
}

void
XGridDrawer::SetWindow(wxWindow * window)
{
    m_window = window;
    if (window != NULL)
        m_status = HAS_WINDOW;
    else
        m_status = INVALID_MEASURER;
}

void
XGridDrawer::GetTextExtent(const wxString & string, int* w, int* h,
                           int* descent, int* externalLeading,
                           wxFont* font) const
{
    wxASSERT(HasMeasurer());
    if (HasDC())
        m_dc->GetTextExtent(string, w, h, descent, externalLeading, font);
    if (HasWindow())
        m_window->GetTextExtent(string, w, h, descent, externalLeading, font);
}


//-----------------------------------------------------------------------------
// Grid / Box / Border sizes
//-----------------------------------------------------------------------------
wxSize
XGridDrawer::GetBestSize() const
{
    if (m_grid == NULL || m_grid->IsEmpty())
        return wxDefaultSize;

    return wxSize( m_grid->GetWidth()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize,

                   m_grid->GetHeight()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize );
}


void
XGridDrawer::UpdateGridSize()
{
    if (m_grid == NULL)
        return;
    // Update width and height of grid
    m_rect.SetWidth ( m_grid->GetWidth()
                      * (m_boxSize + m_borderSize)
                      + m_borderSize );

    m_rect.SetHeight( m_grid->GetHeight()
                      * (m_boxSize + m_borderSize)
                      + m_borderSize );

    // Update left side of grid
    if ((m_align & wxALIGN_RIGHT) != 0)
        m_rect.SetX(m_maxWidth - m_rect.width);
    else if ((m_align & wxALIGN_CENTER_HORIZONTAL) != 0)
        m_rect.SetX( (m_maxWidth  - m_rect.width)  / 2 );
    else
        m_rect.SetX(0);

    // Update top of grid
    if ((m_align & wxALIGN_BOTTOM) != 0)
        m_rect.SetY(m_maxHeight - m_rect.height);
    else if ((m_align & wxALIGN_CENTER_VERTICAL) != 0)
        m_rect.SetY( (m_maxHeight - m_rect.height) / 2 );
    else
        m_rect.SetY(0);

    ScaleFonts();
}


bool
XGridDrawer::SetMaxSize(int max_width, int max_height)
{
    m_maxWidth = max_width;
    m_maxHeight = max_height;

    if (m_grid == NULL)
        return false;

    // If the window is too small to fit the grid, catch it here
    if (m_maxWidth  < (m_grid->GetWidth()  + 1) * m_borderSize
     || m_maxHeight < (m_grid->GetHeight() + 1) * m_borderSize)
    {
        return false;
    }

    const size_t width =
        (m_maxWidth  - (m_grid->GetWidth() + 1)  * m_borderSize)
        / m_grid->GetWidth();

    const size_t height =
        (m_maxHeight - (m_grid->GetHeight() + 1) * m_borderSize)
        / m_grid->GetHeight();

    SetBoxSize(std::min(width, height));
    return true;
}


void
XGridDrawer::SetBoxSize(int size)
{
    m_boxSize = size;
    UpdateGridSize();
}


void
XGridDrawer::SetBorderSize(int size)
{
    m_borderSize = size;
    UpdateGridSize();
}


//-----------------------------------------------------------------------------
// Fonts
//-----------------------------------------------------------------------------

double
XGridDrawer::GetScale(int width, int height, int maxWidth, int maxHeight)
{
    if      (maxWidth == -1)
        return static_cast<double>(maxHeight) / static_cast<double>(height);
    else if (maxHeight == -1)
        return static_cast<double>(maxWidth) / static_cast<double>(width);

    return std::min(
        static_cast<double>(maxWidth)  / static_cast<double>(width),
        static_cast<double>(maxHeight) / static_cast<double>(height)
    );
}


void
XGridDrawer::ScaleFont(wxFont * font, int maxWidth, int maxHeight)
{
    // Can't do anything without a measurer
    if (! HasMeasurer())
        return;

    // Don't test numbers and symbols because they're probably not as wide.
    const wxString text = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // Adjust width to fit the test string
    if (maxWidth != -1)
        maxWidth *= text.length();

    int h, w;
    GetTextExtent(text, &w, &h, NULL, NULL, font);

    // First "guess" should speed things up
    font->SetPointSize(font->GetPointSize()
                       * GetScale(w, h, maxWidth, maxHeight));

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


inline void
XGridDrawer::ScaleLetterFont()
{
    // This function should be called after setting the first letter
    // font.

    // Provide a scaled font for a variable number of letters (1 - 8) in 
    // a given square.

    // 1 letter
    ScaleFont(&m_letterFont[0], -1, GetLetterHeight());

    // 2-3 letters
    m_letterFont[1] = m_letterFont[0];
    ScaleFont(&m_letterFont[1], GetSquareSize() / 4, GetLetterHeight());
    m_letterFont[2] = m_letterFont[1];

    // 4-5 letters
    m_letterFont[3] = m_letterFont[2];
    ScaleFont(&m_letterFont[3], GetSquareSize() / 6, GetLetterHeight());
    m_letterFont[4] = m_letterFont[3];

    // 6+ (6-8) letters
    m_letterFont[5] = m_letterFont[4];
    ScaleFont(&m_letterFont[5], GetSquareSize() / 8, GetLetterHeight());
    m_letterFont[6] = m_letterFont[5];
    m_letterFont[7] = m_letterFont[6];
}



void
XGridDrawer::SetNumberScale(double scale)
{
    m_numberScale = scale;
    ScaleNumberFont();
}



void
XGridDrawer::SetLetterScale(double scale)
{
    m_letterScale = scale;
    ScaleLetterFont();
}


void
XGridDrawer::SetLetterFont(const wxFont & font)
{
    m_letterFont[0] = font;
    ScaleLetterFont();
}


void
XGridDrawer::SetNumberFont(const wxFont & font)
{
    m_numberFont = font;
    if (HasMeasurer())
        ScaleNumberFont();
}



//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------
void
XGridDrawer::DrawSquare(wxDC & dc, const XSquare & square)
{
    if (square.IsWhite())
        DrawSquare(dc, square, GetWhiteSquareColor(), GetPenColor());
    else
        DrawSquare(dc, square, GetBlackSquareColor(), GetPenColor());
}

void
XGridDrawer::DrawSquare(wxDC & dc,
                           const XSquare & square,
                           const wxColour & bgColor,
                           const wxColour & textColor)
{
    // The order of drawing is important to make sure that we don't draw over
    // the more important parts of a square:
    // 1. The background
    // 2. An incorrect / revealed indicator (the little triangle in the corner)
    // 3. A circle
    // 4. The number (with an opaque background so it draws over the circle)
    // 5. The text
    // 6. An X over everything

    // If we are supposed to draw the outline, we'll draw the square with
    // the default square background, and then draw the outline using the
    // bgColor.

    const int x = m_rect.x + m_borderSize + square.GetCol() * GetSquareSize();
    const int y = m_rect.y + m_borderSize + square.GetRow() * GetSquareSize();

    // Draw the outline with a white background
    if (HasFlag(DRAW_OUTLINE))
    {
        const int outlineSize = std::max(m_boxSize / 15, 1);

        wxPen outlinePen(bgColor, outlineSize * 2);
        outlinePen.SetJoin(wxJOIN_MITER);

        if (square.IsWhite())
            dc.SetBrush(wxBrush(GetWhiteSquareColor()));
        else
            dc.SetBrush(wxBrush(GetBlackSquareColor()));

        dc.SetPen(outlinePen);
        dc.DrawRectangle(x + outlineSize,
                         y + outlineSize,
                         m_boxSize - outlineSize * 2 + 1,
                         m_boxSize - outlineSize * 2 + 1);

    }
    else // Just draw the background
    {
        dc.SetBrush(wxBrush(bgColor));
        dc.SetPen  (wxPen(bgColor));
        dc.DrawRectangle(x, y, m_boxSize, m_boxSize);
    }

    // Draw square's flag (top right)
    if (HasFlag(DRAW_FLAG) && square.HasFlag(XFLAG_RED | XFLAG_BLACK))
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


    if (HasFlag(DRAW_CIRCLE) && square.HasFlag(XFLAG_CIRCLE))
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxBLACK, 1));
        dc.DrawCircle(x + m_boxSize/2, y + m_boxSize/2, m_boxSize/2);
    }

    dc.SetTextForeground(textColor);

    // Draw square's number (top left).
    if (HasFlag(DRAW_NUMBER) && square.GetNumber() != 0)
    {
        // Set a solid text background so it will draw over any circles.
        if (! HasFlag(DRAW_OUTLINE))
            dc.SetTextBackground(bgColor);
        else if (square.IsWhite())
            dc.SetTextBackground(GetWhiteSquareColor());
        else
            dc.SetTextBackground(GetBlackSquareColor());
        dc.SetBackgroundMode(wxSOLID);

        dc.SetFont(m_numberFont);
        dc.DrawText(wxString::Format(_T("%d"), square.GetNumber()), x+1, y);
        dc.SetBackgroundMode(wxTRANSPARENT);
    }

    // Draw square's text (bottom and center to avoid conflicts with numbers)
    if (HasFlag(DRAW_TEXT) && ! square.IsBlank())
    {
        wxString text;
        if (HasFlag(DRAW_USER_TEXT))
            text = square.GetText();
        else
        {
            wxASSERT(HasFlag(DRAW_SOLUTION));
            text = square.GetSolution();
        }

        if (text.length() > 4)
        {
            const int len = (text.length() + 1) / 2;
            text.insert(len, _T("\n"));

            dc.SetFont(m_letterFont[3]);
        }
        else
            dc.SetFont(m_letterFont[text.length() - 1]);

        dc.DrawLabel(text,
                     wxRect(x, y + m_boxSize - GetLetterHeight(),
                            m_boxSize, GetLetterHeight()),
                     wxALIGN_CENTER);
    }

    // Draw an X across the square
    if (HasFlag(DRAW_X) && square.HasFlag(XFLAG_X))
    {
        dc.SetPen(wxPen(*wxRED, 2));
        // Funky math here because of the way that DCs draw lines
        dc.DrawLine(x + 1, y + 1, x + m_boxSize - 2, y + m_boxSize - 2);
        dc.DrawLine(x + m_boxSize - 2, y + 1, x + 1, y + m_boxSize - 2);
    }
}



void
XGridDrawer::DrawGrid(wxDC & dc)
{
    if (m_grid == NULL || m_grid->IsEmpty())
    {
        wxLogDebug(_T("Grid is empty"));
        return;
    }

    // Draw black as crossword background
    dc.SetBrush(wxBrush(*wxBLACK));
    dc.SetPen  (wxPen(*wxBLACK));

    dc.DrawRectangle(m_rect);

    for (XSquare * square = m_grid->First();
         square != NULL;
         square = square->Next())
    {
            DrawSquare(dc, *square);
    }
}
