///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_Dialogs.h"

///////////////////////////////////////////////////////////////////////////

LicenseDialog::LicenseDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	m_textCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_MULTILINE|wxTE_READONLY );
	m_textCtrl->SetMaxLength( 0 ); 
	bSizer22->Add( m_textCtrl, 1, wxALL|wxEXPAND, 10 );
	
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2->Realize();
	
	bSizer22->Add( m_sdbSizer2, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	this->SetSizer( bSizer22 );
	this->Layout();
}

LicenseDialog::~LicenseDialog()
{
}

CustomPrintDialogBase::CustomPrintDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("Preset:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer28->Add( m_staticText14, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString m_presetChoices[] = { wxT("Blank Grid"), wxT("Current Progress"), wxT("Solution Grid"), wxT("Clues"), wxT("Sunday (Two Pages)"), wxT("Custom") };
	int m_presetNChoices = sizeof( m_presetChoices ) / sizeof( wxString );
	m_preset = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_presetNChoices, m_presetChoices, 0 );
	m_preset->SetSelection( 0 );
	bSizer28->Add( m_preset, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer26->Add( bSizer28, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Puzzle") ), wxVERTICAL );
	
	m_grid = new wxCheckBox( this, wxID_ANY, wxT("Grid"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer13->Add( m_grid, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	m_numbers = new wxCheckBox( this, wxID_ANY, wxT("Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_numbers, 1, wxALL|wxEXPAND, 5 );
	
	m_text = new wxCheckBox( this, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_text, 1, wxALL|wxEXPAND, 5 );
	
	m_solution = new wxCheckBox( this, wxID_ANY, wxT("Solution"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_solution, 1, wxALL|wxEXPAND, 5 );
	
	
	sbSizer13->Add( bSizer21, 3, wxLEFT|wxEXPAND, 25 );
	
	m_clues = new wxCheckBox( this, wxID_ANY, wxT("Clues"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer13->Add( m_clues, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer22->Add( sbSizer13, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Header") ), wxVERTICAL );
	
	m_title = new wxCheckBox( this, wxID_ANY, wxT("Title"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer12->Add( m_title, 0, wxALL|wxEXPAND, 5 );
	
	m_author = new wxCheckBox( this, wxID_ANY, wxT("Author"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer12->Add( m_author, 0, wxALL|wxEXPAND, 5 );
	
	m_notes = new wxCheckBox( this, wxID_ANY, wxT("Notes"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer12->Add( m_notes, 0, wxALL, 5 );
	
	
	bSizer25->Add( sbSizer12, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_numPagesChoices[] = { wxT("One page"), wxT("Two pages") };
	int m_numPagesNChoices = sizeof( m_numPagesChoices ) / sizeof( wxString );
	m_numPages = new wxRadioBox( this, wxID_ANY, wxT("Print On"), wxDefaultPosition, wxDefaultSize, m_numPagesNChoices, m_numPagesChoices, 1, wxRA_SPECIFY_COLS );
	m_numPages->SetSelection( 1 );
	bSizer25->Add( m_numPages, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer22->Add( bSizer25, 1, wxEXPAND, 5 );
	
	
	bSizer26->Add( bSizer22, 0, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer251;
	bSizer251 = new wxBoxSizer( wxHORIZONTAL );
	
	m_btnPrint = new wxButton( this, wxID_ANY, wxT("&Print"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnPrint->SetDefault(); 
	bSizer251->Add( m_btnPrint, 0, wxALL, 5 );
	
	m_btnPreview = new wxButton( this, wxID_ANY, wxT("Print Preview"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer251->Add( m_btnPreview, 0, wxALL, 5 );
	
	m_btnCancel = new wxButton( this, wxID_ANY, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer251->Add( m_btnCancel, 0, wxALL, 5 );
	
	
	bSizer26->Add( bSizer251, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer26 );
	this->Layout();
	bSizer26->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( CustomPrintDialogBase::OnInit ) );
	m_preset->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( CustomPrintDialogBase::OnPresetSelected ), NULL, this );
	m_grid->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridChecked ), NULL, this );
	m_numbers->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridNumbersChecked ), NULL, this );
	m_text->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridTextChecked ), NULL, this );
	m_solution->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridSolutionChecked ), NULL, this );
	m_clues->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnCluesChecked ), NULL, this );
	m_title->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnTitleChecked ), NULL, this );
	m_author->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnAuthorChecked ), NULL, this );
	m_notes->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnNotesChecked ), NULL, this );
	m_numPages->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( CustomPrintDialogBase::OnPagesSelected ), NULL, this );
	m_btnPrint->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnPrintButton ), NULL, this );
	m_btnPreview->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnPreviewButton ), NULL, this );
	m_btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnCancelButton ), NULL, this );
}

CustomPrintDialogBase::~CustomPrintDialogBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( CustomPrintDialogBase::OnInit ) );
	m_preset->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( CustomPrintDialogBase::OnPresetSelected ), NULL, this );
	m_grid->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridChecked ), NULL, this );
	m_numbers->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridNumbersChecked ), NULL, this );
	m_text->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridTextChecked ), NULL, this );
	m_solution->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnGridSolutionChecked ), NULL, this );
	m_clues->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnCluesChecked ), NULL, this );
	m_title->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnTitleChecked ), NULL, this );
	m_author->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnAuthorChecked ), NULL, this );
	m_notes->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnNotesChecked ), NULL, this );
	m_numPages->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( CustomPrintDialogBase::OnPagesSelected ), NULL, this );
	m_btnPrint->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnPrintButton ), NULL, this );
	m_btnPreview->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnPreviewButton ), NULL, this );
	m_btnCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CustomPrintDialogBase::OnCancelButton ), NULL, this );
	
}
