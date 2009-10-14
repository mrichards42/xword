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


// For checking grid squares
enum
{
    MESSAGE_BOX      = 0x00,
    NO_REVEAL_ANSWER = 0x00,
    REVEAL_ANSWER    = 0x01,
    NO_MESSAGE_BOX   = 0x02,
    CHECK_ALL        = 0x04    // Inclue blanks
};


// Styles
enum GridStyle
{
    PAUSE_ON_SWITCH     = 0x0001,
    BLANK_ON_NEW_WORD   = 0x0002,
    MOVE_AFTER_LETTER   = 0x0004,
    MOVE_TO_NEXT_BLANK  = 0x0008,
    BLANK_ON_DIRECTION  = 0x0010,
    CONTEXT_MENU        = 0x0020,
    MOVE_ON_RIGHT_CLICK = 0x0040,
    CHECK_WHILE_TYPING  = 0x0080,

    DEFAULT_GRID_STYLE = PAUSE_ON_SWITCH
                       | MOVE_AFTER_LETTER
                       | BLANK_ON_NEW_WORD,

    GRID_STYLE_MASK = PAUSE_ON_SWITCH
                    | BLANK_ON_NEW_WORD
                    | MOVE_AFTER_LETTER
                    | MOVE_TO_NEXT_BLANK
                    | BLANK_ON_DIRECTION
                    | CONTEXT_MENU
                    | MOVE_ON_RIGHT_CLICK
                    | CHECK_WHILE_TYPING
};



extern const wxChar * XGridCtrlNameStr;

class XGridRebusHandler;

