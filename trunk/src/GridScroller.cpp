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


#include "GridScroller.hpp"

static const int MAX_WINDOW_SIZE = 5000;
static const int MIN_WINDOW_SIZE = 300;


BEGIN_EVENT_TABLE(GridScroller, wxScrolledWindow)
    EVT_SIZE           (GridScroller::OnSize)
    EVT_MOUSEWHEEL     (GridScroller::OnMouseWheel)
    EVT_MIDDLE_DOWN    (GridScroller::OnMiddleDown)
    EVT_PUZ_GRID_FOCUS (GridScroller::OnGridFocus)
END_EVENT_TABLE()

GridScroller::GridScroller(wxWindow * parent)
    : wxScrolledWindow(parent),
      m_fitGrid(false)
{
    m_grid = new GridCtrl(this);
    m_grid->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial")));
    SetBackgroundColour(*wxWHITE);
    m_grid->SetBackgroundColour(*wxGREEN);

    m_gridSize = GetSize();
    SetScrollRate(20, 20);
    SetGridSize(500, 500);
}

GridScroller::~GridScroller()
{
}


void
GridScroller::SetPuzzle(XPuzzle * puz)
{
    m_grid->SetPuzzle(puz);
    if (m_fitGrid)
        FitGrid();
    else
        SetGridSize(m_gridSize); // Rescale everything to keep scroll units accurate
}


void
GridScroller::SetGridSize(wxSize size)
{
    wxLogDebug(_T("New Size (%d, %d)"), size.x, size.y);
    if (size == wxDefaultSize) {
        FitGrid();
        return;
    }
    m_fitGrid = false;
    if (MIN_WINDOW_SIZE <= size.x && size.x <= MAX_WINDOW_SIZE
        && MIN_WINDOW_SIZE <= size.y && size.y <= MAX_WINDOW_SIZE)
    {
        m_gridSize = size;
        m_grid->SetSize(size);

        // Calculate our own virtual size based on square size and grid size
        if (m_grid->GetXGrid() == NULL)
            SetVirtualSize(0,0);
        else
            SetVirtualSize(m_grid->GetSquareSize() * m_grid->GetXGrid()->GetWidth(),
                           m_grid->GetSquareSize() * m_grid->GetXGrid()->GetHeight());

        wxSize client(GetClientSize());

        // Grid is smaller than the window in both directions, so center it
        if (size.x <= client.x && size.y <= client.y) {
            m_doesGridFit = true;
            m_grid->SetAlign(wxALIGN_CENTER);
            m_grid->CenterOnParent();
        }
        // At least one direction is larger than the window
        else {
            m_doesGridFit = false;

            SetScrollRate(m_grid->GetSquareSize(), m_grid->GetSquareSize());

            // Set grid alignment (within the window, and for the grid itself)
            int x, y;
            CalcScrolledPosition(0,0, &x, &y);
            m_grid->Move(x,y);

            if (size.x <= client.x) {
                m_grid->CenterOnParent(wxHORIZONTAL);
                m_grid->SetAlign(wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL);
            }
            else if (size.y <= client.y) {
                m_grid->CenterOnParent(wxVERTICAL);
                m_grid->SetAlign(wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
            }
            else
                m_grid->SetAlign(wxALIGN_TOP | wxALIGN_LEFT);

            // Make sure the focused square is still visible
            MakeVisible(m_grid->GetFocusedSquare());
        }
    }
}


void
GridScroller::FitGrid(bool fit)
{
    if (fit) {
        m_fitGrid = true;
        m_doesGridFit = true;

        Scroll(0,0);
        // This kills the scrollbars
        SetVirtualSize(wxSize(1,1));
        m_grid->SetSize(0, 0 , GetSize().x, GetSize().y);
        m_grid->SetAlign(wxALIGN_CENTER);
    }
    else {
        SetGridSize(m_gridSize);
    }
}


void
GridScroller::ZoomIn()
{
    double change = 1.25;
    SetGridSize(m_gridSize.x * change, m_gridSize.y * change);
}

void
GridScroller::ZoomOut()
{
    double change = 1. / 1.25;
    SetGridSize(m_gridSize.x * change, m_gridSize.y * change);
}



void
GridScroller::OnSize(wxSizeEvent & evt)
{
    wxLogDebug(_T("OnSize at GridScroller"));
    if (m_fitGrid)
        FitGrid();
    else
        SetGridSize(m_gridSize);
}


void
GridScroller::OnMouseWheel(wxMouseEvent & evt)
{
    if (evt.CmdDown() && ! m_fitGrid) {
        double change = pow(2.0, (double)evt.GetWheelRotation() / (double)evt.GetWheelDelta() / 2.0);
        SetGridSize(m_gridSize.x * change, m_gridSize.y * change);
    }
    else
        evt.Skip();
}


void
GridScroller::OnMiddleDown(wxMouseEvent & evt)
{
    FitGrid(! m_fitGrid);
}



void
GridScroller::MakeVisible(int col, int row)
{
    
    if (DoesGridFit() || m_grid->GetXGrid() == NULL)
        return;

    int startX = -1;
    int startY = -1;

    int viewX, viewY;
    GetViewStart(&viewX, &viewY);
    wxSize client = GetClientSize();

    // Subtract 1 here because the squares are zero-based, not one-based
    int lengthX = int(client.x / m_grid->GetSquareSize()) - 1;
    int lengthY = int(client.y / m_grid->GetSquareSize()) - 1;

    wxLogDebug(_T("View Start (%d, %d) . . . size (%d, %d)"), viewX, viewY, lengthX, lengthY);

    // X Scrolling
    if (col <= viewX || col >= viewX + lengthX) {
        startX = col - lengthX / 2;

        // Fit witihin constraints
        if (startX < 0)
            startX = 0;
        if (startX > m_grid->GetXGrid()->LastCol() - lengthX)
            startX = m_grid->GetXGrid()->LastCol() - lengthX;
    }

    // Y Scrolling
    if (row <= viewY || row >= viewY + lengthY) {
        startY = row - lengthY/ 2;

        // Fit witihin constraints
        if (startY < 0)
            startY = 0;
        if (startY > m_grid->GetXGrid()->LastRow() - lengthY)
            startY = m_grid->GetXGrid()->LastRow() - lengthY;
    }

    Scroll(startX, startY);
}


void
GridScroller::OnGridFocus(wxPuzEvent & evt)
{
    if (! m_fitGrid) {
        const wxPoint & pt = evt.GetGridPosition();
        MakeVisible(pt.x, pt.y);
    }
    evt.Skip();
}
