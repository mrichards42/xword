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

#include "printout.hpp"
#include "puz/XPuzzle.hpp"
#include "App.hpp" // For the global print data pointers, and ConfigManager
#include "utils/wrap.hpp"
#include "MyFrame.hpp"
#include "CluePanel.hpp"
#include "XGridCtrl.hpp"
#include <wx/busyinfo.h>

// Constants
// The actual grid size will be GRID_SIZE - GRID_PADDING
const double GRID_SIZE = 4. / 5.;
const double GRID_PADDING = 0.005;
const int NUMBER_PADDING = 1;
const int COLUMN_PADDING = 0;
const int LINE_PADDING = 0;
const int CLUE_HEADING_PADDING = 5;
// Absolute minimum
const int MIN_POINT_SIZE = 6;
// Minimum before a new grid size is tried
const int MIN_GOOD_POINT_SIZE = 8;

wxString FormatNumber(int num)
{
    return wxString::Format(_T("%d."), num);
}

// Thrown when the text runs off the page.
class FontSizeError
{
public:
    FontSizeError() {}
};



//--------------------------------------------------------------------------------
// Setup functions
//--------------------------------------------------------------------------------

MyPrintout::MyPrintout(MyFrame * frame, XPuzzle * puz, int numPages)
    : wxPrintout(puz->m_title),
      m_frame(frame),
      m_puz(puz),
      m_drawer(&puz->m_grid),
      m_numPages(numPages)
{
    // Don't draw any grid text, incorrect / revealed triangles, Xs for
    // incorrect answers, etc.
    m_drawer.SetFlags(XGridDrawer::DRAW_CIRCLE
                    | XGridDrawer::DRAW_NUMBER);

    m_fontSizeDontCare = false;

    ReadConfig();
}

void
MyPrintout::ReadConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();

    // Fonts config
    config.SetPath(_T("/Printing/Fonts"));
    if (config.ReadBool(_T("useCustomFonts")))
    {
        m_drawer.SetNumberFont(config.ReadFont(_T("gridNumberFont")));
        m_drawer.SetLetterFont(config.ReadFont(_T("gridLetterFont")));
        m_clueFont = config.ReadFont(_T("clueFont"));
    }
    else
    {
        m_drawer.SetNumberFont(m_frame->m_gridCtrl->GetNumberFont());
        m_drawer.SetLetterFont(m_frame->m_gridCtrl->GetLetterFont());
        m_clueFont = m_frame->m_across->GetFont();
    }
    m_drawer.SetNumberScale(config.ReadLong(_T("/Grid/numberScale")) / 100.);
    m_drawer.SetLetterScale(config.ReadLong(_T("/Grid/letterScale")) / 100.);

    SetupFonts();

    // Grid config
    config.SetPath(_T("/Printing"));
    m_gridAlign  = config.ReadLong(_T("gridAlignment"));

    const long brightness = config.ReadLong(_T("blackSquareBrightness"));
    m_drawer.SetBlackSquareColor(wxColour(brightness, brightness, brightness));
}


void
MyPrintout::SetupFonts()
{
    m_numberFont = m_clueFont;
    m_headingFont = m_clueFont;
    m_headingFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_authorFont = m_clueFont;
    m_authorFont.SetPointSize(m_authorFont.GetPointSize() + 1);
    m_titleFont = m_authorFont;
    m_titleFont.SetWeight(wxFONTWEIGHT_BOLD);
}


//--------------------------------------------------------------------------------
// Print framework functions
//--------------------------------------------------------------------------------
bool
MyPrintout::HasPage(int pageNum)
{
    return pageNum <= m_numPages;
}


wxString
MyPrintout::GetTitle()
{
    return m_puz->m_title;
}

void
MyPrintout::GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
{
    *minPage = 1;
    *pageFrom = 1;
    *maxPage = m_numPages;
    *pageTo  = m_numPages;
}

bool
MyPrintout::OnPrintPage(int pageNum)
{
    wxLogDebug(_T("Printing page %d"), pageNum);
    m_drawer.SetDC(GetDC());
    // Turn drawing on.
    m_isDrawing = true;

    LayoutGrid(m_gridScale);
    DrawGrid();
    DrawText();
    wxLogDebug(_T("Done printing page %d"), pageNum);
    return true;
}

