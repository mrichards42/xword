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

#include "XGridDrawer.hpp"
#include "puz/Puzzle.hpp"
#include "puz/Grid.hpp"
#include "puz/Square.hpp"
#include <wx/fontenum.h> // wxFontEnumerator to test for Webdings
#include "utils/string.hpp"
#include "utils/timeit.hpp"
#include "wx/graphics.h"
#include <wx/mstream.h>
#include "utils/color.hpp" // GetBrightness

#define XWORD_USE_GC 0

const int MAX_POINT_SIZE = 150;
const int MIN_POINT_SIZE = 2;

XGridDrawer::XGridDrawer(puz::Puzzle * puz)
    : m_dc(NULL),
      m_status(INVALID_MEASURER)
{
    Init();
    SetPuzzle(puz);
}

XGridDrawer::XGridDrawer(wxDC * dc, puz::Puzzle * puz)
{
    SetDC(dc);
    Init();
    SetPuzzle(puz);
}

XGridDrawer::XGridDrawer(wxWindow * window, puz::Puzzle * puz)
{
    SetWindow(window);
    Init();
    SetPuzzle(puz);
}

void
XGridDrawer::Init()
{
    m_puz = NULL;

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
    // Webdings are used for the symbol font
    if (wxFontEnumerator::IsValidFacename(_T("Webdings")))
    {
        m_hasSymbolFont = true;
        m_symbolFont = wxFont(12, wxFONTFAMILY_DEFAULT,
                              wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                              false, _T("Webdings"));
    }
    else
    {
        m_hasSymbolFont = false;
    }

    SetWhiteSquareColor(*wxWHITE);
    SetBlackSquareColor(*wxBLACK);
    SetThemeColor(*wxWHITE);
    UpdateHighlightColor();
    SetPenColor(*wxBLACK);
    SetRevealedColor(wxColour(246, 36, 0));
    SetCheckedColor(*wxBLACK);
    SetCorrectColor(wxColour(64, 193, 23));

    m_drawOptions = DRAW_ALL;

    UpdateGridSize();
}

puz::Grid * XGridDrawer::GetGrid()
{
    wxASSERT(m_puz);
    return &m_puz->GetGrid();
}

const puz::Grid * XGridDrawer::GetGrid() const
{
    wxASSERT(m_puz);
    return &m_puz->GetGrid();
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
    if (m_puz == NULL || GetGrid()->IsEmpty())
        return wxDefaultSize;

    return wxSize( GetGrid()->GetWidth()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize,

                   GetGrid()->GetHeight()
                   * (m_boxSize + m_borderSize)
                   + m_borderSize );
}


