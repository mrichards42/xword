// This file is part of XWord    
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "MyFrame.hpp"
#include <boost/foreach.hpp>

#include "App.hpp" // To notify that frame has been destroyed

// Puz library
#include "PuzEvent.hpp"

// Windows
#include "dialogs/Layout.hpp"
#include "widgets/SizedText.hpp"
#include "ClueListBox.hpp"
#include "CluePanel.hpp"
#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"

#include <wx/numdlg.h>

//#include "widgets/MyAuiToolBar.hpp" // Change behavior of drop down menu

#include "utils/DragAndDrop.hpp" // File drag and drop

#include "utils/SizerPrinter.hpp"



//------------------------------------------------------------------------------
// Menu and Toolbar IDs
//------------------------------------------------------------------------------

enum
{
    ID_OPEN = wxID_HIGHEST,
    ID_SAVE,
    ID_CLOSE,

    ID_QUIT,

    ID_ZOOM_IN,
    ID_ZOOM_FIT,
    ID_ZOOM_OUT,

    ID_CHECK_LETTER,
    ID_CHECK_WORD,
    ID_CHECK_GRID,
    ID_REVEAL_LETTER,
    ID_REVEAL_WORD,
    ID_REVEAL_INCORRECT,
    ID_REVEAL_GRID,


    ID_SCRAMBLE,
    ID_UNSCRAMBLE,

    ID_LAYOUT_PANES,
    ID_LOAD_LAYOUT,
    ID_SAVE_LAYOUT,

    ID_SHOW_NOTES,
    ID_SHOW_NOTES_NEW,

    ID_TIMER,

#ifdef __WXDEBUG__

    ID_DUMP_STATUS,
    ID_DUMP_LAYOUT

#endif
};



BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU           (ID_OPEN,              MyFrame::OnOpenPuzzle)
    EVT_MENU           (ID_SAVE,              MyFrame::OnSavePuzzle)
    EVT_MENU           (ID_CLOSE,             MyFrame::OnClosePuzzle)
    EVT_MENU           (ID_QUIT,              MyFrame::OnQuit)

    EVT_MENU           (ID_ZOOM_IN,           MyFrame::OnZoomIn)
    EVT_MENU           (ID_ZOOM_FIT,          MyFrame::OnZoomFit)
    EVT_MENU           (ID_ZOOM_OUT,          MyFrame::OnZoomOut)

    EVT_MENU           (ID_CHECK_GRID,        MyFrame::OnCheckGrid)
    EVT_MENU           (ID_CHECK_WORD,        MyFrame::OnCheckWord)
    EVT_MENU           (ID_CHECK_LETTER,      MyFrame::OnCheckLetter)
    EVT_MENU           (ID_REVEAL_GRID,       MyFrame::OnRevealGrid)
    EVT_MENU           (ID_REVEAL_INCORRECT,  MyFrame::OnRevealIncorrect)
    EVT_MENU           (ID_REVEAL_WORD,       MyFrame::OnRevealWord)
    EVT_MENU           (ID_REVEAL_LETTER,     MyFrame::OnRevealLetter)

    EVT_MENU           (ID_SCRAMBLE,          MyFrame::OnScramble)
    EVT_MENU           (ID_UNSCRAMBLE,        MyFrame::OnUnscramble)

    EVT_MENU           (ID_LAYOUT_PANES,      MyFrame::OnLayout)
    EVT_MENU           (ID_LOAD_LAYOUT,       MyFrame::OnLoadLayout)
    EVT_MENU           (ID_SAVE_LAYOUT,       MyFrame::OnSaveLayout)
    EVT_MENU           (ID_SHOW_NOTES,        MyFrame::OnShowNotes)

    EVT_MENU           (ID_TIMER,             MyFrame::OnTimer)
    EVT_TIMER          (wxID_ANY,             MyFrame::OnTimerNotify)

    EVT_PUZ_GRID_FOCUS (                      MyFrame::OnGridFocus)
    EVT_PUZ_CLUE_FOCUS (                      MyFrame::OnClueFocus)
    EVT_PUZ_LETTER     (                      MyFrame::OnGridLetter)

    EVT_ACTIVATE       (                      MyFrame::OnActivate)
    EVT_CLOSE          (                      MyFrame::OnClose)

#ifdef __WXDEBUG__

    EVT_MENU           (ID_DUMP_LAYOUT,       MyFrame::OnDumpLayout)
    EVT_MENU           (ID_DUMP_STATUS,       MyFrame::OnDumpStatus)

#endif

END_EVENT_TABLE()