void
MyPrintout::OnPreparePrinting()
{
    wxLogDebug(_T("OnPreparePrinting"));
    wxLogDebug(_T("DC Size: %d, %d"), GetDC()->GetSize().x, GetDC()->GetSize().y);
    m_drawer.SetDC(GetDC());
    LayoutPages();
}


//--------------------------------------------------------------------------------
// Layout and drawing functions
//--------------------------------------------------------------------------------

void
MyPrintout::LayoutPages()
{
    // Disable windows while laying out the page
    wxWindowDisabler disableAll;
    wxBusyInfo wait(_T("Please wait. Laying out page..."));

    // Suppress drawing
    m_isDrawing = false;

    // Make the borders bigger if we're actually printing, because the DPI of the printer
    // is way higher.
    if (IsPreview())
        m_drawer.SetBorderSize(1);
    else
        m_drawer.SetBorderSize(3);

    // Try successive column scales (1/x) after the font gets too small.
    // Grid scale will be the entire page less the column scale.
    // e.g. column scale of 4 = 1/4; the grid will take up 3/4 of the page
    double colScales[] = { 4, 5, 6, 3 };
    int i = 0;
    const int startingFontSize = m_clueFont.GetPointSize();

    // Measure the grid and text.  If the text doesn't fit, catch the exception that should
    // be thrown, and try a smaller font size or a different grid size.
    for (;;)
    {
        try
        {
            // Measure the grid
            m_gridScale = 1 - (1. / colScales[i]);
            LayoutGrid(m_gridScale);
            DrawText();
            break;
        }
        catch (FontSizeError&)
        {
            wxTheApp->Yield();
            // If we're at the smallest good looking font size, try the next grid size.
            if (m_clueFont.GetPointSize() == MIN_GOOD_POINT_SIZE && i < 2)
            {
                wxLogDebug(_T("Hit min *good* font size"));
                wxLogDebug(_T("Trying again with a different grid size"));
                ++i; // Next grid size in the list
                m_clueFont.SetPointSize(startingFontSize);
                SetupFonts();
            }
            else if (m_clueFont.GetPointSize() <= MIN_POINT_SIZE)
            {
                wxLogDebug(_T("Ran out of size options!"));
                return;
            }
            else
            {
                wxLogDebug(_T("Trying again with font size: %d"), m_clueFont.GetPointSize());
                m_clueFont.SetPointSize(m_clueFont.GetPointSize() - 1);
                SetupFonts();
            }
        }
    }
    wxLogDebug(_T("Done with layout"));
}






