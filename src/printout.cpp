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

// TODO:
// If GetHTML() ever produces a string with no word line breaks, cache it, the
//     font size, and the column size.  If subsequent requests are of the same
//     or smaller font and the same or large column size, return the cached
//     string instead of recreating it.
// Have wxHtmlDCRenderer remember where it left off.  Render() returns the
//     y-position of the first cell not rendered.  Change the default value of
//     "from" to -1, which will mean "start from where we left off".

#include "printout.hpp"
#include "puz/Puzzle.hpp"
#include "App.hpp" // For the global print data pointers, and ConfigManager
#include "utils/wrap.hpp"
#include "MyFrame.hpp"
#include "CluePanel.hpp"
#include "XGridCtrl.hpp"
#include <wx/busyinfo.h>

// Constants
const double GRID_PADDING = 5;
const double COLUMN_PADDING = 5;
const int MIN_FONT_SIZE = 7;
const int MAX_FONT_SIZE = 11;
const double MIN_SQUARE_SIZE = 0.15; // In inches
// These are "nice-looking" sizes, which factor into how well-proportioned
// the layout is.
const double GOOD_SQUARE_SIZE = 0.2;
const int GOOD_FONT_SIZE = 10;
// Proportion of white space in a square to grid line thickness
const double BORDER_SCALE = .035;

//-----------------------------------------------------------------------------
// HTML
//-----------------------------------------------------------------------------
// Most of this section is composed of various hacks to get wxHtmlDCRenderer
// work well with column (=page) and line breaks.  The main class is very
// useful in printing HTML in columns, but these hacks make the result
// much nicer.


// A custom HTML parser that forces table cells to not break across pages.
// The implementation of this (especially the part about preventing a page
// break after the table header) is a terrible hack.
class PrintHtmlWinParser : public wxHtmlWinParser
{
protected:
    // We need to have control over the (singly) linked-list of child
    // cells of the wxHtmltableCell.  In order to do this, we need a pointer
    // to the first child cell.
    wxHtmlContainerCell * m_firstTableCell;
    wxHtmlContainerCell * m_table;

    virtual void AddTag(const wxHtmlTag& tag)
    {
        // Setup (before the tag is parsed)
        if (tag.GetName() == _T("TABLE"))
        {
            m_firstTableCell = NULL;
            m_table = NULL;
        }
        else if (tag.GetName() == _T("TH")
                || (tag.GetName() == _T("TR") && m_table == NULL))
        {
            // The table is a child of the current container;
            m_table = wxDynamicCast(GetContainer()->GetLastChild(), wxHtmlContainerCell);
            // Add the first (empty) container cell
            m_firstTableCell = new wxHtmlContainerCell(m_table);
        }

        // Parse the tag
        wxHtmlWinParser::AddTag(tag);

        // After the table is constructed, we can safely modify the order of
        // cells.
        if (tag.GetName() == _T("TABLE") && m_table)
        {
#if 0 // This became buggy (at least between wx 2.8.10 and 2.8.12
            // Remove the header and first table row (number and clue)
            // and make them children of m_firstTableCell.  This will prevent
            // the header from being separated from the first row.

            // First child is m_firstTableCell; next is the header
            wxHtmlCell * th = m_table->GetFirstChild()->GetNext();
            wxHtmlCell * number = th->GetNext();
            if (! number) return;
            wxHtmlCell * clue = number->GetNext();
            if (! clue) return;
            // The cell after clue will be the second child of the table.
            wxHtmlCell * secondRow = clue->GetNext();

            // Unlink the cells
            th->SetNext(NULL);
            number->SetNext(NULL);
            clue->SetNext(NULL);

            // Add the cells to the first cell
            m_firstTableCell->InsertCell(th);
            m_firstTableCell->InsertCell(number);
            m_firstTableCell->InsertCell(clue);

            // Relink the first cell to secondRow
            m_firstTableCell->SetNext(secondRow);

            // m_firstTableCell doesn't have any size yet, but its children
            // do, so calculate our own size.
            m_firstTableCell->Layout(th->GetWidth());
#endif // 0

            // Loop through the table cells and prevent them from being
            // broken across pages.
            wxHtmlCell * td = m_table->GetFirstChild();
            while (td)
            {
                td->SetCanLiveOnPagebreak(false);
                td = td->GetNext();
            }
        }
    }
};