static const ToolDesc toolDesc[] =
{
    { ID_OPEN,  wxITEM_NORMAL, _T("&Open\tCtrl+O"), _T("open") },
    { ID_SAVE,  wxITEM_NORMAL, _T("&Save\tCtrl+S"), _T("save") },
    { ID_CLOSE, wxITEM_NORMAL, _T("&Close\tCtrl+W") },
    { ID_QUIT,  wxITEM_NORMAL, _T("&Quit\tCtrl+Q") },

    { ID_ZOOM_IN,  wxITEM_NORMAL, _T("Zoom In"),  _T("zoom_in")  },
    { ID_ZOOM_FIT, wxITEM_CHECK,  _T("Zoom Fit"), _T("zoom_fit") },
    { ID_ZOOM_OUT, wxITEM_NORMAL, _T("Zoom Out"), _T("zoom_out") },

    { ID_SCRAMBLE,   wxITEM_NORMAL, _T("Scramble...") },
    { ID_UNSCRAMBLE, wxITEM_NORMAL, _T("Unscramble...") },

    { ID_CHECK_LETTER,  wxITEM_NORMAL, _T("Check Letter"), _T("check_letter") },
    { ID_CHECK_WORD,    wxITEM_NORMAL, _T("Check Word"),   _T("check_word") },
    { ID_CHECK_GRID,    wxITEM_NORMAL, _T("Check All"),    _T("check_grid") },
    { ID_REVEAL_LETTER, wxITEM_NORMAL, _T("Reveal Letter") },
    { ID_REVEAL_WORD,   wxITEM_NORMAL, _T("Reveal Word") },
    { ID_REVEAL_INCORRECT, wxITEM_NORMAL, _T("Reveal Incorrect letters") },
    { ID_REVEAL_GRID,   wxITEM_NORMAL, _T("Reveal Grid") },

    { ID_LAYOUT_PANES,      wxITEM_CHECK,  _T("Layout"), _T("layout"), _T("") },
    { ID_LOAD_LAYOUT,  wxITEM_NORMAL, _T("Load Layout") },
    { ID_SAVE_LAYOUT,  wxITEM_NORMAL, _T("Save Layout"), },

    { ID_SHOW_NOTES,        wxITEM_CHECK,  _T("Notes"), _T("notes") },

    { ID_TIMER, wxITEM_CHECK, _T("Timer"), _T("timer") },

//    { ID_CUSTOMIZE,         _T("Customize . . ."),     _T("") },

#ifdef __WXDEBUG__

    { ID_DUMP_LAYOUT,   wxITEM_NORMAL, _T("Dump layout") },
    { ID_DUMP_STATUS,   wxITEM_NORMAL, _T("Dump status") },

#endif

    { TOOL_NONE }
};




//------------------------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------------------------

MyFrame::MyFrame()
    : wxFrame(NULL, -1, _T("XWord"), wxDefaultPosition, wxSize(700,700)),
      m_timer(this),
      m_isTimerRunning(false)
{
    wxLogDebug(_T("Creating Frame"));
    SetDropTarget(new XWordFileDropTarget(this));

    SetupToolManager();
    ManageTools();

    CreateWindows();

    SetupWindowManager();
    ManageWindows();

    LoadConfig();

    LoadLayout(_T("Default"));
    UpdateLayout();

    SetIcon(wxICON(aa_main_icon));

    ShowPuzzle();
}


MyFrame::~MyFrame()
{
    // Let the App know we've been destroyed
    wxGetApp().m_frame = NULL;

    // Cleanup
    m_mgr.UnInit();
    wxLogDebug(_T("Frame has been destroyed"));
}



//------------------------------------------------------------------------------
// XWord puzzle loading / saving
//------------------------------------------------------------------------------

bool
MyFrame::LoadPuzzle(const wxString & filename, const wxString & ext)
{
    wxStopWatch sw;

    const bool success = m_puz.Load(filename, ext);

    ShowPuzzle();

    if (success)
        SetStatus(wxString::Format(_T("%s   Load time: %d ms"),
                                   m_puz.m_filename,
                                   sw.Time()));
    else
        SetStatus(_T("No file loaded"));

    return success;
}


bool
MyFrame::SavePuzzle(const wxString & filename, const wxString & ext)
{
    wxStopWatch sw;

    const bool success = m_puz.Save(filename, ext);

    // Reset save/save as flag
    m_toolMgr.GetTool(ID_SAVE)->SetLabel(_T("&Save As\tCtrl+S"));

    SetStatus(wxString::Format(_T("%s   Save time: %d ms"),
                               m_puz.m_filename,
                               sw.Time()));

    return success;
}


bool
MyFrame::ClosePuzzle(bool prompt)
{
    // No puzzle is open
    if (! m_puz.IsOk())
        return true;

    if (prompt && m_puz.m_modified)
    {
        int ret = wxMessageBox(
                  _T("Current Puzzle not saved.  Save before closing?"),
                  _T("XWord Message"),
                  wxYES_NO | wxCANCEL | wxICON_QUESTION
              );

        if (ret == wxCANCEL)
            return false;

        if (ret == wxYES)
            SavePuzzle(m_puz.m_filename);
    }

    SetStatus(_T("No file loaded"));
    m_puz.Clear();

    ShowPuzzle();
    return true;
}


