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

// Puz library
#include "PuzEvent.hpp"

// Windows
#include "dialogs/PerspectiveDlg.hpp"
#include "widgets/SizedText.hpp"
#include "ClueListBox.hpp"
#include "CluePanel.hpp"
#include "GridCtrl.hpp"
#include "MyStatusBar.hpp"

#include "widgets/MyAuiToolBar.hpp" // Change behavior of drop down menu

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU           (ID_OPEN,              MyFrame::OnOpenPuzzle)
    EVT_MENU           (ID_SAVE,              MyFrame::OnSavePuzzle)
    EVT_MENU           (ID_QUIT,              MyFrame::OnQuit)

    EVT_MENU           (ID_ZOOM_IN,           MyFrame::OnZoomIn)
    EVT_MENU           (ID_ZOOM_FIT,          MyFrame::OnZoomFit)
    EVT_MENU           (ID_ZOOM_OUT,          MyFrame::OnZoomOut)
    EVT_MENU           (ID_CHECK_GRID,        MyFrame::OnCheckGrid)
    EVT_MENU           (ID_CHECK_WORD,        MyFrame::OnCheckWord)
    EVT_MENU           (ID_CHECK_LETTER,      MyFrame::OnCheckLetter)

    EVT_MENU           (ID_LAYOUT_PANES,      MyFrame::OnLayout)
    EVT_MENU           (ID_LOAD_PERSPECTIVE,  MyFrame::OnLoadPerspective)
    EVT_MENU           (ID_SAVE_PERSPECTIVE,  MyFrame::OnSavePerspective)
    EVT_MENU           (ID_SHOW_NOTES,        MyFrame::OnShowNotes)

    EVT_MENU           (ID_TIMER,             MyFrame::OnTimer)
    EVT_TIMER          (wxID_ANY,             MyFrame::OnTimerNotify)

    EVT_PUZ_GRID_FOCUS (                      MyFrame::OnGridFocus)
    EVT_PUZ_CLUE_FOCUS (                      MyFrame::OnClueFocus)
    EVT_PUZ_LETTER     (                      MyFrame::OnGridLetter)

    EVT_ACTIVATE       (                      MyFrame::OnActivate)
    EVT_CLOSE          (                      MyFrame::OnClose)
END_EVENT_TABLE()



// Toolbar/menu items table
const int iconSize = 24;

static const ToolDesc toolDesc[] =
{
    { ID_OPEN, wxITEM_NORMAL, _T("&Open\tCtrl+O"), _T("open") },
    { ID_SAVE, wxITEM_NORMAL, _T("&Save\tCtrl+S"), _T("save") },
    { ID_QUIT, wxITEM_NORMAL, _T("&Quit\tCtrl+Q") },

    { ID_ZOOM_IN,  wxITEM_NORMAL, _T("Zoom In"),  _T("zoom_in")  },
    { ID_ZOOM_FIT, wxITEM_CHECK,  _T("Zoom Fit"), _T("zoom_fit") },
    { ID_ZOOM_OUT, wxITEM_NORMAL, _T("Zoom Out"), _T("zoom_out") },

    { ID_CHECK_LETTER, wxITEM_NORMAL, _T("Check Letter"), _T("check_letter") },
    { ID_CHECK_WORD,   wxITEM_NORMAL, _T("Check Word"),   _T("check_word") },
    { ID_CHECK_GRID,   wxITEM_NORMAL, _T("Check All"),    _T("check_grid") },

    { ID_LAYOUT_PANES,      wxITEM_CHECK,  _T("Layout"), _T("layout"), _T("") },
    { ID_LOAD_PERSPECTIVE,  wxITEM_NORMAL, _T("Load Perspective") },
    { ID_SAVE_PERSPECTIVE,  wxITEM_NORMAL, _T("Save Perspective"), },

    { ID_SHOW_NOTES,        wxITEM_CHECK,  _T("Notes"), _T("notes") },

    { ID_TIMER, wxITEM_CHECK, _T("Timer"), _T("timer") },

//    { ID_CUSTOMIZE,         _T("Customize . . ."),     _T("") },

    { TOOL_NONE }
};



MyFrame::MyFrame()
    : wxFrame(NULL, -1, _T("XWord"), wxDefaultPosition, wxSize(700,700)),
      m_timer(this),
      m_timerRunning(false)
{
    // Note that the order of these functions is important!
    //  1. CreateWindows() before LoadConfig()
    //       - LoadConfig() sets the GridCtrl style flag
    //  2. LoadConfig() before LoadPerspective()
    //       - LoadConfig() loads the available perspectives
    //  3. SetupManager() and CreateWindows() before ManageWindows()

    wxLogDebug(_T("Creating Frame"));

    m_toolMgr.SetDesc(toolDesc);
    SetupManager();

    CreateWindows();
    ManageWindows();

    LoadConfig();

    LoadPerspective(_T("Default"));
    m_mgr.Update();

    SetIcon(wxICON(aa_main_icon));
}