// Allow word breaks in a long word by inserting the custom <wbr> tag.
// This is a pretty hackish way to break up overly long words, but it's
// better than having long words get cut off, or even worse, breaking the
// entire layout because the font size required to print the word on one
// line is to small.
wxString BreakWord(wxDC * dc, const wxString & word, int maxWidth)
{
    int width;
    // Shortcut if we don't need to break the word
    dc->GetTextExtent(word, &width, NULL);
    if (width <= maxWidth)
        return word;

    wxString ret;
    size_t start = 0;
    size_t length = 1;
    while (start + length < word.Length())
    {
        dc->GetTextExtent(word.substr(start, length), &width, NULL);
        // Break if the current substring is too long
        if (width > maxWidth && length > 1)
        {
            --length;
            ret << word.substr(start, length) << _T("<wbr>");
            start += length;
            length = 0;
        }
        ++length;
    }
    ret.Append(word.substr(start, length));
    return ret;
}


// Add <wbr> wherever it is needed in a line
wxString BreakLine(wxDC * dc, const wxString & line, int width)
{
    // Don't try to break up HTML
    if (line.find_first_of(_T("<>")) != wxString::npos)
        return line;
    wxString ret;
    // Break up the line by whitespace
    size_t start = 0;
    for (;;)
    {
        size_t end = line.find_first_of(_T(" -\n\r\f"), start);
        if (end == wxString::npos)
        {
            ret << BreakWord(dc, line.substr(start), width);
            break;
        }
        else
        {
            ++end;
            ret << BreakWord(dc, line.substr(start, end - start), width);
            start = end;
        }
    }
    return ret;
}

int GetFontSize(int base_size, int offset)
{
    // This is based on wxBuildFontSizes from src/html/winpars.cpp
    // Each increment adjusts font size by a factor of 1.2, except font size
    // -2.
    if (offset <= -1)
        return (base_size * 0.75);
    else
        return base_size * pow((double)1.2, offset);
}

void SetFontSize(wxDC * dc, int base_size, int offset)
{
    wxFont font = dc->GetFont();
    font.SetPointSize(GetFontSize(base_size, offset));
    dc->SetFont(font);
}

wxString
MyPrintout::GetHTML()
{
    // We need to insert <wbr> wherever there could be a line break in
    // our text string, therefore GetHTML() needs to be called every time
    // we do a new layout.
    wxDC * dc = GetDC();
    // ** Make sure to set the font to the standard font before calling
    // this function

    // This will be used for measuring line breaks
    int base_font_size = dc->GetFont().GetPointSize();

    // Assemble the html
    // This will contain the header stuff and then one long table for each
    // clue list.
    wxString html;

    SetFontSize(dc, base_font_size, 0);
    // Clue lists
    puz::Clues::const_iterator clues_it;
    for (clues_it = m_puz->GetClues().begin();
         clues_it != m_puz->GetClues().end();
         ++clues_it)
    {
        // Table for clues
        html << _T("<table cellspacing=0 cellpadding=1 border=0>");
        // Heading
        html << _T("<tr><th colspan=2 align=\"left\"><font size=\"+1\"><b>")
                    << BreakLine(dc, puz2wx(clues_it->first), m_columnWidth)
               << _T("</b></font></th></tr>");
        puz::ClueList::const_iterator it;
        for (it = clues_it->second.begin(); it != clues_it->second.end(); ++it)
        {
            html
                << _T("<tr>")
                    // Clue number
                    << _T("<td align=\"right\" valign=\"top\"><font size=\"-1\"><b>")
                        << puz2wx(it->GetNumber()) // Don't bother to break the number
                    << _T(" </b></font></td>")
                    // Clue text
                    << _T("<td>")
                        // Break the clue at .8 column width
                        << BreakLine(dc, puz2wx(it->GetText()), m_columnWidth * 0.8)
                    << _T("</td>")
                << _T("</tr>");
        }
        html << _T("</table>");
        // Add some space after the clue list
        html << _T("<br><br>");
    }
    return html;
}