void
MyFrame::ShowPuzzle()
{
    // If there is no puzzle, display a blank frame
    if (! m_puz.IsOk())
    {
        m_puz.Clear();
        m_gridCtrl->SetXGrid(NULL);
    }
    else
        m_gridCtrl->SetXGrid     (&m_puz.m_grid);

    // Everything else is OK
    m_across   ->SetClueList(m_puz.m_across);
    m_down     ->SetClueList(m_puz.m_down);
    m_title    ->SetLabel   (m_puz.m_title);
    m_author   ->SetLabel   (m_puz.m_author);
    m_copyright->SetLabel   (m_puz.m_copyright);
    m_notes    ->ChangeValue(m_puz.m_notes);

    StopTimer();
    SetTime(m_puz.m_time);    

    // Reset save/save as flag
    m_toolMgr.GetTool(ID_SAVE)->SetLabel(_T("&Save As\tCtrl+S"));


    // Set the notes bitmap depending on whether there are notes or not
    if (m_puz.m_notes.empty())
        m_toolMgr.GetTool(ID_SHOW_NOTES)->SetIconName(_T("notes"));
    else
        m_toolMgr.GetTool(ID_SHOW_NOTES)->SetIconName(_T("notes_new"));

    if (m_puz.IsOk())
    {
        SetStatus(m_puz.m_filename);
        EnableTools(true);


        const bool scrambled = m_puz.IsScrambled();
        EnableScramble(! scrambled);
        EnableUnscramble(scrambled);
        EnableCheck(! scrambled);
        EnableReveal(! scrambled);

        m_gridCtrl->SetFocusedClue(1, DIR_ACROSS);

        // Inform user if puzzle is already completed
        CheckPuzzle();
    }
    else
    {
        EnableTools(false);
        m_cluePrompt->SetLabel(wxEmptyString);
    }

    m_gridCtrl->Refresh();
}


void
MyFrame::CheckPuzzle()
{
    if (m_gridCtrl->GetBlankCount() == 0)
    {
        if (m_puz.IsScrambled())
        {
            StopTimer();
            m_status->SetAlert(
                _T("The puzzle is completely filled, ")
                _T("but the solution is scrambled"),
                *wxWHITE, wxColor(0, 255, 255));
        }
        else
        {
            if (m_gridCtrl->GetIncorrectCount() == 0)
            {
                StopTimer();
                m_status->SetAlert(_T("The puzzle is filled correctly!"),
                                  *wxWHITE, *wxGREEN);
            }
            else
            {
                m_status->SetAlert(
                    _T("The puzzle is completely filled,")
                    _T("but some letters are incorrect"),
                    *wxWHITE, *wxRED);
            }
        }
    }
    else
        m_status->SetAlert(_T(""));
}



//------------------------------------------------------------------------------
// Window setup functions
//------------------------------------------------------------------------------

void
MyFrame::CreateWindows()
{
    m_gridCtrl = new XGridCtrl(this);

    m_across = new CluePanel(this, wxID_ANY, _T("Across"), DIR_ACROSS);
    m_down   = new CluePanel(this, wxID_ANY, _T("Down"),   DIR_DOWN);

    m_title      = new SizedText (this, wxID_ANY);
    m_author     = new SizedText (this, wxID_ANY);
    m_copyright  = new SizedText (this, wxID_ANY);
    m_cluePrompt = new SizedText (this, wxID_ANY);
    m_notes      = new wxTextCtrl(this, wxID_ANY,
                                  wxEmptyString,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  wxTE_MULTILINE);

#ifdef USE_AUI_TOOLBAR
    m_toolbar = MakeAuiToolBar();
#else // ! USE_AUI_TOOLBAR
    m_toolbar = MakeToolBar();
#endif // USE_AUI_TOOLBAR / !

    m_menubar = MakeMenuBar();
    SetMenuBar(m_menubar);

    m_status = new MyStatusBar(this);
    SetStatusBar(m_status);

    // Fonts
    wxFont clueFont(12, wxFONTFAMILY_SWISS,
                    wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_cluePrompt->SetFont(clueFont);
}


#ifdef USE_AUI_TOOLBAR
wxAuiToolBar * 
MyFrame::MakeAuiToolBar()
{
    wxAuiToolBar * tb = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, 
                                         wxDefaultSize,
                                         wxAUI_TB_HORZ_TEXT);

    tb->GetArtProvider()->SetTextOrientation(wxAUI_TBTOOL_TEXT_BOTTOM);

#else // ! USE_AUI_TOOLBAR
wxToolBar *
MyFrame::MakeToolBar()
{
    wxToolBar * tb = new wxToolBar(this, wxID_ANY,
                                   wxDefaultPosition,
                                   wxDefaultSize,
                                   wxBORDER_NONE | wxTB_HORIZONTAL | wxTB_FLAT);

    tb->SetToolBitmapSize( wxSize(m_toolMgr.GetIconSize_ToolBar(),
                                  m_toolMgr.GetIconSize_ToolBar()) );

#endif // USE_AUI_TOOLBAR / !

    m_toolMgr.Add(tb, ID_OPEN);
    m_toolMgr.Add(tb, ID_SAVE);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_ZOOM_IN);
    m_toolMgr.Add(tb, ID_ZOOM_FIT);
    m_toolMgr.Add(tb, ID_ZOOM_OUT);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_CHECK_LETTER);
    m_toolMgr.Add(tb, ID_CHECK_WORD);
    m_toolMgr.Add(tb, ID_CHECK_GRID);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_LAYOUT_PANES);
    m_toolMgr.Add(tb, ID_SHOW_NOTES);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_TIMER);

