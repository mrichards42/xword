///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_TREEPANELS_H__
#define __WXFB_TREEPANELS_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "fontface.hpp"
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/tglbtn.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/gbsizer.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_FontPickerPanel
///////////////////////////////////////////////////////////////////////////////
class wxFB_FontPickerPanel : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* m_sizer;
		wxBoxSizer* m_top;
		FontFaceCtrl * m_facename;
		wxSpinCtrl* m_pointsize;
		wxBoxSizer* m_bottom;
		wxToggleButton* m_bold;
		wxToggleButton* m_italic;
		wxToggleButton* m_underline;
	
	public:
		
		wxFB_FontPickerPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_FontPickerPanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxFB_GridTweaks
///////////////////////////////////////////////////////////////////////////////
class wxFB_GridTweaks : public wxPanel 
{
	private:
	
	protected:
		wxStaticLine* m_staticline1;
	
	public:
		wxSpinCtrl* m_lineThickness;
		wxSpinCtrl* m_letterScale;
		wxSpinCtrl* m_numberScale;
		
		wxFB_GridTweaks( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~wxFB_GridTweaks();
	
};

#endif //__WXFB_TREEPANELS_H__
