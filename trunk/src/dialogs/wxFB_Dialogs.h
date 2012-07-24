///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 10 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_DIALOGS_H__
#define __WXFB_DIALOGS_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/radiobox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/treectrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include "StyleEditors.hpp"
#include <wx/slider.h>
#include <wx/notebook.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/frame.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class PreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class PreferencesDialogBase : public wxDialog 
{
	private:
		wxStdDialogButtonSizer* m_buttons;
		wxButton* m_buttonsOK;
		wxButton* m_buttonsApply;
		wxButton* m_buttonsCancel;
	
	protected:
		wxNotebook* m_notebook;
		wxRadioBox* m_afterLetter;
		wxCheckBox* m_blankOnDirection;
		wxCheckBox* m_blankOnNewWord;
		wxRadioBox* m_pauseOnSwitch;
		wxCheckBox* m_checkWhileTyping;
		wxCheckBox* m_strictRebus;
		wxCheckBox* m_moveOnRightClick;
		wxCheckBox* m_startTimer;
		wxPanel* styleTreePanel;
		wxTreeCtrl* m_styleTree;
		wxButton* m_simpleStyleButton;
		wxPanel* m_stylePanel;
		wxBoxSizer* m_styleSizer;
		wxPanel* miscPanel;
		wxCheckBox* m_saveFileHistory;
		wxCheckBox* m_reopenLastPuzzle;
		wxStaticText* m_staticText36;
		wxSpinCtrl* m_autoSave;
		wxStaticText* m_staticText351;
		wxStaticText* m_staticText17;
		wxPanel* printPanel;
		wxCheckBox* m_printCustomFonts;
		FontPanel * m_printGridLetterFont;
		FontPanel * m_printGridNumberFont;
		FontPanel * m_printClueFont;
		wxRadioBox* m_printGridAlignment;
		wxSlider* m_printBlackSquareBrightness;
		wxPanel* m_panel8;
		wxPanel* m_printBlackSquarePreview;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) = 0;
		virtual void OnInit( wxInitDialogEvent& event ) = 0;
		virtual void OnStyleTreeSelection( wxTreeEvent& event ) = 0;
		virtual void OnSimpleStyleButton( wxCommandEvent& event ) = 0;
		virtual void OnSaveFileHistory( wxCommandEvent& event ) = 0;
		virtual void OnPrintCustomFonts( wxCommandEvent& event ) = 0;
		virtual void OnBlackSquareBrightness( wxScrollEvent& event ) = 0;
		virtual void OnApply( wxCommandEvent& event ) = 0;
		virtual void OnOK( wxCommandEvent& event ) = 0;
		
	
	public:
		
		PreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~PreferencesDialogBase();
	
};

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
/// Class MetadataFormatHelpDialog
///////////////////////////////////////////////////////////////////////////////
class MetadataFormatHelpDialog : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* m_staticText47;
		wxStaticText* m_staticText15;
		wxStaticText* m_staticText16;
		wxStaticLine* m_staticline1;
		wxStaticLine* m_staticline2;
		wxStaticLine* m_staticline13;
		wxStdDialogButtonSizer* m_sdbSizer3;
		wxButton* m_sdbSizer3OK;
		
		// Virtual event handlers, overide them in your derived class
		void OnClose( wxCloseEvent& event );
		void OnOk( wxCommandEvent& event );
		
	
	public:
		wxPanel* m_panel;
		wxFlexGridSizer* m_sizer;
		
		MetadataFormatHelpDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Display Format Help"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxFRAME_TOOL_WINDOW|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
		
		~MetadataFormatHelpDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DisplayFormatDialogBase
///////////////////////////////////////////////////////////////////////////////
class DisplayFormatDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxBoxSizer* m_sizer;
		wxStaticText* m_functionStart;
		wxStaticText* m_functionEnd;
		wxStaticText* m_result;
		wxStaticLine* m_staticline4;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton* m_sdbSizer4OK;
		wxButton* m_sdbSizer4Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTextUpdated( wxCommandEvent& event ) = 0;
		virtual void OnUseLua( wxCommandEvent& event ) = 0;
		
	
	public:
		wxTextCtrl* m_format;
		wxCheckBox* m_useLua;
		
		DisplayFormatDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Display Format"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,393 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~DisplayFormatDialogBase();
	
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
		wxRadioBox* m_numPages;
		wxStdDialogButtonSizer* m_sdbSizer5;
		wxButton* m_sdbSizer5OK;
		wxButton* m_sdbSizer5Cancel;
		
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
		virtual void OnPagesSelected( wxCommandEvent& event ) = 0;
		
	
	public:
		
		CustomPrintDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Custom Print"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~CustomPrintDialogBase();
	
};

#endif //__WXFB_DIALOGS_H__