#ifdef USE_AUI_TOOLBAR
    // Overflow button
    tb->SetOverflowVisible(true);
#else // ! USE_AUI_TOOLBAR
    SetToolBar(tb);
#endif // USE_AUI_TOOLBAR / !

    tb->Realize();
    return tb;
}


wxMenuBar * 
MyFrame::MakeMenuBar()
{
    wxMenuBar * mb = new wxMenuBar();

    // File Menu
    wxMenu * menu = new wxMenu();
        m_toolMgr.Add(menu, ID_OPEN);
        m_toolMgr.Add(menu, ID_SAVE);
        m_toolMgr.Add(menu, ID_CLOSE);
        m_toolMgr.Add(menu, ID_QUIT);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_ZOOM_IN);
        m_toolMgr.Add(menu, ID_ZOOM_FIT);
        m_toolMgr.Add(menu, ID_ZOOM_OUT);
    mb->Append(menu, _T("&File"));

    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_LAYOUT_PANES);
        m_toolMgr.Add(menu, ID_LOAD_LAYOUT);
        m_toolMgr.Add(menu, ID_SAVE_LAYOUT);
        m_toolMgr.Add(menu, ID_SHOW_NOTES);
    mb->Append(menu, _T("&Layout"));

    // Solution Menu
    menu = new wxMenu();
        wxMenu * subMenu = new wxMenu();
            m_toolMgr.Add(subMenu, ID_CHECK_LETTER);
            m_toolMgr.Add(subMenu, ID_CHECK_WORD);
            m_toolMgr.Add(subMenu, ID_CHECK_GRID);
        menu->AppendSubMenu(subMenu, _T("Check"));
        subMenu = new wxMenu();
            m_toolMgr.Add(subMenu, ID_REVEAL_LETTER);
            m_toolMgr.Add(subMenu, ID_REVEAL_WORD);
            m_toolMgr.Add(subMenu, ID_REVEAL_INCORRECT);
            m_toolMgr.Add(subMenu, ID_REVEAL_GRID);
        menu->AppendSubMenu(subMenu, _T("Reveal"));
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_SCRAMBLE);
        m_toolMgr.Add(menu, ID_UNSCRAMBLE);
    mb->Append(menu, _T("&Solution"));

    // Tools Menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_TIMER);
    mb->Append(menu, _T("&Tools"));

#ifdef __WXDEBUG__

    // Debug menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_DUMP_STATUS);
        m_toolMgr.Add(menu, ID_DUMP_LAYOUT);
    mb->Append(menu, _T("&Debug"));

#endif

    return mb;
}



//------------------------------------------------------------------------------
// Window management
//------------------------------------------------------------------------------

void
MyFrame::SetupWindowManager()
{
    m_mgr.SetManagedWindow(this);
    wxAuiDockArt * art = m_mgr.GetArtProvider();

    wxColor active = wxSystemSettings::GetColour(
                                  wxSYS_COLOUR_ACTIVECAPTION);
    wxColor inactive = wxSystemSettings::GetColour(
                                  wxSYS_COLOUR_INACTIVECAPTION);

    art->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, active);
    active.Set(active.Red()-50, active.Green()-50, active.Blue()-50);
    art->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, active);

    art->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, inactive);
    inactive.Set(inactive.Red()-50, inactive.Green()-50, inactive.Blue()-50);
    art->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, inactive);
}


void
MyFrame::ManageWindows()
{
    // Give everything a name so we can save and load the layout
    m_mgr.AddPane(m_gridCtrl,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .CenterPane()
                  .Caption(_T("Grid"))
                  .Name(_T("Grid")) );

#ifdef USE_AUI_TOOLBAR
    // It would be better to use MinSize, instead of Fixed, but it doesn't work
    // quite right, and we would have to update the MinSize every time the 
    // toolbar was resized . . .
    m_mgr.AddPane(m_toolbar,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Top()
                  .Fixed()
                  .Layer(5)
                  .Caption(_T("Tools"))
                  .Name(_T("Tools")));
#endif // USE_AUI_TOOLBAR

    m_mgr.AddPane(m_across,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(4)
                  .Left()
                  .Caption(_T("Across"))
                  .Name(_T("Across")) );

    m_mgr.AddPane(m_down,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(4)
                  .Left()
                  .Caption(_T("Down"))
                  .Name(_T("Down")) );

    m_mgr.AddPane(m_title,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Title"))
                  .Name(_T("Title")) );

    m_mgr.AddPane(m_author,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Author"))
                  .Name(_T("Author")) );

    m_mgr.AddPane(m_copyright,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Copyright"))
                  .Name(_T("Copyright")) );

    m_mgr.AddPane(m_cluePrompt,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Layer(2)
                  .Top()
                  .Caption(_T("Clue"))
                  .Name(_T("Clue")) );

    m_mgr.AddPane(m_notes,
                  wxAuiPaneInfo()
                  .CaptionVisible(false)
                  .Float()
                  .Hide()
                  .Caption(_T("Notes"))
                  .Name(_T("Notes")));
}


