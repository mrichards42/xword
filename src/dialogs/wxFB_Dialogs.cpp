///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 10 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_Dialogs.h"

///////////////////////////////////////////////////////////////////////////

PreferencesDialogBase::PreferencesDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* sizer;
	sizer = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	wxPanel* solvePanel;
	solvePanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Cursor movement") ), wxVERTICAL );
	
	wxString m_afterLetterChoices[] = { wxT("Do not move"), wxT("Move to next square"), wxT("Move to next blank") };
	int m_afterLetterNChoices = sizeof( m_afterLetterChoices ) / sizeof( wxString );
	m_afterLetter = new wxRadioBox( solvePanel, wxID_ANY, wxT("After entering a letter"), wxDefaultPosition, wxDefaultSize, m_afterLetterNChoices, m_afterLetterChoices, 1, wxRA_SPECIFY_COLS );
	m_afterLetter->SetSelection( 0 );
	sbSizer3->Add( m_afterLetter, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Move to a blank square") ), wxVERTICAL );
	
	m_blankOnDirection = new wxCheckBox( solvePanel, wxID_ANY, wxT("After switching directions"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer5->Add( m_blankOnDirection, 0, wxALL, 5 );
	
	m_blankOnNewWord = new wxCheckBox( solvePanel, wxID_ANY, wxT("After moving to a new word"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer5->Add( m_blankOnNewWord, 0, wxALL, 5 );
	
	
	sbSizer3->Add( sbSizer5, 1, wxALL|wxEXPAND, 5 );
	
	wxString m_pauseOnSwitchChoices[] = { wxT("Move cursor"), wxT("Keep cursor on current square") };
	int m_pauseOnSwitchNChoices = sizeof( m_pauseOnSwitchChoices ) / sizeof( wxString );
	m_pauseOnSwitch = new wxRadioBox( solvePanel, wxID_ANY, wxT("When switching with arrow keys"), wxDefaultPosition, wxDefaultSize, m_pauseOnSwitchNChoices, m_pauseOnSwitchChoices, 1, wxRA_SPECIFY_COLS );
	m_pauseOnSwitch->SetSelection( 0 );
	sbSizer3->Add( m_pauseOnSwitch, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer4->Add( sbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer3->Add( bSizer4, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Solution") ), wxVERTICAL );
	
	m_checkWhileTyping = new wxCheckBox( solvePanel, wxID_ANY, wxT("Check solution while typing"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer41->Add( m_checkWhileTyping, 0, wxALL, 5 );
	
	m_strictRebus = new wxCheckBox( solvePanel, wxID_ANY, wxT("Strict rebus checking"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer41->Add( m_strictRebus, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer41, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Right mouse button") ), wxVERTICAL );
	
	m_moveOnRightClick = new wxCheckBox( solvePanel, wxID_ANY, wxT("Move to mouse position"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer4->Add( m_moveOnRightClick, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer411;
	sbSizer411 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Timer") ), wxVERTICAL );
	
	m_startTimer = new wxCheckBox( solvePanel, wxID_ANY, wxT("Start when a puzzle is opened"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer411->Add( m_startTimer, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer411, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer3->Add( bSizer5, 1, wxALL|wxEXPAND, 5 );
	
	
	solvePanel->SetSizer( bSizer3 );
	solvePanel->Layout();
	bSizer3->Fit( solvePanel );
	m_notebook->AddPage( solvePanel, wxT("Solving"), true );
	styleTreePanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bsizer26;
	bsizer26 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* treesizer;
	treesizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	m_styleTree = new wxTreeCtrl( styleTreePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_NO_LINES|wxTR_SINGLE );
	m_styleTree->SetMinSize( wxSize( 130,-1 ) );
	
	bSizer19->Add( m_styleTree, 1, wxALL|wxEXPAND, 5 );
	
	m_simpleStyleButton = new wxButton( styleTreePanel, wxID_ANY, wxT("Show Advanced Options"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer19->Add( m_simpleStyleButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	treesizer->Add( bSizer19, 0, wxEXPAND, 5 );
	
	m_stylePanel = new wxPanel( styleTreePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_styleSizer = new wxBoxSizer( wxVERTICAL );
	
	
	m_stylePanel->SetSizer( m_styleSizer );
	m_stylePanel->Layout();
	m_styleSizer->Fit( m_stylePanel );
	treesizer->Add( m_stylePanel, 1, wxEXPAND | wxALL, 5 );
	
	
	bsizer26->Add( treesizer, 1, wxALL|wxEXPAND, 5 );
	
	
	styleTreePanel->SetSizer( bsizer26 );
	styleTreePanel->Layout();
	bsizer26->Fit( styleTreePanel );
	m_notebook->AddPage( styleTreePanel, wxT("Styles"), false );
	miscPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("File history") ), wxHORIZONTAL );
	
	m_saveFileHistory = new wxCheckBox( miscPanel, wxID_ANY, wxT("Save a history of recently opened puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer22->Add( m_saveFileHistory, 0, wxALL, 5 );
	
	m_reopenLastPuzzle = new wxCheckBox( miscPanel, wxID_ANY, wxT("Open last puzzle when XWord starts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_reopenLastPuzzle->SetValue(true); 
	sbSizer22->Add( m_reopenLastPuzzle, 0, wxALL, 5 );
	
	
	bSizer42->Add( sbSizer22, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4111;
	sbSizer4111 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("Auto Save") ), wxVERTICAL );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText36 = new wxStaticText( miscPanel, wxID_ANY, wxT("After"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	bSizer24->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_autoSave = new wxSpinCtrl( miscPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer24->Add( m_autoSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText351 = new wxStaticText( miscPanel, wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText351->Wrap( -1 );
	bSizer24->Add( m_staticText351, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	
	bSizer24->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText17 = new wxStaticText( miscPanel, wxID_ANY, wxT("Set to 0 to disable auto save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	m_staticText17->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	
	bSizer24->Add( m_staticText17, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer4111->Add( bSizer24, 1, wxEXPAND, 5 );
	
	
	bSizer42->Add( sbSizer4111, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer20->Add( bSizer42, 1, wxEXPAND, 5 );
	
	
	miscPanel->SetSizer( bSizer20 );
	miscPanel->Layout();
	bSizer20->Fit( miscPanel );
	m_notebook->AddPage( miscPanel, wxT("Misc"), false );
	printPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( printPanel, wxID_ANY, wxT("Fonts") ), wxVERTICAL );
	
	m_printCustomFonts = new wxCheckBox( printPanel, wxID_ANY, wxT("Use custom fonts"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer14->Add( m_printCustomFonts, 0, wxALL, 8 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 2, 2, 10, 10 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText144;
	m_staticText144 = new wxStaticText( printPanel, wxID_ANY, wxT("Grid Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText144->Wrap( -1 );
	fgSizer5->Add( m_staticText144, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridLetterFont = new FontPanel(printPanel, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridLetterFont, 0, 0, 5 );
	
	wxStaticText* m_printGridNumberFontLabel;
	m_printGridNumberFontLabel = new wxStaticText( printPanel, wxID_ANY, wxT("Grid Numbers:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_printGridNumberFontLabel->Wrap( -1 );
	fgSizer5->Add( m_printGridNumberFontLabel, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridNumberFont = new FontPanel(printPanel, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridNumberFont, 0, 0, 5 );
	
	wxStaticText* m_staticText1412;
	m_staticText1412 = new wxStaticText( printPanel, wxID_ANY, wxT("Clues:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1412->Wrap( -1 );
	fgSizer5->Add( m_staticText1412, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printClueFont = new FontPanel(printPanel, wxID_ANY);
	fgSizer5->Add( m_printClueFont, 0, 0, 5 );
	
	
	sbSizer14->Add( fgSizer5, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer371->Add( sbSizer14, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_printGridAlignmentChoices[] = { wxT("Top left"), wxT("Top right"), wxT("Bottom left"), wxT("Bottom right") };
	int m_printGridAlignmentNChoices = sizeof( m_printGridAlignmentChoices ) / sizeof( wxString );
	m_printGridAlignment = new wxRadioBox( printPanel, wxID_ANY, wxT("Grid alignment"), wxDefaultPosition, wxDefaultSize, m_printGridAlignmentNChoices, m_printGridAlignmentChoices, 2, wxRA_SPECIFY_COLS );
	m_printGridAlignment->SetSelection( 0 );
	bSizer58->Add( m_printGridAlignment, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( printPanel, wxID_ANY, wxT("Black square brightness") ), wxHORIZONTAL );
	
	m_printBlackSquareBrightness = new wxSlider( printPanel, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	sbSizer19->Add( m_printBlackSquareBrightness, 1, wxALL|wxEXPAND, 5 );
	
	m_panel8 = new wxPanel( printPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxSUNKEN_BORDER );
	m_panel8->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	m_printBlackSquarePreview = new wxPanel( m_panel8, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	m_printBlackSquarePreview->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	m_printBlackSquarePreview->SetMinSize( wxSize( 30,30 ) );
	
	bSizer21->Add( m_printBlackSquarePreview, 1, wxEXPAND|wxALL, 5 );
	
	
	m_panel8->SetSizer( bSizer21 );
	m_panel8->Layout();
	bSizer21->Fit( m_panel8 );
	sbSizer19->Add( m_panel8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer58->Add( sbSizer19, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer371->Add( bSizer58, 0, wxEXPAND, 5 );
	
	
	printPanel->SetSizer( bSizer371 );
	printPanel->Layout();
	bSizer371->Fit( printPanel );
	m_notebook->AddPage( printPanel, wxT("Printing"), false );
	
	sizer->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );
	
	m_buttons = new wxStdDialogButtonSizer();
	m_buttonsOK = new wxButton( this, wxID_OK );
	m_buttons->AddButton( m_buttonsOK );
	m_buttonsApply = new wxButton( this, wxID_APPLY );
	m_buttons->AddButton( m_buttonsApply );
	m_buttonsCancel = new wxButton( this, wxID_CANCEL );
	m_buttons->AddButton( m_buttonsCancel );
	m_buttons->Realize();
	
	sizer->Add( m_buttons, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( sizer );
	this->Layout();
	sizer->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PreferencesDialogBase::OnClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( PreferencesDialogBase::OnInit ) );
	m_styleTree->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( PreferencesDialogBase::OnStyleTreeSelection ), NULL, this );
	m_simpleStyleButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnSimpleStyleButton ), NULL, this );
	m_saveFileHistory->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnSaveFileHistory ), NULL, this );
	m_printCustomFonts->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_buttonsApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnApply ), NULL, this );
	m_buttonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnOK ), NULL, this );
}

PreferencesDialogBase::~PreferencesDialogBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PreferencesDialogBase::OnClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( PreferencesDialogBase::OnInit ) );
	m_styleTree->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( PreferencesDialogBase::OnStyleTreeSelection ), NULL, this );
	m_simpleStyleButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnSimpleStyleButton ), NULL, this );
	m_saveFileHistory->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnSaveFileHistory ), NULL, this );
	m_printCustomFonts->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( PreferencesDialogBase::OnBlackSquareBrightness ), NULL, this );
	m_buttonsApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnApply ), NULL, this );
	m_buttonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnOK ), NULL, this );
	
}

LicenseDialog::LicenseDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	m_textCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_MULTILINE|wxTE_READONLY );
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

MetadataFormatHelpDialog::MetadataFormatHelpDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	m_panel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText47 = new wxStaticText( m_panel, wxID_ANY, wxT("Plain text is displayed as-is.\n\nText that is enclosed in percent signs (%) is replaced with\nmetadata for the puzzle.  The most common metadata fields\nare displayed below with the values from the current puzzle."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText47->Wrap( -1 );
	bSizer14->Add( m_staticText47, 0, wxALL, 5 );
	
	m_sizer = new wxFlexGridSizer( 0, 2, 5, 20 );
	m_sizer->SetFlexibleDirection( wxBOTH );
	m_sizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText15 = new wxStaticText( m_panel, wxID_ANY, wxT("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	m_sizer->Add( m_staticText15, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText16 = new wxStaticText( m_panel, wxID_ANY, wxT("Value"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	m_sizer->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline1 = new wxStaticLine( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_sizer->Add( m_staticline1, 0, wxEXPAND, 5 );
	
	m_staticline2 = new wxStaticLine( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_sizer->Add( m_staticline2, 0, wxEXPAND, 5 );
	
	
	bSizer14->Add( m_sizer, 0, wxALL, 5 );
	
	m_staticline13 = new wxStaticLine( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer14->Add( m_staticline13, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( m_panel, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3->Realize();
	
	bSizer14->Add( m_sdbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	m_panel->SetSizer( bSizer14 );
	m_panel->Layout();
	bSizer14->Fit( m_panel );
	bSizer18->Add( m_panel, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer18 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MetadataFormatHelpDialog::OnClose ) );
	m_sdbSizer3OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MetadataFormatHelpDialog::OnOk ), NULL, this );
}

MetadataFormatHelpDialog::~MetadataFormatHelpDialog()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MetadataFormatHelpDialog::OnClose ) );
	m_sdbSizer3OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MetadataFormatHelpDialog::OnOk ), NULL, this );
	
}

DisplayFormatDialogBase::DisplayFormatDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	m_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_functionStart = new wxStaticText( this, wxID_ANY, wxT("function(puzzle)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_functionStart->Wrap( -1 );
	m_functionStart->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_functionStart, 0, wxBOTTOM, 5 );
	
	m_format = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE );
	m_format->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_format, 1, wxEXPAND|wxLEFT, 15 );
	
	m_functionEnd = new wxStaticText( this, wxID_ANY, wxT("end"), wxDefaultPosition, wxDefaultSize, 0 );
	m_functionEnd->Wrap( -1 );
	m_functionEnd->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 75, 90, 90, false, wxT("Consolas") ) );
	
	m_sizer->Add( m_functionEnd, 0, wxTOP, 5 );
	
	m_useLua = new wxCheckBox( this, wxID_ANY, wxT("Format as a lua script"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer->Add( m_useLua, 0, wxTOP|wxBOTTOM, 10 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Result") ), wxVERTICAL );
	
	m_result = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_result->Wrap( -1 );
	sbSizer12->Add( m_result, 0, wxALL, 5 );
	
	
	m_sizer->Add( sbSizer12, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );
	
	
	bSizer17->Add( m_sizer, 1, wxEXPAND|wxALL, 10 );
	
	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer17->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();
	
	bSizer17->Add( m_sdbSizer4, 0, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizer17 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_format->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DisplayFormatDialogBase::OnTextUpdated ), NULL, this );
	m_useLua->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DisplayFormatDialogBase::OnUseLua ), NULL, this );
}

DisplayFormatDialogBase::~DisplayFormatDialogBase()
{
	// Disconnect Events
	m_format->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DisplayFormatDialogBase::OnTextUpdated ), NULL, this );
	m_useLua->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DisplayFormatDialogBase::OnUseLua ), NULL, this );
	
}
