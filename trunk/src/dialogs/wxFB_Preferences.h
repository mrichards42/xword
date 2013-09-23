///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_PREFERENCES_H__
#define __WXFB_PREFERENCES_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/treectrl.h>
#include <wx/spinctrl.h>
#include "StyleEditors.hpp"
#include <wx/slider.h>

///////////////////////////////////////////////////////////////////////////

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
/// Class fbSolvePanel
///////////////////////////////////////////////////////////////////////////////
class fbSolvePanel : public wxPanel 
{
	private:
	
	protected:
		wxRadioBox* m_afterLetter;
		wxCheckBox* m_blankOnDirection;
		wxCheckBox* m_blankOnNewWord;
		wxRadioBox* m_pauseOnSwitch;
		wxCheckBox* m_checkWhileTyping;
		wxCheckBox* m_strictRebus;
		wxCheckBox* m_moveOnRightClick;
		wxCheckBox* m_startTimer;
	
	public:
		
		fbSolvePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~fbSolvePanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class fbAppearancePanel
///////////////////////////////////////////////////////////////////////////////
class fbAppearancePanel : public wxPanel 
{
	private:
	
	protected:
		wxTreeCtrl* m_styleTree;
		wxButton* m_simpleStyleButton;
		wxPanel* m_stylePanel;
		wxBoxSizer* m_styleSizer;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnStyleTreeSelection( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnSimpleStyleButton( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		fbAppearancePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~fbAppearancePanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class fbStartupPanel
///////////////////////////////////////////////////////////////////////////////
class fbStartupPanel : public wxPanel 
{
	private:
	
	protected:
		wxCheckBox* m_saveFileHistory;
		wxCheckBox* m_reopenLastPuzzle;
		wxStaticText* m_staticText36;
		wxSpinCtrl* m_autoSave;
		wxStaticText* m_staticText351;
		wxStaticText* m_staticText17;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnSaveFileHistory( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		fbStartupPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~fbStartupPanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class fbPrintPanel
///////////////////////////////////////////////////////////////////////////////
class fbPrintPanel : public wxPanel 
{
	private:
	
	protected:
		wxCheckBox* m_printCustomFonts;
		FontPanel * m_printGridLetterFont;
		FontPanel * m_printGridNumberFont;
		FontPanel * m_printClueFont;
		wxRadioBox* m_printGridAlignment;
		wxSlider* m_printBlackSquareBrightness;
		wxPanel* m_panel8;
		wxPanel* m_printBlackSquarePreview;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnPrintCustomFonts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBlackSquareBrightness( wxScrollEvent& event ) { event.Skip(); }
		
	
	public:
		
		fbPrintPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~fbPrintPanel();
	
};

#endif //__WXFB_PREFERENCES_H__
