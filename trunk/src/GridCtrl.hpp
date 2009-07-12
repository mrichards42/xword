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


// TODO:
//   - Fix moving around the grid (TAB key especially)
//   - Eventually get to entering characters other than A-Z
//        This might require changing how the puz headers store data
//   - Clean up the file


#ifndef MY_GRID_H
#define MY_GRID_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "puz/XGrid.hpp"
#include "defs.hpp"

class wxPuzEvent;


// For checking grid squares
enum
{
    MESSAGE_BOX      = 0x00,
    NO_REVEAL_ANSWER = 0x00,
    REVEAL_ANSWER    = 0x01,
    NO_MESSAGE_BOX   = 0x02
};

// THIS IS NOT TRUE CURRENTLYs
//---------------------------------------------------------------------------
// Note that this class will steal keyboard focus in OnPaint and
//   SetFocus (this overloaded function sets the grid's "focus" --
//   that is, the square that is highlighted)
// Calling wxScrolledWindow::OnFocus from OnPaint is neccesary to overcome a
//   bug in wx/window.h: the WinAPI SetFocus function fails when
//   the frame is minimized and then restored, such that clicking
//   on the frame will not reset the focus
// This focus-stealing behavior is fine in this instance, because
//   the grid window should be the only thing in the application that
//   needs keyboard input.  Just watch out before doing this in other
//   places
//---------------------------------------------------------------------------

class GridCtrl
    : public wxScrolledWindow
{
public:
    GridCtrl() { Init(); }

    explicit GridCtrl(wxWindow * parent,
                      wxWindowID id = -1,
                      const wxPoint & pos = wxDefaultPosition,
                      const wxSize & size = wxDefaultSize,
                      XGrid * grid = NULL,
                      long style = DEFAULT_GRID_STYLE
                                 | wxVSCROLL | wxHSCROLL)
    {
        Init();
        Create(parent, id, pos, size, grid, style);
    }

    bool Create(wxWindow * parent,
                wxWindowID id = -1,
                const wxPoint & pos = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                XGrid * grid = NULL,
                long style = DEFAULT_GRID_STYLE
                           | wxVSCROLL | wxHSCROLL);

    ~GridCtrl();

    void SetGrid(XGrid * grid);
          XGrid * GetXGrid()       { return m_grid; }
    const XGrid * GetXGrid() const { return m_grid; }

    // Will trigger an event
    XSquare * SetSquareFocus(XSquare * square, bool direction);
    XSquare * SetSquareFocus(XSquare * square = NULL)
        { return SetSquareFocus(square, m_direction); }

    void SetDirection(bool direction) { SetSquareFocus(NULL, direction); }
    void SetDirection()               { SetDirection( ! m_direction); }

    void SetFocusedClue   (int cluenum, bool direction)
        { ChangeFocusedClue(cluenum, direction); SetSquareFocus(); }
    void ChangeFocusedClue(int cluenum, bool direction);

    // Will not trigger an event
    XSquare * ChangeSquareFocus(XSquare * square, bool direction);
    XSquare * ChangeSquareFocus(XSquare * square = NULL)
        { return ChangeSquareFocus(square, m_direction); }

    void ChangeDirection(bool direction) { ChangeSquareFocus(NULL, direction); }
    void ChangeDirection()               { ChangeDirection(! m_direction); }

    void RefreshSquare(wxDC & dc, XSquare & square)  { DrawSquare(dc, square); }
    void RefreshSquare(XSquare & square)
        { wxClientDC dc(this); DoPrepareDC(dc); RefreshSquare(dc, square); }

    void SetLetter(XSquare & square, char letter = _T('-'));

    XSquare * GetFocusedSquare() { return m_focusedSquare; }

    void CheckGrid  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckWord  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckLetter(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);

    // Fonts
    bool SetFont(const wxFont & font);
    void SetLetterFont(const wxFont & font);
    void SetNumberFont(const wxFont & font);

    // Colors
    void SetColors(const wxColor & square,
                   const wxColor & focus,
                   const wxColor & word);
    void SetSquareColor(const wxColor & color)
        { m_colors[WHITE_SQUARE]  = color; }
    void SetFocusColor(const wxColor & color)
        { m_colors[LETTER_SQUARE] = color; }
    void SetWordColor(const wxColor & color)  
        { m_colors[WORD_SQUARE]   = color; }

    // Sizes
    wxSize GetBestSize() const;

    // Zooming
    void FitGrid(bool fit = true)
    {
        Freeze();
        m_fit = fit;
        Scale();
        MakeVisible(*m_focusedSquare);
        Thaw();
        Refresh();
    }

    void ZoomIn (double factor = 1.1)
    {
        Freeze();
        m_fit = false;
        Scale(factor);
        MakeVisible(*m_focusedSquare);
        Refresh();
        Thaw();
    }

    void ZoomOut(double factor = 1.1) { ZoomIn(1/factor); }

    // Square/border size
    int  GetSquareSize()            { return m_boxSize + m_borderSize; }
    void SetBorderSize(size_t size) { m_borderSize = size; Scale(); Refresh(); }
    int  GetBorderSize() const      { return m_borderSize; }

    // Style
    void SetGridStyle(int style)
        { SetWindowStyle( GetWindowStyle() & ~ GRID_STYLE_MASK | style ); }
    int  GetGridStyle() const  { return GetWindowStyle() & GRID_STYLE_MASK; }
    bool HasStyle(int style) const { return HasFlag(style); }

    // Information functions
    const XSquare & At(int col, int row) const { return m_grid->At(col, row); }
          XSquare & At(int col, int row)       { return m_grid->At(col, row); }

    int GetBlankCount()     const { return m_blankSquares; }
    int GetIncorrectCount() const { return m_incorrectSquares; }


    XSquare * GetClueNumber     (int num);

    template <typename T>
    XSquare * FindSquare(XSquare * start,
                         T type, bool direction,
                         int increment = FIND_NEXT,
                         bool skipBlack = NO_SKIP_BLACK_SQUARES,
                         bool wrapLines = NO_WRAP_LINES);

    template <typename T>
    XSquare * FindNextSquare(XSquare * start,
                             T type,
                             bool direction,
                             int increment = FIND_NEXT,
                             bool skipBlack = NO_SKIP_BLACK_SQUARES,
                             bool wrapLines = NO_WRAP_LINES);

    bool IsFocusedLetter(const XSquare & square);
    bool IsFocusedWord  (const XSquare & square);

    XSquare * HitTest(int x, int y);
    wxPoint TopLeft(const XSquare & square);
    wxPoint BottomRight(const XSquare & square);

protected:
    void Init();

    // Drawing functions
    void OnPaint(wxPaintEvent & evt);
    void DrawGrid(wxDC & dc, const wxRegion & updateRegion = wxRegion());

    void DrawSquare(wxDC & dc, const XSquare & square, const wxColour & color);

    void DrawSquare(wxDC & dc, const XSquare & square)
        { DrawSquare(dc, square, GetSquareColor(square)); }
    void DrawSquare(wxDC & dc, int col, int row, const wxColour & color)
        { DrawSquare(dc, At(col, row), color); }
    void DrawSquare(wxDC & dc, int col, int row)
        { DrawSquare(dc, At(col, row)); }

    const wxColor & GetSquareColor(const XSquare & square);

    void MakeVisible(const XSquare & square);


    // Scaling functions and members
    void OnSize(wxSizeEvent & evt);
    void Scale(double factor = 1.0);
    void ScaleFont(wxFont * font, double desiredHeight);

    wxRect m_rect;              // Overall grid size
    int m_borderSize;           // Border between squares
    int m_boxSize;              // width/heigth of a square
    int m_lastBoxSize;          // The last non-fitted box size
    bool m_fit;                 // Fit the grid to the window?

    wxFont m_numberFont;
    wxFont m_letterFont;

    // Pointer to puzzle data
    XGrid * m_grid;

    // Focused square/word
    XSquare * m_focusedSquare;
    XSquare * m_focusedStart;
    XSquare * m_focusedEnd;

    // Focused direction
    bool m_direction;

    // For counting missing/blank squares
    int m_blankSquares;
    int m_incorrectSquares;

    // Square colors
    wxColor m_colors[3];

private:
    // Events
    void OnClueFocus  (wxPuzEvent & evt);

    void OnLeftDown   (wxMouseEvent & evt);
    void OnRightDown  (wxMouseEvent & evt);
    void OnKeyDown    (wxKeyEvent & evt);
    void OnLetter     (char key, int mod);
    void OnArrow      (bool arrowDirection, bool increment, int mod);
    void OnTab        (int mod);
    void OnHome       (int mod);
    void OnEnd        (int mod);
    void OnBackspace  (int mod);
    void OnDelete     (int mod);

    DECLARE_EVENT_TABLE()
};







