///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 10 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFB_STYLEPANELS_H__
#define __WXFB_STYLEPANELS_H__

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
/// Class FontPanelBase
///////////////////////////////////////////////////////////////////////////////
class FontPanelBase : public wxPanel 
{
	private:
	
	protected:
		FontFaceCtrl * m_facename;
		wxSpinCtrl* m_pointsize;
		wxBoxSizer* m_styles;
		wxToggleButton* m_bold;
		wxToggleButton* m_italic;
		wxToggleButton* m_underline;
	
	public:
		
		FontPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~FontPanelBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GridTweaksPanel
///////////////////////////////////////////////////////////////////////////////
class GridTweaksPanel : public wxPanel 
{
	private:
	
	protected:
		wxStaticLine* m_staticline1;
	
	public:
		wxSpinCtrl* m_lineThickness;
		wxSpinCtrl* m_letterScale;
		wxSpinCtrl* m_numberScale;
		
		GridTweaksPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~GridTweaksPanel();
	
};

#endif //__WXFB_STYLEPANELS_H__
