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


#ifndef CONVERT_DLG_H
#define CONVERT_DLG_H

#include "wxFB_Dialogs.h"
#include "../puz/XPuzzle.hpp"
#include <wx/filename.h>
#include <list>

class ConvertDialog : public wxFB_ConvertDialog
{
public:
    ConvertDialog(wxWindow * parent = NULL,
                  const wxArrayString & input = wxArrayString(),
                  const wxArrayString & output = wxArrayString(),
                  const wxString & output_directory = wxEmptyString,
                  const wxString & logfile = wxEmptyString,
                  bool overwrite = false,
                  bool strict_errors = false);

    // Files list
    bool AddFile(const wxString & input,
                 const wxString & output = wxEmptyString);
    void RemoveFile(long index);
    void AutoSizeColumn(int index);
    void AutoSizeColumns();
    // Find the list item at this point
    void HitTest(const wxPoint & pos, long * index, int * col);

    void StartConversion();
    void StopConversion();
    int ConvertFile(long index, bool ignore_errors = true);
    int HandleExceptions(long index, bool ignore_errors = true);

    // Options
    void ShowOptions(bool show = true);
    void HideOptions() { ShowOptions(false); }
    void ToggleOptions()
    {
        ShowOptions(! m_mainSizer->IsShown(m_optionsSizer));
    }

protected:
    bool m_isRunning;

    // One item per line in the list.  Value indicates whether or not we
    // still need to process that item.
    // See enum ConversionStatus in Convert.cpp (internal use only anyways).
    std::vector<int> m_conversionStatus;
    long m_conversionIndex;
    
    wxString GetOutputFile(const wxString & input);
    wxString RenameFile(wxFileName filename);
    bool     ShouldRename(const wxFileName & filename);
    wxString GetItemText(long index, int col);

    // This is just for symmetry with GetItemText().
    void SetItemText(long index, int col, const wxString & text)
        { m_list->SetItem(index, col, text); }

    // Event Handlers
    //---------------

    // Options events
    //---------------
    virtual void OnShowOptions(wxCommandEvent & WXUNUSED(evt));
	virtual void OnSpecifyDirectory(wxCommandEvent & evt);
	virtual void OnUseLog(wxCommandEvent & evt);

    // Files list events
    //------------------
	virtual void OnAdd(wxCommandEvent & WXUNUSED(evt));
	virtual void OnRemove(wxCommandEvent & WXUNUSED(evt));
    virtual void OnRunButton(wxCommandEvent & WXUNUSED(evt));
    virtual void OnDoubleClick(wxMouseEvent & evt);
    // This is where the conversion takes place.
    virtual void OnIdle(wxIdleEvent & evt);
};


#endif // CONVERT_DLG_H
