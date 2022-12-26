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

#ifndef MY_GRID_DRAWER_H
#define MY_GRID_DRAWER_H

#include <map>

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

// Forward declarations
namespace puz {
    class Grid;
    class Square;
    class Puzzle;
}

const int TRUNCATED_REBUS_LENGTH = 10;

//=============================================================================
// XGridDrawer is a class designed to scale and draw various parts of the
// crossword grid.  In order to measure text sizes, XGridDrawer needs a 
// pointer to a wxWindow or a wxDC.  These are stored in an anonymous union,
// and can be accessed through the HasDC / HasWindow and GetDC / GetWindow
// functions.
//=============================================================================

class XGridDrawer
{
public:
    XGridDrawer(puz::Puzzle * puz = NULL);
    XGridDrawer(wxDC * dc, puz::Puzzle * puz = NULL);
    XGridDrawer(wxWindow * window, puz::Puzzle * puz = NULL);

    void SetDC(wxDC * dc);
    void SetWindow(wxWindow * window);
    bool HasMeasurer() const { return m_status != INVALID_MEASURER; }
    bool HasDC()       const { return m_status == HAS_DC; }
    bool HasWindow()   const { return m_status == HAS_WINDOW; }
    wxDC * GetDC()           { wxASSERT(HasDC()); return m_dc; }
    wxWindow * GetWindow()   { wxASSERT(HasWindow()); return m_window; }

    // Sizes
    //------

    const wxRect & GetRect() const { return m_rect; }
    wxRect GetSquareRect(const puz::Square & square) const;
    wxRect GetTextRect(const wxRect & square) const;
    int    GetWidth()        const { return m_rect.GetWidth(); }
    int    GetHeight()       const { return m_rect.GetHeight(); }
    int    GetTop()          const { return m_rect.GetTop(); }
    int    GetLeft()         const { return m_rect.GetLeft(); }
    wxSize GetBestSize()     const;
    long   GetAlign()        const { return m_align; }
    int    GetBoxSize()      const { return m_boxSize; }
    int    GetBorderSize()   const { return m_borderSize; }
    int    GetSquareSize()   const { return m_boxSize + m_borderSize; }
    double GetLetterScale()  const { return m_letterScale; }
    double GetNumberScale()  const { return m_numberScale; }
    int    GetNumberHeight() const { return m_boxSize * m_numberScale; }
    int    GetLetterHeight() const { return m_boxSize * m_letterScale; }

    bool SetMaxSize(const wxSize & size) { return SetMaxSize(size.x, size.y); }
    bool SetMaxSize(size_t max_width, size_t max_height);
    void SetAlign(long align) { m_align = align; UpdateGridSize(); }
    void SetBoxSize(int size);
    void SetBorderSize(int size);
    void SetLetterScale(double scale);
    void SetNumberScale(double scale);

    // Fonts
    //------
    const wxFont & GetLetterFont()   const { return m_letterFont[0]; }
    const wxFont & GetNumberFont()   const { return m_numberFont; }
    bool HasSymbolFont() const { return m_hasSymbolFont; }

    void SetLetterFont(const wxFont & font);
    void SetNumberFont(const wxFont & font);

    // Colors
    //-------
    wxColour GetSquareColor(const puz::Square & square) const;
    const wxColour & GetWhiteSquareColor() const { return m_whiteSquareColor; }
    const wxColour & GetBlackSquareColor() const { return m_blackSquareColor; }
    const wxColour & GetHighlightColor()   const { return m_highlightColor; }
    const wxColour & GetThemeColor()       const { return m_themeColor; }
    const wxColour & GetPenColor()         const { return m_penColor; }
    const wxColour & GetCorrectColor()     const { return m_correctColor; }
    const wxColour & GetCheckedColor()     const { return m_checkedColor; }
    const wxColour & GetRevealedColor()    const { return m_revealedColor; }


    void SetWhiteSquareColor(const wxColour & color)
        { m_whiteSquareColor = color; }
    void SetBlackSquareColor(const wxColour & color)
        { m_blackSquareColor = color; UpdateHighlightColor(); }
    void SetThemeColor(const wxColour & color) { m_themeColor = color; }
    void SetPenColor(const wxColour & color) { m_penColor = color; }
    void SetCorrectColor(const wxColour & color) { m_correctColor = color; }
    void SetCheckedColor(const wxColour & color) { m_checkedColor = color; }
    void SetRevealedColor(const wxColour & color) { m_revealedColor = color; }