//---------------------------------------------------
// Frame setup functions
//---------------------------------------------------

void
MyFrame::CreateWindows()
{
    // Grid
    m_grid = new GridCtrl(this);

    // Across Clues
    m_acrossPanel = new CluePanel(this, wxID_ANY, _T("Across"), DIR_ACROSS);
    m_across = m_acrossPanel->m_clueList;

    // Down Clues
    m_downPanel = new CluePanel(this, wxID_ANY, _T("Down"), DIR_DOWN);
    m_down = m_downPanel->m_clueList;

    m_title      = new SizedText (this, wxID_ANY);
    m_author     = new SizedText (this, wxID_ANY);
    m_copyright  = new SizedText (this, wxID_ANY);
    m_cluePrompt = new SizedText (this, wxID_ANY);
    m_notes      = new wxTextCtrl(this, wxID_ANY,
                                  wxEmptyString,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  wxTE_MULTILINE);

    m_toolbar = MakeAuiToolBar();

    m_menubar = MakeMenuBar();
    SetMenuBar(m_menubar);

    m_status = new MyStatusBar(this);
    SetStatusBar(m_status);

    // Fonts
    wxFont clueFont(12, wxFONTFAMILY_SWISS,
                    wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_cluePrompt->SetFont(clueFont);
}



inline wxAuiToolBar * 
MyFrame::MakeAuiToolBar()
{
    MyAuiToolBar * tb = new MyAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_TEXT);
    //tb->SetArtProvider(new MyAuiToolBarArt());
    tb->GetArtProvider()->SetTextOrientation(wxAUI_TBTOOL_TEXT_BOTTOM);
    tb->SetToolManager(&m_toolMgr);

    m_toolMgr.GetTool(ID_OPEN)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_SAVE)->Add(tb, iconSize, iconSize);
    tb->AddSeparator();
    m_toolMgr.GetTool(ID_ZOOM_IN)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_ZOOM_FIT)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_ZOOM_OUT)->Add(tb, iconSize, iconSize);
    tb->AddSeparator();
    m_toolMgr.GetTool(ID_CHECK_LETTER)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_CHECK_WORD)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_CHECK_GRID)->Add(tb, iconSize, iconSize);
    tb->AddSeparator();
    m_toolMgr.GetTool(ID_LAYOUT_PANES)->Add(tb, iconSize, iconSize);
    m_toolMgr.GetTool(ID_SHOW_NOTES)->Add(tb, iconSize, iconSize);
    tb->AddSeparator();
    m_toolMgr.GetTool(ID_TIMER)->Add(tb, iconSize, iconSize);

    // Overflow button
    tb->SetOverflowVisible(true);

    tb->Realize();
    return tb;
}


wxMenuBar * 
MyFrame::MakeMenuBar()
{
    wxMenuBar * mb = new wxMenuBar();

    int menuIconSize = 16;
    // File Menu
    wxMenu * menu = new wxMenu();
        m_toolMgr.GetTool(ID_OPEN)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_SAVE)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_QUIT)->Add(menu, menuIconSize, menuIconSize);
        menu->AppendSeparator();
        m_toolMgr.GetTool(ID_ZOOM_IN)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_ZOOM_FIT)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_ZOOM_OUT)->Add(menu, menuIconSize, menuIconSize);
    mb->Append(menu, _T("&File"));

    menu = new wxMenu();
        m_toolMgr.GetTool(ID_LAYOUT_PANES)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_LOAD_PERSPECTIVE)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_SAVE_PERSPECTIVE)->Add(menu, menuIconSize, menuIconSize);
        m_toolMgr.GetTool(ID_SHOW_NOTES)->Add(menu, menuIconSize, menuIconSize);
    mb->Append(menu, _T("&Layout"));

    // Solution Menu
    menu = new wxMenu();
        wxMenu * subMenu = new wxMenu();
            m_toolMgr.GetTool(ID_CHECK_LETTER)->Add(subMenu, menuIconSize, menuIconSize);
            m_toolMgr.GetTool(ID_CHECK_WORD)->Add(subMenu, menuIconSize, menuIconSize);
            m_toolMgr.GetTool(ID_CHECK_GRID)->Add(subMenu, menuIconSize, menuIconSize);
        menu->AppendSubMenu(subMenu, _T("Check"));
    mb->Append(menu, _T("&Solution"));

    // Tools Menu
    menu = new wxMenu();
        m_toolMgr.GetTool(ID_TIMER)->Add(menu, menuIconSize, menuIconSize);
    mb->Append(menu, _T("&Tools"));

    return mb;
}