void
XGridDrawer::UpdateGridSize()
{
    if (m_puz == NULL)
        return;
    // Update width and height of grid
    m_rect.SetWidth ( GetGrid()->GetWidth()
                      * (m_boxSize + m_borderSize)
                      + m_borderSize );

    m_rect.SetHeight( GetGrid()->GetHeight()
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
XGridDrawer::SetMaxSize(size_t max_width, size_t max_height)
{
    m_maxWidth = max_width;
    m_maxHeight = max_height;

    if (m_puz == NULL)
        return false;

    // If the window is too small to fit the grid, catch it here
    if (m_maxWidth  < (GetGrid()->GetWidth()  + 1) * m_borderSize
     || m_maxHeight < (GetGrid()->GetHeight() + 1) * m_borderSize)
    {
        return false;
    }

    const size_t width =
        (m_maxWidth  - (GetGrid()->GetWidth() + 1)  * m_borderSize)
        / GetGrid()->GetWidth();

    const size_t height =
        (m_maxHeight - (GetGrid()->GetHeight() + 1) * m_borderSize)
        / GetGrid()->GetHeight();

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
    int scaledSize = font->GetPointSize() * GetScale(w, h, maxWidth, maxHeight);
    font->SetPointSize(std::min(std::max(scaledSize, MIN_POINT_SIZE), MAX_POINT_SIZE));

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

    // 2 letters
    m_letterFont[1] = m_letterFont[0];
    ScaleFont(&m_letterFont[1], GetLetterHeight() / 2., GetLetterHeight());
    // 3 letters
    m_letterFont[2] = m_letterFont[1];
    ScaleFont(&m_letterFont[2], GetLetterHeight() / 3., GetLetterHeight());

    // 4+ letters
    m_letterFont[3] = m_letterFont[2];
    ScaleFont(&m_letterFont[3], GetLetterHeight() / 4., GetLetterHeight());
}

void
XGridDrawer::ScaleSymbolFont()
{
    if (! m_hasSymbolFont)
        return;
    ScaleFont(&m_symbolFont, -1, GetLetterHeight());
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
    ScaleSymbolFont();
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
wxRect
XGridDrawer::GetSquareRect(const puz::Square & square) const
{
    const int squareSize = GetSquareSize();
    return wxRect(
        m_rect.x + m_borderSize + square.GetCol() * squareSize,
        m_rect.y + m_borderSize + square.GetRow() * squareSize,
        m_boxSize,
        m_boxSize);
}

wxRect
XGridDrawer::GetTextRect(const wxRect & square) const
{
    return wxRect(square.x,
                  square.y + m_boxSize - GetLetterHeight(),
                  m_boxSize,
                  GetLetterHeight());
}

void
XGridDrawer::DrawSquare(wxDC & dc, const puz::Square & square)
{
    DrawSquare(dc, square, GetSquareColor(square), GetPenColor());
}

// Blend the foreground color and the background color.
wxColour
blendColors(const wxColour& foreground, const wxColour& background) {
    // We average the background and foreground colors, but we darken the background
    // color somewhat to improve the contrast with other selected squares.
    return wxColour(
        (std::max(background.Red() - 50, 0) + foreground.Red()) / 2,
        (std::max(background.Green() - 50, 0) + foreground.Green()) / 2,
        (std::max(background.Blue() - 50, 0) + foreground.Blue()) / 2);
}

void
XGridDrawer::DrawSquare(wxDC & adc,
                        const puz::Square & square,
                        const wxColour & bgColor_,
                        const wxColour & textColor_,
                        const bool blendBackground)
{
    if (square.IsMissing())
        return;

    // The order of drawing is important to make sure that we don't draw over
    // the more important parts of a square:
    // 1. The background
    // 2. An incorrect / revealed indicator (the little triangle in the corner)
    // 3. A correct indicator (green check in the corner)
    // 4. A circle
    // 5. The number (with an opaque background so it draws over the circle)
    // 6. Thicker inner borders when specified
    // 7. The text
    // 8. An X over everything

    // If we are supposed to draw the outline, we'll draw the square with
    // the default square background, and then draw the outline using the
    // bgColor.

    // If the caller wants us to blend the square's inherent color with the given color (e.g. this
    // is a selected/highlighted square), do so as long as the given color is non-white. This keeps
    // the standard highlight color consistent with the user selection most of the time, while
    // providing a visible contrast for selected cells with different backgrounds against other
    // selected cells.
    wxColour bgColor = bgColor_;
    wxColour squareColor = GetSquareColor(square);
    if (blendBackground && squareColor != GetWhiteSquareColor()) {
        bgColor = blendColors(bgColor, squareColor);
    }

    // Check the square background to see if this text color works
    wxColour textColor = textColor_;
    //  If the user chose these colors, respect them
    if (bgColor != GetWhiteSquareColor())
    {
        const signed short bgBrightness = GetBrightness(bgColor);
        const signed short fgBrightness = GetBrightness(textColor);
        if (std::abs(bgBrightness - fgBrightness) < 125)
        {
            // Invert the text color
            textColor.Set(255 - textColor.Red(),
                          255 - textColor.Green(),
                          255 - textColor.Blue());
        }
    }

#if wxUSE_GRAPHICS_CONTEXT && XWORD_USE_GC
    wxGCDC dc((wxWindowDC&)adc);
    //wxDC & dc = adc;
#else
    wxDC & dc = adc;
#endif
    wxRect rect = GetSquareRect(square);
    const int x = rect.x;
    const int y = rect.y;
    // Draw the square using the square's own background color,
    // and Black as the pen (except for annotation squares, which
    // are drawn without an outline).
    dc.SetBrush(wxBrush(GetSquareColor(square)));
    int borderSize = m_borderSize;
    const int offset = borderSize / 2;
    if (square.IsAnnotation()) {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(GetSquareColor(square)));
        dc.DrawRectangle(x + offset, y + offset, m_boxSize, m_boxSize);
    } else {
        wxPen borderPen(GetBlackSquareColor(), borderSize);
        borderPen.SetJoin(wxJOIN_MITER);
        dc.SetPen(borderPen);
        dc.DrawRectangle(x - borderSize + offset,
            y - borderSize + offset,
            m_boxSize + borderSize + 1,
            m_boxSize + borderSize + 1);
    }

    // Background Image
    if (square.HasImage() && m_boxSize > 2) // Prevent errors with scaling
    {
        wxLogNull no_log; // Prevent bad images from displaying errors.

        wxImage img = m_imageMap[&square];
        if (! img.IsOk())
        {
            // Load the image and cache it.
			wxMemoryInputStream stream(square.m_imagedata.c_str(),
									   square.m_imagedata.length());
            img = wxImage(stream);
            m_imageMap[&square] = img;
        }
        if (img.IsOk())
        {
            img.Rescale(m_boxSize, m_boxSize);
            dc.DrawBitmap(wxBitmap(img), x, y);
        }
    }

    // Draw the outline with a clear background
    if (HasFlag(DRAW_OUTLINE))
    {
        const int outlineSize = std::max(m_boxSize / 15, 1);

        wxPen outlinePen(bgColor, outlineSize * 2);
        outlinePen.SetJoin(wxJOIN_MITER);

        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        dc.SetPen(outlinePen);
        dc.DrawRectangle(x + outlineSize,
                         y + outlineSize,
                         m_boxSize - outlineSize * 2 + 1,
                         m_boxSize - outlineSize * 2 + 1);

    }
    // Just draw the background, but don't paint over the background image
    else if (bgColor != GetBlackSquareColor()
             && bgColor != GetWhiteSquareColor())
    {
        dc.SetBrush(wxBrush(bgColor));
        dc.SetPen  (wxPen(bgColor));
        dc.DrawRectangle(x, y, m_boxSize, m_boxSize);
    }

    if ((HasFlag(DRAW_SOLUTION) ? square.IsSolutionBlack() : square.IsBlack()) && !square.IsAnnotation())
        return; // Nothing else to do if it's a black square.

    // Draw square's flag (top right)
    if (HasFlag(DRAW_FLAG) && square.HasFlag(puz::FLAG_REVEALED | puz::FLAG_BLACK))
    {
        wxColor flagColor = GetCheckedColor();
        if (square.HasFlag(puz::FLAG_REVEALED))
            flagColor = GetRevealedColor();
        dc.SetPen(wxPen(flagColor));
        dc.SetBrush(wxBrush(flagColor));

        wxPoint pts[3];
        pts[0] = wxPoint(x + 2./3.*m_boxSize, y);
        pts[1] = wxPoint(x + m_boxSize - 1, y);
        pts[2] = wxPoint(x + m_boxSize - 1, y + 1./3.*m_boxSize);
        dc.DrawPolygon(3, pts);
    }

    if (HasFlag(DRAW_FLAG) && square.HasFlag(puz::FLAG_CORRECT)
        && ! square.HasFlag(puz::FLAG_REVEALED))
    {
        dc.SetPen(wxPen(GetCorrectColor()));
        dc.SetBrush(wxBrush(GetCorrectColor()));
        // Draw a circle at the centroid of the above triangle.
        dc.DrawCircle(x + 8./9.*m_boxSize, y + 1./9.*m_boxSize, m_boxSize/12.);
    }


    if (HasFlag(DRAW_CIRCLE) && square.HasFlag(puz::FLAG_CIRCLE))
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(GetBlackSquareColor(), m_borderSize));
        dc.DrawEllipse(x, y, m_boxSize, m_boxSize);
    }

    // Draw square's number (top left).
    if (HasFlag(DRAW_NUMBER))
    {
        dc.SetFont(m_numberFont);
        dc.SetTextForeground(textColor);

        // Draw corner marks
        wxRect markRect(x+1, y, m_boxSize - 2, m_boxSize - 2);
        if (! square.m_mark[puz::MARK_TL].empty())
            dc.DrawLabel(puz2wx(square.m_mark[puz::MARK_TL]), markRect,
                         wxALIGN_TOP | wxALIGN_LEFT);
        if (! square.m_mark[puz::MARK_TR].empty())
            dc.DrawLabel(puz2wx(square.m_mark[puz::MARK_TR]), markRect,
                         wxALIGN_TOP | wxALIGN_RIGHT);
        if (! square.m_mark[puz::MARK_BL].empty())
            dc.DrawLabel(puz2wx(square.m_mark[puz::MARK_BL]), markRect,
                         wxALIGN_BOTTOM | wxALIGN_LEFT);
        if (square.m_mark[puz::MARK_BR].empty())
            dc.DrawLabel(puz2wx(square.m_mark[puz::MARK_BR]), markRect,
                         wxALIGN_BOTTOM | wxALIGN_RIGHT);

        // Draw the number
        
        // Set a solid text background so it will draw over any circles.
        // It seems like we might want to also have a solid background
        // for marks, but Crossword Solver doesn't . . . its display of
        // the looking glass puzzle looks better without a solid background
        // for marks, so I guess it's a toss-up.
        if (! HasFlag(DRAW_OUTLINE))
            dc.SetTextBackground(bgColor);
        else if (square.IsWhite())
            dc.SetTextBackground(GetWhiteSquareColor());
        dc.SetBackgroundMode(wxSOLID);


        // If we have a diagramless grid and the square's number is
        // somehow broken, draw the number in red.
        if (GetGrid()->IsDiagramless())
        {
            try
            {
                if ( (square.WantsClue(puz::ACROSS) &&
                        m_puz->GetClueList(puzT("Across")).Find(square.GetNumber()) == NULL) ||
                     (square.WantsClue(puz::DOWN) &&
                        m_puz->GetClueList(puzT("Down")).Find(square.GetNumber()) == NULL) )
                {
                    dc.SetTextForeground(*wxRED);
                }
            }
            // In case we don't have "Across" or "Down" clues
            catch (puz::NoClues &)
            {
                dc.SetTextForeground(*wxRED);
            }
        }

        if (square.HasNumber())
            dc.DrawText(puz2wx(square.GetNumber()), x+1, y);

        dc.SetBackgroundMode(wxTRANSPARENT);
    }

    // Draw bars
    const int barSize = m_borderSize * 3;
    const int barOffset = barSize / 2;
    wxPen barPen(GetBlackSquareColor(), barSize);
    barPen.SetCap(wxCAP_BUTT);
    dc.SetPen(barPen);
    if (square.m_bars[puz::BAR_TOP])
        dc.DrawLine(x - 1,
                    y + barOffset,
                    x + m_boxSize,
                    y + barOffset);
    if (square.m_bars[puz::BAR_LEFT])
        dc.DrawLine(x + barOffset,
                    y - 1,
                    x + barOffset,
                    y + m_boxSize);
    if (square.m_bars[puz::BAR_BOTTOM])
        dc.DrawLine(x - 1,
                    y + m_boxSize - barSize + barOffset,
                    x + m_boxSize,
                    y + m_boxSize - barSize + barOffset);
    if (square.m_bars[puz::BAR_RIGHT])
        dc.DrawLine(x + m_boxSize - barSize + barOffset,
                    y - 1,
                    x + m_boxSize - barSize + barOffset,
                    y + m_boxSize);

    // Draw square's text (bottom and center to avoid conflicts with numbers)
    if ((HasFlag(DRAW_USER_TEXT) && ! square.IsBlank()) ||
        (HasFlag(DRAW_SOLUTION) && ! square.IsSolutionBlank()) ||
        square.IsAnnotation())
    {
        dc.SetTextForeground(textColor);
        wxString text;
        bool isSymbol = false;
        // User Text
        if (HasFlag(DRAW_USER_TEXT) || square.IsAnnotation())
        {
            if (square.HasTextSymbol())
            {
                if (m_hasSymbolFont)
                {
                    isSymbol = true;
                    text.append(1, square.GetTextSymbol());
                }
                else
                    text.append(1, static_cast<wxChar>(square.GetPlainText()));
            }
            else
                text = puz2wx(square.GetText());
        }
        // Solution
        else
        {
            wxASSERT(HasFlag(DRAW_SOLUTION));
            if (square.HasSolutionSymbol())
            {
                if (m_hasSymbolFont)
                {
                    isSymbol = true;
                    text = static_cast<wxChar>(square.GetSolutionSymbol());
                }
                else
                    text = static_cast<wxChar>(square.GetPlainSolution());
            }
            else
                text = puz2wx(square.GetSolution());
        }


        // Rebus entries
        if (text.length() > 4)
        {
            if (text.length() > TRUNCATED_REBUS_LENGTH)
                text = text.substr(0, TRUNCATED_REBUS_LENGTH - 5) + _T("[...]");

            const int len = (text.length() + 1) / 2;
            text.insert(len, _T("\n"));

            dc.SetFont(m_letterFont[3]);
        }
        else
        {
            if (! isSymbol)
                dc.SetFont(m_letterFont[text.length() - 1]);
            else
                dc.SetFont(m_symbolFont);
        }
/*
        dc.SetPen(*wxRED_PEN);
        dc.DrawRectangle(GetTextRect(rect));
        */
        dc.DrawLabel(text, GetTextRect(rect), wxALIGN_CENTER);
    }

    // Draw an X across the square
    if (HasFlag(DRAW_X) && square.HasFlag(puz::FLAG_X))
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
    if (m_puz == NULL || GetGrid()->IsEmpty())
        return;

    for (puz::Square * square = GetGrid()->First();
         square != NULL;
         square = square->Next())
    {
            DrawSquare(dc, *square);
    }
}


