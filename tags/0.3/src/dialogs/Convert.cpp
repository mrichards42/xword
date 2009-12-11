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


#include "Convert.hpp"
#include <wx/log.h>
#include "../puz/HandlerBase.hpp" // Loading / saving exceptions

enum ConversionStatus
{
    QUEUED,     // Never attempted
    TRY_AGAIN,  // Conversion failed, not a fatal error
    FAILED,     // Conversion failed with a fatal error
    DONE,       // Conversion completed successfully
    SUCCESS = DONE
};

// For some entirely unknown reason, I can't compile this if the wxApp include
// Is placed before enum ConversionStatus.
#include <wx/app.h> // for wxTheApp->Yield();


// Drop target for the conversion dialog
//--------------------------------------
#include <wx/dnd.h>
class ConvertDialogDropTarget : public wxFileDropTarget
{
public:
    ConvertDialogDropTarget(ConvertDialog * dialog)
        : m_dlg(dialog)
    {}

    virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxArrayString & filenames);
private:
    ConvertDialog * m_dlg;
};



ConvertDialog::ConvertDialog(wxWindow * parent,
                             const wxArrayString & input,
                             const wxArrayString & output,
                             const wxString & output_directory,
                             const wxString & logfile,
                             bool overwrite,
                             bool strict_errors)
    : wxFB_ConvertDialog(parent, wxID_ANY),
      m_conversionIndex(-1)
{
    // Setup the files list
    m_list->InsertColumn(0, _T("Input"), wxLIST_FORMAT_LEFT, 200);
    m_list->InsertColumn(1, _T("Output"), wxLIST_FORMAT_LEFT, 200);
    m_list->InsertColumn(2, _T("Status")), wxLIST_FORMAT_LEFT, 50;

    // Set the output directory options
    const bool hasOutputDir = ! output_directory.IsEmpty();
    m_outputDirectory->Enable(hasOutputDir);
    m_outputDirectory->SetPath(output_directory);
    m_specifyDirectory->SetValue(hasOutputDir);

    // Set the log file options
    const bool hasLogfile = ! logfile.IsEmpty();
    m_logfile->Enable(hasLogfile);
    m_logfile->SetPath(logfile);
    m_useLog->SetValue(hasLogfile);

    // Set the name collision radiobox
    m_overwrite->SetSelection( (overwrite ? 1 : 0) );

    // Set the error handling radiobox
    m_errorHandling->SetSelection( (strict_errors ? 1 : 0) );

    // Input / Output files
    wxASSERT(input.size() == output.size() || output.IsEmpty());
    const bool hasOutput = ! output.IsEmpty();
    for (size_t i = 0; i < input.size(); ++i)
    {
        wxTheApp->Yield();
        if (hasOutput)
            AddFile(input[i], output[i]);
        else
            AddFile(input[i]);
    }
    if (! input.empty())
    {
        AutoSizeColumn(0);
        AutoSizeColumn(1);
    }

    SetMinSize(wxSize(GetSize().GetWidth(), -1));

    // Only show the options if we haven't been given any.
    ShowOptions(! (hasOutputDir || hasLogfile || ! input.IsEmpty()));

    // Make a drop target for files
    SetDropTarget(new ConvertDialogDropTarget(this));

    // Connect the idle event
    Connect(wxEVT_IDLE, wxIdleEventHandler(ConvertDialog::OnIdle));
}



//------------------------------------------------------------------------------
// Files list
//------------------------------------------------------------------------------

bool
ConvertDialog::AddFile(const wxString & input, const wxString & output)
{
    long index = m_list->InsertItem(m_list->GetItemCount(), input);
    if (index == wxNOT_FOUND)
        return false;
    if (! output.IsEmpty())
        SetItemText(index, 1, RenameFile(output));
    else
        SetItemText(index, 1, RenameFile(GetOutputFile(input)));

    // This item hasn't been processed
    m_conversionStatus.push_back(false);

    wxLogDebug(_T("Added: %s, %s, %s"),
                GetItemText(index, 0).c_str(),
                GetItemText(index, 1).c_str(),
                GetItemText(index, 2).c_str());

    return true;
}

void
ConvertDialog::RemoveFile(long index)
{
    wxASSERT(index < m_conversionStatus.size() && index > 0);
    m_list->DeleteItem(index);
    m_conversionStatus.erase(m_conversionStatus.begin() + index);

    // Keep the current conversion index up to date.
    if (index <= m_conversionIndex)
        --m_conversionIndex;
}



void
ConvertDialog::HitTest(const wxPoint & pos, long * index, int * col)
{
    *index = wxNOT_FOUND;
    *col = wxNOT_FOUND;
    // We can get the column using wxListCtrl::HitTest only in Windows, so we'll just
    // figure out a different way to get the column.
    int flags;
    *index = m_list->HitTest(pos, flags, NULL);
    if (*index == wxNOT_FOUND)
        return;

    int width = 0;
    for (*col = 0; *col < m_list->GetColumnCount(); ++*col)
    {
        width += m_list->GetColumnWidth(*col);
        if (pos.x < width)
            return;
    }
    *col = wxNOT_FOUND;
}



