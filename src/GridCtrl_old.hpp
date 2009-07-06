/*
  This file is part of XWord
  Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


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

#include "puz/XPuzzle.hpp"
#include "defs.hpp"
#include "PuzEvent.hpp"


// For checking grid squares
static const int MESSAGE_BOX      = 0x00;
static const int NO_REVEAL_ANSWER = 0x00;
static const int REVEAL_ANSWER    = 0x01;
static const int NO_MESSAGE_BOX   = 0x02;


//---------------------------------------------------------------------------
// Note that this class will steal keyboard focus in OnPaint and
//   SetFocus (this overloaded function sets the grid's "focus" --
//   that is, the square that is highlighted)
// Calling wxWindow::OnFocus from OnPaint is neccesary to overcome a
//   bug in wx/window.h: the WinAPI SetFocus function fails when
//   the frame is minimized and then restored, such that clicking
//   on the frame will not reset the focus
// This focus-stealing behavior is fine in this instance, because
//   the grid window should be the only thing in the application that
//   needs keyboard input.  Just watch out before doing this in other
//   places
//---------------------------------------------------------------------------

class GridCtrl
    : public wxWindow
{
public:
    explicit GridCtrl(wxWindow * parent);
    ~GridCtrl();

    void SetPuzzle(XPuzzle * puz);
          XGrid * GetXGrid()       { return m_grid; }
    const XGrid * GetXGrid() const { return m_grid; }

    // Will not trigger an event
    XSquare * ChangeSquareFocus(XSquare * square, bool direction);
    XSquare * ChangeSquareFocus(XSquare * square = NULL) { return ChangeSquareFocus(square, m_direction); }

    XSquare * SetSquareFocus(XSquare * square, bool direction);
    XSquare * SetSquareFocus(XSquare * square = NULL) { return SetSquareFocus(square, m_direction); }

    void ChangeDirection(bool direction) { ChangeSquareFocus(NULL, direction); }
    void ChangeDirection() { ChangeDirection(! m_direction); }

    void SetDirection(bool direction) { SetSquareFocus(NULL, direction); }
    void SetDirection() { SetDirection( ! m_direction); }

    void SetFocusedClue(int cluenum, bool direction);
    void RefreshSquare(wxDC & dc, XSquare & square)    { DrawSquare(dc, square); }
    void RefreshSquare(XSquare & square)               { wxClientDC dc(this); RefreshSquare(dc, square); }

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
    void SetColors(const wxColor & square, const wxColor & focus, const wxColor & wordFocus);
    void SetSquareColor(const wxColor & square);
    void SetFocusColor(const wxColor & focus);
    void SetWordFocusColor(const wxColor & wordFocus);

    // Sizes
    wxSize GetBestSize() const;

    // Alignment
    void SetAlign(int align) { if (align == m_align) return; m_align = align; Scale(); }
    int GetAlign() const  { return m_align; }

    // Number of pixels to scroll
    int GetSquareSize() { return m_boxSize + m_borderSize; }

    void SetStyle(int style);
    int  GetStyle();
    bool HasStyle(int style);

    // Information functions (multiple overloads)
    const XSquare & At(int col, int row)   const { return m_grid->At(col, row); }
          XSquare & At(int col, int row)         { return m_grid->At(col, row); }

    int GetBlankCount()     const { return m_blankSquares; }
    int GetIncorrectCount() const { return m_incorrectSquares; }


    XSquare * GetClueNumber     (int num);

    template <typename T>
    XSquare * FindSquare(XSquare * start, T type, bool direction, int increment = FIND_NEXT, bool skipBlack = NO_SKIP_BLACK_SQUARES, bool wrapLines = NO_WRAP_LINES)
        { return m_grid->FindSquare(start, type, direction, increment, skipBlack, wrapLines); }

    template <typename T>
    XSquare * FindNextSquare(XSquare * start, T type, bool direction, int increment = FIND_NEXT, bool skipBlack = NO_SKIP_BLACK_SQUARES, bool wrapLines = NO_WRAP_LINES)
        { return m_grid->FindNextSquare(start, type, direction, increment, skipBlack, wrapLines); }

    XSquare * HitTest(int x, int y);
    wxPoint TopLeft(const XSquare & square);
    wxPoint BottomRight(const XSquare & square);

    bool IsFocusedLetter(const XSquare & square);
    bool IsFocusedWord(const XSquare & square);

protected:
    // Drawing functions
    void OnPaint(wxPaintEvent & evt);
    void DrawGrid(wxDC & dc, const wxRegion & updateRegion = wxRegion());

    void DrawSquare(wxDC & dc, const XSquare & square, const wxColour & color);
    void DrawSquare(wxDC & dc, const XSquare & square)                   { DrawSquare(dc, square, GetSquareColor(square)); }
    void DrawSquare(wxDC & dc, int col, int row, const wxColour & color) { DrawSquare(dc, At(col, row), color); }
    void DrawSquare(wxDC & dc, int col, int row)                         { DrawSquare(dc, At(col, row)); }

    const wxColor & GetSquareColor(const XSquare & square);



    // Scaling functions and members
    void OnSize(wxSizeEvent & evt);
    void Scale();
    void ScaleFont(wxFont * font, double desiredHeight);
    wxRect m_rect;              // Overall grid size
    int m_borderSize;           // Border between squaress
    int m_boxSize;              // width/heigth of a square
    wxFont m_numberFont;
    wxFont m_letterFont;

    // Pointers to puzzle data
    XPuzzle * m_puz;
    XGrid * m_grid;


    int m_style;
    int m_align;

    // Focusing, movement, and events
    XSquare * m_focusedSquare;
    XSquare * m_focusedStart;
    XSquare * m_focusedEnd;

    bool m_direction;
    wxColor m_colors[3];

    // For counting missing/blank squares
    int m_blankSquares;
    int m_incorrectSquares;

    void OnClueFocus  (wxPuzEvent & evt);

    void OnKeyDown    (wxKeyEvent & evt);    // Main function to process key strokes
    void OnLetter     (char key, int mod);
    void OnArrow      (bool arrowDirection, bool increment, int mod);
    void OnTab        (int mod);
    void OnHome       (int mod);
    void OnEnd        (int mod);
    void OnBackspace  (int mod);
    void OnDelete     (int mod);
    void OnLeftDown   (wxMouseEvent & evt);
    void OnRightDown  (wxMouseEvent & evt);


    DECLARE_EVENT_TABLE()
};







//---------------------------------------------------------------------
// Inline functions
//---------------------------------------------------------------------

// Colors
inline void
GridCtrl::SetColors(const wxColor & square,
                  const wxColor & focus,
                  const wxColor & wordFocus)
{
    SetSquareColor(square);
    SetFocusColor(focus);
    SetWordFocusColor(wordFocus);
}

inline void
GridCtrl::SetSquareColor(const wxColor & square)
{
    m_colors[WHITE_SQUARE] = square;
}

inline void
GridCtrl::SetFocusColor(const wxColor & focus)
{
    m_colors[LETTER_SQUARE] = focus;
}

inline void
GridCtrl::SetWordFocusColor(const wxColor & wordFocus)
{
    m_colors[WORD_SQUARE] = wordFocus;
}


// Fonts
inline bool
GridCtrl::SetFont(const wxFont & font)
{
    SetNumberFont(font);
    SetLetterFont(font);
    return wxWindow::SetFont(font);
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


inline void
GridCtrl::SetStyle(int style)
{
    m_style = style;
}

inline int
GridCtrl::GetStyle()
{
    return m_style;
}

inline bool
GridCtrl::HasStyle(int style)
{
    return (m_style & style) != 0;
}


inline bool
GridCtrl::IsFocusedLetter(const XSquare & Square)
{
    return m_focusedSquare == &Square;
}



//---------------------------------------------------------------------
// Lots of functions that use "default" parameters that are not possible
//   to specify because they are not static
//---------------------------------------------------------------------


inline wxPoint
GridCtrl::TopLeft(const XSquare & square)
{
    return wxPoint(m_rect.x + m_borderSize + square.col*(m_boxSize + m_borderSize),
                   m_rect.y + m_borderSize + square.row*(m_boxSize + m_borderSize));
}


inline wxPoint
GridCtrl::BottomRight(const XSquare & square)
{
    return wxPoint(m_rect.x + m_borderSize + square.col*(m_boxSize + m_borderSize) + m_borderSize,
                   m_rect.y + m_borderSize + square.row*(m_boxSize + m_borderSize) + m_borderSize);
}


#endif // MY_GRID_H
