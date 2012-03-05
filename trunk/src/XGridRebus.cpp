// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#include "XGridRebus.hpp"
#include "XGridCtrl.hpp"
#include "utils/string.hpp"
#include "puz/Square.hpp"
#include <wx/clipbrd.h>
#include <wx/textctrl.h>
#include <wx/settings.h>
#include "App.hpp"

// This is a wxTextCtrl wrapped in a wxWindow, allowing
// the TextCtrl to be clipped to a particular size.
class RebusTextCtrl : public wxWindow
{
    friend class XGridRebusCtrl;
public:
    RebusTextCtrl(wxWindow * parent, XGridCtrl * grid);
    ~RebusTextCtrl();
    wxTextCtrl * GetTextCtrl() { return m_text; }
    const wxTextCtrl * GetTextCtrl() const { return m_text; }

    wxSize GetTextSize() const; // The extent of text currently in m_text

protected:
    XGridCtrl * m_grid;
    wxTextCtrl * m_text;

    void OnChar(wxKeyEvent & evt);
    void OnPaste(wxClipboardTextEvent & evt);
    void OnSelectAll(wxCommandEvent & evt);
    void OnInsert(wxCommandEvent & evt);

    // Sizing and alignment
    void UpdateSize(const wxRect & rect);

    DECLARE_NO_COPY_CLASS(RebusTextCtrl)
};



RebusTextCtrl::RebusTextCtrl(wxWindow * parent, XGridCtrl * grid)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_DOUBLE),
      m_grid(grid)
{
    m_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                            wxDefaultSize, wxBORDER_NONE);
    m_text->Connect(m_text->GetId(), wxEVT_COMMAND_TEXT_PASTE,
        wxClipboardTextEventHandler(RebusTextCtrl::OnPaste), NULL, this);
    m_text->Connect(wxEVT_CHAR,
        wxKeyEventHandler(RebusTextCtrl::OnChar), NULL, this);

    // Accelerators
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_CTRL,  (int) 'A',     wxID_SELECTALL);
    wxAcceleratorTable accel(1, entries);
    m_text->SetAcceleratorTable(accel);
    m_text->Connect(wxID_SELECTALL, wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler(RebusTextCtrl::OnSelectAll), NULL, this);


    // Config
    ConfigManager::Grid_t & cfg = wxGetApp().GetConfigManager().Grid;
    cfg.whiteSquareColor.AddCallback(this, &RebusTextCtrl::SetBackgroundColour);
    cfg.whiteSquareColor.AddCallback(m_text, &wxTextCtrl::SetBackgroundColour);
    cfg.Update(this);
    cfg.Update(m_text);
}

RebusTextCtrl::~RebusTextCtrl()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
    wxGetApp().GetConfigManager().RemoveCallbacks(m_text);
}

const int textBorder = 2;

wxSize RebusTextCtrl::GetTextSize() const
{
    int w;
    m_text->GetTextExtent(m_text->GetValue(), &w, NULL);
    return wxSize(w + textBorder * 2, m_text->GetCharHeight() + textBorder * 2);
}

void RebusTextCtrl::UpdateSize(const wxRect & rect)
{
    if (rect != GetRect())
    {
        // Center the text box.
        m_text->SetSize(textBorder,
                        textBorder,
                        rect.width - textBorder * 2,
                        rect.height - textBorder * 2);
        SetSize(rect);
    }
}


void RebusTextCtrl::OnChar(wxKeyEvent & evt)
{
    int code = evt.GetKeyCode();
    // Don't filter special keys
    if (code == WXK_RETURN || code == WXK_INSERT)
        m_grid->EndRebusEntry(); // end the rebus entry
    else if (code == WXK_ESCAPE)
        m_grid->EndRebusEntry(false); // user canceled
    else if (code < WXK_SPACE || code == WXK_DELETE || code > WXK_START)
        evt.Skip();
    else if (XGridCtrl::IsValidChar(code))
        m_text->WriteText(puz::Square::ToGrid(code)); // make text uppercase
    else
        wxBell();
}