int
ConvertDialog::HandleExceptions(long index, bool ignore_errors)
{
    try
    {
        throw;
    }
    catch (FatalPuzError & error)
    {
        SetItemText(index, 2,
                    wxString(_T("Failed: ")) + error.what());
        return FAILED;
    }
    catch (BasePuzError & error)
    {
        wxString message;
        if (ignore_errors)
        {
            message = wxString(_T("Done with errors: ")) + error.what();
            SetItemText(index, 2, message);
        }
        else
        {
            message = wxString(_T("Failed: ")) + error.what();
            SetItemText(index, 2, message);
            return TRY_AGAIN;
        }
    }
    catch (...)
    {
        SetItemText(index, 2, _T("Failed: Unknown error"));
        return FAILED;
    }

    return SUCCESS;
}


int
ConvertDialog::ConvertFile(long index, bool ignore_errors)
{
    // Try to load the puzzle.
    //------------------------
    wxFileName in(GetItemText(index, 0));
    XPuzzle puz;
    // If we don't understand this extension, treat it as a text file.
    wxString ext = in.GetExt();
    if (! XPuzzle::CanLoad(in.GetExt()))
        ext = _T("txt");
    try
    {
        puz.Load(in.GetFullPath(), ext);
        SetItemText(m_conversionIndex, 2, _T("Done"));
    }
    catch (...)
    {
        const int ret = HandleExceptions(index, ignore_errors);
        if (ret != SUCCESS)
            return ret;
    }

    // Try to save the file
    //---------------------
    wxFileName out(GetItemText(index, 1));
    // Make sure we have a directory
    out.Mkdir(0777, wxPATH_MKDIR_FULL);

    // As a little quirk, we shouldn't set the Status column's text
    // here, because it was set in the loading phase.
    // If something exceptional happens, all bets are off, and the
    // status column will reflect the problem that happened when saving.
    try
    {
        puz.Save(out.GetFullPath());
        return DONE;
    }
    catch (...)
    {
        return HandleExceptions(index, ignore_errors);
    }
}


void
ConvertDialog::AutoSizeColumn(int index)
{
    m_list->SetColumnWidth(index, wxLIST_AUTOSIZE);
}

void
ConvertDialog::AutoSizeColumns()
{
    for (int i = 0; i < m_list->GetColumnCount(); ++i)
        m_list->SetColumnWidth(i, wxLIST_AUTOSIZE);
}

// Note that the StartConversion and StopConversion functions don't actually
// do anything to start or stop the conversion.  That will all be handled in
// the OnIdle event handler.
void
ConvertDialog::StartConversion()
{
    m_isRunning = true;
    m_runButton->SetLabel(_T("Stop"));
}


void
ConvertDialog::StopConversion()
{
    m_isRunning = false;
    m_runButton->SetLabel(_T("Start"));
}


wxString
ConvertDialog::GetOutputFile(const wxString & input)
{
    wxFileName in(input);
    wxFileName out;
    if (m_specifyDirectory->GetValue())
    {
        out.AssignDir(m_outputDirectory->GetPath());
        out.SetName(in.GetName());
    }
    else
    {
        wxASSERT(m_useInput->GetValue());
        out = in;
    }

    // Set the extension to the "opposite" of the input file.
    if (in.GetExt().Lower() == _T("puz"))
        out.SetExt(_T("txt"));
    else
        out.SetExt(_T("puz"));

    return out.GetFullPath();
}

wxString
ConvertDialog::RenameFile(wxFileName filename)
{
    // Check for a name collision
    if (m_overwrite->GetSelection() == 1 || ! ShouldRename(filename))
        return filename.GetFullPath();

    // Try appending a number to the filename (up to 1000).
    wxString original_name = filename.GetName();
    for (int i = 1; ShouldRename(filename) && i < 1000; ++i)
    {
        filename.SetName(original_name +
                    wxString::Format(_T("_%d"), i));
    }
    // No fallback if this doesn't work.
    // filename_1000.ext will just get overwritten.

    return filename.GetFullPath();
}

bool
ConvertDialog::ShouldRename(const wxFileName & filename)
{
    // Check file existence.
    if (filename.FileExists())
        return true;

    // Check against any other output file names.
    const wxString name = filename.GetFullPath();
    for (int i = 0; i < m_list->GetItemCount(); ++i)
        if (GetItemText(i, 1) == name)
            return true;
    return false;
}



wxString
ConvertDialog::GetItemText(long index, int col)
{
    wxListItem item;
    item.SetId(index);
    item.SetColumn(col);
    item.SetMask(wxLIST_MASK_TEXT);
    m_list->GetItem(item);
    return item.GetText();
}


//------------------------------------------------------------------------------
// Options
//------------------------------------------------------------------------------