//-----------------------------------------------------------------------------
// Setup functions
//-----------------------------------------------------------------------------

MyPrintout::MyPrintout(MyFrame * frame, puz::Puzzle * puz, PrintInfo info)
    : wxPrintout(puz2wx(puz->GetTitle())),
      m_frame(frame),
      m_puz(puz),
      m_drawer(puz),
      m_info(info),
      m_isScaled(false)
{
    m_htmlRenderer = new MyHtmlDCRenderer(new PrintHtmlWinParser);
    // Edit grid drawer options
    m_info.grid_options |= XGridDrawer::DRAW_CIRCLE;
    if (puz->IsDiagramless())
    {
        if (! (m_info.grid_options & XGridDrawer::DRAW_TEXT))
            m_info.grid_options |= XGridDrawer::DRAW_BLANK_DIAGRAMLESS;
    }
    m_drawer.SetFlags(m_info.grid_options);
    // Since print DPI is huge, set the number scale to something reasonable
    m_drawer.SetNumberScale(0.25);
    ReadConfig();
}

MyPrintout::~MyPrintout()
{
    delete m_htmlRenderer;
}

void
MyPrintout::ReadConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();

    // Fonts config

    if (config.Printing.Fonts.useCustomFonts())
    {
        m_drawer.SetNumberFont(config.Printing.Fonts.gridNumberFont());
        m_drawer.SetLetterFont(config.Printing.Fonts.gridLetterFont());
        m_clueFont = config.Printing.Fonts.clueFont();
    }
    else
    {
        m_drawer.SetNumberFont(m_frame->m_XGridCtrl->GetNumberFont());
        m_drawer.SetLetterFont(m_frame->m_XGridCtrl->GetLetterFont());
        m_clueFont = m_frame->m_clues.begin()->second->GetFont();
    }
    m_drawer.SetNumberScale(config.Grid.numberScale() / 100.);
    m_drawer.SetLetterScale(config.Grid.letterScale() / 100.);

    // Grid config
    m_gridAlign  = config.Printing.gridAlignment();

    const long brightness = config.Printing.blackSquareBrightness();
    m_drawer.SetBlackSquareColor(wxColour(brightness, brightness, brightness));
    m_drawer.SetThemeColor(config.Grid.themeSquareColor());
}


void
MyPrintout::SetPointSize(int pointSize)
{
    // This doesn't do anything since we're using HTML
    /*
    m_clueFont.SetPointSize(pointSize);
    m_numberFont = m_clueFont;
    m_numberFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_headingFont = m_clueFont;
    m_headingFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_authorFont = m_clueFont;
    m_authorFont.SetPointSize(m_authorFont.GetPointSize() + 1);
    m_titleFont = m_authorFont;
    m_titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    */
}

//-----------------------------------------------------------------------------
// Scaling functions
//-----------------------------------------------------------------------------
void MyPrintout::ScaleDC()
{
    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);

    // This scales the DC so that the printout roughly represents the the screen
    // scaling. The text point size _should_ be the right size but in fact is
    // too small for some reason. This is a detail that will need to be
    // addressed at some point but can be fudged for the moment.
    float scale = (float)((float)ppiPrinterX/(float)ppiScreenX);

    /* scale our PageSize and Margins to the "screen" */
    int pageWidth, pageHeight;
    int w, h;
    GetDC()->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);


    // If printer pageWidth == current DC width, then this doesn't change. But w
    // might be the preview bitmap width, so scale down.
    float overallScale = scale * (float)(w/(float)pageWidth);
    GetDC()->SetUserScale(overallScale, overallScale);

    // Set margins
    float factor = (float)(ppiPrinterX/(scale*25.4));
    wxPoint offset = g_pageSetupData->GetMarginTopLeft();
    GetDC()->SetDeviceOrigin(offset.x * factor, offset.y * factor);

    //MapScreenSizeToPageMargins(*g_pageSetupData);
    m_isScaled = true;
}

