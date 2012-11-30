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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/settings.h>
#   include <wx/dc.h>
#endif

#include "vcombo.hpp"
#include <wx/vlbox.h>

// ----------------------------------------------------------------------------
// The custom popup class
// ----------------------------------------------------------------------------

const int VirtualComboBox::s_popupBorder = 3;

class VirtualComboPopup : public wxComboPopup,
                          public wxVListBox
{
public:
    // ------------------------------------------------------------------------
    // Base class overrides
    // ------------------------------------------------------------------------

    // Create popup control
    virtual bool Create(wxWindow* parent)
    {
        return wxVListBox::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    }

    // Return pointer to the created control
    virtual wxWindow * GetControl() { return this; }

    // Translate string into a list selection
    virtual void SetStringValue(const wxString& s)
    {
        VirtualComboBox * combo = GetCombo();
        int idx = combo->FindItem(s);
        if (idx != wxNOT_FOUND)
        {
            size_t count = combo->GetCount();
            if (GetItemCount() != count)
                SetItemCount(count);
            SetSelection(idx);
        }
    }

    // Get list selection as a string
    virtual wxString GetStringValue() const
    {
        int idx = wxVListBox::GetSelection();
        if (idx != wxNOT_FOUND)
            return GetCombo()->GetItem(static_cast<size_t>(idx));
        return wxEmptyString;
    }

    virtual void OnPopup()
    {
        // Select the current item and scroll to it
        VirtualComboBox * combo = GetCombo();
        combo->OnPopup();
        int idx = combo->FindItem(combo->GetValue());
        SetSelection(idx);
        if (idx == wxNOT_FOUND)
            wxVListBox::ScrollToLine(0);
        else
            wxVListBox::ScrollToLine(static_cast<size_t>(idx));
    }

    wxSize VirtualComboPopup::GetAdjustedSize(int minWidth, int prefHeight, int maxHeight);

protected:
    VirtualComboBox * GetCombo() { return (VirtualComboBox*)m_combo; }
    const VirtualComboBox * GetCombo() const { return (VirtualComboBox*)m_combo; }

    // Do mouse hot-tracking (which is typical in list popups)
    void OnMouseMove(wxMouseEvent & evt)
    {
        int idx = wxVListBox::HitTest(evt.GetPosition());
        if (idx != wxNOT_FOUND)
            SetSelection(idx);
    }

    // On mouse left up, set the value and close the popup
    void OnMouseClick(wxMouseEvent & WXUNUSED(evt))
    {
        Dismiss();
    }

    void OnDismiss()
    {
        // Send an event
        wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, m_combo->GetId());
        evt.SetEventObject(m_combo);
        evt.SetInt(GetSelection());
        evt.SetString(GetStringValue());
        m_combo->GetEventHandler()->AddPendingEvent(evt);
    }

    // VListBox methods
    void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
    {
        // Set the text color and font
        wxColour textColor;
        if (IsSelected(n))
            textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
        else
            textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
        dc.SetTextForeground(textColor);
        dc.SetFont(GetFont());
        GetCombo()->OnDrawItem(dc, rect, n);
    }

    void PaintComboControl(wxDC & dc, const wxRect & rect)
    {
        VirtualComboBox * combo = GetCombo();
        if (combo->GetWindowStyle() & wxCB_READONLY)
        {
            dc.SetFont(GetFont());
            GetCombo()->OnDrawCtrl(dc, rect);
        }
    }

    void OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const
    {
        wxColour bg;
        if (IsSelected(n))
            bg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
        else
            bg = GetBackgroundColour();
        dc.SetBrush(wxBrush(bg, wxSOLID));
        dc.SetPen(*wxTRANSPARENT_PEN);
        GetCombo()->OnDrawBackground(dc, rect, n);
    }

    wxCoord OnMeasureItem(size_t n) const
    {
        return GetCombo()->OnMeasureItem(n);
    }


private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(VirtualComboPopup, wxVListBox)
    EVT_MOTION(VirtualComboPopup::OnMouseMove)
    EVT_LEFT_UP(VirtualComboPopup::OnMouseClick)
END_EVENT_TABLE()



// Stolen from wxOwnerDrawnComboBox
wxSize VirtualComboPopup::GetAdjustedSize(int minWidth, int prefHeight, int maxHeight)
{
    // Make sure the popup has the correct number of items.
    size_t count = GetCombo()->GetCount();
    if (GetItemCount() != count)
        SetItemCount(count);

    int height = 250;

    maxHeight -= 2;  // Must take borders into account

    if (count > 0)
    {
        if (prefHeight > 0)
            height = prefHeight;

        if (height > maxHeight)
            height = maxHeight;

        int totalHeight = EstimateTotalHeight(); // + 3;

#if defined(__WXMAC__)
        // Take borders into account, or there will be scrollbars even for one or two items.
        totalHeight += 2;
#endif
        if ( height >= totalHeight )
        {
            height = totalHeight;
        }
        else
        {
            // Adjust height to a multiple of the height of the first item
            // NB: Calculations that take variable height into account
            //     are unnecessary.
            int flh = OnGetRowHeight(0);
            height -= height % flh;
        }
    }
    else
        height = 50;

    // Take scrollbar into account in width calculations
    return wxSize(minWidth, height+2);
}



// ----------------------------------------------------------------------------
// The ComboCtrl class
// ----------------------------------------------------------------------------

VirtualComboBox::VirtualComboBox(wxWindow * parent, wxWindowID id,
                                 const wxString & value, const wxPoint & pos,
                                 const wxSize & size, long style)
    : wxComboCtrl(parent, id, value, pos, size, style)
{
    // Create the popup ctrl
    SetPopupControl(new VirtualComboPopup());
    SetPopupMaxHeight(200);
}


void VirtualComboBox::OnDrawBackground(wxDC & dc, const wxRect & rect, size_t n) const
{
    dc.DrawRectangle(rect);
}

wxCoord VirtualComboBox::OnMeasureItem(size_t n) const
{
    return GetCharHeight() + s_popupBorder * 2;
}


void VirtualComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    // Draw the text (with a border)
    wxRect smallRect = rect;
    smallRect.Deflate(s_popupBorder);
    dc.DrawLabel(GetItem(n), smallRect, wxALIGN_CENTER_VERTICAL);
}

void VirtualComboBox::OnDrawCtrl(wxDC & dc, const wxRect & rect)
{
    wxRect text(rect);
    text.Offset(GetTextIndent(), 0);
    dc.DrawLabel(GetValue(), text, wxALIGN_CENTER_VERTICAL);
}