class XGridCtrl
    : public wxScrolledWindow
{
    friend class XGridRebusHandler;
public: // Enums
    // Colors
    enum Color
    {
        WHITE = 0,
        BLACK,
        LETTER,
        WORD,

        PEN,
        PENCIL,
        COLOR_COUNT
    };

public:
    XGridCtrl() { Init(); }

    explicit XGridCtrl(wxWindow * parent,
                      wxWindowID id = -1,
                      const wxPoint & pos = wxDefaultPosition,
                      const wxSize & size = wxDefaultSize,
                      XGrid * grid = NULL,
                      long style = DEFAULT_GRID_STYLE
                                 | wxVSCROLL | wxHSCROLL,
                      const wxString & name = XGridCtrlNameStr)
    {
        Init();
        Create(parent, id, pos, size, grid, style, name);
    }

    bool Create(wxWindow * parent,
                wxWindowID id = -1,
                const wxPoint & pos = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                XGrid * grid = NULL,
                long style = DEFAULT_GRID_STYLE
                           | wxVSCROLL | wxHSCROLL,
                const wxString & name = XGridCtrlNameStr);

    ~XGridCtrl();

    void SetPaused(bool pause = true) { m_isPaused = pause; Refresh(); }

    void SetFocus() { SetFocusIgnoringChildren(); }

    void SetXGrid(XGrid * grid);
          XGrid * GetXGrid()       { return m_grid; }
    const XGrid * GetXGrid() const { return m_grid; }
    bool IsEmpty() const { return m_grid == NULL || m_grid->IsEmpty(); }

    void RecheckGrid();

    // Will trigger an event
    XSquare * SetSquareFocus(XSquare * square, bool direction);
    XSquare * SetSquareFocus(XSquare * square = NULL)
        { return SetSquareFocus(square, m_direction); }

    void SetDirection(bool direction) { SetSquareFocus(NULL, direction); }
    void SetDirection()               { SetDirection( ! m_direction); }
    bool GetDirection() const { return m_direction; }

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

    void RefreshSquare() { wxASSERT(m_focusedSquare != NULL); RefreshSquare(*m_focusedSquare); }

    void RefreshWord()
    {
        wxClientDC dc(this);
        DoPrepareDC(dc);
        wxASSERT(m_focusedStart != NULL && m_focusedEnd != NULL);
        for (XSquare * square = m_focusedStart;
             square != m_focusedEnd->Next(m_direction);
             square = square->Next(m_direction))
        {
            DrawSquare(dc, *square);
        }
    }

    void SetSquareText(XSquare & square, const wxString & text = _T(""));

    XSquare * GetFocusedSquare() { return m_focusedSquare; }

    void CheckGrid  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckWord  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckLetter(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);

    // Fonts
    bool SetFont(const wxFont & font);
    void SetLetterFont(const wxFont & font);
    void SetNumberFont(const wxFont & font);

    // Colors
    void SetFocusedLetterColor(const wxColor & color)
        { m_colors[LETTER]  = color; if (! IsEmpty()) RefreshSquare();}
    void SetFocusedWordColor(const wxColor & color)
        { m_colors[WORD] = color; if (! IsEmpty()) RefreshWord(); }
    void SetWhiteSquareColor(const wxColor & color)  
        { m_colors[WHITE]   = color; if (! IsEmpty()) Refresh();}
    void SetBlackSquareColor(const wxColor & color)  
        { m_colors[BLACK]   = color; if (! IsEmpty()) Refresh();}
    void SetPenColor(const wxColor & color)
        { m_colors[PEN]  = color; if (! IsEmpty()) Refresh(); }
    void SetPencilColor(const wxColor & color)
        { m_colors[PENCIL] = color; if (! IsEmpty()) Refresh(); }

    const wxColor & GetFocusedLetterColor() const { return m_colors[LETTER]; }
    const wxColor & GetFocusedWordColor()   const { return m_colors[WORD]; }
    const wxColor & GetWhiteSquareColor()   const { return m_colors[WHITE]; }
    const wxColor & GetBlackSquareColor()   const { return m_colors[BLACK]; }
    const wxColor & GetPenColor()           const { return m_colors[PEN]; }
    const wxColor & GetPencilColor()        const { return m_colors[PENCIL]; }

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
    int   GetSquareSize()             { return m_boxSize + m_borderSize; }
    void  SetBorderSize(size_t size)  { m_borderSize = size; Scale(); Refresh(); }
    int   GetBorderSize()  const      { return m_borderSize; }

    void  SetNumberScale(double scale) { m_numberScale = scale; }
    void  SetLetterScale(double scale) { m_letterScale = scale; }
    double GetNumberScale() const      { return m_numberScale; }
    double GetLetterScale() const      { return m_letterScale; }

    int GetNumberHeight() const   { return m_boxSize * m_numberScale; }
    int GetLetterHeight() const   { return m_boxSize * m_letterScale; }

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
    bool IsCorrect();


    XSquare * GetClueNumber     (int num);

    template <typename T>
    XSquare * FindSquare(XSquare * start,
                         T type,
                         bool direction,
                         bool increment = FIND_NEXT,
                         bool skipBlack = NO_SKIP_BLACK_SQUARES,
                         bool wrapLines = NO_WRAP_LINES);

    template <typename T>
    XSquare * FindNextSquare(XSquare * start,
                             T type,
                             bool direction,
                             bool increment = FIND_NEXT,
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
    void DrawPauseMessage(wxDC & dc);
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
    void ScaleFont(wxFont * font, int width, int height);

    wxRect m_rect;              // Overall grid size
    int m_borderSize;           // Border between squares
    int m_boxSize;              // width/heigth of a square
    int m_lastBoxSize;          // The last non-fitted box size
    bool m_fit;                 // Fit the grid to the window?
    bool m_isPaused;            // Trigger a "Paused" message?

    wxFont m_numberFont;
    wxFont m_letterFont[8];  // Cache fonts for all rebus lengths
    double m_numberScale;
    double m_letterScale;

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
    wxColor m_colors[COLOR_COUNT];

    // Rebus
    bool m_wantsRebus;

private:
    // Events
    void ConnectEvents();
    void DisconnectEvents();

    void OnLeftDown    (wxMouseEvent & evt);
    void OnRightDown   (wxMouseEvent & evt);
    void OnKeyDown     (wxKeyEvent & evt);
    void OnChar        (wxKeyEvent & evt);

    void OnLetter      (wxChar key, int mod);
    void MoveAfterLetter();
    void OnArrow       (bool arrowDirection, bool increment, int mod);
    void OnTab         (int mod);
    void OnHome        (int mod);
    void OnEnd         (int mod);
    void OnBackspace   (int mod);
    void OnDelete      (int mod);
    void OnInsert      (int mod);

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(XGridCtrl)
    DECLARE_DYNAMIC_CLASS(XGridCtrl)
};







//---------------------------------------------------------------------
// Inline functions
//---------------------------------------------------------------------

// Fonts
inline bool
XGridCtrl::SetFont(const wxFont & font)
{
    SetNumberFont(font);
    SetLetterFont(font);
    Refresh();
    return wxScrolledWindow::SetFont(font);
}

inline void
XGridCtrl::SetNumberFont(const wxFont & font)
{
    m_numberFont = font;
    ScaleFont(&m_numberFont, -1, GetNumberHeight());
}

inline void
XGridCtrl::SetLetterFont(const wxFont & font)
{
    // 1 letter
    m_letterFont[0] = font;
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


inline bool
XGridCtrl::IsFocusedLetter(const XSquare & Square)
{
    return m_focusedSquare == &Square;
}




inline wxPoint
XGridCtrl::TopLeft(const XSquare & square)
{
    return wxPoint(m_rect.x + m_borderSize + square.GetCol() * GetSquareSize(),
                   m_rect.y + m_borderSize + square.GetRow() * GetSquareSize());
}


inline wxPoint
XGridCtrl::BottomRight(const XSquare & square)
{
    wxPoint pt = TopLeft(square);
    pt.x += GetSquareSize();
    pt.y += GetSquareSize();
    return pt;
}







template <typename T>
inline
XSquare *
XGridCtrl::FindSquare(XSquare * start,
                      T type,
                      bool direction,
                      bool increment,
                      bool skipBlack,
                      bool wrapLines)
{
    return m_grid->FindSquare(start, type, direction, increment,
                              skipBlack, wrapLines);
}


template <typename T>
inline
XSquare *
XGridCtrl::FindNextSquare(XSquare * start,
                          T type,
                          bool direction,
                          bool increment,
                          bool skipBlack,
                          bool wrapLines)
{
    return m_grid->FindNextSquare(start, type, direction, increment,
                                  skipBlack, wrapLines);
}

#endif // MY_GRID_H