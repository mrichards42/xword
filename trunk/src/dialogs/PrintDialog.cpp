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


#include "PrintDialog.hpp"
#include "../MyFrame.hpp"
#include "../XGridDrawer.hpp" // Grid options

CustomPrintDialog::CustomPrintDialog(MyFrame * frame)
    : CustomPrintDialogBase(frame, wxID_ANY),
      has_solution(true),
      m_puz(&frame->GetPuzzle())
{
    if (m_puz->IsScrambled()
        || ! m_puz->GetGrid().HasSolution())
    {
        has_solution = false;
        m_preset->Delete(2); // Remove the "Solution Grid" preset
    }
}


void CustomPrintDialog::UpdatePrintInfo()
{
    // Make sure our info is internally consistent
    // Can't draw solution if we don't have one
    if (! has_solution && (m_info.grid_options & XGridDrawer::DRAW_SOLUTION))
    {
        m_info.grid_options &= ~ XGridDrawer::DRAW_SOLUTION;
    }
    // Can't draw text and solution
    if ((m_info.grid_options & XGridDrawer::DRAW_USER_TEXT)
        && (m_info.grid_options & XGridDrawer::DRAW_SOLUTION))
    {
        m_info.grid_options &= ~ XGridDrawer::DRAW_SOLUTION;
    }
    // Can't have two pages if we don't have both grid and clues
    if (! (m_info.grid && m_info.clues))
        m_info.two_pages = false;

    if (m_puz->IsDiagramless())
        m_info.grid_options &= ~XGridDrawer::DRAW_NUMBER;
    if (! m_puz->HasMeta(puzT("title")))
    {
        m_title->Disable();
        m_info.title = false;
    }
    if (! m_puz->HasMeta(puzT("author")))
    {
        m_author->Disable();
        m_info.author = false;
    }
    if (! m_puz->HasMeta(puzT("notes")))
    {
        m_notes->Disable();
        m_info.notes = false;
    }

    // Check and enable ctrls
    m_grid->SetValue(m_info.grid);

    m_numbers->SetValue(m_info.grid_options & XGridDrawer::DRAW_NUMBER);
    m_text->SetValue(m_info.grid_options & XGridDrawer::DRAW_USER_TEXT);
    m_solution->SetValue(m_info.grid_options & XGridDrawer::DRAW_SOLUTION);

    m_numbers->Enable(m_info.grid);
    m_text->Enable(m_info.grid);
    m_solution->Enable(m_info.grid);
    if (m_puz->IsDiagramless())
        m_numbers->Disable();
    if (! has_solution)
        m_solution->Disable();

    m_clues->SetValue(m_info.clues);

    m_title->SetValue(m_info.title);
    m_author->SetValue(m_info.author);
    m_notes->SetValue(m_info.notes);

    m_numPages->SetSelection(m_info.two_pages ? 1 : 0);
    m_numPages->Enable(m_info.grid && m_info.clues);
}

void CustomPrintDialog::OnInit(wxInitDialogEvent & evt)
{
    m_info.grid = true;
    m_info.grid_options = XGridDrawer::DRAW_NUMBER;
    m_info.clues = true;
    m_info.title = true;
    m_info.author = true;
    m_info.two_pages = false;
    UpdatePrintInfo();
}


void CustomPrintDialog::OnPresetSelected(wxCommandEvent & evt)
{
    int selection = evt.GetSelection();
     // Adjust selection if "Solution Grid" is not an option
    if (! has_solution && selection > 1)
        ++selection;
    switch (selection)
    {
        case 0: // Blank Grid
            m_info.grid = true;
            m_info.grid_options = XGridDrawer::DRAW_NUMBER;
            m_info.clues = true;
            m_info.title = true;
            m_info.author = true;
            m_info.two_pages = false;
            break;
        case 1: // Current Progress
            m_info.grid = true;
            m_info.grid_options = XGridDrawer::DRAW_NUMBER
                                  | XGridDrawer::DRAW_USER_TEXT;
            m_info.clues = true;
            m_info.title = true;
            m_info.author = true;
            m_info.two_pages = false;
            break;
        case 2: // Solution Grid
            m_info.grid = true;
            m_info.grid_options = XGridDrawer::DRAW_SOLUTION;
            m_info.clues = false;
            m_info.title = true;
            m_info.author = true;
            m_info.two_pages = false;
            break;
        case 3: // Clues
            m_info.grid = false;
            m_info.clues = true;
            m_info.title = true;
            m_info.author = true;
            m_info.two_pages = false;
            break;
        case 4: // Sunday (Two Pages)
            m_info.grid = true;
            m_info.grid_options = XGridDrawer::DRAW_NUMBER;
            m_info.clues = true;
            m_info.title = true;
            m_info.author = true;
            m_info.two_pages = true;
            break;
        default: // Custom
            return;
    }
    UpdatePrintInfo();
    evt.Skip();
}


void CustomPrintDialog::OnGridChecked(wxCommandEvent & evt)
{
    m_info.grid = evt.IsChecked();
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnGridTextChecked(wxCommandEvent & evt)
{
    if (evt.IsChecked())
    {
        m_info.grid_options |= XGridDrawer::DRAW_USER_TEXT;
        if (m_solution->IsChecked())
            m_info.grid_options &= ~XGridDrawer::DRAW_SOLUTION;
    }
    else
    {
        m_info.grid_options &= ~XGridDrawer::DRAW_USER_TEXT;
    }
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnGridSolutionChecked(wxCommandEvent & evt)
{
    if (evt.IsChecked())
    {
        m_info.grid_options |= XGridDrawer::DRAW_SOLUTION;
        if (m_text->IsChecked())
            m_info.grid_options &= ~XGridDrawer::DRAW_USER_TEXT;
    }
    else
    {
        m_info.grid_options &= ~XGridDrawer::DRAW_SOLUTION;
    }
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnGridNumbersChecked(wxCommandEvent & evt)
{
    if (evt.IsChecked())
        m_info.grid_options |= XGridDrawer::DRAW_NUMBER;
    else
        m_info.grid_options &= ~XGridDrawer::DRAW_NUMBER;
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnCluesChecked(wxCommandEvent & evt)
{
    m_info.clues = evt.IsChecked();
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnTitleChecked(wxCommandEvent & evt)
{
    m_info.title = evt.IsChecked();
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnAuthorChecked(wxCommandEvent & evt)
{
    m_info.author = evt.IsChecked();
    UpdatePrintInfo();
    evt.Skip();
}

void CustomPrintDialog::OnNotesChecked(wxCommandEvent & evt)
{
    m_info.notes = evt.IsChecked();
    UpdatePrintInfo();
    evt.Skip();
}


void CustomPrintDialog::OnPagesSelected(wxCommandEvent & evt)
{
    m_info.two_pages = evt.GetSelection() == 1;
    UpdatePrintInfo();
    evt.Skip();
}