void MyPrintout::UnscaleDC()
{
    wxDC * dc = GetDC();
    dc->SetUserScale(1, 1);
    dc->SetDeviceOrigin(0, 0);
    m_isScaled = false;
}

//-----------------------------------------------------------------------------
// Print framework functions
//-----------------------------------------------------------------------------
bool
MyPrintout::HasPage(int pageNum)
{
    return pageNum < 2 || (pageNum == 2 && m_info.two_pages);
}


wxString
MyPrintout::GetTitle()
{
    return puz2wx(m_puz->GetTitle());
}

void
MyPrintout::GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
{
    *minPage = 1;
    *pageFrom = 1;
    *maxPage = m_info.two_pages ? 2 : 1;
    *pageTo  = *maxPage;
}

bool
MyPrintout::OnPrintPage(int pageNum)
{
    m_isDrawing = true;
    DrawHeader();
    if (! m_info.two_pages)
    {
        if (m_info.grid)
        {
            LayoutGrid(m_gridScale);
            DrawGrid();
        }
        if (m_info.clues)
            DrawText(m_columns, m_fontSize);
    }
    // Two page printing (always inclues clues and grid)
    else
    {
        wxASSERT(m_info.clues && m_info.grid);
        if (pageNum == 1)
        {
            LayoutGrid(m_gridScale);
            DrawGrid();
        }
        else // pageNum == 2
        {
            DrawText(m_columns, m_fontSize);
        }
    }
    return true;
}

void
MyPrintout::OnPreparePrinting()
{
    m_isDrawing = false;
    LayoutPages();
}


//-----------------------------------------------------------------------------
// Full Page Layout
//-----------------------------------------------------------------------------

wxRect
MyPrintout::GetPageRect()
{
    // Remove the header from the page rect
    wxRect rect = GetLogicalPageMarginsRect(*g_pageSetupData);
    double x, y;
    GetDC()->GetUserScale(&x, &y);
    rect.y += m_headerHeight / y;
    rect.height -= m_headerHeight / y;
    return rect;
}