void XGridDrawer::UpdateHighlightColor()
{
    // Highlight is 25% as dark as black
    wxImage::RGBValue rgb(m_blackSquareColor.Red(),
                          m_blackSquareColor.Green(),
                          m_blackSquareColor.Blue());
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
    hsv.value = (1 - (1 - hsv.value) * .25);
    rgb = wxImage::HSVtoRGB(hsv);
    m_highlightColor.Set(rgb.red, rgb.green, rgb.blue);
}

wxColour
XGridDrawer::GetSquareColor(const puz::Square & square) const
{
    if (! HasFlag(DRAW_SOLUTION))
    {
        if (square.IsWhite() || HasFlag(DRAW_BLANK_DIAGRAMLESS) || square.IsAnnotation())
        {
            if (square.HasHighlight())
                return GetHighlightColor();
            else if (square.HasColor())
                return wxColor(square.m_red, square.m_green, square.m_blue);
            else if (HasFlag(DRAW_THEME) && square.IsTheme())
                return GetThemeColor();
            else
                return GetWhiteSquareColor();
        }
        else if (square.IsBlack())
        {
            if (square.HasColor())
                return wxColor(square.m_red, square.m_green, square.m_blue);
            else
                return GetBlackSquareColor();
        }
    }
    else
    {
        if (square.IsSolutionWhite() || HasFlag(DRAW_BLANK_DIAGRAMLESS))
        {
            if (square.HasHighlight())
                return GetHighlightColor();
            else if (square.HasColor())
                return wxColor(square.m_red, square.m_green, square.m_blue);
            else if (HasFlag(DRAW_THEME) && square.IsTheme())
                return GetThemeColor();
            else
                return GetWhiteSquareColor();
        }
        else if (square.IsSolutionBlack())
        {
            return GetBlackSquareColor();
        }
    }
    return wxNullColour;
}
