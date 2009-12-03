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
                  const wxString & logfile = wxEmptyString)
        : wxFB_ConvertDialog(parent, wxID_ANY)
    {
        // Setup the files list
        m_list->InsertColumn(0, _T("Input"));
        m_list->InsertColumn(1, _T("Output"));
        m_list->InsertColumn(2, _T("Status"));

        wxASSERT(input.size() == output.size() || output.IsEmpty());

        // Set the output directory options
        const bool hasOutputDir = ! output_directory.IsEmpty();
        m_outputDirectory->Enable(hasOutputDir);
        m_outputDirectory->SetPath(output_directory);
        m_specifyDirectory->SetValue(hasOutputDir);

        // Set the log file options
        const bool hasLogfile = ! logfile.IsEmpty();
        m_logfile->Enable(hasLogfile);
        m_logfile->SetPath(logfile);
        m_useLog->SetValue(hasOutputDir);

        // Input / Output files
        const bool hasOutput = ! output.IsEmpty();
        for (int i = 0; i < input.size(); ++i)
        {
            if (hasOutput)
                AddFile(input[i], output[i]);
            else
                AddFile(input[i]);
        }

        // Only show the options if we haven't been given any.
        ShowOptions(! (hasOutputDir || hasLogfile || ! input.IsEmpty()));
    }

protected:
    bool m_isRunning;
    // [index] = ( (input, output), status )
    std::list< std::pair< std::pair<wxString, wxString>, int > > m_items;

    // Show / hide the options pane
    //-----------------------------

    void ShowOptions(bool show = true)
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
        Freeze();
        m_mainSizer->Layout();
        Fit();
        Thaw();
    }

    void HideOptions() { ShowOptions(false); }

    void ToggleOptions()
    {
        ShowOptions(! m_mainSizer->IsShown(m_optionsSizer));
    }

    // ListCtrl Adding / editing
    //--------------------------
    wxString GetOutputFile(const wxString & input)
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

    bool AddFile(const wxString & input,
                 const wxString & output = wxEmptyString)
    {
        long index = m_list->InsertItem(m_list->GetItemCount(), input);
        if (index == wxNOT_FOUND)
            return false;
        if (! output.IsEmpty())
            m_list->SetItem(index, 1, output);
        else
            m_list->SetItem(index, 1, GetOutputFile(input));

        m_items.push_back(
            std::make_pair(std::make_pair(input, output), index)
        )

        return true;
    }

    void RemoveFile(int index)
    {
        m_list->DeleteItem(index);
        m_items.remove(index);
    }

    void MakeQueue()
    {
        m_queue.clear();
        wxListItem item;
        int index = -1;
        for (;;)
        {
            index = m_list->GetNextItem(index);
            if (index == wxNOT_FOUND)
                break;
            item.SetId(index);
            item.SetColumn(2); // The "Status" column
            m_list->GetItem(item);
            if (item.GetText() != _T("Done"))
            {
                wxASSERT(item.GetText() == wxEmptyString);
                
                item.SetColumn(0);
                m_list->GetItem(item);
                const wxString input = item.GetText();

                item.SetColumn(1);
                m_list->GetItem(item);
                const wxString output = item.GetText();

                m_queue.push_back( std::make_pair(input, output) );
            }
        }
    }

    bool ConvertNextFile()
    {
        std::pair<wxString, wxString> file_pair = m_queue.front();
        m_queue.pop_front();

        wxFileName in(file_pair.first);
        wxFileName out(file_pair.second);
        // Make sure we have a directory
        out.Mkdir(0777, wxPATH_MKDIR_FULL);

        XPuzzle puz;
        // If we don't understand this extension, treat it as a text file.
        if (XPuzzle::CanLoad(in.GetExt()))
            puz.Load(in.GetFullPath());
        else
            puz.Load(in.GetFullPath(), _T("txt"));

        if (! puz.IsOk() || ! puz.Save(out.GetFullPath()))
        {

        }
    }

    // Event Handlers
    //---------------

    // Options events
    //---------------
    virtual void OnShowOptions(wxCommandEvent & WXUNUSED(evt))
    {
        ToggleOptions();
    }

	virtual void OnSpecifyDirectory(wxCommandEvent & evt)
    {
        m_outputDirectory->Enable(evt.IsChecked());
    }

	virtual void OnUseLog(wxCommandEvent & evt)
    {
        m_logfile->Enable(evt.IsChecked());
    }


    // Files list events
    //------------------
	virtual void OnAdd(wxCommandEvent & WXUNUSED(evt))
    {
        wxFileDialog dlg(this,
                         _T("Choose input file(s)"),
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
    }

	virtual void OnRemove(wxCommandEvent & WXUNUSED(evt))
    {
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

    virtual void OnRunButton(wxCommandEvent & WXUNUSED(evt))
    {
        m_isRunning = ! m_isRunning;
        if (m_isrunning)
        {
            m_runButton->SetLabel(_T("Stop"));
            MakeQueue();
            ConvertNextFile();
        }
        else
        {
            m_runButton->SetLabel(_T("Start"));
        }
    }
};