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


#ifndef CUSTOM_PRINT_DLG_H
#define CUSTOM_PRINT_DLG_H

#include "wxFB_Dialogs.h"

namespace puz { class Puzzle; }
class MyFrame;

struct PrintInfo
{
    PrintInfo(int options = 0)
        : clues(true),
          grid(true),
          grid_options(options),
          two_pages(false),
          title(true),
          author(true),
          notes(false)
    {}

    bool clues;
    bool grid;
    int grid_options;
    bool two_pages;
    bool author;
    bool title;
    bool notes;
};

class CustomPrintDialog : public CustomPrintDialogBase
{
public:
    CustomPrintDialog(MyFrame * frame);

    PrintInfo GetPrintInfo() { return m_info; }

    enum buttons
    {
        ID_PRINT,
        ID_PREVIEW,
        ID_CANCEL
    };

protected:
    // Event Handlers
    virtual void OnInit(wxInitDialogEvent & evt);

    virtual void OnPresetSelected(wxCommandEvent & evt);
    virtual void OnGridChecked(wxCommandEvent & evt);
    virtual void OnGridNumbersChecked(wxCommandEvent & evt);
    virtual void OnGridTextChecked(wxCommandEvent & evt);
    virtual void OnGridSolutionChecked(wxCommandEvent & evt);
    virtual void OnCluesChecked(wxCommandEvent & evt);
    virtual void OnTitleChecked(wxCommandEvent & evt);
    virtual void OnAuthorChecked(wxCommandEvent & evt);
    virtual void OnNotesChecked(wxCommandEvent & evt);
    virtual void OnPagesSelected(wxCommandEvent & evt);

    virtual void OnPrintButton(wxCommandEvent & WXUNUSED(evt)) { EndModal(ID_PRINT); }
    virtual void OnPreviewButton(wxCommandEvent & WXUNUSED(evt)) { EndModal(ID_PREVIEW); }
    virtual void OnCancelButton(wxCommandEvent & WXUNUSED(evt)) { EndModal(ID_CANCEL); }

    void UpdatePrintInfo();
    
    PrintInfo m_info;
    puz::Puzzle * m_puz;
    bool has_solution;
};


#endif // PREFERENCES_DLG_H