void
MyFrame::SetupManager()
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
    // Add to the frame manager
    wxAuiPaneInfo NoCaption = wxAuiPaneInfo().CaptionVisible(false);

    // Give everything a name so we can save and load the perspective
    m_mgr.AddPane(m_grid,
                  NoCaption
                  .CenterPane()
                  .Caption(_T("Grid"))
                  .Name(_T("Grid")) );

    m_mgr.AddPane(m_acrossPanel,
                  NoCaption
                  .Layer(4)
                  .Left()
                  .Caption(_T("Across"))
                  .Name(_T("Across")) );

    m_mgr.AddPane(m_downPanel,
                  NoCaption
                  .Layer(4)
                  .Left()
                  .Caption(_T("Down"))
                  .Name(_T("Down")) );

    m_mgr.AddPane(m_title,
                  NoCaption
                  .Layer(3)
                  .Top()
                  .Caption(_T("Title"))
                  .Name(_T("Title")) );

    m_mgr.AddPane(m_author,
                  NoCaption
                  .Layer(3)
                  .Top()
                  .Caption(_T("Author"))
                  .Name(_T("Author")) );

    m_mgr.AddPane(m_copyright,
                  NoCaption
                  .Layer(3)
                  .Top()
                  .Caption(_T("Copyright"))
                  .Name(_T("Copyright")) );

    m_mgr.AddPane(m_cluePrompt,
                  NoCaption
                  .Layer(2)
                  .Top()
                  .Caption(_T("Clue"))
                  .Name(_T("Clue")) );

    m_mgr.AddPane(m_notes,
                  NoCaption
                  .Float()
                  .Caption(_T("Notes"))
                  .Name(_T("Notes")));

    m_mgr.AddPane(m_toolbar,
                  NoCaption
                  .Top()
                  .Fixed()
                  .Layer(1)
                  .Caption(_T("Tools"))
                  .Name(_T("Tools")));
}


void
MyFrame::LoadConfig()
{
    m_configFile = wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
    m_configFile << wxFileName::GetPathSeparator() << _T("config.ini");
    
    wxLogDebug(_T("Config file: %s"), m_configFile);

    // Create a blank file it it doesn't exist
    if (! wxFileName::FileExists(m_configFile))
    {
        wxFile(m_configFile, wxFile::write);
    }

    // Setup wxFileConfig
    wxFileInputStream file(m_configFile);
    wxFileConfig::Set( new wxFileConfig(file) );

    wxConfigBase * config = wxFileConfig::Get();

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

    m_grid->SetGridStyle(grid_style);
    if (fit_grid)
        m_toolMgr.Check(ID_ZOOM_FIT);

    config->SetPath(_T("/"));
}




MyFrame::~MyFrame()
{
    // Cleanup
    m_mgr.UnInit();
    wxLogDebug(_T("Frame has been destroyed"));
}


void
MyFrame::SaveConfig()
{
    SavePerspective(_T("Default"));

    // Save window settings
    wxFileConfig * config = dynamic_cast<wxFileConfig*>(wxFileConfig::Get());
    wxASSERT(config != NULL);

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
    config->Write(_T("style"), m_grid->GetGridStyle());
    config->Write(_T("fit"),   m_toolMgr.IsChecked(ID_ZOOM_FIT));

    // Save config file
    wxFileOutputStream file(m_configFile);
    config->Save( file );
}



bool
MyFrame::LoadPuzzle(const wxString & filename, const wxString & ext)
{
    wxStopWatch sw;
    // Just in case someone tries to redraw while we're loading the puzzle
    Freeze();

    m_puz.Load(filename, ext);

    wxLogDebug(_T("Puz Loaded? %s"), m_puz.IsOk() ? _T("True") : _T("False"));

    ShowPuzzle();

    if (m_puz.IsOk())
        SetStatus(wxString::Format(_T("%s   Load time: %d ms"),
                  m_puz.m_filename, sw.Time()));
    Thaw();

    return m_puz.IsOk();
}



