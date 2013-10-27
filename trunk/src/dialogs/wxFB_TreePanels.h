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
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

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
