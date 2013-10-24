///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_DIALOGS_H__
#define __WXFB_DIALOGS_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class LicenseDialog
///////////////////////////////////////////////////////////////////////////////
class LicenseDialog : public wxDialog 
{
	private:
	
	protected:
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
	
	public:
		wxTextCtrl* m_textCtrl;
		
		LicenseDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("XWord License"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 371,262 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~LicenseDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CustomPrintDialogBase
///////////////////////////////////////////////////////////////////////////////
class CustomPrintDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText14;
		wxChoice* m_preset;
		wxCheckBox* m_grid;
		wxCheckBox* m_numbers;
		wxCheckBox* m_text;
		wxCheckBox* m_solution;
		wxCheckBox* m_clues;
		wxCheckBox* m_title;
		wxCheckBox* m_author;
		wxCheckBox* m_notes;
		wxRadioBox* m_numPages;
		wxButton* m_btnPrint;
		wxButton* m_btnPreview;
		wxButton* m_btnCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInit( wxInitDialogEvent& event ) = 0;
		virtual void OnPresetSelected( wxCommandEvent& event ) = 0;
		virtual void OnGridChecked( wxCommandEvent& event ) = 0;
		virtual void OnGridNumbersChecked( wxCommandEvent& event ) = 0;
		virtual void OnGridTextChecked( wxCommandEvent& event ) = 0;
		virtual void OnGridSolutionChecked( wxCommandEvent& event ) = 0;
		virtual void OnCluesChecked( wxCommandEvent& event ) = 0;
		virtual void OnTitleChecked( wxCommandEvent& event ) = 0;
		virtual void OnAuthorChecked( wxCommandEvent& event ) = 0;
		virtual void OnNotesChecked( wxCommandEvent& event ) = 0;
		virtual void OnPagesSelected( wxCommandEvent& event ) = 0;
		virtual void OnPrintButton( wxCommandEvent& event ) = 0;
		virtual void OnPreviewButton( wxCommandEvent& event ) = 0;
		virtual void OnCancelButton( wxCommandEvent& event ) = 0;
		
	
	public:
		
		CustomPrintDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Custom Print"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~CustomPrintDialogBase();
	
};

#endif //__WXFB_DIALOGS_H__
