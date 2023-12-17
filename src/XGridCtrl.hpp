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


#ifndef MY_GRID_H
#define MY_GRID_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "puz/Grid.hpp"
#include "puz/Word.hpp"
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
    MOVE_ON_RIGHT_CLICK = 0x0020,
    CHECK_WHILE_TYPING  = 0x0040,
    STRICT_REBUS        = 0x0080,
    CONTEXT_MENU        = 0x0100, // Not yet(?) implemented
    SWAP_ON_DCLICK      = 0x0200,
    SWAP_ON_SPACE       = 0x0400,

    DEFAULT_GRID_STYLE = PAUSE_ON_SWITCH
                       | MOVE_AFTER_LETTER
                       | BLANK_ON_NEW_WORD
                       | STRICT_REBUS,

    GRID_STYLE_MASK = PAUSE_ON_SWITCH
                    | BLANK_ON_NEW_WORD
                    | MOVE_AFTER_LETTER
                    | MOVE_TO_NEXT_BLANK
                    | BLANK_ON_DIRECTION
                    | CONTEXT_MENU
                    | MOVE_ON_RIGHT_CLICK
                    | CHECK_WHILE_TYPING
                    | STRICT_REBUS
                    | SWAP_ON_DCLICK
                    | SWAP_ON_SPACE
};

enum CorrectStatus
{
    UNCHECKABLE_PUZZLE = -2,
    INCOMPLETE_PUZZLE = -1,
    INCORRECT_PUZZLE, // = false
    CORRECT_PUZZLE // = true
};

struct GridStats
{
    CorrectStatus correct;
    int blank;
    int black;
    int white;
    // blank_correct squares are blank with a blank solution
    int blank_correct;
};


// Forward declarations
namespace puz {
    class Puzzle;
}
extern const wxChar * XGridCtrlNameStr;

class XGridRebusCtrl;
class GridSelectionHandler;
class GridSelectionEvent;
class GridCheckSelectionClass;
class ConfigManager;