//------------------------------------------------------------------------------
// Tool management
//------------------------------------------------------------------------------

void
MyFrame::SetupToolManager()
{
    m_toolMgr.SetIconSize_AuiToolBar(24);
    m_toolMgr.SetIconSize_ToolBar(24);
    m_toolMgr.SetIconSize_Menu(16);
}


void
MyFrame::ManageTools()
{
    m_toolMgr.SetDesc(toolDesc);
}


void
MyFrame::EnableTools(bool enable)
{
    EnableSave(enable);
    EnableClose(enable);
    EnableGridSize(enable);
    EnableScramble(enable);
    EnableUnscramble(enable);
    EnableCheck(enable);
    EnableReveal(enable);
    EnableNotes(enable);
    EnableTimer(enable);
}

void
MyFrame::EnableSave(bool enable)
{
    m_toolMgr.Enable(ID_SAVE,         enable);
}

void
MyFrame::EnableClose(bool enable)
{
    m_toolMgr.Enable(ID_CLOSE,        enable);
}

void
MyFrame::EnableGridSize(bool enable)
{
    m_toolMgr.Enable(ID_ZOOM_IN,      enable);
    m_toolMgr.Enable(ID_ZOOM_OUT,     enable);
    m_toolMgr.Enable(ID_ZOOM_FIT,     enable);
}

void
MyFrame::EnableScramble(bool enable)
{
    m_toolMgr.Enable(ID_SCRAMBLE,     enable);
}

void
MyFrame::EnableUnscramble(bool enable)
{
    m_toolMgr.Enable(ID_UNSCRAMBLE,     enable);
}

void
MyFrame::EnableCheck(bool enable)
{
    m_toolMgr.Enable(ID_CHECK_LETTER, enable);
    m_toolMgr.Enable(ID_CHECK_WORD,   enable);
    m_toolMgr.Enable(ID_CHECK_GRID,   enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("Solution"), _T("Check")),
                      enable);

}

void
MyFrame::EnableReveal(bool enable)
{
    m_toolMgr.Enable(ID_REVEAL_LETTER,    enable);
    m_toolMgr.Enable(ID_REVEAL_WORD,      enable);
    m_toolMgr.Enable(ID_REVEAL_INCORRECT, enable);
    m_toolMgr.Enable(ID_REVEAL_GRID,      enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("Solution"),_T("Reveal")),
                      enable);
}

void
MyFrame::EnableNotes(bool enable)
{
    m_toolMgr.Enable(ID_SHOW_NOTES,   enable);
}

void
MyFrame::EnableTimer(bool enable)
{
    m_toolMgr.Enable(ID_TIMER,        enable);
}



//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------

void
MyFrame::LoadConfig()
{
    wxString configPath = GetConfigPath();
    wxLogDebug(_T("Config file: %s"), configPath);

    // Create a blank file it it doesn't exist
    if (! wxFileName::FileExists(configPath))
        wxFile(configPath, wxFile::write);

    // Setup wxFileConfig
    wxFileInputStream file(configPath);
    wxFileConfig::Set( new wxFileConfig(file) );

    wxFileConfig * config = GetConfig();

    // Load settings
    //--------------
    config->SetPath(_T("/Window"));
    long x, y, w, h;
    bool maximized;
    bool hasData = true;
    hasData = hasData && config->Read(_T("width"),     &w);
    hasData = hasData && config->Read(_T("height"),    &h);
    hasData = hasData && config->Read(_T("top"),       &y);
    hasData = hasData && config->Read(_T("left"),      &x);
    hasData = hasData && config->Read(_T("maximized"), &maximized);
    if (! hasData)
    {
        x = 20;
        y = 20;
        w = 500;
        h = 500;
        maximized = false;
    }

    config->SetPath(_T("/Grid"));
    bool fit_grid;
    int  grid_style;
    hasData = true;
    hasData = hasData && config->Read(_T("fit"),   &fit_grid);
    hasData = hasData && config->Read(_T("style"), &grid_style);
    if (! hasData)
    {
        fit_grid = true;
        grid_style = DEFAULT_GRID_STYLE;
    }


    // Apply the settings
    // -------------------
    if (maximized)
        Maximize();
    else
        SetSize(x, y, w, h);

    m_gridCtrl->SetGridStyle(grid_style);
    if (fit_grid)
        m_toolMgr.Check(ID_ZOOM_FIT);

    config->SetPath(_T("/"));
}


