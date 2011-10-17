///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Mar 22 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wxFB_Dialogs__
#define __wxFB_Dialogs__

#include <wx/string.h>
#include <wx/radiobox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/textctrl.h>
#include <wx/gbsizer.h>
#include <wx/slider.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/filepicker.h>
#include <wx/scrolwin.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_PreferencesDialog
///////////////////////////////////////////////////////////////////////////////
class wxFB_PreferencesDialog : public wxDialog 
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
		wxStaticText* m_staticText36;
		wxSpinCtrl* m_autoSave;
		wxStaticText* m_staticText351;
		wxStaticBoxSizer* sbSizer36;
		wxColourPickerCtrl* m_penColor;
		wxColourPickerCtrl* m_selectedLetterColor;
		wxColourPickerCtrl* m_selectedWordColor;
		wxColourPickerCtrl* m_gridSelectionColor;
		wxColourPickerCtrl* m_gridBackgroundColor;
		wxColourPickerCtrl* m_whiteSquareColor;
		wxColourPickerCtrl* m_blackSquareColor;
		wxColourPickerCtrl* m_cluePromptText;
		wxColourPickerCtrl* m_cluePromptBackground;
		wxColourPickerCtrl* m_clueText;
		wxColourPickerCtrl* m_clueBackground;
		wxColourPickerCtrl* m_selectedClueText;
		wxColourPickerCtrl* m_selectedClueBackground;
		wxColourPickerCtrl* m_crossingClueText;
		wxColourPickerCtrl* m_crossingClueBackground;
		wxColourPickerCtrl* m_clueHeadingText;
		wxColourPickerCtrl* m_clueHeadingBackground;
		wxFontPickerCtrl* m_gridLetterFont;
		wxFontPickerCtrl* m_gridNumberFont;
		wxFontPickerCtrl* m_cluePromptFont;
		wxFontPickerCtrl* m_clueFont;
		wxFontPickerCtrl* m_clueHeadingFont;
		wxPanel* miscPanel;
		wxCheckBox* m_saveFileHistory;
		wxCheckBox* m_reopenLastPuzzle;
		wxTextCtrl* m_cluePromptFormat;
		wxSpinCtrl* m_letterScale;
		wxSpinCtrl* m_numberScale;
		wxSpinCtrl* m_lineThickness;
		wxPanel* printPanel;
		wxCheckBox* m_printCustomFonts;
		wxFontPickerCtrl* m_printGridLetterFont;
		wxFontPickerCtrl* m_printGridNumberFont;
		wxFontPickerCtrl* m_printClueFont;
		wxRadioBox* m_printGridAlignment;
		wxSlider* m_printBlackSquareBrightness;
		wxPanel* m_panel8;
		wxPanel* m_printBlackSquarePreview;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInit( wxInitDialogEvent& event ) = 0;
		virtual void OnSaveFileHistory( wxCommandEvent& event ) = 0;
		virtual void OnPrintCustomFonts( wxCommandEvent& event ) = 0;
		virtual void OnBlackSquareBrightness( wxScrollEvent& event ) = 0;
		virtual void OnApply( wxCommandEvent& event ) = 0;
		virtual void OnOK( wxCommandEvent& event ) = 0;
		
	
	public:
		
		wxFB_PreferencesDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~wxFB_PreferencesDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_ConvertDialog
///////////////////////////////////////////////////////////////////////////////
class wxFB_ConvertDialog : public wxDialog 
{
	private:
	
	protected:
		wxBoxSizer* m_mainSizer;
		wxListCtrl* m_list;
		wxButton* m_optionsButton;
		wxButton* m_add;
		wxButton* m_remove;
		wxButton* m_runButton;
		wxStaticBoxSizer* m_optionsSizer;
		wxRadioBox* m_overwrite;
		wxRadioBox* m_errorHandling;
		wxRadioButton* m_useInput;
		wxRadioButton* m_specifyDirectory;
		wxDirPickerCtrl* m_outputDirectory;
		wxCheckBox* m_useLog;
		wxFilePickerCtrl* m_logfile;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnDoubleClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnShowOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemove( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRunButton( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSpecifyDirectory( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUseLog( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxFB_ConvertDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Convert"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~wxFB_ConvertDialog();
	
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
/// Class wxFB_CharactersPanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_CharactersPanel : public wxPanel 
{
	private:
	
	protected:
	
	public:
		wxScrolledWindow* m_scroller;
		wxPanel* m_panel;
		wxGridSizer* m_sizer;
		
		wxFB_CharactersPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_CharactersPanel();
	
};

#endif //__wxFB_Dialogs__