void
ConvertDialog::ShowOptions(bool show)
{
    if (show == m_mainSizer->IsShown(m_optionsSizer))
        return;

    if (show)
    {
        m_optionsButton->SetLabel(_T("Options <<<"));
        m_mainSizer->Show(m_optionsSizer, true);
    }
    else
    {
        m_optionsButton->SetLabel(_T("Options >>>"));
        m_mainSizer->Show(m_optionsSizer, false);
    }

    // Calculate the new size.
    // Don't just use Fit() becaues it could make the window smaller
    // after the user enlarged the window.
    const wxSize old_size = GetSize();
    const wxSize new_size = ClientToWindowSize(m_mainSizer->GetMinSize());
    m_mainSizer->Layout();
    SetSize( wxSize(std::max(old_size.x, new_size.x),
                    std::max(old_size.y, new_size.y)) );
}






//------------------------------------------------------------------------------
// Event Handlers
//------------------------------------------------------------------------------

void
ConvertDialog::OnAdd(wxCommandEvent & WXUNUSED(evt))
{
    wxFileDialog dlg(this,
                     _T("Select input file(s)"),
                     wxEmptyString,
                     wxEmptyString,
                     XPuzzle::GetLoadTypeString(),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        for (wxArrayString::iterator it = files.begin();
             it != files.end();
             ++it)
        {
            AddFile(*it);
        }
    }
    AutoSizeColumn(0);
    AutoSizeColumn(1);
}

void
ConvertDialog::OnRemove(wxCommandEvent & WXUNUSED(evt))
{
    // Iterate through all selected lines and remove them
    int index = -1;
    for (;;)
    {
        index = m_list->GetNextItem(index, wxLIST_NEXT_ALL,
                                           wxLIST_STATE_SELECTED);
        if (index == wxNOT_FOUND)
            break;

        RemoveFile(index);
    }
}

void
ConvertDialog::OnDoubleClick(wxMouseEvent & evt)
{
    long index;
    int col;
    HitTest(evt.GetPosition(), &index, &col);
    if (index == wxNOT_FOUND || col == wxNOT_FOUND)
        return;

    wxFileName filename(GetItemText(index, col));
    wxString result;
    if (col == 0)
    {
        result = 
            wxFileSelector(_T("Select the input file"),
                           filename.GetPath(),
                           filename.GetFullName(),
                           filename.GetExt(),
                           XPuzzle::GetLoadTypeString(),
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    }
    else if (col == 1)
    {
        result =
            wxFileSelector(_T("Select the output file"),
                           filename.GetPath(),
                           filename.GetFullName(),
                           filename.GetExt(),
                           XPuzzle::GetSaveTypeString(),
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    }
    if (result.IsEmpty())
        return;
    SetItemText(index, col, result);
    AutoSizeColumn(col);
}



void
ConvertDialog::OnRunButton(wxCommandEvent & WXUNUSED(evt))
{
    if (! m_isRunning)
        StartConversion();
    else
        StopConversion();
}


void
ConvertDialog::OnIdle(wxIdleEvent & evt)
{
    if (! m_isRunning)
    {
        m_conversionIndex = -1;
        evt.RequestMore(false);
        return;
    }

    const bool ignore_errors = m_errorHandling->GetSelection() == 0;

    // When m_conversionIndex < 0, it means that the conversion has either
    // been stopped or was never started.
    // In either case, we need to start from the beginning.
    if (m_conversionIndex < 0)
        m_conversionIndex = 0;

    for (;;)
    {
        if (m_conversionIndex >= m_conversionStatus.size())
        {
            StopConversion();
            evt.RequestMore(false);
            return;
        }

        const int status = m_conversionStatus.at(m_conversionIndex);
        if (status == QUEUED || (status == TRY_AGAIN && ignore_errors))
            // We've found one that needs to be processed
            break;
        ++m_conversionIndex;
    }

    // Do the conversion
    wxLogDebug(_T("Converting line number: %d"), m_conversionIndex);
    m_conversionStatus.at(m_conversionIndex) = ConvertFile(m_conversionIndex);
    AutoSizeColumn(2);
    ++m_conversionIndex;
    evt.RequestMore(true);
}



void
ConvertDialog::OnShowOptions(wxCommandEvent & WXUNUSED(evt))
{
    ToggleOptions();
}

void
ConvertDialog::OnSpecifyDirectory(wxCommandEvent & evt)
{
    m_outputDirectory->Enable(evt.IsChecked());
}

void
ConvertDialog::OnUseLog(wxCommandEvent & evt)
{
    m_logfile->Enable(evt.IsChecked());
}





bool
ConvertDialogDropTarget::OnDropFiles(wxCoord WXUNUSED(x),
                                     wxCoord WXUNUSED(y),
                                     const wxArrayString & filenames)
{
    for (wxArrayString::const_iterator it = filenames.begin();
         it != filenames.end();
         ++ it)
    {
        wxTheApp->Yield();
        m_dlg->AddFile(*it);
    }
    m_dlg->AutoSizeColumn(0);
    m_dlg->AutoSizeColumn(1);
    return true;
}

