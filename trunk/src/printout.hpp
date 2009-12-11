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

#ifndef MY_PRINTOUT_H
#define MY_PRINTOUT_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/frame.h>
#endif

#include <wx/print.h>
#include "XGridDrawer.hpp"
#include "puz/XPuzzle.hpp"

class MyFrame;

class MyPrintout : public wxPrintout
{
public:
    MyPrintout(MyFrame * frame, XPuzzle * puz, int numPages);

    bool HasPage(int pageNum);
    wxString GetTitle();
    void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo);
    bool OnPrintPage(int pageNum);
    void OnPreparePrinting();

protected:
    void ReadConfig();
    void LayoutPages();

    XPuzzle * m_puz;
    MyFrame * m_frame;
    int m_numPages;

    // Fonts
    wxFont m_numberFont;
    wxFont m_clueFont;
    wxFont m_headingFont;
    wxFont m_authorFont;
    wxFont m_titleFont;
    bool m_fontSizeDontCare;

    void SetupFonts();

    // Measuring / drawing member variables
    //-------------------------------------
    bool m_isDrawing;  // Are we drawing or just measuring?
    wxRect m_pageRect;

    // Grid
    XGridDrawer m_drawer;
    double m_gridScale;
    long m_gridAlign;
    wxRect m_gridRect;
    int m_minBoxSize;

    void LayoutGrid(double gridScale = 0.75);
    void DrawGrid();

    // Text
    int m_columnWidth;
    int m_numberWidth;
    int m_clueWidth;
    int m_minFontSize;

    void DrawText();
    void LayoutColumns();
    void DrawClue(const XPuzzle::Clue & clue, int * x, int * y);
    void DrawTextLine(const wxString & text,
                      int * x, int * y,
                      int * width=NULL, int * height=NULL);
    wxString WrapText(const wxString & text, int maxWidth);
    void AdjustColumn(int * x, int * y, int textWidth, int textHeight);
    void NewColumn(int * x, int * y);

    // In order to get more accurate text measuring results, we have to do
    // the measuring at the device scale, not the user scale.
    double m_scaleX;
    double m_scaleY;
    void SaveUserScale();
    void RestoreUserScale();
    void MeasureText(const wxString & text, int * width, int * height);
};

#endif // MY_PRINTOUT_H
