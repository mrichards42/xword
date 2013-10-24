///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_PreferencesPanels.h"

///////////////////////////////////////////////////////////////////////////

wxFB_SolvePanel::wxFB_SolvePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Cursor movement") ), wxVERTICAL );
	
	wxString m_afterLetterChoices[] = { wxT("Do not move"), wxT("Move to next square"), wxT("Move to next blank") };
	int m_afterLetterNChoices = sizeof( m_afterLetterChoices ) / sizeof( wxString );
	m_afterLetter = new wxRadioBox( this, wxID_ANY, wxT("After entering a letter"), wxDefaultPosition, wxDefaultSize, m_afterLetterNChoices, m_afterLetterChoices, 1, wxRA_SPECIFY_COLS );
	m_afterLetter->SetSelection( 0 );
	sbSizer3->Add( m_afterLetter, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Move to a blank square") ), wxVERTICAL );
	
	m_blankOnDirection = new wxCheckBox( this, wxID_ANY, wxT("After switching directions"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer5->Add( m_blankOnDirection, 0, wxALL, 5 );
	
	m_blankOnNewWord = new wxCheckBox( this, wxID_ANY, wxT("After moving to a new word"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer5->Add( m_blankOnNewWord, 0, wxALL, 5 );
	
	
	sbSizer3->Add( sbSizer5, 1, wxALL|wxEXPAND, 5 );
	
	wxString m_pauseOnSwitchChoices[] = { wxT("Move cursor"), wxT("Keep cursor on current square") };
	int m_pauseOnSwitchNChoices = sizeof( m_pauseOnSwitchChoices ) / sizeof( wxString );
	m_pauseOnSwitch = new wxRadioBox( this, wxID_ANY, wxT("When switching with arrow keys"), wxDefaultPosition, wxDefaultSize, m_pauseOnSwitchNChoices, m_pauseOnSwitchChoices, 1, wxRA_SPECIFY_COLS );
	m_pauseOnSwitch->SetSelection( 0 );
	sbSizer3->Add( m_pauseOnSwitch, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer4->Add( sbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer3->Add( bSizer4, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Solution") ), wxVERTICAL );
	
	m_checkWhileTyping = new wxCheckBox( this, wxID_ANY, wxT("Check solution while typing"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer41->Add( m_checkWhileTyping, 0, wxALL, 5 );
	
	m_strictRebus = new wxCheckBox( this, wxID_ANY, wxT("Strict rebus checking"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer41->Add( m_strictRebus, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer41, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Right mouse button") ), wxVERTICAL );
	
	m_moveOnRightClick = new wxCheckBox( this, wxID_ANY, wxT("Move to mouse position"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer4->Add( m_moveOnRightClick, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer411;
	sbSizer411 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Timer") ), wxVERTICAL );
	
	m_startTimer = new wxCheckBox( this, wxID_ANY, wxT("Start when a puzzle is opened"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer411->Add( m_startTimer, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer411, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer3->Add( bSizer5, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer3 );
	this->Layout();
	bSizer3->Fit( this );
}

wxFB_SolvePanel::~wxFB_SolvePanel()
{
}

wxFB_AppearancePanel::wxFB_AppearancePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bsizer26;
	bsizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_advancedChoiceChoices[] = { wxT("Basic"), wxT("Advanced") };
	int m_advancedChoiceNChoices = sizeof( m_advancedChoiceChoices ) / sizeof( wxString );
	m_advancedChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_advancedChoiceNChoices, m_advancedChoiceChoices, 0 );
	m_advancedChoice->SetSelection( 1 );
	bsizer26->Add( m_advancedChoice, 0, wxALL, 5 );
	
	
	bsizer26->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_defaultsBtn = new wxButton( this, wxID_ANY, wxT("Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
	bsizer26->Add( m_defaultsBtn, 0, wxALL, 5 );
	
	
	bSizer11->Add( bsizer26, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer11 );
	this->Layout();
	bSizer11->Fit( this );
	
	// Connect Events
	m_advancedChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxFB_AppearancePanel::OnAdvancedChoice ), NULL, this );
	m_defaultsBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_AppearancePanel::OnResetDefaults ), NULL, this );
}

wxFB_AppearancePanel::~wxFB_AppearancePanel()
{
	// Disconnect Events
	m_advancedChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxFB_AppearancePanel::OnAdvancedChoice ), NULL, this );
	m_defaultsBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_AppearancePanel::OnResetDefaults ), NULL, this );
	
}

wxFB_StartupPanel::wxFB_StartupPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("File history") ), wxHORIZONTAL );
	
	m_saveFileHistory = new wxCheckBox( this, wxID_ANY, wxT("Save a history of recently opened puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer22->Add( m_saveFileHistory, 0, wxALL, 5 );
	
	m_reopenLastPuzzle = new wxCheckBox( this, wxID_ANY, wxT("Open last puzzle when XWord starts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_reopenLastPuzzle->SetValue(true); 
	sbSizer22->Add( m_reopenLastPuzzle, 0, wxALL, 5 );
	
	
	bSizer42->Add( sbSizer22, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4111;
	sbSizer4111 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Auto Save") ), wxVERTICAL );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText36 = new wxStaticText( this, wxID_ANY, wxT("After"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	bSizer24->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_autoSave = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer24->Add( m_autoSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText351 = new wxStaticText( this, wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText351->Wrap( -1 );
	bSizer24->Add( m_staticText351, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	
	bSizer24->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, wxT("Set to 0 to disable auto save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	m_staticText17->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	
	bSizer24->Add( m_staticText17, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer4111->Add( bSizer24, 1, wxEXPAND, 5 );
	
	
	bSizer42->Add( sbSizer4111, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer20->Add( bSizer42, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer20 );
	this->Layout();
	bSizer20->Fit( this );
	
	// Connect Events
	m_saveFileHistory->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_StartupPanel::OnSaveFileHistory ), NULL, this );
}

wxFB_StartupPanel::~wxFB_StartupPanel()
{
	// Disconnect Events
	m_saveFileHistory->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_StartupPanel::OnSaveFileHistory ), NULL, this );
	
}

wxFB_PrintPanel::wxFB_PrintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Fonts") ), wxVERTICAL );
	
	m_printCustomFonts = new wxCheckBox( this, wxID_ANY, wxT("Use custom fonts"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer14->Add( m_printCustomFonts, 0, wxALL, 8 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 3, 2, 10, 10 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText144;
	m_staticText144 = new wxStaticText( this, wxID_ANY, wxT("Grid Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText144->Wrap( -1 );
	fgSizer5->Add( m_staticText144, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridLetterFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridLetterFont, 0, 0, 5 );
	
	wxStaticText* m_printGridNumberFontLabel;
	m_printGridNumberFontLabel = new wxStaticText( this, wxID_ANY, wxT("Grid Numbers:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_printGridNumberFontLabel->Wrap( -1 );
	fgSizer5->Add( m_printGridNumberFontLabel, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridNumberFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridNumberFont, 0, 0, 5 );
	
	wxStaticText* m_staticText1412;
	m_staticText1412 = new wxStaticText( this, wxID_ANY, wxT("Clues:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1412->Wrap( -1 );
	fgSizer5->Add( m_staticText1412, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printClueFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printClueFont, 0, 0, 5 );
	
	
	sbSizer14->Add( fgSizer5, 0, wxALL, 5 );
	
	
	bSizer371->Add( sbSizer14, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_printGridAlignmentChoices[] = { wxT("Top left"), wxT("Top right"), wxT("Bottom left"), wxT("Bottom right") };
	int m_printGridAlignmentNChoices = sizeof( m_printGridAlignmentChoices ) / sizeof( wxString );
	m_printGridAlignment = new wxRadioBox( this, wxID_ANY, wxT("Grid alignment"), wxDefaultPosition, wxDefaultSize, m_printGridAlignmentNChoices, m_printGridAlignmentChoices, 2, wxRA_SPECIFY_COLS );
	m_printGridAlignment->SetSelection( 0 );
	bSizer58->Add( m_printGridAlignment, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Black square brightness") ), wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( this, wxID_ANY, wxT("White"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	sbSizer19->Add( m_staticText13, 0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printBlackSquareBrightness = new wxSlider( this, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE );
	sbSizer19->Add( m_printBlackSquareBrightness, 1, wxTOP|wxALIGN_CENTER_VERTICAL, 3 );
	
	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("Black"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	sbSizer19->Add( m_staticText14, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_panel8 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxSUNKEN_BORDER );
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
	
	
	this->SetSizer( bSizer371 );
	this->Layout();
	bSizer371->Fit( this );
	
	// Connect Events
	m_printCustomFonts->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PrintPanel::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
}

wxFB_PrintPanel::~wxFB_PrintPanel()
{
	// Disconnect Events
	m_printCustomFonts->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PrintPanel::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxFB_PrintPanel::OnBlackSquareBrightness ), NULL, this );
	
}