//---------------------------------------------------------------------
// Inline functions
//---------------------------------------------------------------------

// Colors
inline void
GridCtrl::SetColors(const wxColor & square,
                    const wxColor & focus,
                    const wxColor & word)
{
    SetSquareColor(square);
    SetFocusColor(focus);
    SetWordColor(word);
}



// Fonts
inline bool
GridCtrl::SetFont(const wxFont & font)
{
    SetNumberFont(font);
    SetLetterFont(font);
    return wxScrolledWindow::SetFont(font);
}

inline void
GridCtrl::SetNumberFont(const wxFont & font)
{
    m_numberFont = font;
    ScaleFont(&m_numberFont, m_boxSize * NUMBER_SCALE);
}

inline void
GridCtrl::SetLetterFont(const wxFont & font)
{
    m_letterFont = font;
    ScaleFont(&m_letterFont, m_boxSize * LETTER_SCALE);
}


inline bool
GridCtrl::IsFocusedLetter(const XSquare & Square)
{
    return m_focusedSquare == &Square;
}




inline wxPoint
GridCtrl::TopLeft(const XSquare & square)
{
    return wxPoint(m_rect.x + m_borderSize + square.col*GetSquareSize(),
                   m_rect.y + m_borderSize + square.row*GetSquareSize());
}


inline wxPoint
GridCtrl::BottomRight(const XSquare & square)
{
    return wxPoint(m_rect.x + 2 * m_borderSize + square.col*GetSquareSize(),
                   m_rect.y + 2 * m_borderSize + square.row*GetSquareSize());
}








template <typename T>
XSquare *
GridCtrl::FindSquare(XSquare * start, T type, bool direction, int increment,
                     bool skipBlack, bool wrapLines)
{
    return m_grid->FindSquare(start, type, direction, increment,
                              skipBlack, wrapLines);
}


template <typename T>
XSquare *
GridCtrl::FindNextSquare(XSquare * start, T type, bool direction, int increment,
                         bool skipBlack, bool wrapLines)
{
    return m_grid->FindNextSquare(start, type, direction, increment,
                                  skipBlack, wrapLines);
}

#endif // MY_GRID_H