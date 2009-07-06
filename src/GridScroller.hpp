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


// TODO
// - Make this derive from both wxScrolledWindow and GridCtrl
//    so that redrawing works well

#ifndef GRID_SCROLLER_H
#define GRID_SCROLLER_H

// For compilers that don't support precompilation, include "wx/wx.h"
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

#include "PuzEvent.hpp"
#include "GridCtrl.hpp"

class GridScroller
    : public wxScrolledWindow
{
public:
    explicit GridScroller(wxWindow * parent);
    ~GridScroller();

    void SetPuzzle(XPuzzle * puz);
    void SetGridSize(wxSize size);
    void SetGridSize(int width, int height) { SetGridSize(wxSize(width, height)); }
    void FitGrid(bool fit = true);
    void ZoomIn();
    void ZoomOut();

    bool SetBackgroundColour(const wxColour & colour) {
        return m_grid->SetBackgroundColour(colour)
                && wxScrolledWindow::SetBackgroundColour(colour);
    }

    GridCtrl * m_grid;

    bool DoesGridFit() const { return m_doesGridFit; }

protected:
    wxSize m_gridSize;
    bool m_fitGrid;

    bool m_doesGridFit;     // Does the grid fit in the window?

    void MakeVisible(const XSquare * square) { if (square == NULL) return; MakeVisible(square->col, square->row); }
    void MakeVisible(int col, int row);

    void OnGridFocus(wxPuzEvent & evt);
    void OnSize(wxSizeEvent & evt);
    void OnMouseWheel(wxMouseEvent & evt);
    void OnMiddleDown(wxMouseEvent & evt);

    DECLARE_EVENT_TABLE()
};

#endif // GRID_SCROLLER_H