bool
MyPrintout::LayoutPages()
{
    // Disable windows while laying out the page
    wxWindowDisabler disableAll;
    wxBusyInfo wait(_T("Please wait. Laying out page..."));

    // Figure out how much space to allot for the header
    DrawHeader();

    // If we're doing a two-page layout, just figure out how to layout the
    // text
    if (m_info.two_pages || ! (m_info.grid && m_info.clues))
    {
        // Try columns in this order
        int columns[] = { 4, 3, 5, 6 };
        for (int pt = MAX_FONT_SIZE; pt >= MIN_FONT_SIZE; --pt)
        {
            for (int i = 0; i < 4; ++i)
            {
                // If the layout worked, we're done
                if (LayoutText(columns[i], pt))
                {
                    m_gridScale = 1; // Grid takes up one page
                    m_fontSize = pt;
                    m_columns = columns[i];
                    return true;
                }
            }
        }
        return false;
    }

    // Otherwise layout the grid and text on one page

    // Start with the preferred grid size and column layout.
    // Try to layout the text wrapping around the grid.
    // If the text doesn't fit, try a different column layout.
    // If that doesn't work, try making the grid take up less space.
    // If that doesn't work, try smaller point sizes.

    // The ideal layout is one where the grid and text have to compensate
    // as little as possible.  spaceFilled will keep track of the amount of
    // total grid and text.  Higher values mean that more space is taken up
    // on the page.  gridWeight is grid_ratio - text_ratio, a measure of 
    // how well-proportioned the page is.  Ideally this is 0.
    // For our purposes, grid ratio will be defined as
    // log(box_size / good_box_size) and text ratio as
    // log(font_size / good_font_size)
    double spaceFilled = 0;
    double gridWeight = 0;

    // A structure to keep our best layout so far
    struct _bestData {
        double spaceFilled;
        double gridWeight;
        int fontSize;
        int columns;
        double gridScale;
    } best;
    best.fontSize = -1; // The invalid state

    // Set the min box size.
    // I can't figure out how to reliably set min box size to an inch
    // measurement, so this will have to do.
    m_pageRect = GetPageRect();
    int w, h;
    GetPageSizeMM(&w, &h);
    w -= (g_pageSetupData->GetMarginTopLeft().x + g_pageSetupData->GetMarginBottomRight().x);
    double ppi = 25.4 * m_pageRect.width / w;
    int minBoxSize = ppi * MIN_SQUARE_SIZE;
    int goodBoxSize = ppi * GOOD_SQUARE_SIZE;

    // Try columns in this order
    int columns[] = { 4, 5, 6, 3 };

    for (int pt = MAX_FONT_SIZE; pt >= MIN_FONT_SIZE; --pt)
    {
        double textRatio = log((double)pt / (double)GOOD_FONT_SIZE);
        for (int i = 0; i < 4; ++i)
        {
            int nColumns = columns[i];
            // Columns that are all text
            for (double textCols = 1.; textCols < (double)nColumns/2; ++textCols)
            {
                // The grid takes up all cols that are not all text
                m_gridScale = 1 - (textCols / nColumns);
                wxLogDebug(_T("Laying out cols=%d, textCols=%d, pt=%d, grid=%g"), nColumns, (int)textCols, pt, m_gridScale);
                if (m_gridScale < .1)
                    continue;

                LayoutGrid(m_gridScale);
                wxLogDebug(_T("Box Size: %d"), m_drawer.GetBoxSize());
                if (m_drawer.GetBoxSize() < minBoxSize)
                    continue;
                if (LayoutText(nColumns, pt))
                {
                    double gridRatio = log((double)m_drawer.GetBoxSize() / (double)goodBoxSize);
                    spaceFilled = gridRatio + textRatio;
                    gridWeight = gridRatio - textRatio;
                    wxLogDebug(_T("space: %g, weight: %g"), spaceFilled, gridWeight);
                    // See if this is a better layout than the previous best.
                    // "Better" here means that the amount of space gained
                    // is greater than the half the amount of proportion lost.
                    if (best.fontSize == -1
                        || (spaceFilled > best.spaceFilled
                            && spaceFilled - best.spaceFilled >
                                (std::abs(gridWeight) - std::abs(best.gridWeight)) / 2))
                    {
                        wxLogDebug(_T("(current best)"));
                        best.gridWeight = gridWeight;
                        best.spaceFilled = spaceFilled;
                        best.gridScale = m_gridScale;
                        best.fontSize = pt;
                        best.columns = nColumns;
                    }
                }
            }
        }
        // The textRatio is going to be getting smaller, so if our
        // best gridWeight is already > 0, the layout will never get
        // better.
        if (best.fontSize != -1 && best.gridWeight > 0)
            break;
    }
    if (best.fontSize != -1)
    {
        m_gridScale = best.gridScale;
        m_fontSize = best.fontSize;
        m_columns = best.columns;
        wxLogDebug(_T("Best layout with cols=%d, pt=%d, grid=%g, space=%g, weight=%g"), m_columns, m_fontSize, m_gridScale, best.spaceFilled, best.gridWeight);
        return true;
    }
    else
        return false;
}