void
MyFrame::SaveConfig()
{
    SaveLayout(_T("Default"));

    // Save window settings
    wxFileConfig * config = GetConfig();

    config->SetPath(_T("/Window"));

    // Only save window position if it is not maximized
    if (! IsMaximized())
    {
        int x, y, w, h;
        GetSize(&w, &h);
        GetPosition(&x, &y);
        if (w < 200) w = 500;
        if (h < 200) h = 500;
        if (x < 0 || x >= wxSystemSettings::GetMetric(wxSYS_SCREEN_X)) x = 20;
        if (y < 0 || y >= wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)) y = 20;
        config->Write(_T("width"),     w);
        config->Write(_T("height"),    h);
        config->Write(_T("top"),       y);
        config->Write(_T("left"),      x);
    }
    config->Write(_T("maximized"), IsMaximized());

    config->SetPath(_T("/Grid"));
    config->Write(_T("style"), m_gridCtrl->GetGridStyle());
    config->Write(_T("fit"),   m_toolMgr.IsChecked(ID_ZOOM_FIT));

    // Save config file
    wxFileOutputStream file(GetConfigPath());
    config->Save( file );
}


//------------------------------------------------------------------------------
// Menu and toolbar events
//------------------------------------------------------------------------------

void
MyFrame::OnOpenPuzzle(wxCommandEvent & WXUNUSED(evt))
{
    wxString filename = wxFileSelector(
                            _T("Open Puzzle"),
                            wxEmptyString,
                            wxEmptyString,
                            _T("puz"),
                            XPuzzle::GetLoadTypeString()
                                + _T("|All Files (*.*)|*.*"),
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST
                        );

    if (! filename.empty())
        LoadPuzzle(filename);
}




void
MyFrame::OnSavePuzzle(wxCommandEvent & WXUNUSED(evt))
{
    m_puz.m_notes = m_notes->GetValue();
    m_puz.m_time = m_time;
    m_puz.m_complete = (m_gridCtrl->GetIncorrectCount() + m_gridCtrl->GetBlankCount() == 0);

    wxString filename;
    if (! m_puz.m_modified)
        filename = wxFileSelector(
                        _T("Save Puzzle As"),
                        wxEmptyString, wxEmptyString, _T("puz"),
                        XPuzzle::GetSaveTypeString()
                            + _T("|All Files (*.*)|*.*"),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
                   );
    else
        filename = m_puz.m_filename;

    if (! filename.empty())
        SavePuzzle(filename);
}


void
MyFrame::OnZoomFit(wxCommandEvent & WXUNUSED(evt))
{
    bool fit = m_toolMgr.Toggle(ID_ZOOM_FIT);

    // Don't enable/disagle Zoom buttons
    /*
    m_toolMgr.Enable(ID_ZOOM_OUT, !fit);
    m_toolMgr.Enable(ID_ZOOM_IN,  !fit);
    */

    m_gridCtrl->FitGrid(fit);
}


void
MyFrame::OnZoomIn(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(ID_ZOOM_FIT, false);
    m_gridCtrl->ZoomIn();
}


void
MyFrame::OnZoomOut(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(ID_ZOOM_FIT, false);
    m_gridCtrl->ZoomOut();
}


void
MyFrame::OnScramble(wxCommandEvent & WXUNUSED(evt))
{
    wxASSERT(m_gridCtrl->GetXGrid() != NULL);
    int key = wxGetNumberFromUser(
                    _T("Enter a four-digit key"),
                    _T("Key (0 to generate automatically):"),
                    _T("Scrambing solution"),
                    0,
                    0,
                    9999);
    if (key < 0)
        return;

    if (key < 1000)
        key = 0;

    if (m_gridCtrl->GetXGrid()->ScrambleSolution(key))
    {
        wxMessageBox(wxString::Format(_T("Solution scrambled.  Key is %d"),
                                      m_gridCtrl->GetXGrid()->GetKey()),
                     _T("XWord Message"));

        m_gridCtrl->RecheckGrid();
        CheckPuzzle();

        EnableScramble(false);
        EnableUnscramble(true);

        EnableCheck(false);
        EnableReveal(false);
    }
    else // This should never be the case
    {
        wxMessageBox(_T("Scrambling failed!"));

        wxTrap();
    }
}