bool
MyFrame::SavePuzzle(const wxString & filename, const wxString & ext)
{
    wxStopWatch sw;
    bool success = m_puz.Save(filename, ext);

    // Reset save/save as flag
    m_toolMgr.GetTool(ID_SAVE)->SetLabel(_T("&Save As\tCtrl+S"));

    SetStatus(m_puz.m_filename + wxString::Format(_T("   Save time: %d ms"),
                                                   sw.Time()) );
    return success;
}



void
MyFrame::ShowPuzzle()
{
    // If puzzle loading failed, display a blank puzzle
    if (! m_puz.IsOk())
    {
        m_puz.Clear();
        m_grid->SetGrid(NULL);
    }
    else
        m_grid->SetGrid     (&m_puz.m_grid);

    // Everything else is clear
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
        m_grid->SetSquareFocus(m_grid->GetClueNumber(1), DIR_ACROSS);
        m_across->SetClueNumber(1);//, FOCUSED_CLUE);
        m_down  ->SetClueNumber(1);//, CROSSING_CLUE);
        m_cluePrompt->SetLabel(m_across->GetClueText());
        // Inform user if puzzle is already completed
        CheckPuzzle();
    }
    else
    {
        SetStatus(_T("Error opening file"));
        EnableTools(false);
        m_cluePrompt->SetLabel(wxEmptyString);
    }
    m_grid->Refresh();
}




//--------------------------------------------------------
// Tool and Menu event handlers
//--------------------------------------------------------

// Used to generate file type strings for open and save
inline wxString
MakeTypeString(std::vector<PuzLoader::TypeDesc> types)
{
    wxString typeStr;
    BOOST_FOREACH(PuzLoader::TypeDesc & desc, types)
    {
        typeStr << wxString::Format(_T("%s (*.%s)|*.%s|"),
                                    desc.description,
                                    desc.ext,
                                    desc.ext);
    }

    wxString alltypes;
    BOOST_FOREACH(PuzLoader::TypeDesc & desc, types)
    {
        alltypes << _T("*.") << desc.ext << _T(";");
    }
    alltypes.RemoveLast();

    typeStr << wxString::Format(_T("Supported types (%s)|%s"),
                                alltypes,
                                alltypes)
            << _T("|All Files (*.*)|*.*");

    return typeStr;
}

void
MyFrame::OnOpenPuzzle(wxCommandEvent & WXUNUSED(evt))
{
    wxString filename = wxFileSelector(
                            _T("Open Puzzle"),
                            wxEmptyString, wxEmptyString, _T("puz"),
                            MakeTypeString(PuzLoader::GetLoadTypes()),
                            wxFD_OPEN
                        );

    if (filename.empty())
        return;

    LoadPuzzle(filename);
}




