///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Mar 22 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFB_Dialogs.h"

///////////////////////////////////////////////////////////////////////////

wxFB_PreferencesDialog::wxFB_PreferencesDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
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
	
	wxStaticBoxSizer* sbSizer4111;
	sbSizer4111 = new wxStaticBoxSizer( new wxStaticBox( solvePanel, wxID_ANY, wxT("Auto Save") ), wxVERTICAL );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText36 = new wxStaticText( solvePanel, wxID_ANY, wxT("After"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	bSizer24->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_autoSave = new wxSpinCtrl( solvePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer24->Add( m_autoSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText351 = new wxStaticText( solvePanel, wxID_ANY, wxT("seconds (0 = disabled)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText351->Wrap( -1 );
	bSizer24->Add( m_staticText351, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	sbSizer4111->Add( bSizer24, 1, wxEXPAND, 5 );
	
	bSizer5->Add( sbSizer4111, 0, wxEXPAND|wxALL, 5 );
	
	bSizer3->Add( bSizer5, 1, wxALL|wxEXPAND, 5 );
	
	solvePanel->SetSizer( bSizer3 );
	solvePanel->Layout();
	bSizer3->Fit( solvePanel );
	m_notebook->AddPage( solvePanel, wxT("Solving"), true );
	wxPanel* colorPanel;
	colorPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxVERTICAL );
	
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Grid") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 3, 5 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_penColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer3->Add( m_penColor, 0, 0, 5 );
	
	wxStaticText* m_staticText12;
	m_staticText12 = new wxStaticText( colorPanel, wxID_ANY, wxT("Pen"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer3->Add( m_staticText12, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_selectedLetterColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer3->Add( m_selectedLetterColor, 0, 0, 5 );
	
	wxStaticText* m_staticText1;
	m_staticText1 = new wxStaticText( colorPanel, wxID_ANY, wxT("Focused square"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	fgSizer3->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_selectedWordColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_selectedWordColor, 0, 0, 5 );
	
	wxStaticText* m_staticText11;
	m_staticText11 = new wxStaticText( colorPanel, wxID_ANY, wxT("Focused word"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	fgSizer3->Add( m_staticText11, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_gridSelectionColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_gridSelectionColor, 0, 0, 5 );
	
	wxStaticText* m_staticText114;
	m_staticText114 = new wxStaticText( colorPanel, wxID_ANY, wxT("Multi-square selection"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText114->Wrap( -1 );
	fgSizer3->Add( m_staticText114, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_gridBackgroundColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer3->Add( m_gridBackgroundColor, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText111;
	m_staticText111 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText111->Wrap( -1 );
	fgSizer3->Add( m_staticText111, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_whiteSquareColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_whiteSquareColor, 0, 0, 5 );
	
	wxStaticText* m_staticText112;
	m_staticText112 = new wxStaticText( colorPanel, wxID_ANY, wxT("White square"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText112->Wrap( -1 );
	fgSizer3->Add( m_staticText112, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_blackSquareColor = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_blackSquareColor, 0, 0, 5 );
	
	wxStaticText* m_staticText113;
	m_staticText113 = new wxStaticText( colorPanel, wxID_ANY, wxT("Black square"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText113->Wrap( -1 );
	fgSizer3->Add( m_staticText113, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer36->Add( fgSizer3, 1, wxEXPAND, 5 );
	
	bSizer33->Add( sbSizer36, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Clue Prompt") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 2, 3, 5 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_cluePromptText = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer6->Add( m_cluePromptText, 0, 0, 5 );
	
	wxStaticText* m_staticText13512;
	m_staticText13512 = new wxStaticText( colorPanel, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13512->Wrap( -1 );
	fgSizer6->Add( m_staticText13512, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_cluePromptBackground = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer6->Add( m_cluePromptBackground, 0, 0, 5 );
	
	wxStaticText* m_staticText13511;
	m_staticText13511 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13511->Wrap( -1 );
	fgSizer6->Add( m_staticText13511, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer7->Add( fgSizer6, 1, wxEXPAND, 5 );
	
	bSizer33->Add( sbSizer7, 0, wxALL|wxEXPAND, 5 );
	
	bSizer35->Add( bSizer33, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Clue List") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 3, 5 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_clueText = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer7->Add( m_clueText, 0, 0, 5 );
	
	wxStaticText* m_staticText132;
	m_staticText132 = new wxStaticText( colorPanel, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText132->Wrap( -1 );
	fgSizer7->Add( m_staticText132, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_clueBackground = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer7->Add( m_clueBackground, 0, 0, 5 );
	
	wxStaticText* m_staticText131;
	m_staticText131 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText131->Wrap( -1 );
	fgSizer7->Add( m_staticText131, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer6->Add( fgSizer7, 0, wxEXPAND|wxRIGHT|wxLEFT, 7 );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Current clue") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 2, 2, 3, 5 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_selectedClueText = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer8->Add( m_selectedClueText, 0, 0, 5 );
	
	wxStaticText* m_staticText134;
	m_staticText134 = new wxStaticText( colorPanel, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText134->Wrap( -1 );
	fgSizer8->Add( m_staticText134, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_selectedClueBackground = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer8->Add( m_selectedClueBackground, 0, 0, 5 );
	
	wxStaticText* m_staticText133;
	m_staticText133 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText133->Wrap( -1 );
	fgSizer8->Add( m_staticText133, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer16->Add( fgSizer8, 0, wxEXPAND, 5 );
	
	sbSizer6->Add( sbSizer16, 0, wxEXPAND|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer17;
	sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Crossing clue") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 2, 2, 3, 5 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_crossingClueText = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer9->Add( m_crossingClueText, 0, 0, 5 );
	
	wxStaticText* m_staticText1351;
	m_staticText1351 = new wxStaticText( colorPanel, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1351->Wrap( -1 );
	fgSizer9->Add( m_staticText1351, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_crossingClueBackground = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer9->Add( m_crossingClueBackground, 0, 0, 5 );
	
	wxStaticText* m_staticText135;
	m_staticText135 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText135->Wrap( -1 );
	fgSizer9->Add( m_staticText135, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer17->Add( fgSizer9, 0, wxEXPAND, 5 );
	
	sbSizer6->Add( sbSizer17, 0, wxEXPAND|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( colorPanel, wxID_ANY, wxT("Heading") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 2, 2, 3, 5 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_clueHeadingText = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer10->Add( m_clueHeadingText, 0, 0, 5 );
	
	wxStaticText* m_staticText135131;
	m_staticText135131 = new wxStaticText( colorPanel, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText135131->Wrap( -1 );
	fgSizer10->Add( m_staticText135131, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_clueHeadingBackground = new wxColourPickerCtrl( colorPanel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
	fgSizer10->Add( m_clueHeadingBackground, 0, 0, 5 );
	
	wxStaticText* m_staticText13513;
	m_staticText13513 = new wxStaticText( colorPanel, wxID_ANY, wxT("Background"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13513->Wrap( -1 );
	fgSizer10->Add( m_staticText13513, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer18->Add( fgSizer10, 0, wxEXPAND, 5 );
	
	sbSizer6->Add( sbSizer18, 1, wxEXPAND|wxALL, 3 );
	
	bSizer37->Add( sbSizer6, 1, wxALL|wxEXPAND, 5 );
	
	bSizer35->Add( bSizer37, 1, wxEXPAND|wxALL, 5 );
	
	colorPanel->SetSizer( bSizer35 );
	colorPanel->Layout();
	bSizer35->Fit( colorPanel );
	m_notebook->AddPage( colorPanel, wxT("Colors"), false );
	wxPanel* fontPanel;
	fontPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 2, 2, 10, 10 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_gridLetterFont = new wxFontPickerCtrl( fontPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_gridLetterFont->SetMaxPointSize( 100 ); 
	fgSizer4->Add( m_gridLetterFont, 0, 0, 5 );
	
	wxStaticText* m_staticText14;
	m_staticText14 = new wxStaticText( fontPanel, wxID_ANY, wxT("Grid text (font size is ignored)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	fgSizer4->Add( m_staticText14, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_gridNumberFont = new wxFontPickerCtrl( fontPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_gridNumberFont->SetMaxPointSize( 100 ); 
	fgSizer4->Add( m_gridNumberFont, 0, 0, 5 );
	
	wxStaticText* m_staticText143;
	m_staticText143 = new wxStaticText( fontPanel, wxID_ANY, wxT("Grid numbers (font size is ignored)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText143->Wrap( -1 );
	fgSizer4->Add( m_staticText143, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_cluePromptFont = new wxFontPickerCtrl( fontPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_cluePromptFont->SetMaxPointSize( 100 ); 
	fgSizer4->Add( m_cluePromptFont, 0, 0, 5 );
	
	wxStaticText* m_staticText142;
	m_staticText142 = new wxStaticText( fontPanel, wxID_ANY, wxT("Clue prompt (font size is ignored)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText142->Wrap( -1 );
	fgSizer4->Add( m_staticText142, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_clueFont = new wxFontPickerCtrl( fontPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_clueFont->SetMaxPointSize( 100 ); 
	fgSizer4->Add( m_clueFont, 0, 0, 5 );
	
	wxStaticText* m_staticText141;
	m_staticText141 = new wxStaticText( fontPanel, wxID_ANY, wxT("Clue list"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	fgSizer4->Add( m_staticText141, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_clueHeadingFont = new wxFontPickerCtrl( fontPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_clueHeadingFont->SetMaxPointSize( 100 ); 
	fgSizer4->Add( m_clueHeadingFont, 0, 0, 5 );
	
	wxStaticText* m_staticText1411;
	m_staticText1411 = new wxStaticText( fontPanel, wxID_ANY, wxT("Clue list heading"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1411->Wrap( -1 );
	fgSizer4->Add( m_staticText1411, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer30->Add( fgSizer4, 1, wxEXPAND|wxALL, 15 );
	
	fontPanel->SetSizer( bSizer30 );
	fontPanel->Layout();
	bSizer30->Fit( fontPanel );
	m_notebook->AddPage( fontPanel, wxT("Fonts"), false );
	miscPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("File history") ), wxHORIZONTAL );
	
	m_saveFileHistory = new wxCheckBox( miscPanel, wxID_ANY, wxT("Save a history of recently opened puzzles"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer22->Add( m_saveFileHistory, 0, wxALL, 5 );
	
	m_reopenLastPuzzle = new wxCheckBox( miscPanel, wxID_ANY, wxT("Open last puzzle when XWord starts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_reopenLastPuzzle->SetValue(true); 
	sbSizer22->Add( m_reopenLastPuzzle, 0, wxALL, 5 );
	
	bSizer42->Add( sbSizer22, 1, wxEXPAND|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("Clue prompt format") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 3, 5 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText23;
	m_staticText23 = new wxStaticText( miscPanel, wxID_ANY, wxT("Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	fgSizer1->Add( m_staticText23, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_cluePromptFormat = new wxTextCtrl( miscPanel, wxID_ANY, wxT("<center>%N. %T</center>"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_cluePromptFormat, 0, wxEXPAND, 5 );
	
	sbSizer13->Add( fgSizer1, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("Format help") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 4, 0, 0 );
	fgSizer2->AddGrowableCol( 1 );
	fgSizer2->AddGrowableCol( 3 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText32;
	m_staticText32 = new wxStaticText( miscPanel, wxID_ANY, wxT("%T"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	fgSizer2->Add( m_staticText32, 0, wxALL, 5 );
	
	wxStaticText* m_staticText33;
	m_staticText33 = new wxStaticText( miscPanel, wxID_ANY, wxT("Clue text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText33->Wrap( -1 );
	fgSizer2->Add( m_staticText33, 0, wxALL, 5 );
	
	wxStaticText* m_staticText34;
	m_staticText34 = new wxStaticText( miscPanel, wxID_ANY, wxT("%N"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText34->Wrap( -1 );
	fgSizer2->Add( m_staticText34, 0, wxALL, 5 );
	
	wxStaticText* m_staticText35;
	m_staticText35 = new wxStaticText( miscPanel, wxID_ANY, wxT("Clue number"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText35->Wrap( -1 );
	fgSizer2->Add( m_staticText35, 0, wxALL, 5 );
	
	sbSizer12->Add( fgSizer2, 1, wxEXPAND, 5 );
	
	sbSizer13->Add( sbSizer12, 0, wxEXPAND|wxALL, 5 );
	
	bSizer42->Add( sbSizer13, 0, wxEXPAND|wxALL, 10 );
	
	wxStaticBoxSizer* sbSizer191;
	sbSizer191 = new wxStaticBoxSizer( new wxStaticBox( miscPanel, wxID_ANY, wxT("Grid tweaks") ), wxVERTICAL );
	
	wxGridBagSizer* gbSizer1;
	gbSizer1 = new wxGridBagSizer( 3, 8 );
	gbSizer1->SetFlexibleDirection( wxBOTH );
	gbSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText330;
	m_staticText330 = new wxStaticText( miscPanel, wxID_ANY, wxT("Percent of square for text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText330->Wrap( -1 );
	gbSizer1->Add( m_staticText330, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_letterScale = new wxSpinCtrl( miscPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 95, 42 );
	gbSizer1->Add( m_letterScale, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText331;
	m_staticText331 = new wxStaticText( miscPanel, wxID_ANY, wxT("Percent of square for clue number:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText331->Wrap( -1 );
	gbSizer1->Add( m_staticText331, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_numberScale = new wxSpinCtrl( miscPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 95, 75 );
	gbSizer1->Add( m_numberScale, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticText* m_staticText341;
	m_staticText341 = new wxStaticText( miscPanel, wxID_ANY, wxT("Note: these numbers do not have to add up to 100%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText341->Wrap( -1 );
	m_staticText341->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	
	gbSizer1->Add( m_staticText341, wxGBPosition( 2, 0 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER_VERTICAL|wxBOTTOM, 10 );
	
	wxStaticText* m_staticText41;
	m_staticText41 = new wxStaticText( miscPanel, wxID_ANY, wxT("Line thickness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	gbSizer1->Add( m_staticText41, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_lineThickness = new wxSpinCtrl( miscPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 1 );
	gbSizer1->Add( m_lineThickness, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer191->Add( gbSizer1, 1, wxEXPAND|wxALL, 5 );
	
	bSizer42->Add( sbSizer191, 0, wxEXPAND|wxALL, 10 );
	
	miscPanel->SetSizer( bSizer42 );
	miscPanel->Layout();
	bSizer42->Fit( miscPanel );
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
	
	m_printGridLetterFont = new wxFontPickerCtrl( printPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_printGridLetterFont->SetMaxPointSize( 100 ); 
	fgSizer5->Add( m_printGridLetterFont, 0, 0, 5 );
	
	wxStaticText* m_staticText144;
	m_staticText144 = new wxStaticText( printPanel, wxID_ANY, wxT("Grid text (font size is ignored)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText144->Wrap( -1 );
	fgSizer5->Add( m_staticText144, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printGridNumberFont = new wxFontPickerCtrl( printPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_printGridNumberFont->SetMaxPointSize( 100 ); 
	fgSizer5->Add( m_printGridNumberFont, 0, 0, 5 );
	
	wxStaticText* m_printGridNumberFontLabel;
	m_printGridNumberFontLabel = new wxStaticText( printPanel, wxID_ANY, wxT("Grid numbers (font size is ignored)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_printGridNumberFontLabel->Wrap( -1 );
	fgSizer5->Add( m_printGridNumberFontLabel, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_printClueFont = new wxFontPickerCtrl( printPanel, wxID_ANY, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL );
	m_printClueFont->SetMaxPointSize( 100 ); 
	fgSizer5->Add( m_printClueFont, 0, 0, 5 );
	
	wxStaticText* m_staticText1412;
	m_staticText1412 = new wxStaticText( printPanel, wxID_ANY, wxT("Clue list"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1412->Wrap( -1 );
	fgSizer5->Add( m_staticText1412, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer14->Add( fgSizer5, 1, wxALL|wxEXPAND, 5 );
	
	bSizer371->Add( sbSizer14, 0, wxEXPAND|wxALL, 5 );
	
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
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( wxFB_PreferencesDialog::OnInit ) );
	m_saveFileHistory->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnSaveFileHistory ), NULL, this );
	m_printCustomFonts->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_buttonsApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnApply ), NULL, this );
	m_buttonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnOK ), NULL, this );
}

wxFB_PreferencesDialog::~wxFB_PreferencesDialog()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( wxFB_PreferencesDialog::OnInit ) );
	m_saveFileHistory->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnSaveFileHistory ), NULL, this );
	m_printCustomFonts->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnPrintCustomFonts ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_printBlackSquareBrightness->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxFB_PreferencesDialog::OnBlackSquareBrightness ), NULL, this );
	m_buttonsApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnApply ), NULL, this );
	m_buttonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_PreferencesDialog::OnOK ), NULL, this );
	
}

wxFB_ConvertDialog::wxFB_ConvertDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Files") ), wxVERTICAL );
	
	m_list = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 400,150 ), wxLC_REPORT );
	sbSizer2->Add( m_list, 1, wxALL|wxEXPAND, 5 );
	
	bSizer1->Add( sbSizer2, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_optionsButton = new wxButton( this, wxID_ANY, wxT("Options <<<"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer33->Add( m_optionsButton, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	m_add = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_add, 0, wxALL, 5 );
	
	m_remove = new wxButton( this, wxID_ANY, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_remove, 0, wxALL, 5 );
	
	m_runButton = new wxButton( this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_runButton, 0, wxALL, 5 );
	
	bSizer35->Add( bSizer32, 0, wxALIGN_RIGHT, 5 );
	
	bSizer34->Add( bSizer35, 1, wxEXPAND, 5 );
	
	bSizer33->Add( bSizer34, 1, wxEXPAND, 5 );
	
	bSizer1->Add( bSizer33, 0, wxEXPAND, 5 );
	
	m_mainSizer->Add( bSizer1, 1, wxEXPAND, 5 );
	
	m_optionsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Options") ), wxVERTICAL );
	
	wxBoxSizer* bSizer351;
	bSizer351 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_overwriteChoices[] = { wxT("Rename"), wxT("Overwrite") };
	int m_overwriteNChoices = sizeof( m_overwriteChoices ) / sizeof( wxString );
	m_overwrite = new wxRadioBox( this, wxID_ANY, wxT("Name Collisions"), wxDefaultPosition, wxDefaultSize, m_overwriteNChoices, m_overwriteChoices, 1, wxRA_SPECIFY_ROWS );
	m_overwrite->SetSelection( 0 );
	bSizer351->Add( m_overwrite, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_errorHandlingChoices[] = { wxT("Ignore if possible"), wxT("Always fail") };
	int m_errorHandlingNChoices = sizeof( m_errorHandlingChoices ) / sizeof( wxString );
	m_errorHandling = new wxRadioBox( this, wxID_ANY, wxT("Error Handling"), wxDefaultPosition, wxDefaultSize, m_errorHandlingNChoices, m_errorHandlingChoices, 1, wxRA_SPECIFY_ROWS );
	m_errorHandling->SetSelection( 1 );
	bSizer351->Add( m_errorHandling, 0, wxALL|wxEXPAND, 5 );
	
	m_optionsSizer->Add( bSizer351, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Output Directory") ), wxVERTICAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_useInput = new wxRadioButton( this, wxID_ANY, wxT("Use input folder"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_useInput->SetValue( true ); 
	bSizer3->Add( m_useInput, 0, wxALL, 5 );
	
	m_specifyDirectory = new wxRadioButton( this, wxID_ANY, wxT("Specify a directory"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_specifyDirectory, 0, wxALL, 5 );
	
	sbSizer4->Add( bSizer3, 0, 0, 5 );
	
	m_outputDirectory = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Default output folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL );
	m_outputDirectory->Enable( false );
	
	sbSizer4->Add( m_outputDirectory, 1, wxALL|wxEXPAND, 5 );
	
	m_optionsSizer->Add( sbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Log") ), wxHORIZONTAL );
	
	m_useLog = new wxCheckBox( this, wxID_ANY, wxT("Use log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_useLog->Hide();
	
	sbSizer5->Add( m_useLog, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_logfile = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL );
	m_logfile->Enable( false );
	m_logfile->Hide();
	
	sbSizer5->Add( m_logfile, 1, wxALL|wxEXPAND, 5 );
	
	m_optionsSizer->Add( sbSizer5, 0, wxEXPAND|wxALL, 5 );
	
	m_mainSizer->Add( m_optionsSizer, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( m_mainSizer );
	this->Layout();
	m_mainSizer->Fit( this );
	
	// Connect Events
	m_list->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( wxFB_ConvertDialog::OnDoubleClick ), NULL, this );
	m_optionsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnShowOptions ), NULL, this );
	m_add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnAdd ), NULL, this );
	m_remove->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnRemove ), NULL, this );
	m_runButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnRunButton ), NULL, this );
	m_specifyDirectory->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( wxFB_ConvertDialog::OnSpecifyDirectory ), NULL, this );
	m_useLog->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnUseLog ), NULL, this );
}

wxFB_ConvertDialog::~wxFB_ConvertDialog()
{
	// Disconnect Events
	m_list->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( wxFB_ConvertDialog::OnDoubleClick ), NULL, this );
	m_optionsButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnShowOptions ), NULL, this );
	m_add->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnAdd ), NULL, this );
	m_remove->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnRemove ), NULL, this );
	m_runButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnRunButton ), NULL, this );
	m_specifyDirectory->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( wxFB_ConvertDialog::OnSpecifyDirectory ), NULL, this );
	m_useLog->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxFB_ConvertDialog::OnUseLog ), NULL, this );
	
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

wxFB_CharactersPanel::wxFB_CharactersPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	m_scroller = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL );
	m_scroller->SetScrollRate( 0, 10 );
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxVERTICAL );
	
	m_panel = new wxPanel( m_scroller, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	
	m_sizer = new wxGridSizer( 0, 8, 1, 1 );
	
	m_panel->SetSizer( m_sizer );
	m_panel->Layout();
	m_sizer->Fit( m_panel );
	bSizer241->Add( m_panel, 0, 0, 5 );
	
	m_scroller->SetSizer( bSizer241 );
	m_scroller->Layout();
	bSizer241->Fit( m_scroller );
	bSizer24->Add( m_scroller, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer24 );
	this->Layout();
	bSizer24->Fit( this );
}

wxFB_CharactersPanel::~wxFB_CharactersPanel()
{
}