    void UpdateHighlightColor();

    // Selective Drawing
    //------------------
    enum
    {
        DRAW_USER_TEXT  = 0x01,
        DRAW_SOLUTION   = 0x02,
        DRAW_NUMBER     = 0x04,
        DRAW_FLAG       = 0x08,
        DRAW_X          = 0x10,
        DRAW_CIRCLE     = 0x20,
        DRAW_OUTLINE    = 0x40,
        DRAW_BLANK_DIAGRAMLESS = 0x80,
        DRAW_THEME      = 0x100,

        DRAW_ALL = DRAW_USER_TEXT
                 | DRAW_NUMBER
                 | DRAW_FLAG
                 | DRAW_X
                 | DRAW_CIRCLE
                 | DRAW_THEME,

        // Text Mask
        DRAW_TEXT = DRAW_USER_TEXT
                  | DRAW_SOLUTION
    };
    int  GetFlags  ()         const { return m_drawOptions; }
    bool HasFlag   (int flag) const { return (m_drawOptions & flag) != 0; }
    void AddFlag   (int flag)       { m_drawOptions |= flag; }
    void RemoveFlag(int flag)       { m_drawOptions &= ~ flag; }
    void SetFlags  (int flags)      { m_drawOptions = flags; }

    void ShowThemeHighlight(bool doit = true)
    {
        if (doit)
            AddFlag(DRAW_THEME);
        else
            RemoveFlag(DRAW_THEME);
    }

    // Grid
    //------
    puz::Puzzle * GetPuzzle() { return m_puz; }
    const puz::Puzzle * GetPuzzle() const { return m_puz; }
    puz::Grid * GetGrid();
    const puz::Grid * GetGrid() const;
    void SetPuzzle(puz::Puzzle * puz)
        { m_puz = puz; UpdateGridSize(); m_imageMap.clear(); }

    void DrawSquare(wxDC & dc, const puz::Square & square,
                    const wxColour & bgColor,
                    const wxColour & textColor,
                    const bool blendBackground = false);

    void DrawSquare(wxDC & dc, const puz::Square & square);

    void DrawGrid(wxDC & dc);

private:
    void Init();

    // The two different measuring windows
    union {
        wxDC * m_dc;
        wxWindow * m_window;
    };
    short m_status;
    // Status for the union
    enum
    {
        INVALID_MEASURER,
        HAS_DC,
        HAS_WINDOW
    };

    // Get text extent with whatever measurer exists
    void GetTextExtent(const wxString & string, int* w, int* h,
                       int* descent = NULL, int* externalLeading = NULL,
                       wxFont* font = NULL) const;

    puz::Grid * m_grid;
    puz::Puzzle * m_puz;

    // Size
    long m_align;
    int m_boxSize;
    int m_borderSize;
    double m_numberScale;
    double m_letterScale;
    wxRect m_rect;
    size_t m_maxWidth;
    size_t m_maxHeight;

    void UpdateGridSize();

    // Fonts
    wxFont m_numberFont;
    wxFont m_letterFont[4];  // Cache fonts for text (1, 2, 3, 4+ letters)
    wxFont m_symbolFont;
    bool m_hasSymbolFont;

    void ScaleFont(wxFont * font, int maxWidth, int maxHeight);
    void ScaleFonts() { ScaleNumberFont(); ScaleLetterFont(); ScaleSymbolFont(); }
    void ScaleNumberFont() { ScaleFont(&m_numberFont, -1, GetNumberHeight()); }
    void ScaleLetterFont();
    void ScaleSymbolFont();
    static double GetScale(int width, int height, int maxWidth, int maxHeight);

    // Colors
    wxColour m_whiteSquareColor;
    wxColour m_blackSquareColor;
    wxColour m_highlightColor;
    wxColour m_themeColor;
    wxColour m_penColor;
    wxColour m_revealedColor;
    wxColour m_correctColor;
    wxColour m_checkedColor;

    // Images
    std::map<const puz::Square *, wxImage> m_imageMap;

    // Selective Drawing
    int m_drawOptions;
};

#endif // MY_GRID_DRAWER_H
