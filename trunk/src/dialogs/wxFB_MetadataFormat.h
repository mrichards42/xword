///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_METADATAFORMAT_H__
#define __WXFB_METADATAFORMAT_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_MetadataFormatDialog
///////////////////////////////////////////////////////////////////////////////
class wxFB_MetadataFormatDialog : public wxFrame 
{
	private:
	
	protected:
		wxPanel* m_panel5;
		wxBoxSizer* m_sizer;
		wxStaticText* m_functionStart;
		wxStaticText* m_functionEnd;
		wxStaticText* m_result;
		wxStaticLine* m_staticline4;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton* m_sdbSizer4OK;
		wxButton* m_sdbSizer4Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTextChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUseLua( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxTextCtrl* m_format;
		wxCheckBox* m_useLua;
		
		wxFB_MetadataFormatDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Metadata Display Format"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxFRAME_TOOL_WINDOW|wxRESIZE_BORDER|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
		
		~wxFB_MetadataFormatDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_MetadataFormatHelpPanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_MetadataFormatHelpPanel : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_text1;
		wxStaticText* m_text;
	
	public:
		wxFlexGridSizer* m_sizer;
		
		wxFB_MetadataFormatHelpPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_MetadataFormatHelpPanel();
	
};

#endif //__WXFB_METADATAFORMAT_H__
