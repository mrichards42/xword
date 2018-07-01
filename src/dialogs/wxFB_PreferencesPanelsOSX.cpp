///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version May 29 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_PreferencesPanelsOSX.h"

///////////////////////////////////////////////////////////////////////////

wxFB_SolvePanel::wxFB_SolvePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer411;
	bSizer411 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText911;
	m_staticText911 = new wxStaticText( this, wxID_ANY, wxT("Timer"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText911->Wrap( -1 );
	m_staticText911->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer411->Add( m_staticText911, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer1211;
	bSizer1211 = new wxBoxSizer( wxVERTICAL );
	
	m_startTimer = new wxCheckBox( this, wxID_ANY, wxT("Start when a puzzle is opened"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1211->Add( m_startTimer, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer411->Add( bSizer1211, 0, wxLEFT, 25 );
	
	
	bSizer3->Add( bSizer411, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText91;
	m_staticText91 = new wxStaticText( this, wxID_ANY, wxT("Solution"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText91->Wrap( -1 );
	m_staticText91->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer41->Add( m_staticText91, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );
	
	m_checkWhileTyping = new wxCheckBox( this, wxID_ANY, wxT("Check solution while typing"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer121->Add( m_checkWhileTyping, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_strictRebus = new wxCheckBox( this, wxID_ANY, wxT("Strict rebus checking"), wxDefaultPosition, wxDefaultSize, 0 );
	m_strictRebus->SetToolTip( wxT("Require rebus entries to exactly match the solution") );
	
	bSizer121->Add( m_strictRebus, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_showCompletionStatus = new wxCheckBox( this, wxID_ANY, wxT("Show completion status"), wxDefaultPosition, wxDefaultSize, 0 );
	m_showCompletionStatus->SetValue(true); 
	m_showCompletionStatus->SetToolTip( wxT("Show percentage completion while solving and whether a completed puzzle is correct or incorrect, and stop the timer when the puzzle is completed correctly.") );
	
	bSizer121->Add( m_showCompletionStatus, 0, wxALL, 5 );
	
	
	bSizer41->Add( bSizer121, 0, wxLEFT, 25 );
	
	
	bSizer3->Add( bSizer41, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText9;
	m_staticText9 = new wxStaticText( this, wxID_ANY, wxT("Cursor Movement"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	m_staticText9->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer4->Add( m_staticText9, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	m_moveAfterLetter = new wxCheckBox( this, wxID_ANY, wxT("Move cursor after entering a letter"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveAfterLetter->SetValue(true); 
	bSizer12->Add( m_moveAfterLetter, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	m_nextSquare = new wxRadioButton( this, wxID_ANY, wxT("Move to next square"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_nextSquare, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_nextBlank = new wxRadioButton( this, wxID_ANY, wxT("Move to next blank"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_nextBlank, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer12->Add( bSizer13, 0, wxLEFT, 25 );
	
	m_blankOnDirection = new wxCheckBox( this, wxID_ANY, wxT("Move to blank after switching direction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_blankOnDirection->SetValue(true); 
	bSizer12->Add( m_blankOnDirection, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_blankOnNewWord = new wxCheckBox( this, wxID_ANY, wxT("Move to blank after moving to a new word"), wxDefaultPosition, wxDefaultSize, 0 );
	m_blankOnNewWord->SetValue(true); 
	bSizer12->Add( m_blankOnNewWord, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_pauseOnSwitch = new wxCheckBox( this, wxID_ANY, wxT("Pause when switching direction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pauseOnSwitch->SetValue(true); 
	bSizer12->Add( m_pauseOnSwitch, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_moveOnRightClick = new wxCheckBox( this, wxID_ANY, wxT("Move to mouse position on right click"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_moveOnRightClick, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer4->Add( bSizer12, 0, wxLEFT, 25 );
	
	
	bSizer3->Add( bSizer4, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText92;
	m_staticText92 = new wxStaticText( this, wxID_ANY, wxT("Auto Saving"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText92->Wrap( -1 );
	m_staticText92->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer42->Add( m_staticText92, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer122;
	bSizer122 = new wxBoxSizer( wxVERTICAL );
	
	m_useAutoSave = new wxCheckBox( this, wxID_ANY, wxT("Automatically save puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	m_useAutoSave->SetValue(true); 
	bSizer122->Add( m_useAutoSave, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxHORIZONTAL );
	
	m_stAfter = new wxStaticText( this, wxID_ANY, wxT("After"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stAfter->Wrap( -1 );
	bSizer241->Add( m_stAfter, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_autoSave = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer241->Add( m_autoSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_stSeconds = new wxStaticText( this, wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stSeconds->Wrap( -1 );
	bSizer241->Add( m_stSeconds, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer122->Add( bSizer241, 0, wxLEFT, 25 );
	
	
	bSizer42->Add( bSizer122, 0, wxLEFT, 25 );
	
	
	bSizer3->Add( bSizer42, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer421;
	bSizer421 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText921;
	m_staticText921 = new wxStaticText( this, wxID_ANY, wxT("File History"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText921->Wrap( -1 );
	m_staticText921->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer421->Add( m_staticText921, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer1221;
	bSizer1221 = new wxBoxSizer( wxVERTICAL );
	
	m_saveFileHistory = new wxCheckBox( this, wxID_ANY, wxT("Save a history of recently opened puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1221->Add( m_saveFileHistory, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_reopenLastPuzzle = new wxCheckBox( this, wxID_ANY, wxT("Open last puzzle when XWord starts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_reopenLastPuzzle->SetValue(true); 
	bSizer1221->Add( m_reopenLastPuzzle, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer421->Add( bSizer1221, 0, wxLEFT, 25 );
	
	
	bSizer3->Add( bSizer421, 0, wxBOTTOM, 5 );
	
	
	bSizer24->Add( bSizer3, 1, wxEXPAND|wxRIGHT|wxLEFT, 20 );
	
	
	this->SetSizer( bSizer24 );
	this->Layout();
	bSizer24->Fit( this );
	
	// Connect Events
	m_moveAfterLetter->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnMoveAfterLetter ), NULL, this );
	m_useAutoSave->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnUseAutoSave ), NULL, this );
	m_saveFileHistory->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnSaveFileHistory ), NULL, this );
}

wxFB_SolvePanel::~wxFB_SolvePanel()
{
	// Disconnect Events
	m_moveAfterLetter->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnMoveAfterLetter ), NULL, this );
	m_useAutoSave->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnUseAutoSave ), NULL, this );
	m_saveFileHistory->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_SolvePanel::OnSaveFileHistory ), NULL, this );
	
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
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer411;
	bSizer411 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText911;
	m_staticText911 = new wxStaticText( this, wxID_ANY, wxT("Fonts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText911->Wrap( -1 );
	m_staticText911->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer411->Add( m_staticText911, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer1211;
	bSizer1211 = new wxBoxSizer( wxVERTICAL );
	
	m_printCustomFonts = new wxCheckBox( this, wxID_ANY, wxT("Use custom fonts for printing"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1211->Add( m_printCustomFonts, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 3, 2, 5, 5 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText144;
	m_staticText144 = new wxStaticText( this, wxID_ANY, wxT("Grid Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText144->Wrap( -1 );
	fgSizer5->Add( m_staticText144, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_printGridLetterFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridLetterFont, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_printGridNumberFontLabel;
	m_printGridNumberFontLabel = new wxStaticText( this, wxID_ANY, wxT("Grid Numbers:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_printGridNumberFontLabel->Wrap( -1 );
	fgSizer5->Add( m_printGridNumberFontLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 5 );
	
	m_printGridNumberFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printGridNumberFont, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText1412;
	m_staticText1412 = new wxStaticText( this, wxID_ANY, wxT("Clues:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1412->Wrap( -1 );
	fgSizer5->Add( m_staticText1412, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_printClueFont = new FontPickerPanel(this, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
	fgSizer5->Add( m_printClueFont, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer1211->Add( fgSizer5, 0, wxLEFT, 25 );
	
	
	bSizer411->Add( bSizer1211, 0, wxLEFT, 25 );
	
	
	bSizer371->Add( bSizer411, 1, wxEXPAND|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer4111;
	bSizer4111 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText9111;
	m_staticText9111 = new wxStaticText( this, wxID_ANY, wxT("Grid Alignment"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9111->Wrap( -1 );
	m_staticText9111->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer4111->Add( m_staticText9111, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_alignTL = new wxRadioButton( this, wxID_ANY, wxT("Top left"), wxDefaultPosition, wxDefaultSize, 0 );
	m_alignTL->SetValue( true ); 
	gSizer3->Add( m_alignTL, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_alignTR = new wxRadioButton( this, wxID_ANY, wxT("Top right"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_alignTR, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_alignBL = new wxRadioButton( this, wxID_ANY, wxT("Bottom left"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_alignBL, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_alignBR = new wxRadioButton( this, wxID_ANY, wxT("Bottom right"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_alignBR, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer4111->Add( gSizer3, 0, wxLEFT, 25 );
	
	
	bSizer371->Add( bSizer4111, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer41111;
	bSizer41111 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* m_staticText91111;
	m_staticText91111 = new wxStaticText( this, wxID_ANY, wxT("Black Square Color"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText91111->Wrap( -1 );
	m_staticText91111->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer41111->Add( m_staticText91111, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( this, wxID_ANY, wxT("White"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer33->Add( m_staticText13, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_printBlackSquareBrightness = new wxSlider( this, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE );
	bSizer33->Add( m_printBlackSquareBrightness, 1, wxALIGN_CENTER_VERTICAL|wxTOP, 3 );
	
	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("Black"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer33->Add( m_staticText14, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_panel8 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxSUNKEN_BORDER );
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	m_printBlackSquarePreview = new wxPanel( m_panel8, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	m_printBlackSquarePreview->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	m_printBlackSquarePreview->SetMinSize( wxSize( 30,30 ) );
	
	bSizer21->Add( m_printBlackSquarePreview, 1, wxEXPAND|wxALL, 5 );
	
	
	m_panel8->SetSizer( bSizer21 );
	m_panel8->Layout();
	bSizer21->Fit( m_panel8 );
	bSizer33->Add( m_panel8, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer41111->Add( bSizer33, 0, wxLEFT, 25 );
	
	
	bSizer371->Add( bSizer41111, 0, wxBOTTOM, 5 );
	
	
	bSizer34->Add( bSizer371, 1, wxEXPAND|wxRIGHT|wxLEFT, 20 );
	
	
	this->SetSizer( bSizer34 );
	this->Layout();
	bSizer34->Fit( this );
	
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