void
MyFrame::OnUnscramble(wxCommandEvent & WXUNUSED(evt))
{
    wxASSERT(m_gridCtrl->GetXGrid() != NULL);

    int key = m_gridCtrl->GetXGrid()->GetKey();

    if (key == 0)
    {
        key = wxGetNumberFromUser(
                    _T("Enter the four-digit key"),
                    _T("Key:"),
                    _T("Unscrambling solution"),
                    0,
                    1000,
                    9999);
        if (key < 1000)
            return;
    }

    if (m_gridCtrl->GetXGrid()->UnscrambleSolution(key))
    {
        wxMessageBox(_T("Solution unscrambled!"),
                     _T("XWord Message"));

        m_gridCtrl->RecheckGrid();
        CheckPuzzle();

        EnableScramble(true);
        EnableUnscramble(false);

        EnableCheck(true);
        EnableReveal(true);
    }
    else
    {
        wxMessageBox(_T("Wrong Key!"));
    }
}



void
MyFrame::OnLayout(wxCommandEvent & WXUNUSED(evt))
{
    bool allowMove = m_toolMgr.Toggle(ID_LAYOUT_PANES);

    wxAuiPaneInfoArray & panes = m_mgr.GetAllPanes();

    // Loop through all panes and make each one movable
    for (size_t i = 0; i < panes.Count(); ++i)
    {
        wxAuiPaneInfo & info = panes.Item(i);
        info.Floatable(allowMove).Dockable(allowMove);
        info.CaptionVisible(allowMove);
        info.CloseButton(allowMove);
    }
    m_mgr.Update();
}


void
MyFrame::OnLoadLayout(wxCommandEvent & WXUNUSED(evt))
{
    wxFileConfig * config = GetConfig();
    wxArrayString arrayStr;

    // Enumerate all layouts
    
    // Eummy enumeration variables
    wxString str;
    long dummy;

    config->SetPath(_T("/Layouts"));
    bool bCont = config->GetFirstEntry(str, dummy);
    while (bCont)
    {
        arrayStr.Add(str);
        bCont = config->GetNextEntry(str, dummy);
    }
    config->SetPath(_T("/"));


    // Show the dialog
    if (arrayStr.empty())
    {
        wxMessageBox(_T("No layouts found"),
                     _T("XWord Error"),
                     wxICON_EXCLAMATION | wxOK);
        return;
    }
    else
    {
        wxString oldLayout = m_mgr.SavePerspective();
        LayoutDialog dlg(this,
                         _T("Choose a layout"),
                         _T("Load Layout"),
                         arrayStr);

        if (dlg.ShowModal() != wxID_OK)
            LoadLayoutString(oldLayout, true);
    }
}


void
MyFrame::OnSaveLayout(wxCommandEvent & WXUNUSED(evt))
{
    wxString name = wxGetTextFromUser( _T("Enter a name for this layout"),
                                       _T("Save Layout") );

    if (name.empty())
        return;
    SaveLayout(name);
}


void
MyFrame::OnShowNotes(wxCommandEvent & WXUNUSED(evt))
{
    bool show = m_toolMgr.Toggle(ID_SHOW_NOTES);
    ShowPane(_T("Notes"), show);
}



void
MyFrame::OnTimer(wxCommandEvent & WXUNUSED(evt))
{
    bool running = m_toolMgr.Toggle(ID_TIMER);

    if (running)
        m_timer.Start(1000);
    else
        m_timer.Stop();
}


void
MyFrame::OnTimerNotify(wxTimerEvent & WXUNUSED(evt))
{
    SetTime(m_time+1);
}



//------------------------------------------------------------------------------
// XGridCtrl and CluePanel events
//------------------------------------------------------------------------------

void
MyFrame::OnGridFocus(wxPuzEvent & evt)
{
    // Update everything

    m_across->SetClueNumber(evt.GetAcrossClue(),
                            evt.GetDirection() == DIR_ACROSS);

    m_down  ->SetClueNumber(evt.GetDownClue(),
                            evt.GetDirection() == DIR_DOWN);

    m_cluePrompt->SetLabel (evt.GetDirection() == DIR_ACROSS
                          ? m_across->GetClueText()
                          : m_down  ->GetClueText());

    // We shouldn't need this if we're fast enough
    // m_cluePrompt->Update();
}


void
MyFrame::OnClueFocus(wxPuzEvent & evt)
{
    m_gridCtrl->ChangeFocusedClue(evt.GetClueNumber(), evt.GetDirection());

    m_cluePrompt->SetLabel(evt.GetClueText());

    int crossingClue = m_gridCtrl->GetFocusedSquare()
        ->GetWordStart(! evt.GetDirection())->GetNumber();

    if (evt.GetDirection() == DIR_ACROSS)
        m_down->  SetClueNumber(crossingClue, CROSSING_CLUE);
    else
        m_across->SetClueNumber(crossingClue, CROSSING_CLUE);
}


void
MyFrame::OnGridLetter(wxPuzEvent & evt)
{
    // Change the save/save as button
    if (! m_puz.m_modified)
    {
        m_puz.m_modified = true;
        m_toolMgr.GetTool(ID_SAVE)->SetLabel(_T("&Save\tCtrl+S"));
    }
    CheckPuzzle();
}



//------------------------------------------------------------------------------
// Frame events
//------------------------------------------------------------------------------

