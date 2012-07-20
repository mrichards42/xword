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

#ifndef MY_PRINTOUT_H
#define MY_PRINTOUT_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

#include <wx/print.h>
#include "html/render.hpp" // Html text printing
#include "XGridDrawer.hpp"
#include "puz/Puzzle.hpp"

class MyFrame;

class MyPrintout : public wxPrintout
{
public:
    MyPrintout(MyFrame * frame, puz::Puzzle * puz, int options, int numPages);
    ~MyPrintout();

    bool HasPage(int pageNum);
    wxString GetTitle();
    void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo);
    bool OnPrintPage(int pageNum);
    void OnPreparePrinting();

protected:
    void ReadConfig();
    bool LayoutPages();

    wxString GetHTML();
    MyHtmlDCRenderer * m_htmlRenderer;
    wxString m_html;

    puz::Puzzle * m_puz;
    MyFrame * m_frame;
    int m_numPages;

    // Fonts
    wxFont m_numberFont;
    wxFont m_clueFont;
    wxFont m_headingFont;
    wxFont m_authorFont;
    wxFont m_titleFont;
    void SetPointSize(int size);

    // Measuring / drawing member variables
    //-------------------------------------
    bool m_isDrawing;  // Are we drawing or just measuring?
    wxRect m_pageRect;
    wxRect m_gridRect;
    double m_gridScale;
    int m_columnWidth;
    int m_numberWidth;
    int m_clueWidth;
    int m_columns;
    int m_fontSize;
    int m_x, m_y; // Current position for text
    int m_w, m_h; // width and height of last-drawn text (set by DrawTextLine)

    // Grid
    XGridDrawer m_drawer;
    long m_gridAlign;

    void LayoutGrid(double gridScale);
    void DrawGrid();

    // Text
    bool LayoutText(int columns, int fontSize);
    bool DrawText(int columns, int fontSize);
    int GetNumberWidth();
    bool DrawTextLine(const wxString & text)
        { return DrawTextLine(text, m_columnWidth); }
    bool DrawTextLine(const wxString & text, int width);
    bool DrawClue(const puz::Clue & clue);
    bool AdjustColumn();
    void NewColumn();

    // Scaling
    bool m_isScaled;
    void ScaleDC();
    void UnscaleDC();
    bool IsScaled() const { return m_isScaled; }
};

#endif // MY_PRINTOUT_H