//--------------------------------------------------------------------------------
// Header
//--------------------------------------------------------------------------------
void
MyPrintout::DrawHeader()
{
    if (! (m_info.author || m_info.title || m_info.notes))
    {
        m_headerHeight = 0;
        return;
    }
    wxDC * dc = GetDC();
    ScaleDC();

    wxRect rect = GetLogicalPageMarginsRect(*g_pageSetupData);

    // Setup the HTML Renderer
    m_htmlRenderer->SetDC(dc);
    m_htmlRenderer->SetSize(rect.width, rect.height);
    m_htmlRenderer->SetStandardFonts(12, m_clueFont.GetFaceName());
    dc->SetFont(m_clueFont);

    // Get the header Text

    wxString html;
    html << _T("<table border=0 cellpadding=2 width=100%><tr>");
    // Title
    if (m_info.title && m_puz->HasMeta(puzT("title")))
    {
        html << _T("<td align=left><font size=\"+1\"><b>") 
                    << puz2wx(m_puz->GetTitle())
                << _T("<b></font></td>");
    }
    // Author / Editor
    if (m_info.author && m_puz->HasMeta(puzT("author")))
    {

        html << _T("<td align=right>") << puz2wx(m_puz->GetAuthor());
        if (m_puz->HasMeta(puzT("editor")))
            html << _T(" / ") << puz2wx(m_puz->GetMeta(puzT("editor")));
        html << _T("</td>");
    }
    html << _T("</tr></table>");
    // Notes
    if (m_info.notes && m_puz->HasMeta(puzT("notes")))
    {

        html << _T("<table border=0 cellpadding=5 width=100%><tr><td>")
             << puz2wx(m_puz->GetNotes())
             << _T("</td></tr></table>");
    }

    m_htmlRenderer->SetHtmlText(html);

    m_headerHeight = m_htmlRenderer->Render(rect.x, rect.y, 0, ! m_isDrawing);
    double x, y;
    dc->GetUserScale(&x, &y);
    m_headerHeight *= y;
}


//--------------------------------------------------------------------------------
// Text Layout
//--------------------------------------------------------------------------------

bool
MyPrintout::LayoutText(int columns, int fontSize)
{
    m_isDrawing = false;
    return DrawText(columns, fontSize);
}


bool
MyPrintout::DrawText(int columns, int fontSize)
{
    wxDC * dc = GetDC();
    ScaleDC();
    m_pageRect = GetPageRect();
    // Figure out the column width
    m_columnWidth = (m_pageRect.width - (COLUMN_PADDING * (columns - 1)))
                        / (double)columns;

    // Setup the HTML Renderer
    m_htmlRenderer->SetDC(dc);
    m_htmlRenderer->SetSize(m_columnWidth, 1000);
    m_htmlRenderer->SetStandardFonts(fontSize, m_clueFont.GetFaceName());
    dc->SetFont(m_clueFont);
    m_htmlRenderer->SetHtmlText(GetHTML());

    if (! m_isDrawing && m_htmlRenderer->GetTotalWidth() > m_columnWidth)
        return false;

    // Layout the text into columns
    int x = m_pageRect.x;
    int html_y = 0;
    int html_height = m_htmlRenderer->GetTotalHeight();
    while (html_y < html_height)
    {
        wxRect colRect(x, m_pageRect.y, m_columnWidth, m_pageRect.height);

        // Make sure we're still on the page
        if (! m_pageRect.Contains(x, m_pageRect.y))
            return false;

        if (! m_info.two_pages && m_info.grid)
        {
            // Make sure we don't start this column in the grid
            if (m_gridRect.Contains(colRect.GetTopLeft()))
            {
                colRect.y = m_gridRect.GetBottom() + GRID_PADDING;
                colRect.SetBottom(m_pageRect.GetBottom());
            }
            // Make sure we don't overlap the grid
            if (m_gridRect.Intersects(colRect)) // The column is too long
            {
                colRect.SetBottom(m_gridRect.GetTop() - GRID_PADDING);
            }
        }

        // Draw the column
        m_htmlRenderer->SetSize(m_columnWidth, colRect.height);
        html_y = m_htmlRenderer->Render(colRect.x, colRect.y, html_y, ! m_isDrawing);
        // Move to the next column
        x += m_columnWidth + COLUMN_PADDING;
    }

#if 0
    // DEBUG: draw columns and page rect
    if (m_isDrawing)
    {
        dc->SetPen(*wxRED_PEN);
        dc->SetBrush(*wxTRANSPARENT_BRUSH);
        dc->DrawRectangle(m_pageRect);
        dc->SetPen(*wxGREY_PEN);
        for (int i = 0; i < columns; ++i)
        {
            int x = m_pageRect.x + i * (m_columnWidth + COLUMN_PADDING);
            dc->DrawLine(x, m_pageRect.y, x, m_pageRect.height + m_pageRect.y);
            dc->DrawLine(x + m_columnWidth, m_pageRect.y, x + m_columnWidth, m_pageRect.height + m_pageRect.y);
        }
    }
#endif // 0 (debug drawing)
    return true;
}




