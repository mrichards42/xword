///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version May 29 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_PreferencesPanels.h"

///////////////////////////////////////////////////////////////////////////

wxFB_SolvePanel::wxFB_SolvePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer411;
	sbSizer411 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Timer") ), wxVERTICAL );
	
	m_startTimer = new wxCheckBox( sbSizer411->GetStaticBox(), wxID_ANY, wxT("Start when a puzzle is opened"), wxDefaultPosition, wxDefaultSize, 0 );
	m_startTimer->SetValue(true); 
	sbSizer411->Add( m_startTimer, 0, wxALL, 5 );
	
	
	bSizer4->Add( sbSizer411, 0, wxEXPAND|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Cursor movement") ), wxVERTICAL );
	
	m_moveAfterLetter = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Move after entering a letter"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveAfterLetter->SetValue(true); 
	sbSizer3->Add( m_moveAfterLetter, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	m_nextSquare = new wxRadioButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("To the next square"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_nextSquare, 0, wxALL, 5 );
	
	m_nextBlank = new wxRadioButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("To the next blank square"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_nextBlank, 0, wxALL, 5 );
	
	
	sbSizer3->Add( bSizer12, 0, wxLEFT, 20 );
	
	m_staticText10 = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Move to a blank square"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	sbSizer3->Add( m_staticText10, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	m_blankOnDirection = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("After switching directions"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_blankOnDirection, 0, wxALL, 5 );
	
	m_blankOnNewWord = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("After moving to a new word"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_blankOnNewWord, 0, wxALL, 5 );
	
	
	sbSizer3->Add( bSizer13, 0, wxLEFT, 20 );
	
	m_pauseOnSwitch = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Pause when switching direction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pauseOnSwitch->SetValue(true); 
	sbSizer3->Add( m_pauseOnSwitch, 0, wxALL, 5 );
	
	m_moveOnRightClick = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Move to mouse on right click"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveOnRightClick->SetValue(true); 
	sbSizer3->Add( m_moveOnRightClick, 0, wxALL, 5 );
	
	
	bSizer4->Add( sbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer3->Add( bSizer4, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Solution") ), wxVERTICAL );
	
	m_checkWhileTyping = new wxCheckBox( sbSizer41->GetStaticBox(), wxID_ANY, wxT("Check solution while typing"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer41->Add( m_checkWhileTyping, 0, wxALL, 5 );
	
	m_strictRebus = new wxCheckBox( sbSizer41->GetStaticBox(), wxID_ANY, wxT("Strict rebus checking"), wxDefaultPosition, wxDefaultSize, 0 );
	m_strictRebus->SetToolTip( wxT("Require rebus entries to exactly match the solution") );
	
	sbSizer41->Add( m_strictRebus, 0, wxALL, 5 );
	
	m_showCompletionStatus
	= new wxCheckBox( sbSizer41->GetStaticBox(), wxID_ANY, wxT("Show completion status"), wxDefaultPosition, wxDefaultSize, 0 );
	m_showCompletionStatus
	->SetValue(true);
	m_showCompletionStatus
	->SetToolTip( wxT("Show percentage completion while solving and whether a completed puzzle is correct or incorrect, and stop the timer when the puzzle is completed correctly.") );
	
	sbSizer41->Add( m_showCompletionStatus
	, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer41, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("File history") ), wxVERTICAL );
	
	m_saveFileHistory = new wxCheckBox( sbSizer22->GetStaticBox(), wxID_ANY, wxT("Save a history of recent puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	m_saveFileHistory->SetValue(true); 
	sbSizer22->Add( m_saveFileHistory, 0, wxALL, 5 );
	
	m_reopenLastPuzzle = new wxCheckBox( sbSizer22->GetStaticBox(), wxID_ANY, wxT("Open last puzzle on startup"), wxDefaultPosition, wxDefaultSize, 0 );
	m_reopenLastPuzzle->SetValue(true); 
	sbSizer22->Add( m_reopenLastPuzzle, 0, wxALL, 5 );
	
	
	bSizer5->Add( sbSizer22, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4111;
	sbSizer4111 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Auto Save") ), wxVERTICAL );
	
	m_useAutoSave = new wxCheckBox( sbSizer4111->GetStaticBox(), wxID_ANY, wxT("Automatically save puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	m_useAutoSave->SetValue(true); 
	sbSizer4111->Add( m_useAutoSave, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_stAfter = new wxStaticText( sbSizer4111->GetStaticBox(), wxID_ANY, wxT("After"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stAfter->Wrap( -1 );
	bSizer24->Add( m_stAfter, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_autoSave = new wxSpinCtrl( sbSizer4111->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), wxSP_ARROW_KEYS, 0, 99, 10 );
	bSizer24->Add( m_autoSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_stSeconds = new wxStaticText( sbSizer4111->GetStaticBox(), wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stSeconds->Wrap( -1 );
	bSizer24->Add( m_stSeconds, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	
	sbSizer4111->Add( bSizer24, 0, wxLEFT, 20 );
	
	
	bSizer5->Add( sbSizer4111, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer3->Add( bSizer5, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer3 );
	this->Layout();
	bSizer3->Fit( this );
	
	// Connect Events
	m_moveAfterLetter->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnMoveAfterLetter ), NULL, this );
	m_saveFileHistory->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnSaveFileHistory ), NULL, this );
	m_useAutoSave->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnUseAutoSave ), NULL, this );
}

wxFB_SolvePanel::~wxFB_SolvePanel()
{
	// Disconnect Events
	m_moveAfterLetter->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnMoveAfterLetter ), NULL, this );
	m_saveFileHistory->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnSaveFileHistory ), NULL, this );
	m_useAutoSave->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnUseAutoSave ), NULL, this );
	
}

wxFB_AppearancePanel::wxFB_AppearancePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
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
	
	
	bSizer11->Add( bsizer26, 0, wxEXPAND|wxALL, 5 );
	
	
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

wxFB_PrintPanel::wxFB_PrintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Fonts") ), wxVERTICAL );
	
	m_printCustomFonts = new wxCheckBox( sbSizer14->GetStaticBox(), wxID_ANY, wxT("Use custom fonts"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer14->Add( m_printCustomFonts, 0, wxALL, 8 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 3, 2, 10, 10 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText144;
	m_staticText144 = new wxStaticText( sbSizer14->GetStaticBox(), wxID_ANY, wxT("Grid Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText144->Wrap( -1 );
	fgSizer5->Add( m_staticText144, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridLetterFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridLetterFont, 0, 0, 5 );
	
	wxStaticText* m_printGridNumberFontLabel;
	m_printGridNumberFontLabel = new wxStaticText( sbSizer14->GetStaticBox(), wxID_ANY, wxT("Grid Numbers:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_printGridNumberFontLabel->Wrap( -1 );
	fgSizer5->Add( m_printGridNumberFontLabel, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridNumberFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridNumberFont, 0, 0, 5 );
	
	wxStaticText* m_staticText1412;
	m_staticText1412 = new wxStaticText( sbSizer14->GetStaticBox(), wxID_ANY, wxT("Clues:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1412->Wrap( -1 );
	fgSizer5->Add( m_staticText1412, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printClueFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printClueFont, 0, 0, 5 );
	
	
	sbSizer14->Add( fgSizer5, 0, wxALL, 5 );
	
	
	bSizer371->Add( sbSizer14, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 10 );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Grid alignment") ), wxVERTICAL );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_alignTL = new wxRadioButton( sbSizer8->GetStaticBox(), wxID_ANY, wxT("Top left"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_alignTL, 0, wxALL, 5 );
	
	m_alignTR = new wxRadioButton( sbSizer8->GetStaticBox(), wxID_ANY, wxT("Top right"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_alignTR, 0, wxALL, 5 );
	
	m_alignBL = new wxRadioButton( sbSizer8->GetStaticBox(), wxID_ANY, wxT("Bottom left"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_alignBL, 0, wxALL, 5 );
	
	m_alignBR = new wxRadioButton( sbSizer8->GetStaticBox(), wxID_ANY, wxT("Bottom right"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_alignBR, 0, wxALL, 5 );
	
	
	sbSizer8->Add( gSizer1, 1, wxEXPAND, 5 );
	
	
	bSizer58->Add( sbSizer8, 0, wxEXPAND|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Black square brightness") ), wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( sbSizer19->GetStaticBox(), wxID_ANY, wxT("White"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	sbSizer19->Add( m_staticText13, 0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printBlackSquareBrightness = new wxSlider( sbSizer19->GetStaticBox(), wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE );
	sbSizer19->Add( m_printBlackSquareBrightness, 1, wxTOP|wxALIGN_CENTER_VERTICAL, 3 );
	
	m_staticText14 = new wxStaticText( sbSizer19->GetStaticBox(), wxID_ANY, wxT("Black"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	sbSizer19->Add( m_staticText14, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_panel8 = new wxPanel( sbSizer19->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxSUNKEN_BORDER );
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
	
	
	bSizer371->Add( bSizer58, 0, wxEXPAND|wxALL, 5 );
	
	
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
