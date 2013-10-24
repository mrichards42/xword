///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_TreePanels.h"

///////////////////////////////////////////////////////////////////////////

wxFB_FontPickerPanel::wxFB_FontPickerPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	m_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_top = new wxBoxSizer( wxHORIZONTAL );
	
	m_facename = new FontFaceCtrl(this, wxID_ANY);
	m_top->Add( m_facename, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND, 0 );
	
	m_pointsize = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 5, 100, 5 );
	m_pointsize->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	m_top->Add( m_pointsize, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 2 );
	
	
	m_sizer->Add( m_top, 0, wxEXPAND, 5 );
	
	m_bottom = new wxBoxSizer( wxHORIZONTAL );
	
	m_bold = new wxToggleButton( this, wxID_ANY, wxT("B"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_bold->SetFont( wxFont( 10, 72, 90, 92, false, wxEmptyString ) );
	
	m_bottom->Add( m_bold, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2 );
	
	m_italic = new wxToggleButton( this, wxID_ANY, wxT("I"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_italic->SetFont( wxFont( 10, 72, 93, 90, false, wxEmptyString ) );
	
	m_bottom->Add( m_italic, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2 );
	
	m_underline = new wxToggleButton( this, wxID_ANY, wxT("U"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_underline->SetFont( wxFont( 10, 72, 90, 90, true, wxEmptyString ) );
	
	m_bottom->Add( m_underline, 0, wxALIGN_CENTER_VERTICAL, 2 );
	
	
	m_sizer->Add( m_bottom, 0, 0, 5 );
	
	
	this->SetSizer( m_sizer );
	this->Layout();
	m_sizer->Fit( this );
}

wxFB_FontPickerPanel::~wxFB_FontPickerPanel()
{
}

wxFB_GridTweaks::wxFB_GridTweaks( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxGridBagSizer* gbSizer1;
	gbSizer1 = new wxGridBagSizer( 3, 3 );
	gbSizer1->SetFlexibleDirection( wxBOTH );
	gbSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText41;
	m_staticText41 = new wxStaticText( this, wxID_ANY, wxT("Line Thickness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	gbSizer1->Add( m_staticText41, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_lineThickness = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxSP_ARROW_KEYS, 1, 10, 1 );
	gbSizer1->Add( m_lineThickness, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText411;
	m_staticText411 = new wxStaticText( this, wxID_ANY, wxT("pixel(s)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText411->Wrap( -1 );
	gbSizer1->Add( m_staticText411, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	gbSizer1->Add( m_staticline1, wxGBPosition( 1, 0 ), wxGBSpan( 1, 3 ), wxEXPAND|wxTOP|wxBOTTOM, 5 );
	
	wxStaticText* m_staticText341;
	m_staticText341 = new wxStaticText( this, wxID_ANY, wxT("Percent of the square taken up by text and number:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText341->Wrap( -1 );
	m_staticText341->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	gbSizer1->Add( m_staticText341, wxGBPosition( 2, 0 ), wxGBSpan( 1, 3 ), wxALIGN_CENTER_VERTICAL, 10 );
	
	wxStaticText* m_staticText3411;
	m_staticText3411 = new wxStaticText( this, wxID_ANY, wxT("Does not have to add up to 100."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3411->Wrap( -1 );
	m_staticText3411->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	
	gbSizer1->Add( m_staticText3411, wxGBPosition( 3, 0 ), wxGBSpan( 1, 3 ), wxBOTTOM, 5 );
	
	wxStaticText* m_staticText330;
	m_staticText330 = new wxStaticText( this, wxID_ANY, wxT("Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText330->Wrap( -1 );
	gbSizer1->Add( m_staticText330, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_letterScale = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxSP_ARROW_KEYS, 5, 95, 40 );
	gbSizer1->Add( m_letterScale, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText3301;
	m_staticText3301 = new wxStaticText( this, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3301->Wrap( -1 );
	gbSizer1->Add( m_staticText3301, wxGBPosition( 4, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText331;
	m_staticText331 = new wxStaticText( this, wxID_ANY, wxT("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText331->Wrap( -1 );
	gbSizer1->Add( m_staticText331, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_numberScale = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxSP_ARROW_KEYS, 5, 95, 75 );
	gbSizer1->Add( m_numberScale, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText3311;
	m_staticText3311 = new wxStaticText( this, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3311->Wrap( -1 );
	gbSizer1->Add( m_staticText3311, wxGBPosition( 5, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( gbSizer1 );
	this->Layout();
	gbSizer1->Fit( this );
}

wxFB_GridTweaks::~wxFB_GridTweaks()
{
}
