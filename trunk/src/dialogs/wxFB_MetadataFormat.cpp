///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_MetadataFormat.h"

///////////////////////////////////////////////////////////////////////////

wxFB_MetadataFormatDialog::wxFB_MetadataFormatDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	m_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_functionStart = new wxStaticText( m_panel5, wxID_ANY, wxT("function(puzzle)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_functionStart->Wrap( -1 );
	m_functionStart->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_functionStart, 0, wxBOTTOM, 5 );
	
	m_format = new wxTextCtrl( m_panel5, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE );
	m_format->SetMaxLength( 0 ); 
	m_format->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_format, 1, wxEXPAND|wxLEFT, 15 );
	
	m_functionEnd = new wxStaticText( m_panel5, wxID_ANY, wxT("end"), wxDefaultPosition, wxDefaultSize, 0 );
	m_functionEnd->Wrap( -1 );
	m_functionEnd->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_functionEnd, 0, wxTOP, 5 );
	
	m_useLua = new wxCheckBox( m_panel5, wxID_ANY, wxT("Format as a lua script"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer->Add( m_useLua, 0, wxTOP|wxBOTTOM, 10 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( m_panel5, wxID_ANY, wxT("Result") ), wxVERTICAL );
	
	m_result = new wxStaticText( m_panel5, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_result->Wrap( -1 );
	sbSizer12->Add( m_result, 0, wxALL, 5 );
	
	
	m_sizer->Add( sbSizer12, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );
	
	
	bSizer17->Add( m_sizer, 1, wxEXPAND|wxALL, 10 );
	
	m_staticline4 = new wxStaticLine( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer17->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( m_panel5, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( m_panel5, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();
	
	bSizer17->Add( m_sdbSizer4, 0, wxEXPAND|wxALL, 5 );
	
	
	m_panel5->SetSizer( bSizer17 );
	m_panel5->Layout();
	bSizer17->Fit( m_panel5 );
	bSizer18->Add( m_panel5, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer18 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_format->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxFB_MetadataFormatDialog::OnTextChanged ), NULL, this );
	m_useLua->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_MetadataFormatDialog::OnUseLua ), NULL, this );
}

wxFB_MetadataFormatDialog::~wxFB_MetadataFormatDialog()
{
	// Disconnect Events
	m_format->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxFB_MetadataFormatDialog::OnTextChanged ), NULL, this );
	m_useLua->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_MetadataFormatDialog::OnUseLua ), NULL, this );
	
}

wxFB_MetadataFormatHelpPanel::wxFB_MetadataFormatHelpPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	m_text1 = new wxStaticText( this, wxID_ANY, wxT("Format Help"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( -1 );
	m_text1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	
	bSizer14->Add( m_text1, 0, wxALL, 5 );
	
	m_text = new wxStaticText( this, wxID_ANY, wxT("Text between percent signs (%) is replaced with puzzle metadata.  Simple HTML formatting is allowed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text->Wrap( -1 );
	bSizer14->Add( m_text, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_sizer = new wxFlexGridSizer( 0, 2, 5, 20 );
	m_sizer->SetFlexibleDirection( wxBOTH );
	m_sizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	bSizer14->Add( m_sizer, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	this->SetSizer( bSizer14 );
	this->Layout();
	bSizer14->Fit( this );
}

wxFB_MetadataFormatHelpPanel::~wxFB_MetadataFormatHelpPanel()
{
}
