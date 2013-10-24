///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_PREFERENCESPANELS_H__
#define __WXFB_PREFERENCESPANELS_H__

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
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include "fontpicker.hpp"
#include <wx/slider.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_SolvePanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_SolvePanel : public wxPanel 
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
		
		wxFB_SolvePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_SolvePanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_AppearancePanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_AppearancePanel : public wxPanel 
{
	private:
	
	protected:
		wxChoice* m_advancedChoice;
		wxButton* m_defaultsBtn;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnAdvancedChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnResetDefaults( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxFB_AppearancePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_AppearancePanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_StartupPanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_StartupPanel : public wxPanel 
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
		
		wxFB_StartupPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_StartupPanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_PrintPanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_PrintPanel : public wxPanel 
{
	private:
	
	protected:
		wxCheckBox* m_printCustomFonts;
		FontPickerPanel * m_printGridLetterFont;
		FontPickerPanel * m_printGridNumberFont;
		FontPickerPanel * m_printClueFont;
		wxRadioBox* m_printGridAlignment;
		wxStaticText* m_staticText13;
		wxSlider* m_printBlackSquareBrightness;
		wxStaticText* m_staticText14;
		wxPanel* m_panel8;
		wxPanel* m_printBlackSquarePreview;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnPrintCustomFonts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBlackSquareBrightness( wxScrollEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxFB_PrintPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_PrintPanel();
	
};

#endif //__WXFB_PREFERENCESPANELS_H__