void RebusTextCtrl::OnPaste(wxClipboardTextEvent & evt)
{
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            m_text->WriteText(puz2wx(puz::Square::ToGrid(wx2puz(data.GetText()))));
        }
        wxTheClipboard->Close();
    }
    else
        evt.Skip();
}

void RebusTextCtrl::OnSelectAll(wxCommandEvent & WXUNUSED(evt))
{
    m_text->SelectAll();
}






XGridRebusCtrl::XGridRebusCtrl(XGridCtrl * grid)
    : wxWindow(grid, wxID_ANY, wxDefaultPosition, wxSize(0,0), wxBORDER_SIMPLE),
      m_grid(grid)
{
    m_text = new RebusTextCtrl(this, grid);
    Connect(GetTextCtrl()->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(XGridRebusCtrl::OnTextChanged));
    SetBackgroundColour(grid->GetFocusedLetterColor());
    // Config
    ConfigManager::Grid_t & cfg = wxGetApp().GetConfigManager().Grid;
    cfg.focusedLetterColor.AddCallback(this, &XGridRebusCtrl::SetBackgroundColour);
    cfg.Update(this);
    UpdateSize();
}


XGridRebusCtrl::~XGridRebusCtrl()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);
}

wxTextCtrl * XGridRebusCtrl::GetTextCtrl()
{
    return m_text->GetTextCtrl();
}

const wxTextCtrl * XGridRebusCtrl::GetTextCtrl() const
{
    return m_text->GetTextCtrl();
}


void XGridRebusCtrl::UpdateSize()
{
    puz::Square * square = m_grid->GetFocusedSquare();
    XGridDrawer & drawer = m_grid->GetDrawer();
    wxRect squareRect = drawer.GetSquareRect(*square);
    // Size the TextCtrl
    // ----
    wxTextCtrl * ctrl = GetTextCtrl();
    const int outline = ctrl->GetFont().GetPointSize() / 3;
    wxSize borderSize(
        outline * 2 + wxSystemSettings::GetMetric(wxSYS_BORDER_X),
        outline * 2 + wxSystemSettings::GetMetric(wxSYS_BORDER_Y));
    wxSize minTextSize(m_text->GetTextSize().x + ctrl->GetCharWidth() * 2,
                       ctrl->GetEffectiveMinSize().y);
    // Minimum is the size of the square or the size of the textctrl.
    wxSize ownSize(std::max(squareRect.width, minTextSize.x + borderSize.y),
                   std::max(squareRect.height, minTextSize.y + borderSize.y));
    // Max width is four times the height.
    if (ownSize.x > ownSize.y * 4)
        ownSize.x = ownSize.y * 4;
    // Shrink the text size by the border size
    wxSize textSize = ownSize - borderSize;
    m_text->UpdateSize(wxRect(wxPoint(outline, outline), textSize));
    // Center on the square
    wxRect rect(wxPoint(0,0), ownSize);
    rect = rect.CenterIn(squareRect);
    // Calculate our own size (including the outline)
    // ----
    // Don't let this window run off the grid
    wxRect gridRect = drawer.GetRect();
    if (rect.x < gridRect.x)
        rect.x = gridRect.x + drawer.GetBorderSize();
    else if (rect.GetRight() > gridRect.GetRight())
        rect.x = gridRect.GetRight() - rect.width - drawer.GetBorderSize();
    if (rect.y < gridRect.y)
        rect.y = gridRect.y + drawer.GetBorderSize();
    else if (rect.GetBottom() > gridRect.GetBottom())
        rect.y = gridRect.GetBottom() - rect.height - drawer.GetBorderSize();

    SetSize(rect);
}

void XGridRebusCtrl::OnTextChanged(wxCommandEvent & evt)
{
    UpdateSize();
    // Move the insertion point to the beginning and back to scroll the entire
    // word into view
    wxTextCtrl * text = GetTextCtrl();
    long old = text->GetInsertionPoint();
    text->SetInsertionPoint(0);
    text->SetInsertionPoint(old);
    evt.Skip();
}