void
MyPrintout::LayoutGrid(double gridScale)
{
    // Calculate the grid rectangle and prepare m_drawer for drawing the grid.
    // After calling this function, m_gridRect will contain the bounding
    // rectangle for the grid.

    // We'll draw the grid without scaling the DC, because our scaling is bound
    // to look *much* better than if we used wxDC::SetUserScale, which would
    // distort the grid.
    // The grid will rarely take exactly the space we give it, since
    // XGridDrawer ensures that all squares are the same size.

    wxDC * dc = GetDC();
    dc->SetUserScale(1, 1);
    dc->SetDeviceOrigin(0, 0);
    const wxRect pageRect = GetLogicalPageMarginsRect(*g_pageSetupData);

    // Calculate the grid width and height
    //------------------------------------

    // The most space we will allow the grid to take up
    const double maxGridWidth  = pageRect.width  * (gridScale - GRID_PADDING);
    const double maxGridHeight = pageRect.height * (gridScale - GRID_PADDING);

    // Calculate the size of each square
    const int borderSize = m_drawer.GetBorderSize();
    const int gridWidth  = m_puz->m_grid.GetWidth();
    const int gridHeight = m_puz->m_grid.GetHeight();

    // The largest a square can be is the total allowed space less the borders
    // divided by the number of squares in a row / col.
    const double boxWidth =
        (maxGridWidth  - borderSize * (gridWidth + 1)) / gridWidth;

    const double boxHeight =
        (maxGridHeight - borderSize * (gridHeight + 1)) / gridHeight;

    const double boxSize = std::min(boxWidth, boxHeight);

    // Calculate the grid size based on the size of a square.
    // This is the boxSize times the number of squares in the row / col plus
    // borders.
    m_gridRect.width  = (boxSize + borderSize) * gridWidth  + borderSize;
    m_gridRect.height = (boxSize + borderSize) * gridHeight + borderSize;

    // Setup the grid drawer with our calculated size
    m_drawer.SetMaxSize(m_gridRect.width, m_gridRect.height);
    m_drawer.SetAlign(m_gridAlign);

    // Add padding to the bounding rect
    const int padding = std::min(pageRect.width, pageRect.height) * GRID_PADDING;
    m_gridRect.width  += padding;
    m_gridRect.height += padding;

    // Align the grid
    //---------------
    m_gridRect.x = pageRect.x;
    m_gridRect.y = pageRect.y;

    if ((m_gridAlign & wxALIGN_RIGHT) != 0)
        m_gridRect.x += pageRect.width - m_gridRect.width;
    if ((m_gridAlign & wxALIGN_BOTTOM) != 0)
        m_gridRect.y += pageRect.height - m_gridRect.height;

    // Scale the DC.
    MapScreenSizeToPageMargins(*g_pageSetupData);

    // Adapt the grid rect to the user scale.
    m_gridRect.x      = dc->DeviceToLogicalX(m_gridRect.x);
    m_gridRect.y      = dc->DeviceToLogicalY(m_gridRect.y);
    m_gridRect.width  = dc->DeviceToLogicalXRel(m_gridRect.width);
    m_gridRect.height = dc->DeviceToLogicalYRel(m_gridRect.height);
}


void
MyPrintout::DrawGrid()
{
    // This must be called after LayoutGrid().

    // Offset the origin to draw the grid at the right point
    MapScreenSizeToPageMargins(*g_pageSetupData);
    wxDC * dc = GetDC();
    const int offsetX = dc->LogicalToDeviceX(m_gridRect.x);
    const int offsetY = dc->LogicalToDeviceY(m_gridRect.y);

    dc->SetUserScale(1, 1);
    dc->SetDeviceOrigin(offsetX, offsetY);
    m_drawer.DrawGrid(*dc);
}



void
MyPrintout::DrawText()
{
    wxDC * dc = GetDC();

    // Scale the DC.
    MapScreenSizeToPageMargins(*g_pageSetupData);

    m_pageRect = GetLogicalPageMarginsRect(*g_pageSetupData);
    int x = m_pageRect.x;
    int y = m_pageRect.y;

    LayoutColumns();

    // Title and author
    dc->SetFont(m_titleFont);
    DrawTextLine(WrapText(m_puz->m_title, m_columnWidth), &x, &y);

    dc->SetFont(m_authorFont);
    DrawTextLine(WrapText(m_puz->m_author, m_columnWidth), &x, &y);

    // Draw clue lists
    const wxString heading_list[2] = { _T("ACROSS"), _T("DOWN") };
    const XPuzzle::ClueList * clue_list[2] = { &m_puz->m_across, &m_puz->m_down };
    for (int i = 0; i < 2; ++i)
    {
        // Add some space above ACROSS and DOWN clues.
        y += CLUE_HEADING_PADDING;

        // Draw the heading
        dc->SetFont(m_headingFont);
        DrawTextLine(WrapText(heading_list[i], m_columnWidth), &x, &y);

        // Draw the clues
        for (XPuzzle::ClueList::const_iterator it = clue_list[i]->begin();
             it != clue_list[i]->end();
             ++it)
        {
            DrawClue(*it, &x, &y);
        }
    }
}

void
MyPrintout::LayoutColumns()
{
    m_pageRect = GetLogicalPageMarginsRect(*g_pageSetupData);

    // A full column
    m_columnWidth = m_pageRect.width
                  - (m_gridRect.width - GRID_PADDING)
                  - COLUMN_PADDING;


    // Unscale the DC for measuring.
    wxDC * dc = GetDC();
    SaveUserScale();
    // Measure the width of the largest clue number
    const int max_clue_number = std::max(m_puz->m_across.back().Number(),
                                         m_puz->m_down  .back().Number());
    dc->SetFont(m_numberFont);
    if (max_clue_number >= 100)
        dc->GetTextExtent(FormatNumber(999), &m_numberWidth, NULL);
    else
        dc->GetTextExtent(FormatNumber(99),  &m_numberWidth, NULL);
    RestoreUserScale();

    // The portion of the column that is taken up by clue text.
    m_clueWidth = m_columnWidth - (m_numberWidth + NUMBER_PADDING);
}