void
MyFrame::OnActivate(wxActivateEvent & evt)
{
    if (evt.GetActive())
    {
        wxLogDebug(_T("Frame Activate"));
        if (! IsIconized())
        {
            // Keep focus on the XGridCtrl
            // This isn't the best solution, but seems to work despite all the
            // SetFocus() failed messages
            m_gridCtrl->SetFocus();
        }
    }

    evt.Skip();
}



void
MyFrame::OnClose(wxCloseEvent & evt)
{
    if (ClosePuzzle() || ! evt.CanVeto())
    {
        SaveConfig();
        Destroy();
        return;
    }
    else
    {
        evt.Veto();
    }
}



//------------------------------------------------------------------------------
// Debug stuff
//------------------------------------------------------------------------------

#ifdef __WXDEBUG__

void
MyFrame::ShowDebugDialog(const wxString & title, const wxString & str)
{
    wxDialog dlg(this, wxID_ANY,
                 wxString::Format(_T("Debug: %s"), title),
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    wxTextCtrl * text = new wxTextCtrl(&dlg, wxID_ANY,
                                       str,
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       wxTE_MULTILINE);

    text->SetFont( wxFont(8, wxFONTFAMILY_MODERN,
                          wxFONTSTYLE_NORMAL,
                          wxFONTWEIGHT_NORMAL) );


    wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(text, 1, wxEXPAND | wxALL, 5);
    dlg.SetSizer(sizer);
    dlg.ShowModal();
}




void
MyFrame::OnDumpLayout(wxCommandEvent & WXUNUSED(evt))
{
    ShowDebugDialog(_T("Layout"), SizerPrinter::Serialize(this));
}

void
MyFrame::OnDumpStatus(wxCommandEvent & WXUNUSED(evt))
{
    if (! m_puz.IsOk())
    {
        ShowDebugDialog(_T("Current puzzle status"), _T("No file is loaded"));
        return;
    }

    wxString str;

    XGrid * grid = m_gridCtrl->GetXGrid();

    str << _T("Grid size (w x h): ") << _T("\n")
        << (int)grid->GetWidth() << _T(" x ")
        << (int)grid->GetHeight() << _T("\n");

    str << _T("\n");


    str << _T("Solution:") << _T("\n");

    for (XSquare * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << static_cast<wxChar>(square->GetPlainSolution());
        if (square->IsLast(DIR_ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("User grid:") << _T("\n");

    for (XSquare * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << static_cast<wxChar>(square->GetPlainText());
        if (square->IsLast(DIR_ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("Grid flag:") << _T("\n");
    if (! grid->HasSolution())
        str << _T("NO_SOLUTION") << _T("\n");
    if (grid->IsScrambled())
        str << _T("SCRAMBLED") << _T("\n");
    if (grid->HasSolution() && ! grid->IsScrambled())
        str << _T("NORMAL") << _T("\n");

    str << _T("\n");

    str << _T("Grid type:") << _T("\n");
    if (grid->IsDiagramless())
        str << _T("DIAGRAMLESS") << _T("\n");
    else
        str << _T("NORMAL") << _T("\n");

    str << _T("\n");

    str << _T("Grid key:") << _T("\n");
    if (grid->GetKey() == 0)
        str << _T("(None)") << _T("\n");
    else
        str << grid->GetKey() << _T("\n");

    str << _T("\n");

    str << _T("Grid checksum (hex):") << _T("\n");
    if (grid->GetCksum() == 0)
        str << _T("(None)") << _T("\n");
    else
        str << wxString::Format(_T("%x"), grid->GetCksum())  << _T("\n");

    str << _T("\n");

    str << _T("User rebus:") <<_T("\n");
    for (XSquare * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << _T("[");
        if (square->HasTextRebus())
        {
            if (square->HasTextSymbol())
                str << _T("[")
                    << static_cast<int>(square->GetText().at(2))
                    << _T("]");
            else
                str << square->GetText();
        }
        else
            str << _T(" ");
        str << _T("]");

        if (square->IsLast(DIR_ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("Solution rebus:") <<_T("\n");
    for (XSquare * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << _T("[");
        if (square->HasSolutionRebus())
        {
            if (square->HasSolutionSymbol())
                str << _T("[")
                    << static_cast<int>(square->GetSolution().at(2))
                    << _T("]");
            else
                str << square->GetSolution();
        }
        else
            str << _T(" ");
        str << _T("]");

        if (square->IsLast(DIR_ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("Unrecognized sections:") <<_T("\n");

    std::vector<XPuzzle::section>::iterator it;
    for (it  = m_puz.m_extraSections.begin();
         it != m_puz.m_extraSections.end();
         ++it)
    {
        str << it->name
            << wxString::Format(_T("  (length = %d)"), it->data.size())
            << _T("\n");
    }

    ShowDebugDialog(_T("Current puzzle status"), str);
}
#endif // __WXDEBUG__