//--------------------------------------------------------------------------------
// Grid Layout
//--------------------------------------------------------------------------------

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
    m_drawer.SetDC(dc);

    // Unscale the DC for layout calculations
    UnscaleDC();

    const wxRect pageRect = GetPageRect();

    // Calculate the grid width and height
    //------------------------------------

    // The most space we will allow the grid to take up
    const double maxGridWidth  = pageRect.width  * (gridScale);
    const double maxGridHeight = pageRect.height * (gridScale);

    // Calculate the border (grid lines) size
    const int rows = m_puz->GetGrid().GetWidth();
    const int cols = m_puz->GetGrid().GetHeight();

    const int borderSize = std::max(1, (int)std::min(
        (maxGridWidth  / cols) * BORDER_SCALE,
        (maxGridHeight / rows) * BORDER_SCALE));
    m_drawer.SetBorderSize(borderSize);

    // Calculate the size of each square

    // The largest a square can be is the total allowed space less the borders
    // divided by the number of squares in a row / col.
    const double boxWidth =
        (maxGridWidth  - borderSize * (cols + 1)) / cols;

    const double boxHeight =
        (maxGridHeight - borderSize * (rows + 1)) / rows;

    const double boxSize = std::min(boxWidth, boxHeight);

    // Calculate the grid size based on the size of a square.
    // This is the boxSize times the number of squares in the row / col plus
    // borders.
    m_gridRect.width  = (boxSize + borderSize) * cols  + borderSize;
    m_gridRect.height = (boxSize + borderSize) * rows + borderSize;

    // Align the grid
    //---------------
    m_gridRect.x = pageRect.x;
    m_gridRect.y = pageRect.y;

    if (m_info.two_pages || ! m_info.clues)
    {
        // Center the grid
        m_gridRect.x += (pageRect.width - m_gridRect.width) / 2;
        m_gridRect.y += (pageRect.height - m_gridRect.height) / 2;
    }
    else
    {
        if ((m_gridAlign & wxALIGN_RIGHT) != 0)
            m_gridRect.x += pageRect.width - m_gridRect.width;
        if ((m_gridAlign & wxALIGN_BOTTOM) != 0)
            m_gridRect.y += pageRect.height - m_gridRect.height;
    }

    m_drawer.SetAlign(m_gridAlign);
    m_drawer.SetMaxSize(m_gridRect.GetSize());

    // Scale the DC so we can transform the gridRect dimensions to logical
    // dimensions.
    ScaleDC();
    m_gridRect.x      = dc->DeviceToLogicalX(m_gridRect.x);
    m_gridRect.y      = dc->DeviceToLogicalY(m_gridRect.y);
    m_gridRect.width  = dc->DeviceToLogicalXRel(m_gridRect.width);
    m_gridRect.height = dc->DeviceToLogicalYRel(m_gridRect.height);
}


void
MyPrintout::DrawGrid()
{
    m_drawer.SetDC(GetDC());
    UnscaleDC();

    // This must be called after LayoutGrid().

    // Offset the origin to draw the grid at the right point
    ScaleDC();
    wxDC * dc = GetDC();
    const int offsetX = dc->LogicalToDeviceX(m_gridRect.x);
    const int offsetY = dc->LogicalToDeviceY(m_gridRect.y);
    UnscaleDC();
    dc->SetDeviceOrigin(offsetX, offsetY);

    m_drawer.DrawGrid(*dc);
}