void
MyPrintout::DrawClue(const XPuzzle::Clue & clue, int * x, int * y)
{
    // Draw clue text

    // Adjust x to account for the clue number width before drawing the clue
    // text.  Return x to its previous position afterwards so that the clue
    // number ends up in the right place.
    wxDC * dc = GetDC();

    *x += m_numberWidth + NUMBER_PADDING;
    dc->SetFont(m_clueFont);
    int height;
    DrawTextLine(WrapText(clue.Text(), m_clueWidth), x, y, NULL, &height);
    *x -= m_numberWidth + NUMBER_PADDING;

    if (m_isDrawing)
    {
        // Draw clue number
        dc->SetFont(m_numberFont);
        dc->DrawLabel(FormatNumber(clue.Number()),
                      wxRect(*x, *y - height, m_numberWidth, dc->GetCharHeight()),
                      wxALIGN_RIGHT);
    }
}



// Column wrapping functions
void
MyPrintout::NewColumn(int * x, int * y)
{
    // Start a new column
    *x += m_columnWidth + COLUMN_PADDING;
    if (m_gridRect.Contains(*x, m_pageRect.GetTop()))
        *y = m_gridRect.GetBottom();
    else
        *y = m_pageRect.GetTop();
}

void
MyPrintout::AdjustColumn(int * x, int * y, int textWidth, int textHeight)
{
    // Decide if we need a new column.  If so, adjust x and y so that the
    // next item is drawn at the start of a new column.

    if (m_gridRect.Contains(*x, *y)
        || m_gridRect.Contains(*x + textWidth, *y + textHeight))
    {
        // If there is still room under the grid, don't start a new
        // column, just move y down.
        if (m_gridRect.GetBottom() + textHeight < m_pageRect.GetBottom())
            *y = m_gridRect.GetBottom();
        else // Otherwise start a new column
            NewColumn(x, y);
    }
    // If we're below the page, start a new column.
    else if (*y + textHeight > m_pageRect.GetBottom())
    {
        NewColumn(x, y);
    }
}


// This should throw an exception if the text runs off the page when we are
// measuring.
void
MyPrintout::DrawTextLine(const wxString & text,
                         int * x, int * y,
                         int * width, int * height)
{
    // Measure the text, draw it, and move the x and y variables to the location
    // that the next text line should be drawn.
    // The caller must ensure that the text is wrapped.

    int w, h;
    MeasureText(text, &w, &h);
    h += LINE_PADDING;

    AdjustColumn(x, y, w, h);

    const wxRect textRect(*x, *y, w, h);
    if (m_isDrawing)
        GetDC()->DrawLabel(text, textRect);
    else if (! m_pageRect.Contains(textRect))
        throw FontSizeError();

    if (width != NULL)
        *width = w;
    if (height != NULL)
        *height = h;

    *y += h;
}


void
MyPrintout::SaveUserScale()
{
    wxDC * dc = GetDC();
    dc->GetUserScale(&m_scaleX, &m_scaleY);
    dc->SetUserScale(1, 1);
}

void
MyPrintout::RestoreUserScale()
{
    wxASSERT(m_scaleX != -1 && m_scaleY != -1);
    GetDC()->SetUserScale(m_scaleX, m_scaleY);

#ifdef __WXDEBUG__
    // Set the scale variables to an invalid state.
    m_scaleX = -1;
    m_scaleY = -1;
#endif // __WXDEBUG__
}


void
MyPrintout::MeasureText(const wxString & text, int * width, int * height)
{
    // Unscale the DC for measuring.
    SaveUserScale();
    GetDC()->GetMultiLineTextExtent(text, width, height);
    RestoreUserScale();
}


wxString
MyPrintout::WrapText(const wxString & text, int maxWidth)
{
    return ::Wrap(GetDC(), text, maxWidth);
}