void
MyFrame::OnSavePuzzle(wxCommandEvent & WXUNUSED(evt))
{
    m_puz.m_notes = m_notes->GetValue();
    m_puz.m_time = m_time;
    m_puz.m_complete = (m_grid->GetIncorrectCount() + m_grid->GetBlankCount() == 0);

    wxString filename;
    if (! m_puz.m_modified)
        filename = wxFileSelector(_T("Save Puzzle As"),
                                  wxEmptyString, wxEmptyString, _T("puz"),
                                  MakeTypeString(PuzLoader::GetSaveTypes()),
                                  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    else
        filename = m_puz.m_filename;

    if (filename.empty())
        return;

    SavePuzzle(filename);
}



void
MyFrame::OnClose(wxCloseEvent & evt)
{
    if (! m_puz.m_modified || ! evt.CanVeto())
    {
        Destroy();
        return;
    }

    int ret = wxMessageBox(
                  _T("Current Puzzle not saved.  Save before closing?"),
                  _T("XWord Message"),
                  wxYES_NO | wxCANCEL | wxICON_QUESTION
              );

    if (ret == wxCANCEL)
        evt.Veto();
    else
    {
        if (ret == wxYES)
            SavePuzzle(m_puz.m_filename);
        Destroy();
    }
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

    m_grid->FitGrid(fit);
}

void
MyFrame::OnZoomIn(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(ID_ZOOM_FIT, false);
    m_grid->ZoomIn();
}

void
MyFrame::OnZoomOut(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(ID_ZOOM_FIT, false);
    m_grid->ZoomOut();
}



void
MyFrame::OnLayout(wxCommandEvent & WXUNUSED(evt))
{
    bool allowMove = m_toolMgr.Toggle(ID_LAYOUT_PANES);

    wxAuiPaneInfoArray & panels = m_mgr.GetAllPanes();

    size_t i;
    for (i = 0; i < panels.Count(); ++i) {
        wxAuiPaneInfo & info = panels.Item(i);
        info.Floatable(allowMove).Dockable(allowMove);
        info.CaptionVisible(allowMove);
        info.CloseButton(allowMove);
    }
    m_mgr.Update();
}

void
MyFrame::OnShowNotes(wxCommandEvent & WXUNUSED(evt))
{
    bool show = m_toolMgr.GetTool(ID_SHOW_NOTES)->Toggle();
    ShowPane(_T("Notes"), show);
}


void
MyFrame::OnTimer(wxCommandEvent & WXUNUSED(evt))
{
    bool running = m_toolMgr.GetTool(ID_TIMER)->Toggle();

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



void
MyFrame::OnActivate(wxActivateEvent & evt) {
    // The Activate event can be sent even when IsIconized() returns true.
    // When this happens, wxWindow::SetFocus() fails (probably because the 
    //    window is actually iconized).
    // When Activate is sent and IsIconized() is true, we eat the event
    // Deactivate event seems to work fine.

     // Activate
    if (evt.GetActive())
    {
        wxLogDebug(IsIconized() ?
                      _T("Frame Iconized (Activate from wxWidgets)")
                    : _T("Frame Activated"));

        if (! IsIconized())
        {
            if (m_timerRunning)
                StartTimer();
        }
#ifdef __WXMSW__
        // If this is actually an Iconizing event, eat the event here.
        // This seems to prevent wxWindow::SetFocus() failure.
        else
        {
            return;
        }
#endif
    }
    // Deactivate
    else {
        wxLogDebug(_T("Frame Deactivated"));
        m_timerRunning = m_timer.IsRunning();
        StopTimer();
    }

    evt.Skip();
}



void
MyFrame::OnSavePerspective(wxCommandEvent & WXUNUSED(evt))
{
    wxString name = wxGetTextFromUser( _T("Enter a name for this perspective"),
                                       _T("Save Perspective") );

    if (name.empty())
        return;
    SavePerspective(name);
}


void
MyFrame::OnLoadPerspective(wxCommandEvent & WXUNUSED(evt))
{
    wxConfigBase * config = wxFileConfig::Get();
    wxArrayString arrayStr;

    // dummy enumeration variables
    wxString str;
    long dummy;

    config->SetPath(_T("/Perspectives"));
    bool bCont = config->GetFirstEntry(str, dummy);
    while (bCont)
    {
        arrayStr.Add(str);
        bCont = config->GetNextEntry(str, dummy);
    }
    config->SetPath(_T("/"));


    if (arrayStr.empty())
    {
        wxMessageBox(_T("No perspectives found"),
                     _T("XWord Error"),
                     wxICON_EXCLAMATION | wxOK);
        return;
    }
    else
    {
        wxString oldPerspective = m_mgr.SavePerspective();
        PerspectiveDialog dlg(this,
                              _T("Choose a perspective"),
                              _T("Load Perspective"),
                              arrayStr);

        if (dlg.ShowModal() != wxID_OK)
            LoadPerspectiveString(oldPerspective, true);
    }
}



//--------------------------------------------------------
// Crossword controls event handlers
//--------------------------------------------------------

void
MyFrame::OnGridFocus(wxPuzEvent & evt)
{
    // Update everything
    // If there is a performance bottleneck anywhere, it is in SetClueNumber
    // However, that bottleneck is in VListBox::SetSelection

    m_across->SetClueNumber(evt.GetAcrossClue());//, evt.GetDirection() == DIR_ACROSS);
    m_down  ->SetClueNumber(evt.GetDownClue());//,   evt.GetDirection() == DIR_DOWN);
    m_cluePrompt->SetLabel (evt.GetDirection() == DIR_ACROSS
                          ? m_across->GetClueText()
                          : m_down  ->GetClueText());
    //m_cluePrompt->Update(); // We shouldn't really need this
}


void
MyFrame::OnClueFocus(wxPuzEvent & evt)
{
    m_grid->SetFocusedClue(evt.GetClueNumber(), evt.GetDirection());
    m_cluePrompt->SetLabel(evt.GetClueText());

    int crossingClue = m_grid->GetFocusedSquare()->WordStart(! evt.GetDirection())->number;
    if (evt.GetDirection() == DIR_ACROSS)
        m_down->SetClueNumber(crossingClue);//, CROSSING_CLUE);
    else
        m_across->SetClueNumber(crossingClue);//, CROSSING_CLUE);
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


void
MyFrame::CheckPuzzle()
{
    // Check to see if the puzzle is filled
    if (m_grid->GetBlankCount() == 0)
    {
        if (m_grid->GetIncorrectCount() == 0)
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
    else
        m_status->SetAlert(_T(""));
}