class XGridCtrl
    : public wxScrolledCanvas
{
    friend class XGridRebusCtrl;
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
                      puz::Puzzle * puz = NULL,
                      long style = DEFAULT_GRID_STYLE
                                 | wxVSCROLL | wxHSCROLL,
                      const wxString & name = XGridCtrlNameStr)
    {
        Init();
        Create(parent, id, pos, size, puz, style, name);
    }

    bool Create(wxWindow * parent,
                wxWindowID id = -1,
                const wxPoint & pos = wxDefaultPosition,
                const wxSize & size = wxDefaultSize,
                puz::Puzzle * puz = NULL,
                long style = DEFAULT_GRID_STYLE
                           | wxVSCROLL | wxHSCROLL,
                const wxString & name = XGridCtrlNameStr);

    ~XGridCtrl();

    void SetPaused(bool pause = true)
    {
        const bool refresh = (pause != m_isPaused);
        m_isPaused = pause;
        if (refresh)
            Refresh();
    }

    void SetFocus();

    void SetPuzzle(puz::Puzzle * puz);
          puz::Grid * GetGrid()       { return m_grid; }
    const puz::Grid * GetGrid() const { return m_grid; }
    bool IsEmpty() const { return m_grid == NULL || m_grid->IsEmpty(); }

    bool UnscrambleSolution(unsigned short key);

    short OppositeFocusDirection(puz::Square* square, puz::Word * word, short direction);

    // Set* functions trigger an event.
    // Move* functions check BLANK_ON_NEW_WORD
    // Essentially this is SetFocusedSquare([square, ][word, ][direction])
    puz::Square * SetFocusedSquare(puz::Square *, puz::Word *, short);
    puz::Square * SetFocusedSquare(puz::Square * square, short direction)
        { return SetFocusedSquare(square, NULL, direction); }
    puz::Square * SetFocusedSquare(puz::Square * square = NULL, puz::Word * word = NULL)
        { return SetFocusedSquare(square, word, m_focusedDirection); }

    puz::Square * MoveFocusedSquare(puz::Square *, puz::Word *, short);
    puz::Square * MoveFocusedSquare(puz::Square * square, short direction)
        { return MoveFocusedSquare(square, NULL, direction); }
    puz::Square * MoveFocusedSquare(puz::Square * square, puz::Word * word = NULL)
        { return MoveFocusedSquare(square, word, m_focusedDirection); }
    puz::Square * MoveFocusedWord(puz::Word * word)
        { return MoveFocusedSquare(word->front(), word, m_focusedDirection); }

    void SendEvent(int type);

    short GetDirection() const;

    void SetFocusedWord(puz::Word * word, short direction);
    void SetFocusedWord(puz::Word * word)
        { SetFocusedWord(word, m_focusedDirection); }

    void SetFocusedDirection(short direction)
        { SetFocusedSquare(NULL, direction); }

    // Drawing functions
    void RefreshSquare(wxDC & dc, const puz::Square & square)  { DrawSquare(dc, square); }
    void RefreshSquare(const puz::Square & square)
        { wxClientDC dc(this); DoPrepareDC(dc); RefreshSquare(dc, square); }

    void RefreshSquare() { wxASSERT(m_focusedSquare != NULL); RefreshSquare(*m_focusedSquare); }

    void RefreshWord()
    {
        wxClientDC dc(this);
        DoPrepareDC(dc);
        puz::square_iterator it;
        for (it = m_focusedWord->begin(); it != m_focusedWord->end(); ++it)
            DrawSquare(dc, *it);
    }

    bool SetSquareText(puz::Square & square, const wxString & text = _T(""));

    const puz::Square * GetFocusedSquare() const { return m_focusedSquare; }
          puz::Square * GetFocusedSquare()       { return m_focusedSquare; }
    const puz::Word * GetFocusedWord() const { return m_focusedWord; }
          puz::Word * GetFocusedWord()       { return m_focusedWord; }

    const puz::Square * FirstWhite() const;
          puz::Square * FirstWhite();
    const puz::Square * LastWhite() const;
          puz::Square * LastWhite();

    // Create this class to start a selection and call a function afterward
    void StartSelection(wxObjectEventFunction func, wxEvtHandler * evtSink = NULL);
    std::vector<puz::Square *> GetSelection();

    void CheckGrid  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckSelection(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckWord  (int options = NO_REVEAL_ANSWER | MESSAGE_BOX);
    void CheckLetter(int options = NO_REVEAL_ANSWER | MESSAGE_BOX);

    XGridDrawer & GetDrawer() { return m_drawer; }

    // Fonts
    bool SetFont(const wxFont & font);
    void SetLetterFont(const wxFont & font);
    void SetNumberFont(const wxFont & font);
    const wxFont & GetLetterFont() const { return m_drawer.GetLetterFont(); }
    const wxFont & GetNumberFont() const { return m_drawer.GetNumberFont(); }

    // Colors
    void SetFocusedLetterColor(const wxColor & color)
        { m_colors[LETTER]  = color; if (! IsEmpty()) RefreshSquare(); }
    void SetFocusedWordColor(const wxColor & color)
        { m_colors[WORD] = color; if (! IsEmpty()) RefreshWord(); }
    void SetSelectionColor(const wxColor & color)
        { m_colors[SELECTION] = color; if (HasSelection()) Refresh(); }
    void SetWhiteSquareColor(const wxColor & color)  
        { m_drawer.SetWhiteSquareColor(color); if (! IsEmpty()) Refresh(); }
    void SetBlackSquareColor(const wxColor & color)  
        { m_drawer.SetBlackSquareColor(color); if (! IsEmpty()) Refresh(); }
    void ShowThemeHighlight(bool doit = true)
        { m_drawer.ShowThemeHighlight(doit); if (! IsEmpty()) Refresh(); }
    void SetThemeSquareColor(const wxColor & color)  
        { m_drawer.SetThemeColor(color); if (! IsEmpty()) Refresh(); }
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
    const wxColor & GetThemeSquareColor()   const
        { return m_drawer.GetThemeColor(); }
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

    void SetBoxSize(int size)         { m_drawer.SetBoxSize(size); Scale(); }
    void SetBorderSize(int size)      { m_drawer.SetBorderSize(size); Scale(); if (! IsEmpty()) Refresh(); }
    void SetLetterScale(long scale)   { m_drawer.SetLetterScale(scale / 100.); if (! IsEmpty()) Refresh(); }
    void SetNumberScale(long scale)   { m_drawer.SetNumberScale(scale / 100.); if (! IsEmpty()) Refresh(); }

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
        { SetWindowStyle( (GetWindowStyle() & ~ GRID_STYLE_MASK) | style ); }
    int  GetGridStyle() const  { return GetWindowStyle() & GRID_STYLE_MASK; }
    bool HasStyle(int style) const { return HasFlag(style); }

    // Information functions
    const puz::Square & At(int col, int row) const { return m_grid->At(col, row); }
          puz::Square & At(int col, int row)       { return m_grid->At(col, row); }

    CorrectStatus IsCorrect() const;
    void GetStats(GridStats * stats) const;

    puz::Square * GetClueNumber     (const wxString & num);

    bool IsFocusedLetter(const puz::Square & square);
    bool IsFocusedWord  (const puz::Square & square);

    bool HasSelection() { return m_selectionStart != NULL; }
    bool IsSelected(const puz::Square & square);

    void HitTest(int x, int y, int * col, int * row);
    puz::Square * HitTest(int x, int y);
    puz::Square * HitTestNearest(int x, int y);

    // Rebus
    void StartRebusEntry();
    void EndRebusEntry(bool success = true);
    bool IsRebusEntry() const { return m_rebusCtrl != NULL; }

    // Misc
    static bool IsValidChar(wxChar ch)
        { return puz::Square::IsValidChar(static_cast<int>(ch)); }

    void ConnectEvents();
    void DisconnectEvents();

protected:
    void Init();

    // Common CheckXXX function
    void DoCheckSelection(puz::Square * start, puz::Square * end, int options);
    void Check(puz::square_iterator begin, puz::square_iterator end, int options);
    bool CheckSquare(puz::Square * square, int options, wxDC & dc);

    // Drawing functions
    void OnPaint(wxPaintEvent & evt);
    void DrawPauseMessage(wxDC & dc);
    void DrawGrid(wxDC & dc, const wxRegion & updateRegion = wxRegion());

    void DrawSquare(wxDC & dc, const puz::Square & square, const wxColour & color, bool propagate = true);

    void DrawSquare(wxDC & dc, const puz::Square & square)
        { DrawSquare(dc, square, GetSquareColor(square)); }
    void DrawSquare(wxDC & dc, int col, int row, const wxColour & color)
        { DrawSquare(dc, At(col, row), color); }
    void DrawSquare(wxDC & dc, int col, int row)
        { DrawSquare(dc, At(col, row)); }

    const wxColor & GetSquareColor(const puz::Square & square);

    // Return true if we had to scroll
    bool MakeVisible(const puz::Square & square);


    // Scaling functions and members
    void OnSize(wxSizeEvent & evt);
    void Scale(double factor = 1.0);
    void ScaleFont(wxFont * font, int width, int height);


    XGridDrawer m_drawer;
    int m_lastBoxSize;          // The last non-fitted box size
    bool m_fit;                 // Fit the grid to the window?
    bool m_isPaused;            // Trigger a "Paused" message?

    // Pointer to puzzle data
    puz::Puzzle * m_puz;
    puz::Grid * m_grid;

    // Focused info
    puz::Square * m_focusedSquare;
    puz::Word * m_focusedWord;
    short m_focusedDirection;
    void RecalcDirection();
    // Make sure that we have a focused word, even if we have to invent one.
    void DoSetFocusedWord(puz::Square *, puz:: Word *, short);
    void DoSetFocusedWord(puz::Square * square, puz:: Word * word)
        { DoSetFocusedWord(square, word, m_focusedDirection); }
    bool m_ownsFocusedWord;
    puz::Word * CalcFocusedWord(puz::Square *, puz::Word *, short);
    // A selection block (NULL if there is no selection)
    puz::Square * m_selectionStart;
    puz::Square * m_selectionEnd;
    bool m_isSelecting;
    void EndSelection(bool success = true);

    // Square colors
    wxColor m_colors[COLOR_COUNT];

    // Rebus
    XGridRebusCtrl * m_rebusCtrl;

private:
    // Events
    bool m_areEventsConnected;

    void OnLeftDown    (wxMouseEvent & evt);
    void OnLeftDClick  (wxMouseEvent & evt);
    void OnRightDown   (wxMouseEvent & evt);
    void OnContextMenu (wxContextMenuEvent & evt);
    void OnMouseMove   (wxMouseEvent & evt);
    void OnKeyDown     (wxKeyEvent & evt);
    void OnChar        (wxKeyEvent & evt);

    void OnLetter       (wxChar key, int mod);
    void MoveAfterLetter();
    void OnArrow        (puz::GridDirection arrowDirection, int mod);
    void OnTab          (int mod);
    void OnHome         (int mod);
    void OnEnd          (int mod);
    void OnBackspace    (int mod);
    void OnDelete       (int mod);
    void OnRebusKey     (int mod);

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
    return wxScrolledCanvas::SetFont(font);
}

inline void
XGridCtrl::SetNumberFont(const wxFont & font)
{
    m_drawer.SetNumberFont(font);
    Refresh();
}

inline void
XGridCtrl::SetLetterFont(const wxFont & font)
{
    m_drawer.SetLetterFont(font);
    Refresh();
}


inline bool
XGridCtrl::IsFocusedLetter(const puz::Square & square)
{
    return m_focusedSquare == &square;
}

inline bool
XGridCtrl::IsFocusedWord(const puz::Square & square)
{
    return m_focusedWord && m_focusedWord->Contains(&square);
}

inline bool
XGridCtrl::IsSelected(const puz::Square & square)
{
    wxASSERT(HasSelection());
    return square.IsBetween(m_selectionStart, m_selectionEnd);
}


#endif // MY_GRID_H
