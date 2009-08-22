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

void GetLogicalPageSize(wxDC * dc, wxPageSetupDialogData printData, int * maxX, int * maxY)
{
    const wxSize dcSize = dc->GetSize();
    const wxPoint topLeft = printData.GetMarginTopLeft();
    const wxPoint bottomRight = printData.GetMarginBottomRight();

    const double dpmX = static_cast<double>(-dc->DeviceToLogicalX(0)) / topLeft.x;
    const double dpmY = static_cast<double>(-dc->DeviceToLogicalX(0)) / topLeft.y;

    const int logicalWidth = dc->DeviceToLogicalX(dcSize.GetWidth()) -
                             dc->DeviceToLogicalX(0);
    const int logicalHeight = dc->DeviceToLogicalY(dcSize.GetHeight()) -
                              dc->DeviceToLogicalY(0);

    *maxX = logicalWidth -  dpmX * (topLeft.x + bottomRight.x);
    *maxY = logicalHeight - dpmY * (topLeft.y + bottomRight.y);
}


bool
MyPrintout::OnPrintPage(int pageNum)
{
    wxASSERT(pageNum == 1);

    // Set the dc's mapping (so text prints the right size)

    // TODO: We should be getting this from a page setup dialog.
    wxPageSetupDialogData data;
    data.SetMarginTopLeft(wxPoint(15,15));
    data.SetMarginBottomRight(wxPoint(15,15));
    data.SetPaperId(wxPAPER_LETTER);
    MapScreenSizeToPageMargins(data);

    wxDC * dc = GetDC();

    int maxX, maxY;
    GetLogicalPageSize(dc, data, &maxX, &maxY);

    // Fit the grid to the page
    const int gridW = maxX * 0.75;
    const int gridH = maxY * 0.75;

    const int gridX = maxX - gridW;
    const int gridY = maxY - gridH;

    // For now just draw a rectangle for the grid.
    dc->DrawRectangle(gridX, gridY, gridW, gridH);


    // Draw the clues
    //---------------
    wxFont clue_font = *wxSWISS_FONT;
    wxFont num_font  = *wxSWISS_FONT;
    num_font.SetWeight(wxFONTWEIGHT_BOLD);
    wxFont heading_font = num_font;

    const int text_width = 1 * dpi;
    const int num_width = 25;

    int y = 0
    int x = num_width + 5;

    wxString heading = _T("ACROSS");
    std::vector<XPuzzle::Clue> * clues = &m_acrossClues;
    for (int i = 0; i < 2; ++i)
    {
        // Draw the heading
        dc->SetFont(heading_font);
        int width, height;
        dc->GetTextExtent(heading, &width, &height);

        if ( y + height > maxY || (x + text_width > gridX && y + height > gridY) )
        {
            x += text_width + num_width + 10;
            y = 0;
        }

        dc->DrawLabel(heading, wxRect(x - (num_width + 5), y, width + (num_width + 5), height));

        y += height;

        // Draw the clues
        for (std::vector<XPuzzle::Clue>::const_iterator it = clues->begin();
             it != clues->end();
             ++it)
        {
            dc->SetFont(clue_font);
            wxString text = ::Wrap(dc, it->Text(), text_width);

            dc->GetMultiLineTextExtent(text, &width, &height, NULL, NULL);

            if ( y + height > maxY || (x + text_width > gridX && y + height > gridY) )
            {
                x += text_width + num_width + 10;
                y = 0;
            }

            dc->DrawLabel( text, wxRect(x,y, width,height) );
            dc->SetFont(num_font)
            dc.DrawLabel( it->Number() + _T("."), wxRect(x - (num_width + 5), y,
                                                         num_width, h),
                                                  wxALIGN_RIGHT );
            dc.DrawRectangle(x,y,width,height);
            y += height;
        }

        // Next iteration
        wxString heading = _T("DOWN");
        clues = &m_downClues;
    }
}
