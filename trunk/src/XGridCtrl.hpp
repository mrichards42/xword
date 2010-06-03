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


#ifndef MY_GRID_H
#define MY_GRID_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "puz/Grid.hpp"
#include "XGridDrawer.hpp"


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

class GridRebusHandler;
class GridSelectionHandler;
class GridSelectionEvent;
class GridCheckSelectionClass;

class XGridCtrl
    : public wxScrolledWindow
{
    friend class GridRebusHandler;
    friend class GridSelectionHandler;
    friend class GridCheckSelectionClass;

public: // Enums
    // Colors
    enum Color
    {
        LETTER = 0,
        SELECTION,
        WORD,

        PENCIL,
        COLOR_COUNT,
    };

public:
    XGridCtrl() { Init(); }

    explicit XGridCtrl(wxWindow * parent,
                      wxWindowID id = -1,
                      const wxPoint & pos = wxDefaultPosition,
                      const wxSize & size = wxDefaultSize,
                      puz::Grid * grid = NULL,
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
                puz::Grid * grid = NULL,
                long style = DEFAULT_GRID_STYLE
                           | wxVSCROLL | wxHSCROLL,
                const wxString & name = XGridCtrlNameStr);

    ~XGridCtrl();

    void SetPaused(bool pause = true) { m_isPaused = pause; Refresh(); }

    void SetFocus() { SetFocusIgnoringChildren(); }

    void SetGrid(puz::Grid * grid);
          puz::Grid * GetGrid()       { return m_grid; }
    const puz::Grid * GetGrid() const { return m_grid; }
    bool IsEmpty() const { return m_grid == NULL || m_grid->IsEmpty(); }

    void RecheckGrid();
    bool UnscrambleSolution(unsigned short key);

    // Will trigger an event
    puz::Square * SetSquareFocus(puz::Square * square, puz::GridDirection direction);
    puz::Square * SetSquareFocus(puz::Square * square = NULL)
        { return SetSquareFocus(square, m_direction); }

    void SetDirection(puz::GridDirection direction) { SetSquareFocus(NULL, direction); }
    puz::GridDirection GetDirection() const { return m_direction; }

    void SetFocusedClue   (int cluenum, puz::GridDirection direction)
        { ChangeFocusedClue(cluenum, direction); SetSquareFocus(); }
    void ChangeFocusedClue(int cluenum, puz::GridDirection direction);

    // Will not trigger an event
    puz::Square * ChangeSquareFocus(puz::Square * square, puz::GridDirection direction);
    puz::Square * ChangeSquareFocus(puz::Square * square = NULL)
        { return ChangeSquareFocus(square, m_direction); }

    void ChangeDirection(puz::GridDirection direction) { ChangeSquareFocus(NULL, direction); }
    void ChangeDirection() { ChangeDirection(m_direction == puz::ACROSS ? puz::DOWN : puz::ACROSS); }

    void RefreshSquare(wxDC & dc, puz::Square & square)  { DrawSquare(dc, square); }
    void RefreshSquare(puz::Square & square)
        { wxClientDC dc(this); DoPrepareDC(dc); RefreshSquare(dc, square); }

    void RefreshSquare() { wxASSERT(m_focusedSquare != NULL); RefreshSquare(*m_focusedSquare); }

    void RefreshWord()
    {
        wxClientDC dc(this);
        DoPrepareDC(dc);
        wxASSERT(m_focusedStart != NULL && m_focusedEnd != NULL);
        for (puz::Square * square = m_focusedStart;
             square != m_focusedEnd->Next(m_direction);
             square = square->Next(m_direction))
        {
            DrawSquare(dc, *square);
        }
    }

    bool SetSquareText(puz::Square & square, const wxString & text = _T(""));

    puz::Square * GetFocusedSquare() { return m_focusedSquare; }
    void GetFocusedWord(puz::Square ** start, puz::Square ** end)
        { *start = m_focusedStart; * end = m_focusedEnd; }

    // Create this class to start a selection and call a function afterward
    void StartSelection(wxObjectEventFunction func, wxEvtHandler * evtSink = NULL);
    std::vector<puz::Square *> GetSelection();

    void CheckGrid  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckSelection(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckWord  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckLetter(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);

    // Fonts
    bool SetFont(const wxFont & font);
    void SetLetterFont(const wxFont & font);
    void SetNumberFont(const wxFont & font);
    const wxFont & GetLetterFont() const { return m_drawer.GetLetterFont(); }
    const wxFont & GetNumberFont() const { return m_drawer.GetNumberFont(); }

    // Colors
    void SetFocusedLetterColor(const wxColor & color)
        { m_colors[LETTER]  = color; if (! IsEmpty()) RefreshSquare();}
    void SetFocusedWordColor(const wxColor & color)
        { m_colors[WORD] = color; if (! IsEmpty()) RefreshWord(); }
    void SetSelectionColor(const wxColor & color)
        { m_colors[SELECTION] = color; if (HasSelection()) Refresh(); }
    void SetWhiteSquareColor(const wxColor & color)  
        { m_drawer.SetWhiteSquareColor(color); if (! IsEmpty()) Refresh();}
    void SetBlackSquareColor(const wxColor & color)  
        { m_drawer.SetBlackSquareColor(color); if (! IsEmpty()) Refresh();}
    void SetPenColor(const wxColor & color)
        { m_drawer.SetPenColor(color); if (! IsEmpty()) Refresh(); }
    void SetPencilColor(const wxColor & color)
        { m_colors[PENCIL] = color; if (! IsEmpty()) Refresh(); }

    const wxColor & GetFocusedLetterColor() const { return m_colors[LETTER]; }
    const wxColor & GetFocusedWordColor()   const { return m_colors[WORD]; }
    const wxColor & GetSelectionColor()   const { return m_colors[SELECTION]; }
    const wxColor & GetWhiteSquareColor()   const
        { return m_drawer.GetWhiteSquareColor(); }
    const wxColor & GetBlackSquareColor()   const
        { return m_drawer.GetBlackSquareColor(); }
    const wxColor & GetPenColor()           const
        { return m_drawer.GetPenColor(); }
    const wxColor & GetPencilColor()        const { return m_colors[PENCIL]; }

    // Sizing / Drawing
    int    GetWidth()        const { return m_drawer.GetWidth(); }
    int    GetHeight()       const { return m_drawer.GetHeight(); }
    int    GetTop()          const { return m_drawer.GetTop(); }
    int    GetLeft()         const { return m_drawer.GetLeft(); }
    wxSize GetBestSize()     const { return m_drawer.GetBestSize(); }
    int    GetBoxSize()      const { return m_drawer.GetBoxSize(); }
    int    GetBorderSize()   const { return m_drawer.GetBorderSize(); }
    int    GetSquareSize()   const { return m_drawer.GetSquareSize(); }
    double GetLetterScale()  const { return m_drawer.GetLetterScale(); }
    double GetNumberScale()  const { return m_drawer.GetNumberScale(); }
    int    GetNumberHeight() const { return m_drawer.GetNumberHeight(); }
    int    GetLetterHeight() const { return m_drawer.GetLetterHeight(); }

    void SetBoxSize(int size)         { m_drawer.SetBoxSize(size); }
    void SetBorderSize(int size)      { m_drawer.SetBorderSize(size); }
    void SetLetterScale(double scale) { m_drawer.SetLetterScale(scale); }
    void SetNumberScale(double scale) { m_drawer.SetNumberScale(scale); }

    // Zooming
    void FitGrid(bool fit = true)
    {
        Freeze();
        m_fit = fit;
        Scale();
        Thaw();
        Refresh();
    }

    void ZoomIn (double factor = 1.1)
    {
        Freeze();
        m_fit = false;
        Scale(factor);
        Refresh();
        Thaw();
    }

    void ZoomOut(double factor = 1.1) { ZoomIn(1/factor); }

    // Style
    void SetGridStyle(int style)
        { SetWindowStyle( GetWindowStyle() & ~ GRID_STYLE_MASK | style ); }
    int  GetGridStyle() const  { return GetWindowStyle() & GRID_STYLE_MASK; }
    bool HasStyle(int style) const { return HasFlag(style); }

    // Information functions
    const puz::Square & At(int col, int row) const { return m_grid->At(col, row); }
          puz::Square & At(int col, int row)       { return m_grid->At(col, row); }

    int GetBlankCount()     const { return m_blankSquares; }
    int GetIncorrectCount() const { return m_incorrectSquares; }
    bool IsCorrect();


    puz::Square * GetClueNumber     (int num);

    template <typename T>
    puz::Square * FindSquare(puz::Square * start,
                             T type,
                             puz::GridDirection direction,
                             puz::FindDirection increment = puz::NEXT,
                             bool skipBlack = puz::NO_SKIP_BLACK_SQUARES,
                             bool wrapLines = puz::NO_WRAP_LINES);

    template <typename T>
    puz::Square * FindNextSquare(puz::Square * start,
                                 T type,
                                 puz::GridDirection direction,
                                 puz::FindDirection increment = puz::NEXT,
                                 bool skipBlack = puz::NO_SKIP_BLACK_SQUARES,
                                 bool wrapLines = puz::NO_WRAP_LINES);

    bool IsFocusedLetter(const puz::Square & square);
    bool IsFocusedWord  (const puz::Square & square);

    bool HasSelection() { return m_selectionStart != NULL; }
    bool IsSelected(const puz::Square & square);

    void HitTest(int x, int y, int * col, int * row);
    puz::Square * HitTest(int x, int y);
    puz::Square * HitTestNearest(int x, int y);

protected:
    void Init();

    // Common CheckXXX function
    void PostCheck(std::vector<puz::Square *> & incorrect, int options);
    void DoCheckSelection(puz::Square * start, puz::Square * end, int options);

    // Drawing functions
    void OnPaint(wxPaintEvent & evt);
    void DrawPauseMessage(wxDC & dc);
    void DrawGrid(wxDC & dc, const wxRegion & updateRegion = wxRegion());

    void DrawSquare(wxDC & dc, const puz::Square & square, const wxColour & color);

    void DrawSquare(wxDC & dc, const puz::Square & square)
        { DrawSquare(dc, square, GetSquareColor(square)); }
    void DrawSquare(wxDC & dc, int col, int row, const wxColour & color)
        { DrawSquare(dc, At(col, row), color); }
    void DrawSquare(wxDC & dc, int col, int row)
        { DrawSquare(dc, At(col, row)); }

    const wxColor & GetSquareColor(const puz::Square & square);

    void MakeVisible(const puz::Square & square);


    // Scaling functions and members
    void OnSize(wxSizeEvent & evt);
    void Scale(double factor = 1.0);
    void ScaleFont(wxFont * font, int width, int height);


    XGridDrawer m_drawer;
    int m_lastBoxSize;          // The last non-fitted box size
    bool m_fit;                 // Fit the grid to the window?
    bool m_isPaused;            // Trigger a "Paused" message?

    // Pointer to puzzle data
    puz::Grid * m_grid;

    // Focused square/word
    puz::Square * m_focusedSquare;
    puz::Square * m_focusedStart;
    puz::Square * m_focusedEnd;
    // A selection block (NULL if there is no selection)
    puz::Square * m_selectionStart;
    puz::Square * m_selectionEnd;
    bool m_isSelecting;
    void EndSelection(bool success = true);

    // Focused direction
    puz::GridDirection m_direction;

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
    bool m_areEventsConnected;

    void OnLeftDown    (wxMouseEvent & evt);
    void OnRightDown   (wxMouseEvent & evt);
    void OnKeyDown     (wxKeyEvent & evt);
    void OnChar        (wxKeyEvent & evt);

    void OnLetter      (wxChar key, int mod);
    void MoveAfterLetter();
    void OnArrow       (puz::GridDirection arrowDirection, puz::FindDirection increment, int mod);
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
    m_drawer.SetNumberFont(font);
}

inline void
XGridCtrl::SetLetterFont(const wxFont & font)
{
    m_drawer.SetLetterFont(font);
}


inline bool
XGridCtrl::IsFocusedLetter(const puz::Square & square)
{
    return m_focusedSquare == &square;
}

inline bool
XGridCtrl::IsFocusedWord(const puz::Square & square)
{
    return m_grid->IsBetween(&square, m_focusedStart, m_focusedEnd);
}

inline bool
XGridCtrl::IsSelected(const puz::Square & square)
{
    wxASSERT(HasSelection());
    return m_grid->IsBetween(&square, m_selectionStart, m_selectionEnd);
}







template <typename T>
inline
puz::Square *
XGridCtrl::FindSquare(puz::Square * start,
                      T type,
                      puz::GridDirection direction,
                      puz::FindDirection increment,
                      bool skipBlack,
                      bool wrapLines)
{
    return m_grid->FindSquare(start, type, direction, increment,
                              skipBlack, wrapLines);
}


template <typename T>
inline
puz::Square *
XGridCtrl::FindNextSquare(puz::Square * start,
                          T type,
                          puz::GridDirection direction,
                          puz::FindDirection increment,
                          bool skipBlack,
                          bool wrapLines)
{
    return m_grid->FindNextSquare(start, type, direction, increment,
                                  skipBlack, wrapLines);
}

#endif // MY_GRID_H