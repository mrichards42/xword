///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 11 2009)
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
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/clrpicker.h>
#include <wx/stattext.h>
#include <wx/fontpicker.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/filepicker.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_PreferencesDialog
///////////////////////////////////////////////////////////////////////////////
class wxFB_PreferencesDialog : public wxDialog 
{
	private:
		wxStdDialogButtonSizer* m_buttons;
		wxButton* m_buttonsOK;
		wxButton* m_buttonsCancel;
	
	protected:
		wxNotebook* m_notebook;
		wxRadioBox* m_afterLetter;
		wxCheckBox* m_blankOnDirection;
		wxCheckBox* m_blankOnNewWord;
		wxRadioBox* m_pauseOnSwitch;
		wxCheckBox* m_moveOnRightClick;
		wxCheckBox* m_checkWhileTyping;
		wxStaticBoxSizer* sbSizer36;
		wxColourPickerCtrl* m_selectedLetterColor;
		wxColourPickerCtrl* m_selectedWordColor;
		wxColourPickerCtrl* m_penColor;
		wxColourPickerCtrl* m_pencilColor;
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
		wxStaticText* m_staticText23;
		wxTextCtrl* m_cluePromptFormat;
		wxStaticText* m_staticText32;
		wxStaticText* m_staticText33;
		wxStaticText* m_staticText36;
		wxStaticText* m_staticText37;
		wxStaticText* m_staticText34;
		wxStaticText* m_staticText35;
		wxStaticText* m_staticText38;
		wxStaticText* m_staticText39;
		wxStaticText* m_staticText330;
		wxSpinCtrl* m_letterScale;
		wxStaticText* m_staticText331;
		wxSpinCtrl* m_numberScale;
		wxStaticText* m_staticText341;
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
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInit( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnAfterLetter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBlankOnDirection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBlankOnNewWord( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPauseOnSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMoveOnRightClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCheckWhileTyping( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectedLetterColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnSelectedWordColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnPenColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnPencilColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnPromptTextColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnPromptBackgroundColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnClueTextColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnClueBackgroundColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnSelectedClueTextColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnSelectedClueBackgroundColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnCrossingClueTextColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnCrossingClueBackgroundColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnClueHeadingTextColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnClueHeadingBackgroundColor( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void OnGridLetterFont( wxFontPickerEvent& event ) { event.Skip(); }
		virtual void OnGridNumberFont( wxFontPickerEvent& event ) { event.Skip(); }
		virtual void OnCluePromptFont( wxFontPickerEvent& event ) { event.Skip(); }
		virtual void OnClueFont( wxFontPickerEvent& event ) { event.Skip(); }
		virtual void OnClueHeadingFont( wxFontPickerEvent& event ) { event.Skip(); }
		virtual void OnCluePromptFormat( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLetterScale( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnNumberScale( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnPrintCustomFonts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPrintBlackSquareBrightness( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		
	
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

#endif //__wxFB_Dialogs__
