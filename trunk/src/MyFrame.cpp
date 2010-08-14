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
#include "paths.hpp"
#include "messages.hpp"

// For the global printing pointers and application activation / timer starting
// and stopping.
#include "App.hpp"
#include <wx/print.h>
#include <wx/printdlg.h>
#include "printout.hpp"

// Puz library
#include "PuzEvent.hpp"
#include "puz/Scrambler.hpp"
#include "puz/exceptions.hpp"
#include "puz/Puzzle.hpp"

// Dialogs
#include "dialogs/Layout.hpp"
#include "dialogs/Preferences.hpp"
#include "dialogs/Characters.hpp"
#include "dialogs/wxFB_Dialogs.h"
#include <wx/aboutdlg.h>


// Windows
#include "widgets/SizedText.hpp"
#include "ClueListBox.hpp"
#include "CluePanel.hpp"
#include "CluePrompt.hpp"
#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"

#ifdef XWORD_USE_LUA
#   include "xwordlua.hpp"
#endif // XWORD_USE_LUA

// For the scrambling dialogs
#include <wx/numdlg.h>

#include "utils/SizerPrinter.hpp"

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "../images/xword.xpm"
#endif


//------------------------------------------------------------------------------
// Menu and Toolbar IDs
//------------------------------------------------------------------------------

enum toolIds
{
    ID_DUMMY_FIRST = wxID_HIGHEST,
    //wxID_OPEN,
    //wxID_SAVE,
    //wxID_SAVEAS,
    //wxID_CLOSE,

    //wxID_EXIT,

    //wxID_ZOOM_IN,
    //wxID_ZOOM_FIT,
    //wxID_ZOOM_OUT,

    ID_CHECK_LETTER,
    ID_CHECK_WORD,
    ID_CHECK_SELECTION,
    ID_CHECK_GRID,
    ID_REVEAL_LETTER,
    ID_REVEAL_WORD,
    ID_REVEAL_INCORRECT,
    ID_REVEAL_INCORRECT_SELECTION,
    ID_REVEAL_SELECTION,
    ID_REVEAL_GRID,


    ID_SCRAMBLE,
    ID_UNSCRAMBLE,

    ID_LAYOUT_PANES,
    ID_LOAD_LAYOUT,
    ID_SAVE_LAYOUT,

    ID_SHOW_NOTES,

    ID_TIMER,
    ID_CHARACTER_MAP,

    //wxID_PREFERENCES,

    //wxID_PREVIEW,
    ID_PAGE_SETUP,
    //wxID_PRINT,

    //wxID_ABOUT,
    ID_LICENSE,

#ifdef XWORD_USE_LUA
    ID_LUA_SCRIPT,
#endif // XWORD_USE_LUA

#ifdef __WXDEBUG__

    ID_DUMP_STATUS,
    ID_DUMP_LAYOUT,
    ID_FORCE_UNSCRAMBLE,
#endif // __WXDEBUG__
};



BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_TIMER          (wxID_ANY,             MyFrame::OnTimerNotify)

#ifdef XWORD_USE_LUA
    EVT_LUA_PRINT      (wxID_ANY,             MyFrame::OnLuaPrint)
    EVT_LUA_ERROR      (wxID_ANY,             MyFrame::OnLuaError)
#endif // XWORD_USE_LUA

    EVT_PUZ_GRID_FOCUS (                      MyFrame::OnGridFocus)
    EVT_PUZ_CLUE_FOCUS (                      MyFrame::OnClueFocus)
    EVT_PUZ_LETTER     (                      MyFrame::OnGridLetter)

    EVT_ACTIVATE       (                      MyFrame::OnActivate)
    EVT_CLOSE          (                      MyFrame::OnClose)

    EVT_AUI_PANE_CLOSE (                      MyFrame::OnPaneClose)

    EVT_UPDATE_UI      (ID_SHOW_NOTES,        MyFrame::OnUpdateUI)
    EVT_UPDATE_UI      (ID_CHARACTER_MAP,     MyFrame::OnUpdateUI)

END_EVENT_TABLE()

// This function is here instead of with the rest of the MyFrame implementation
// because it seems to fit better with the other tables (tool id enum, event table).
// The table needs to be in a function because it references protected functions (all of the
// event handlers).

// wxCommandEventHandler is long and pollutes the tool table
#define _handler wxCommandEventHandler

void
MyFrame::ManageTools()
{
    const ToolDesc tools [] =
    {
        { wxID_OPEN,       wxITEM_NORMAL, _T("&Open\tCtrl+O"), _T("open"), NULL,
                     _handler(MyFrame::OnOpenPuzzle) },

        { wxID_SAVE,        wxITEM_NORMAL, _T("&Save\tCtrl+S"), _T("save"), NULL,
                     _handler(MyFrame::OnSavePuzzle) },

        { wxID_SAVEAS,      wxITEM_NORMAL, _T("&Save As..."),   _T("save"), NULL,
                     _handler(MyFrame::OnSavePuzzleAs) },

        { wxID_CLOSE,       wxITEM_NORMAL, _T("&Close\tCtrl+W"), NULL, NULL,
                     _handler(MyFrame::OnClosePuzzle) },

        { wxID_PREFERENCES, wxITEM_NORMAL, _T("Preferences..."), NULL, NULL,
                     _handler(MyFrame::OnPreferences) },

        { ID_PAGE_SETUP,    wxITEM_NORMAL, _T("Page Setup..."), NULL, NULL,
                     _handler(MyFrame::OnPageSetup) },

        { wxID_PREVIEW,     wxITEM_NORMAL, _T("Print Preview"), NULL, NULL,
                     _handler(MyFrame::OnPrintPreview) },

        { wxID_PRINT,       wxITEM_NORMAL, _T("Print..."), NULL, NULL,
                     _handler(MyFrame::OnPrint) },

        { wxID_EXIT,        wxITEM_NORMAL, _T("&Quit\tCtrl+Q"), NULL, NULL,
                     _handler(MyFrame::OnQuit) },


        { wxID_ZOOM_IN,  wxITEM_NORMAL, _T("Zoom &In\tCtrl+="),  _T("zoom_in"), NULL,
                    _handler(MyFrame::OnZoomIn) },

        { wxID_ZOOM_FIT, wxITEM_CHECK,  _T("Zoom &Fit\tCtrl+0"), _T("zoom_fit"), NULL,
                    _handler(MyFrame::OnZoomFit) },

        { wxID_ZOOM_OUT, wxITEM_NORMAL, _T("Zoom &Out\tCtrl+-"), _T("zoom_out"), NULL,
                    _handler(MyFrame::OnZoomOut) },


        { ID_SCRAMBLE,   wxITEM_NORMAL, _T("&Scramble..."), NULL, NULL,
                    _handler(MyFrame::OnScramble) },

        { ID_UNSCRAMBLE, wxITEM_NORMAL, _T("&Unscramble..."), NULL, NULL,
                    _handler(MyFrame::OnUnscramble) },

        { ID_CHECK_LETTER,     wxITEM_NORMAL, _T("Check &Letter\tCtrl+Shift+L"), _T("check_letter"), NULL,
                    _handler(MyFrame::OnCheckLetter) },

        { ID_CHECK_WORD,       wxITEM_NORMAL, _T("Check &Word\tCtrl+Shift+W"),   _T("check_word"), NULL,
                    _handler(MyFrame::OnCheckWord) },

        { ID_CHECK_SELECTION,  wxITEM_NORMAL, _T("Check &Selection..."),    NULL, NULL,
                   _handler(MyFrame::OnCheckSelection) },

        { ID_CHECK_GRID,       wxITEM_NORMAL, _T("Check &Grid\tCtrl+Shift+G"),    _T("check_grid"), NULL,
                   _handler(MyFrame::OnCheckGrid) },

        { ID_REVEAL_LETTER,    wxITEM_NORMAL, _T("Reveal &Letter"), NULL, NULL,
                   _handler(MyFrame::OnRevealLetter) },

        { ID_REVEAL_WORD,      wxITEM_NORMAL, _T("Reveal &Word"), NULL, NULL,
                   _handler(MyFrame::OnRevealWord) },

        { ID_REVEAL_INCORRECT, wxITEM_NORMAL, _T("Reveal &Incorrect letters"), NULL, NULL,
                   _handler(MyFrame::OnRevealIncorrect) },

        { ID_REVEAL_INCORRECT_SELECTION, wxITEM_NORMAL, _T("Reveal Incorrect letters (selection)..."), NULL, NULL,
                   _handler(MyFrame::OnRevealIncorrectSelection) },

        { ID_REVEAL_SELECTION,      wxITEM_NORMAL, _T("Reveal &Selection..."), NULL, NULL,
                   _handler(MyFrame::OnRevealSelection) },

        { ID_REVEAL_GRID,      wxITEM_NORMAL, _T("Reveal &Grid"), NULL, NULL,
                   _handler(MyFrame::OnRevealGrid) },


        { ID_LAYOUT_PANES, wxITEM_CHECK,  _T("&Edit Layout"), _T("layout"), NULL,
                   _handler(MyFrame::OnEditLayout) },

        { ID_LOAD_LAYOUT,  wxITEM_NORMAL, _T("&Load Layout"), NULL, NULL,
                   _handler(MyFrame::OnLoadLayout) },

        { ID_SAVE_LAYOUT,  wxITEM_NORMAL, _T("&Save Layout"), NULL, NULL,
                   _handler(MyFrame::OnSaveLayout) },

        { ID_SHOW_NOTES,   wxITEM_CHECK,  _T("&Notes\tCtrl+N"), _T("notes"), NULL,
                   _handler(MyFrame::OnShowNotes) },


        { ID_TIMER,          wxITEM_CHECK, _T("&Timer\tCtrl+T"), _T("timer"), NULL,
                   _handler(MyFrame::OnTimer) },

        { ID_CHARACTER_MAP,  wxITEM_CHECK, _T("&Character Map\tCtrl+M"), NULL, NULL,
                   _handler(MyFrame::OnCharacterMap) },


#ifdef XWORD_USE_LUA
        { ID_LUA_SCRIPT, wxITEM_NORMAL, _T("Run &Script"), NULL, NULL,
                   _handler(MyFrame::OnLuaScript) },
#endif // XWORD_USE_LUA

        { wxID_ABOUT, wxITEM_NORMAL, _T("&About ") XWORD_APP_NAME _T("..."), NULL, NULL,
                   _handler(MyFrame::OnAbout) },

        { ID_LICENSE, wxITEM_NORMAL, _T("&License..."), NULL, NULL,
                   _handler(MyFrame::OnLicense) },

#ifdef __WXDEBUG__
        { ID_DUMP_STATUS,   wxITEM_NORMAL, _T("Dump status"), NULL, NULL,
                   _handler(MyFrame::OnDumpStatus) },

        { ID_DUMP_LAYOUT,   wxITEM_NORMAL, _T("Dump layout"), NULL, NULL,
                   _handler(MyFrame::OnDumpLayout) },

        { ID_FORCE_UNSCRAMBLE, wxITEM_NORMAL, _T("Brute force unscramble"), NULL, NULL,
                   _handler(MyFrame::OnBruteForceUnscramble) },


#endif // __WXDEBUG__

        { TOOL_NONE }
    };

    m_toolMgr.SetDesc(tools);
    m_toolMgr.SetManagedWindow(this);
    m_toolMgr.ConnectEvents();
}

#undef _handler






#include <wx/dnd.h>

// Drop target for the main frame
//-------------------------------
class XWordFileDropTarget : public wxFileDropTarget
{
public:
    XWordFileDropTarget(MyFrame * frame)
        : m_frame(frame)
    {}

    virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxArrayString & filenames)
    {
#if XWORD_USE_LUA
        // Run the file as a script if it ends with .lua
        if (filenames.Item(0).EndsWith(_T(".lua")))
            m_frame->RunLuaScript(filenames.Item(0));
        // Otherwise try to open as a puzzle
        else
#endif
            m_frame->LoadPuzzle(filenames.Item(0));
        return true;
    }

private:
    MyFrame * m_frame;
};


//------------------------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------------------------

MyFrame::MyFrame()
    : wxFrame(NULL, -1, XWORD_APP_NAME, wxDefaultPosition, wxSize(700,700)),
      m_timer(this),
      m_isTimerRunning(false),
      m_preferencesDialog(NULL),
      m_charactersPanel(NULL),
      m_mgr(this),
      m_isIdleConnected(false)
{
#ifdef __WXDEBUG__
    // Debug window
    wxTextCtrl * logctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxTE_MULTILINE);
    new wxLogChain(new wxLogTextCtrl(logctrl));
    m_mgr.AddPane(logctrl,
                  wxAuiPaneInfo()
                  .BestSize(350, 300)
                  .PinButton()
                  .CloseButton()
                  .Layer(900)
                  .Left()
                  .Caption(_T("Logger"))
                  .Name(_T("Logger")) );
#endif // __WXDEBUG__

    // Set the initial timer amount
    m_timer.Start(1000);
    m_timer.Stop();

    wxLogDebug(_T("Creating Frame"));
    wxImage::AddHandler(new wxPNGHandler());

    SetDropTarget(new XWordFileDropTarget(this));

    SetupToolManager();  // This will check for image files
    ManageTools();

    CreateWindows();

    SetupWindowManager();
    ManageWindows();

    LoadConfig();

    LoadLayout(_T("(Previous)"));
    m_mgr.Update();

    // Check to see if we know about some windows.
    if (m_mgr.HasCachedPane(_T("Characters")))
        ShowCharacterMap();

#if defined(__WXMSW__) && !defined(__WXPM__)
    SetIcon(wxIcon(_T("aa_main_icon")));
#else
    SetIcon(wxIcon(xword_xpm));
#endif // __WXMSW__ && ! __WXPM__

#ifdef XWORD_USE_LUA
    LuaInit();
#endif // XWORD_USE_LUA

    ShowPuzzle();
}


MyFrame::~MyFrame()
{
    // Let the App know we've been destroyed
    wxGetApp().m_frame = NULL;

    // Cleanup
    m_toolMgr.DisconnectEvents();
    m_toolMgr.UnInit();
    m_mgr.UnInit();
    wxLogDebug(_T("Frame has been destroyed"));
}



//------------------------------------------------------------------------------
// XWord puzzle loading / saving
//------------------------------------------------------------------------------

wxString
MyFrame::GetLoadTypeString()
{
    return _T("Across Lite Format (*.puz)|*.puz")
           _T("|")
           _T("Plain Text Format (*.txt)|*.txt")
           _T("|")
           _T("Supported Types (*.puz;*.txt)|*.puz;*.txt")
           _T("|")
           _T("All Files (*.*)|*.*");
}

wxString
MyFrame::GetSaveTypeString()
{
    return _T("Across Lite Format (*.puz)|*.puz");
}


bool
MyFrame::LoadPuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
{
    if ( ! ClosePuzzle(true) ) // Prompt for save
        return false;

    wxStopWatch sw;

    try
    {
        m_isModified = false;
        m_puz.Load(wx2puz(filename), handler);
    }
    catch (puz::ChecksumError &)
    {
        m_puz.SetOk(XWordPrompt(MSG_CORRUPT_PUZ));
    }
    catch (...)
    {
        // We can't recover from any other exception.
        m_puz.SetOk(false);
        HandlePuzException();
    }

    ShowPuzzle();

    m_filename = filename;

    if (m_puz.IsOk())
    {
        SetStatus(wxString::Format(_T("%s   Load time: %d ms"),
                                   m_filename.c_str(),
                                   sw.Time()));
        m_XGridCtrl->SetFocus();
    }
    else
        SetStatus(_T("No file loaded"));

    return m_puz.IsOk();
}


bool
MyFrame::SavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
{
    m_puz.m_notes = wx2puz(m_notes->GetValue());
    m_puz.m_time = m_time;
    m_puz.m_isTimerRunning = IsTimerRunning();

    wxString fn = filename;
    if (fn.empty())
        fn = wxFileSelector(
                 _T("Save Puzzle As"),
                 wxEmptyString, wxEmptyString, _T("puz"),
                 GetSaveTypeString(),
                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT
            );

    if (fn.empty())
        return false;

    wxStopWatch sw;

    try
    {
        m_puz.Save(wx2puz(fn), handler);
        m_filename = fn;
        m_isModified = false;

        // Reset save/save as flag
        EnableSaveAs();

        SetStatus(wxString::Format(_T("%s   Save time: %d ms"),
                                   m_filename.c_str(),
                                   sw.Time()));
        return true;
    }
    catch (...)
    {
        HandlePuzException();
        return false;
    }
}

// Catch everything that wasn't already caught.
void
MyFrame::HandlePuzException()
{
    try
    {
        throw;
    }
    catch (puz::Exception & err)
    {
        XWordMessage(MSG_PUZ_ERROR, puz2wx(err.what()).c_str());
    }
    catch (std::exception & err)
    {

        XWordMessage(MSG_STD_EXCEPTION, err.what());
    }
    catch (...)
    {
        XWordMessage(MSG_UNKNOWN_ERROR);
    }
}


bool
MyFrame::ClosePuzzle(bool prompt)
{
    // No puzzle is open
    if (! m_puz.IsOk())
        return true;

    if (prompt && m_isModified)
    {
        int ret = XWordMessage(MSG_SAVE_PUZ);

        if (ret == wxCANCEL)
            return false;

        if (ret == wxYES)
            SavePuzzle(m_filename);
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
        m_puz.Clear();

    // Enable / disable tools
    EnableSaveAs();
    if (m_puz.IsOk())
    {
        SetStatus(m_filename);
        EnableTools(true);
    }
    else
    {
        EnableTools(false);
        m_cluePrompt->Clear();
    }

    // Update the GUI
    ShowClues();
    ShowAuthor();
    ShowTitle();
    ShowCopyright();
    ShowNotes();
    ShowGrid();

    // Timer
    StopTimer();
    SetTime(m_puz.m_time);
    if (m_puz.m_isTimerRunning)
        StartTimer();
}

void
MyFrame::ShowGrid()
{
    if (! m_puz.IsOk())
        m_XGridCtrl->SetGrid(NULL);
    else
    {
        // Make sure the grid has clue numbers assigned, etc.
        m_puz.m_grid.SetupGrid();

        m_XGridCtrl->SetGrid(&m_puz.m_grid);

        const bool scrambled = m_puz.IsScrambled();
        // Enable / disable scrambling tools
        m_toolMgr.Enable(ID_SCRAMBLE,   ! scrambled);
        m_toolMgr.Enable(ID_UNSCRAMBLE, scrambled);
        EnableCheck(! scrambled);
        EnableReveal(! scrambled);

        m_XGridCtrl->SetFocusedClue(1, puz::ACROSS);

        // Inform user if puzzle is already completed
        CheckPuzzle();
    }

    m_XGridCtrl->SetPaused(false);
    m_XGridCtrl->Refresh();
}


void
MyFrame::ShowClues()
{
    m_across->SetClueList(m_puz.m_across);
    m_down  ->SetClueList(m_puz.m_down);
}


void
MyFrame::ShowTitle()
{
    if (! m_puz.IsOk())
    {
        SetTitle(XWORD_APP_NAME);
        m_title->SetLabel(_T(""));
        m_title->SetToolTip(_T(""));
    }
    else
    {
        SetTitle(puz2wx(m_puz.m_title) + _T(" - ") XWORD_APP_NAME);
        m_title->SetLabel(puz2wx(m_puz.m_title));
        m_title->SetToolTip(puz2wx(m_puz.m_title));
    }
}

void
MyFrame::ShowAuthor()
{
    m_author->SetLabel(puz2wx(m_puz.m_author));
    m_author->SetToolTip(puz2wx(m_puz.m_author));
}

void
MyFrame::ShowCopyright()
{
    m_copyright->SetLabel(puz2wx(m_puz.m_copyright));
    m_copyright->SetToolTip(puz2wx(m_puz.m_copyright));
}


void
MyFrame::ShowNotes()
{
    m_notes->ChangeValue(puz2wx(m_puz.m_notes));
    // Set the notes bitmap depending on whether there are notes or not
    if (m_puz.m_notes.empty())
        m_toolMgr.SetIconName(ID_SHOW_NOTES, _T("notes"));
    else
        m_toolMgr.SetIconName(ID_SHOW_NOTES, _T("notes_new"));

}

void
MyFrame::CheckPuzzle()
{
    if (m_XGridCtrl->GetBlankCount() == 0)
    {
        if (m_XGridCtrl->IsCorrect())
        {
            StopTimer();
            m_status->SetAlert(_T("The puzzle is filled correctly!"),
                              *wxWHITE, *wxGREEN);
        }
        else
        {
            m_status->SetAlert(
                _T("The puzzle is completely filled, ")
                _T("but some letters are incorrect."),
                *wxWHITE, *wxRED);
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
    m_XGridCtrl = new XGridCtrl(this);

    m_across = new CluePanel(this, wxID_ANY, _T("Across"), puz::ACROSS);
    m_down   = new CluePanel(this, wxID_ANY, _T("Down"),   puz::DOWN);

    m_title      = new SizedText (this, wxID_ANY);
    m_author     = new SizedText (this, wxID_ANY);
    m_copyright  = new SizedText (this, wxID_ANY);
    m_cluePrompt = new CluePrompt (this, wxID_ANY);
    m_cluePrompt->SetAlign(wxALIGN_CENTER);

    m_notes      = new wxTextCtrl(this, wxID_ANY,
                                  wxEmptyString,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  wxTE_MULTILINE);

    if (m_toolMgr.GetIconLocation() != wxEmptyString)
    {
#ifdef USE_AUI_TOOLBAR
        m_toolbar = CreateAuiToolBar();
#else // ! USE_AUI_TOOLBAR
        m_toolbar = CreateToolBar();
        SetToolBar(m_toolbar);
#endif // USE_AUI_TOOLBAR / !
    }
    else
    {
        m_toolbar = NULL;
    }

    m_menubar = CreateMenuBar();
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
MyFrame::CreateAuiToolBar()
{
    wxAuiToolBar * tb = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
                                         wxDefaultSize,
                                         wxAUI_TB_HORZ_TEXT);

    tb->GetArtProvider()->SetTextOrientation(wxAUI_TBTOOL_TEXT_BOTTOM);

#else // ! USE_AUI_TOOLBAR
wxToolBar *
MyFrame::CreateToolBar()
{
    wxToolBar * tb = new wxToolBar(this, wxID_ANY,
                                   wxDefaultPosition,
                                   wxDefaultSize,
                                   wxBORDER_NONE | wxTB_HORIZONTAL | wxTB_FLAT);

    tb->SetToolBitmapSize( wxSize(m_toolMgr.GetIconSize_ToolBar(),
                                  m_toolMgr.GetIconSize_ToolBar()) );

#endif // USE_AUI_TOOLBAR / !

    m_toolMgr.Add(tb, wxID_OPEN);
    m_toolMgr.Add(tb, wxID_SAVE);
    tb->AddSeparator();
    m_toolMgr.Add(tb, wxID_ZOOM_IN);
    m_toolMgr.Add(tb, wxID_ZOOM_FIT);
    m_toolMgr.Add(tb, wxID_ZOOM_OUT);
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
MyFrame::CreateMenuBar()
{
    wxMenuBar * mb = new wxMenuBar();

    // File Menu
    wxMenu * menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_OPEN);
        m_toolMgr.Add(menu, wxID_SAVE);
        m_toolMgr.Add(menu, wxID_SAVEAS);
        m_toolMgr.Add(menu, wxID_CLOSE);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, wxID_PREFERENCES);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_PAGE_SETUP);
        m_toolMgr.Add(menu, wxID_PREVIEW);
        m_toolMgr.Add(menu, wxID_PRINT);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, wxID_EXIT);
    mb->Append(menu, _T("&File"));

    menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_ZOOM_IN);
        m_toolMgr.Add(menu, wxID_ZOOM_FIT);
        m_toolMgr.Add(menu, wxID_ZOOM_OUT);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_SHOW_NOTES);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_LAYOUT_PANES);
        m_toolMgr.Add(menu, ID_LOAD_LAYOUT);
        m_toolMgr.Add(menu, ID_SAVE_LAYOUT);
        wxMenu * paneMenu = new wxMenu();
        m_mgr.SetManagedMenu(paneMenu);
        menu->AppendSubMenu(paneMenu, _T("Panes"));
    mb->Append(menu, _T("&View"));

    // Solution Menu
    menu = new wxMenu();
        wxMenu * subMenu = new wxMenu();
            m_toolMgr.Add(subMenu, ID_CHECK_LETTER);
            m_toolMgr.Add(subMenu, ID_CHECK_WORD);
            m_toolMgr.Add(subMenu, ID_CHECK_SELECTION);
            m_toolMgr.Add(subMenu, ID_CHECK_GRID);
        menu->AppendSubMenu(subMenu, _T("&Check"));
        subMenu = new wxMenu();
            m_toolMgr.Add(subMenu, ID_REVEAL_LETTER);
            m_toolMgr.Add(subMenu, ID_REVEAL_WORD);
            m_toolMgr.Add(subMenu, ID_REVEAL_INCORRECT);
            m_toolMgr.Add(subMenu, ID_REVEAL_INCORRECT_SELECTION);
            subMenu->AppendSeparator();
            m_toolMgr.Add(subMenu, ID_REVEAL_SELECTION);
            m_toolMgr.Add(subMenu, ID_REVEAL_GRID);
        menu->AppendSubMenu(subMenu, _T("&Reveal"));
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_SCRAMBLE);
        m_toolMgr.Add(menu, ID_UNSCRAMBLE);
    mb->Append(menu, _T("&Solution"));

    // Tools Menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_TIMER);
        m_toolMgr.Add(menu, ID_CHARACTER_MAP);
#ifdef XWORD_USE_LUA
        m_toolMgr.Add(menu, ID_LUA_SCRIPT);
#endif // XWORD_USE_LUA
    mb->Append(menu, _T("&Tools"));

    // Help Menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_ABOUT);
        m_toolMgr.Add(menu, ID_LICENSE);
    mb->Append(menu, _T("&Help"));

#ifdef __WXDEBUG__

    // Debug menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_DUMP_STATUS);
        m_toolMgr.Add(menu, ID_DUMP_LAYOUT);
        m_toolMgr.Add(menu, ID_FORCE_UNSCRAMBLE);
    mb->Append(menu, _T("&Debug"));

#endif // __WXDEBUG__

    return mb;
}



//------------------------------------------------------------------------------
// Window management
//------------------------------------------------------------------------------

void
MyFrame::SetupWindowManager()
{
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


// This is an external entry point for adding Aui panes (i.e. through lua)
void
MyFrame::AddPane(wxWindow * window, const wxAuiPaneInfo & info)
{
    // The default wxAuiPaneInfo is good, except all panes should
    // have a pin button.
    wxAuiPaneInfo the_info = info;
    the_info.PinButton();
    m_mgr.AddPane(window, the_info);
    m_mgr.Update();
}

void
MyFrame::OnPaneClose(wxAuiManagerEvent & evt)
{
    // Pass pane close to the child window
    wxAuiPaneInfo * pane = evt.GetPane();
    if (pane && pane->window)
        pane->window->GetEventHandler()->ProcessEvent(evt);
    evt.Skip();
}

void
MyFrame::ManageWindows()
{
    // The default wxAuiPaneInfo.
    // ** We have to "declare" all the buttons we want to use here
    // or they won't ever show up!
    wxAuiPaneInfo baseInfo;
    baseInfo.CaptionVisible(false)
            .PinButton()
            .CloseButton()
            .MinSize(15,15);

    // Give everything a name so we can save and load the layout
    m_mgr.AddPane(m_XGridCtrl,
                  wxAuiPaneInfo(baseInfo)
                  .CenterPane()
                  .Caption(_T("Grid"))
                  .Name(_T("Grid")) );

#ifdef USE_AUI_TOOLBAR
    // It would be better to use MinSize, instead of Fixed, but it doesn't work
    // quite right, and we would have to update the MinSize every time the
    // toolbar was resized . . .
    if (m_toolbar != NULL)
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
                  wxAuiPaneInfo(baseInfo)
                  .BestSize(300,-1)
                  .Layer(4)
                  .Left()
                  .Caption(_T("Across"))
                  .Name(_T("Across")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_across), m_across->m_heading);

    m_mgr.AddPane(m_down,
                  wxAuiPaneInfo(baseInfo)
                  .BestSize(300,-1)
                  .Layer(4)
                  .Left()
                  .Caption(_T("Down"))
                  .Name(_T("Down")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_down), m_down->m_heading);

    m_mgr.AddPane(m_title,
                  wxAuiPaneInfo(baseInfo)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Title"))
                  .Name(_T("Title")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_title), m_title);

    m_mgr.AddPane(m_author,
                  wxAuiPaneInfo(baseInfo)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Author"))
                  .Name(_T("Author")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_author), m_author);

    m_mgr.AddPane(m_copyright,
                  wxAuiPaneInfo(baseInfo)
                  .Layer(3)
                  .Top()
                  .Caption(_T("Copyright"))
                  .Name(_T("Copyright")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_copyright), m_copyright);

    m_mgr.AddPane(m_cluePrompt,
                  wxAuiPaneInfo(baseInfo)
                  .BestSize(-1, 75)
                  .Layer(2)
                  .Top()
                  .Caption(_T("Clue Prompt"))
                  .Name(_T("Clue")) );
    m_mgr.SetContextWindow(m_mgr.GetPane(m_cluePrompt), m_cluePrompt);

    m_mgr.AddPane(m_notes,
                  wxAuiPaneInfo(baseInfo)
                  .CaptionVisible(true)
                  .Float()
                  .FloatingSize(250,250)
                  .Hide()
                  .Caption(_T("Notes"))
                  .Name(_T("Notes")));

    SaveLayout(_T("XWord Default"));
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
    wxString imagesdir = GetImagesDir();
    if (wxDirExists(imagesdir))
    {
        m_toolMgr.SetIconLocation(imagesdir);
    }
    else
    {
        XWordErrorMessage(_T("Cannot find images directory:\n%s"), imagesdir.c_str());
        m_toolMgr.SetIconLocation(_T(""));
    }
}



void
MyFrame::EnableTools(bool enable)
{
    EnableGridSize(enable);
    EnableCheck(enable);
    EnableReveal(enable);

    // Tools that are only enabled or disabled when a puzzle
    // is shown or closed.  These don't have any special logic.
    m_toolMgr.Enable(ID_UNSCRAMBLE, enable);
    m_toolMgr.Enable(ID_SCRAMBLE,   enable);
    m_toolMgr.Enable(wxID_SAVE,    enable);
    m_toolMgr.Enable(wxID_SAVEAS, enable);
    m_toolMgr.Enable(wxID_CLOSE, enable);
    m_toolMgr.Enable(ID_TIMER, enable);
    m_toolMgr.Enable(wxID_PREVIEW, enable);
    m_toolMgr.Enable(wxID_PRINT, enable);
}


void
MyFrame::EnableSave(bool enable)
{
    m_toolMgr.Enable(wxID_SAVE,         enable);

    if (m_toolbar == NULL)
        return;

    // Swap the toolbar icons
    if (enable)
    {
        const int pos = m_toolbar->GetToolPos(wxID_SAVEAS);
        if (pos != wxNOT_FOUND)
        {
            m_toolMgr.Delete(m_toolbar, wxID_SAVEAS);
            m_toolMgr.Insert(m_toolbar, wxID_SAVE, pos);
        }
        wxASSERT(  m_toolMgr.IsAttached(wxID_SAVE, m_toolbar) &&
                 ! m_toolMgr.IsAttached(wxID_SAVEAS, m_toolbar) );
    }
    else // disable save (enable save as)
    {
        const int pos = m_toolbar->GetToolPos(wxID_SAVE);
        if (pos != wxNOT_FOUND)
        {
            m_toolMgr.Delete(m_toolbar, wxID_SAVE);
            m_toolMgr.Insert(m_toolbar, wxID_SAVEAS, pos);
        }
        wxASSERT(! m_toolMgr.IsAttached(wxID_SAVE, m_toolbar) &&
                   m_toolMgr.IsAttached(wxID_SAVEAS, m_toolbar) );
    }
    // Have to call Realize() since we have inserted a tool.
    m_toolbar->Realize();
}

void
MyFrame::EnableGridSize(bool enable)
{
    m_toolMgr.Enable(wxID_ZOOM_IN,      enable);
    m_toolMgr.Enable(wxID_ZOOM_OUT,     enable);
    m_toolMgr.Enable(wxID_ZOOM_FIT,     enable);
}


void
MyFrame::EnableCheck(bool enable)
{
    m_toolMgr.Enable(ID_CHECK_LETTER,    enable);
    m_toolMgr.Enable(ID_CHECK_WORD,      enable);
    m_toolMgr.Enable(ID_CHECK_SELECTION, enable);
    m_toolMgr.Enable(ID_CHECK_GRID,      enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("Solution"), _T("Check")),
                      enable);

}

void
MyFrame::EnableReveal(bool enable)
{
    m_toolMgr.Enable(ID_REVEAL_LETTER,    enable);
    m_toolMgr.Enable(ID_REVEAL_WORD,      enable);
    m_toolMgr.Enable(ID_REVEAL_INCORRECT, enable);
    m_toolMgr.Enable(ID_REVEAL_INCORRECT_SELECTION, enable);
    m_toolMgr.Enable(ID_REVEAL_SELECTION, enable);
    m_toolMgr.Enable(ID_REVEAL_GRID,      enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("Solution"),_T("Reveal")),
                      enable);
}




//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------

void
MyFrame::LoadConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();
    // Window size and position
    //-------------------------
    config.SetPath(_T("/Window"));
    if (config.ReadBool(_T("maximized")))
    {
        Maximize();
    }
    else
    {
        SetSize( config.ReadLong(_T("left")),
                 config.ReadLong(_T("top")),
                 config.ReadLong(_T("width")),
                 config.ReadLong(_T("height")) );
    }


    // Grid
    //-----
    config.SetPath(_T("/Grid"));

    if (config.ReadBool(_T("fit")))
    {
        m_toolMgr.Check(wxID_ZOOM_FIT);
        m_XGridCtrl->FitGrid();
    }
    m_XGridCtrl->SetGridStyle(config.ReadLong(_T("style")) );

    m_XGridCtrl->SetLetterFont(config.ReadFont(_T("letterFont")) );
    m_XGridCtrl->SetNumberFont(config.ReadFont(_T("numberFont")) );

    m_XGridCtrl->SetBorderSize(config.ReadLong(_T("lineThickness")));

    m_XGridCtrl->SetFocusedLetterColor(
        config.ReadColor(_T("focusedLetterColor")) );
    m_XGridCtrl->SetFocusedWordColor(
        config.ReadColor(_T("focusedWordColor")) );
    m_XGridCtrl->SetBackgroundColour(
        config.ReadColor(_T("backgroundColor")) );
    m_XGridCtrl->SetWhiteSquareColor(
        config.ReadColor(_T("whiteSquareColor")) );
    m_XGridCtrl->SetBlackSquareColor(
        config.ReadColor(_T("blackSquareColor")) );
    m_XGridCtrl->SetSelectionColor(
        config.ReadColor(_T("selectionColor")) );
    m_XGridCtrl->SetPenColor(
        config.ReadColor(_T("penColor")) );
    m_XGridCtrl->SetPencilColor(
        config.ReadColor(_T("pencilColor")) );
    m_XGridCtrl->SetNumberScale(
        config.ReadLong(_T("numberScale")) / 100. );
    m_XGridCtrl->SetLetterScale(
        config.ReadLong(_T("letterScale")) / 100. );

    // Clue Boxes
    //-----------
    config.SetPath(_T("/Clue"));

    for (CluePanel * panel = m_across; ; panel = m_down)
    {
        panel->SetFont(config.ReadFont(_T("font")));
        panel->SetMargins(config.ReadPoint(_T("spacing")));

        panel->SetForegroundColour(
            config.ReadColor(_T("listForegroundColor")) );
        panel->SetBackgroundColour(
            config.ReadColor(_T("listBackgroundColor")) );
        panel->SetSelectionForeground(
            config.ReadColor(_T("selectedForegroundColor")) );
        panel->SetSelectionBackground(
            config.ReadColor(_T("selectedBackgroundColor")) );
        panel->SetCrossingForeground(
            config.ReadColor(_T("crossingForegroundColor")) );
        panel->SetCrossingBackground(
            config.ReadColor(_T("crossingBackgroundColor")) );

        panel->SetHeadingFont(config.ReadFont(_T("headingFont")));
        panel->SetHeadingForeground(
            config.ReadColor(_T("headingForegroundColor")) );
        panel->SetHeadingBackground(
            config.ReadColor(_T("headingBackgroundColor")) );

        if (panel == m_down)
            break;
    }

    // Clue Prompt
    //------------
    config.SetPath(_T("/Clue Prompt"));
    m_cluePrompt->SetFont(config.ReadFont(_T("font")));
    m_cluePrompt->SetBackgroundColour(config.ReadColor(_T("backgroundColor")));
    m_cluePrompt->SetForegroundColour(config.ReadColor(_T("foregroundColor")));
    m_cluePrompt->SetDisplayFormat(config.ReadString(_T("displayFormat")));

    // Printing
    //---------
    config.SetPath(_T("/Printing"));
    g_printData->SetPaperId(static_cast<wxPaperSize>(config.ReadLong(_T("paperID"))));
    g_printData->SetOrientation(config.ReadLong(_T("orientation")));
    g_pageSetupData->SetPaperId(g_printData->GetPaperId());

    config.SetPath(_T("/Printing/Margins"));
    g_pageSetupData->SetMarginTopLeft(wxPoint(config.ReadLong(_T("left")),
                                              config.ReadLong(_T("top"))));
    g_pageSetupData->SetMarginBottomRight(wxPoint(config.ReadLong(_T("right")),
                                                  config.ReadLong(_T("bottom"))));

    config.SetPath(_T("/"));
}


void
MyFrame::SaveConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();

    // Window settings
    //----------------
    config.SetPath(_T("/Window"));

    // Only save window position if it is not maximized
    if (! IsMaximized())
    {
        int x, y, w, h;
        GetSize(&w, &h);
        GetPosition(&x, &y);

        // Make sure the sizes are within acceptable bounds
        if (w < 200) w = 500;
        if (h < 200) h = 500;
        if (x < 0 || x >= wxSystemSettings::GetMetric(wxSYS_SCREEN_X)) x = 20;
        if (y < 0 || y >= wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)) y = 20;
        config.WriteLong(_T("width"),     w);
        config.WriteLong(_T("height"),    h);
        config.WriteLong(_T("top"),       y);
        config.WriteLong(_T("left"),      x);
    }
    config.WriteBool(_T("maximized"), IsMaximized());


    // Grid Settings
    //--------------
    config.SetPath(_T("/Grid"));

    config.WriteLong(_T("style"), m_XGridCtrl->GetGridStyle());
    config.WriteBool(_T("fit"),   m_toolMgr.IsChecked(wxID_ZOOM_FIT));
    config.WriteFont(_T("letterFont"),  m_XGridCtrl->GetLetterFont());
    config.WriteFont(_T("numberFont"),  m_XGridCtrl->GetNumberFont());
    config.WriteLong(_T("lineThickness"), m_XGridCtrl->GetBorderSize());

    config.WriteColor(_T("backgroundColor"),    m_XGridCtrl->GetBackgroundColour());
    config.WriteColor(_T("selectionColor"),     m_XGridCtrl->GetSelectionColor());
    config.WriteColor(_T("focusedLetterColor"), m_XGridCtrl->GetFocusedLetterColor());
    config.WriteColor(_T("focusedWordColor"),   m_XGridCtrl->GetFocusedWordColor());
    config.WriteColor(_T("whiteSquareColor"),   m_XGridCtrl->GetWhiteSquareColor());
    config.WriteColor(_T("blackSquareColor"),   m_XGridCtrl->GetBlackSquareColor());
    config.WriteColor(_T("penColor"),           m_XGridCtrl->GetPenColor());
    config.WriteColor(_T("pencilColor"),        m_XGridCtrl->GetPencilColor());
    config.WriteLong(_T("numberScale"),         m_XGridCtrl->GetNumberScale() * 100);
    config.WriteLong(_T("letterScale"),         m_XGridCtrl->GetLetterScale() * 100);


    // Clue Panel
    //-----------
    config.SetPath(_T("/Clue"));

    config.WriteFont(_T("font"), m_across->GetFont());
    config.WritePoint(_T("spacing"), m_across->GetMargins());

    config.WriteColor(_T("listForegroundColor"),
        m_across->GetForegroundColour() );
    config.WriteColor(_T("listBackgroundColor"),
        m_across->GetBackgroundColour() );

    config.WriteColor(_T("selectedForegroundColor"),
        m_across->GetSelectionForeground() );
    config.WriteColor(_T("selectedBackgroundColor"),
        m_across->GetSelectionBackground() );

    config.WriteColor(_T("crossingForegroundColor"),
        m_across->GetCrossingForeground() );
    config.WriteColor(_T("crossingBackgroundColor"),
        m_across->GetCrossingBackground() );

    config.WriteFont(_T("headingFont"), m_across->GetHeadingFont());
    config.WriteColor(_T("headingForegroundColor"),
        m_across->GetHeadingForeground() );
    config.WriteColor(_T("headingBackgroundColor"),
        m_across->GetHeadingBackground() );

    // Clue Prompt
    //------------
    config.SetPath(_T("/Clue Prompt"));

    config.WriteFont(_T("font"), m_cluePrompt->GetFont());

    config.WriteColor(_T("foregroundColor"),
                      m_cluePrompt->GetForegroundColour());

    config.WriteColor(_T("backgroundColor"),
                      m_cluePrompt->GetBackgroundColour());

    config.WriteString(_T("displayFormat"), m_cluePrompt->GetDisplayFormat());

    // Layout
    SaveLayout(_T("(Previous)"));

    // Printing
    //---------
    config.SetPath(_T("/Printing"));
    config.WriteLong(_T("paperID"), g_pageSetupData->GetPaperId());
    config.WriteLong(_T("orientation"), g_printData->GetOrientation());
    config.SetPath(_T("/Printing/Margins"));
    config.WriteLong(_T("left"), g_pageSetupData->GetMarginTopLeft().x);
    config.WriteLong(_T("right"), g_pageSetupData->GetMarginBottomRight().x);
    config.WriteLong(_T("top"), g_pageSetupData->GetMarginTopLeft().y);
    config.WriteLong(_T("bottom"), g_pageSetupData->GetMarginBottomRight().y);
}



//------------------------------------------------------------------------------
// Access to grid (mostly for Lua)
//------------------------------------------------------------------------------
puz::Square *
MyFrame::GetFocusedSquare()
{
    if (m_XGridCtrl->IsEmpty())
        return NULL;
    else
        return m_XGridCtrl->GetFocusedSquare();
}

puz::Square *
MyFrame::SetFocusedSquare(puz::Square * square)
{
    if (m_XGridCtrl->IsEmpty())
        return false;
    else
        // TODO:These function names should really be consistent
        return m_XGridCtrl->SetSquareFocus(square);
}

void
MyFrame::GetFocusedWord(puz::Square ** start, puz::Square ** end)
{
    if (m_XGridCtrl->IsEmpty())
    {
        *start = NULL;
        *end = NULL;
    }
    else
        m_XGridCtrl->GetFocusedWord(start, end);
}

puz::GridDirection
MyFrame::GetFocusedDirection() const
{
    return m_XGridCtrl->GetDirection();
}

void
MyFrame::SetFocusedDirection(puz::GridDirection direction)
{
    m_XGridCtrl->SetDirection(direction);
}

const puz::Puzzle::Clue *
MyFrame::GetFocusedClue()
{
    puz::Square * start;
    puz::Square * end;
    GetFocusedWord(&start, &end);
    if (start == NULL)
        return NULL;
    else if (GetFocusedDirection() == puz::ACROSS)
        return &*m_puz.GetAcross().Find(start->GetNumber());
    else
        return &*m_puz.GetDown().Find(start->GetNumber());
}

bool
MyFrame::SetSquareText(puz::Square * square, const wxString & text)
{
    const bool ret = m_XGridCtrl->SetSquareText(*square, text);
    m_XGridCtrl->RefreshSquare(*square);
    return ret;
}

//------------------------------------------------------------------------------
// Menu and toolbar events
//------------------------------------------------------------------------------


// Open / Save
//------------
void
MyFrame::OnOpenPuzzle(wxCommandEvent & WXUNUSED(evt))
{
    wxString filename = wxFileSelector(
                            _T("Open Puzzle"),
                            wxEmptyString,
                            wxEmptyString,
                            _T("puz"),
                            GetLoadTypeString(),
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST
                        );

    if (! filename.empty())
        LoadPuzzle(filename);
}


void
MyFrame::OnSavePuzzleAs(wxCommandEvent & WXUNUSED(evt))
{
    SavePuzzle(wxEmptyString);
}


void
MyFrame::OnSavePuzzle(wxCommandEvent & WXUNUSED(evt))
{
    SavePuzzle(m_filename);
}


// Zoom
//---------------
void
MyFrame::OnZoomFit(wxCommandEvent & evt)
{
    m_XGridCtrl->FitGrid(evt.IsChecked());
}


void
MyFrame::OnZoomIn(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(wxID_ZOOM_FIT, false);
    m_XGridCtrl->ZoomIn();
}


void
MyFrame::OnZoomOut(wxCommandEvent & WXUNUSED(evt))
{
    m_toolMgr.Check(wxID_ZOOM_FIT, false);
    m_XGridCtrl->ZoomOut();
}


// Check / Reveal
//---------------
void
MyFrame::OnCheckGrid(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckGrid();
}

void
MyFrame::OnCheckSelection(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckSelection();
}

void
MyFrame::OnCheckWord(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckWord();
}

void
MyFrame::OnCheckLetter(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckLetter();
}

void
MyFrame::OnRevealGrid(wxCommandEvent & WXUNUSED(evt))
{
    if (XWordPrompt(MSG_REVEAL_ALL))
        m_XGridCtrl->CheckGrid(REVEAL_ANSWER | CHECK_ALL);
}

void
MyFrame::OnRevealSelection(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckSelection(REVEAL_ANSWER | CHECK_ALL);
}

void
MyFrame::OnRevealIncorrectSelection(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckSelection(REVEAL_ANSWER);
}

void
MyFrame::OnRevealIncorrect(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckGrid(REVEAL_ANSWER);
}

void
MyFrame::OnRevealWord(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckWord(REVEAL_ANSWER | CHECK_ALL);
}

void
MyFrame::OnRevealLetter(wxCommandEvent & WXUNUSED(evt))
{
    m_XGridCtrl->CheckLetter(REVEAL_ANSWER | CHECK_ALL);
}


// Scramble / Unscramble
//----------------------
void
MyFrame::OnScramble(wxCommandEvent & WXUNUSED(evt))
{
    wxASSERT(m_XGridCtrl->GetGrid() != NULL);
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

    if (m_XGridCtrl->GetGrid()->ScrambleSolution(key))
    {
        XWordMessage(MSG_SCRAMBLE, m_XGridCtrl->GetGrid()->GetKey());

        m_XGridCtrl->RecheckGrid();
        CheckPuzzle();

        m_toolMgr.Enable(ID_SCRAMBLE, false);
        m_toolMgr.Enable(ID_UNSCRAMBLE, true);

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
    wxASSERT(m_XGridCtrl->GetGrid() != NULL);

    int key = m_XGridCtrl->GetGrid()->GetKey();

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

    if (m_XGridCtrl->UnscrambleSolution(key))
    {
        XWordMessage(MSG_UNSCRAMBLE);

        CheckPuzzle();

        m_toolMgr.Enable(ID_SCRAMBLE,   true);
        m_toolMgr.Enable(ID_UNSCRAMBLE, false);

        EnableCheck(true);
        EnableReveal(true);
    }
    else
    {
        XWordMessage(MSG_WRONG_KEY);
    }
}



// Window Layout
//--------------
void
MyFrame::OnEditLayout(wxCommandEvent & evt)
{
    // Save which panes have captions so we can restore them
    if (evt.IsChecked())
    {
        // Make all the captions visible
        m_hasPaneCaption.clear();
        wxAuiPaneInfoArray & panes = m_mgr.GetAllPanes();
        for (size_t i = 0; i < panes.Count(); ++i)
        {
            wxAuiPaneInfo & pane = panes.Item(i);
            if (pane.name.StartsWith(_T("__")))
                continue;
            m_hasPaneCaption[pane.name] = pane.HasCaption();
            pane.CaptionVisible(true);
        }
    }
    else
    {
        // Restore the captions to their previous state
        wxAuiPaneInfoArray & panes = m_mgr.GetAllPanes();
        for (size_t i = 0; i < panes.Count(); ++i)
        {
            wxAuiPaneInfo & pane = panes.Item(i);
            if (pane.name.StartsWith(_T("__")))
                continue;
            std::map<wxString, bool>::iterator it = m_hasPaneCaption.find(pane.name);
            if (it == m_hasPaneCaption.end())
                pane.CaptionVisible(true);
            else
                pane.CaptionVisible(it->second);
        }
        m_hasPaneCaption.clear();
    }
    m_mgr.Update();
}


void
MyFrame::OnLoadLayout(wxCommandEvent & WXUNUSED(evt))
{
    wxFileConfig * config = GetConfig();
    wxArrayString nameArray;
    wxArrayString layoutArray;

    nameArray.push_back(_T("(Current)"));
    layoutArray.push_back(m_mgr.SavePerspective());

    // Enumerate all layouts

    // Dummy enumeration variables
    wxString str;
    long dummy;

    config->SetPath(_T("/Layouts"));
    bool bCont = config->GetFirstEntry(str, dummy);
    while (bCont)
    {
        nameArray.push_back(str);
        layoutArray.push_back(config->Read(str, wxEmptyString));
        wxLogDebug(_T("Layout %s = %s"), nameArray.back().c_str(), layoutArray.back().c_str());

        // Make sure this isn't an empty entry
        if (layoutArray.back() == wxEmptyString)
        {
            nameArray.pop_back();
            layoutArray.pop_back();
        }

        bCont = config->GetNextEntry(str, dummy);
    }
    config->SetPath(_T("/"));

    wxASSERT(nameArray.size() == layoutArray.size());
    wxASSERT(nameArray.size() > 1);

    // Show the dialog

    LayoutDialog dlg(this,
                     _T("Choose a layout"),
                     _T("Load Layout"),
                     nameArray,
                     layoutArray);

    // If the dialog is canceled, load the previous layout
    if (dlg.ShowModal() != wxID_OK)
        m_mgr.LoadPerspective(layoutArray.front(), true);

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
MyFrame::OnShowNotes(wxCommandEvent & evt)
{
    ShowPane(_T("Notes"), evt.IsChecked());
}

void
MyFrame::OnUpdateUI(wxUpdateUIEvent & evt)
{
    switch (evt.GetId())
    {
        case ID_SHOW_NOTES:
            evt.Check(m_mgr.GetPane(_T("Notes")).IsShown());
            break;
        case ID_CHARACTER_MAP:
        {
            wxAuiPaneInfo & pane = m_mgr.GetPane(_T("Characters"));
            evt.Check(pane.IsOk() && pane.IsShown());
        }
            break;
    }
}


// Timer
//------
void
MyFrame::StartTimer()
{
    m_timer.Start();
    m_toolMgr.Check(ID_TIMER);
}

void
MyFrame::StopTimer()
{
    m_timer.Stop();
    m_toolMgr.Check(ID_TIMER, false);
}

void
MyFrame::OnTimer(wxCommandEvent & evt)
{
    wxASSERT(evt.IsChecked() != IsTimerRunning());
    ToggleTimer();
}


void
MyFrame::OnTimerNotify(wxTimerEvent & WXUNUSED(evt))
{
    SetTime(m_time+1);
}


// Character Map
//--------------
void
MyFrame::OnCharacterMap(wxCommandEvent & evt)
{
    if (evt.IsChecked())
        ShowCharacterMap();
    else
        ShowPane(_T("Characters"), false);
}

// Show the character map, creating the panel if necessary
void
MyFrame::ShowCharacterMap()
{
    if (m_charactersPanel == NULL)
    {
        m_charactersPanel = new CharactersPanel(this);
        AddPane(m_charactersPanel, wxAuiPaneInfo()
                                    .Name(_T("Characters"))
                                    .Caption(_T("Character Map"))
                                    .FloatingSize(250,250)
                                    .Float());
    }
    wxAuiPaneInfo & pane = m_mgr.GetPane(m_charactersPanel);
    if (m_mgr.IsPaneClosed(pane))
    {
        pane.Show();
        m_mgr.Update();
    }
}



#ifdef XWORD_USE_LUA

void
MyFrame::LuaInit()
{
    // Initialze wxLua.
    XWORD_LUA_IMPLEMENT_BIND_ALL
    m_lua = wxLuaState(this, wxID_ANY);
    xword_setup_lua(m_lua);

    // Initialize the lua additions to the xword package
    RunLuaScript(GetScriptsDir() + _T("/xword/init.lua"));
}

void
MyFrame::LuaUninit()
{
    // Cleanup the lua additions to the xword package
    RunLuaScript(GetScriptsDir() + _T("/xword/cleanup.lua"));

    // Close lua itself
    m_lua.CloseLuaState(true);
}

void
MyFrame::RunLuaScript(const wxString & filename)
{
    m_lua.RunFile(filename);
}

void
MyFrame::OnLuaScript(wxCommandEvent & WXUNUSED(evt))
{
    const wxString & filename = wxFileSelector(_T("Select a script to run"));
    if (! filename.IsEmpty())
        RunLuaScript(filename);
}

void
MyFrame::OnLuaPrint(wxLuaEvent & evt)
{
    // Escape % to %% for printing
    wxLogDebug(_T("%s"), evt.GetString());
}


void
MyFrame::OnLuaError(wxLuaEvent & evt)
{
    // Escape % to %% for printing
    XWordErrorMessage(_T("%s"), evt.GetString());
}

#endif // XWORD_USE_LUA

// Preferences
//------------
void
MyFrame::OnPreferences(wxCommandEvent & WXUNUSED(evt))
{
    SaveConfig();
    if (m_preferencesDialog == NULL)
    {
        m_preferencesDialog = new PreferencesDialog(this);
        m_preferencesDialog->Connect(
            wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::OnPreferencesDialogCancel),
            NULL, this);
    }
    m_preferencesDialog->Show();
}


void
MyFrame::OnPreferencesDialogCancel(wxCommandEvent & evt)
{
    // Reload the config from the previously saved config
    LoadConfig();
    Refresh();

    evt.Skip();
}



// About
//------------
void
MyFrame::OnAbout(wxCommandEvent & WXUNUSED(evt))
{
    wxAboutDialogInfo info;
    info.SetName(XWORD_APP_NAME);

    info.SetVersion(XWORD_VERSION_STRING _T(" (") __TDATE__ _T(")"));

    info.SetCopyright(XWORD_COPYRIGHT_STRING);

    info.SetDescription(_T("http://sourceforge.net/projects/wx-xword/"));
    wxAboutBox(info);
}

void
MyFrame::OnLicense(wxCommandEvent & WXUNUSED(evt))
{
    const wxString licenseText(
        XWORD_APP_NAME _T(" ") XWORD_VERSION_STRING _T("\n")
        _T("Copyright ") XWORD_COPYRIGHT_STRING _T("\n")
        _T("\n")
        _T("This program is free software; you can redistribute it and/or ")
        _T("modify it under the terms of the GNU General Public License ")
        _T("as published by the Free Software Foundation; either ")
        _T("version 3 of the License, or (at your option) any later version.\n")
        _T("\n")
        _T("This program is distributed in the hope that it will be useful, ")
        _T("but WITHOUT ANY WARRANTY; without even the implied warranty of ")
        _T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ")
        _T("GNU General Public License for more details.\n")
        _T("\n")
        _T("You should have received a copy of the GNU General Public License ")
        _T("along with this program; if not, write to the Free Software ")
        _T("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n")
    );

    LicenseDialog dlg(this);
    dlg.m_textCtrl->SetValue(licenseText);
    dlg.CenterOnParent();
    dlg.ShowModal();
}

//------------------------------------------------------------------------------
// Printing events
//------------------------------------------------------------------------------
// Most of this is only slightly adapted from the wxWidgets printing sample.

void
MyFrame::OnPageSetup(wxCommandEvent & WXUNUSED(evt))
{
    // For some reason the paperID doesn't persist between page setup sessions . . .
    g_pageSetupData->SetPrintData(*g_printData);

    wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
    pageSetupDialog.ShowModal();

    *g_pageSetupData = pageSetupDialog.GetPageSetupDialogData();
    *g_printData = g_pageSetupData->GetPrintData();
}

void
MyFrame::OnPrintPreview(wxCommandEvent & WXUNUSED(evt))
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(*g_printData);
    wxPrintPreview * preview = new wxPrintPreview(new MyPrintout(this, &m_puz, 1),
                                                  new MyPrintout(this, &m_puz, 1),
                                                  &printDialogData);
    if (! preview->Ok())
    {
        delete preview;
        wxMessageBox(_T("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _T("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
                                               _T("Demo Print Preview"));
    frame->Centre();
    frame->Initialize();
    frame->Show();
}

void
MyFrame::OnPrint(wxCommandEvent & WXUNUSED(evt))
{
    wxPrintDialogData printDialogData(*g_printData);

    wxPrinter printer(& printDialogData);
    MyPrintout printout(this, &m_puz, 1);
    if (!printer.Print(this, &printout, true /*prompt*/))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox(_T("There was a problem printing.\n")
                         _T("Perhaps your current printer is not set correctly?"),
                         _T("Printing"),
                         wxOK);
    }
    else
    {
        *g_printData = printer.GetPrintDialogData().GetPrintData();
    }
}


//------------------------------------------------------------------------------
// XGridCtrl and CluePanel events
//------------------------------------------------------------------------------

void
MyFrame::OnGridFocus(wxPuzEvent & evt)
{
    // Update everything

    if (evt.GetDirection() == puz::ACROSS)
    {
        m_across->SetClueNumber(evt.GetAcrossClue(), CluePanel::FOCUSED);
        m_down  ->SetClueNumber(evt.GetDownClue(), CluePanel::CROSSING);
        m_cluePrompt->SetClue(evt.GetAcrossClue(), puz::ACROSS,
                              m_across->GetClueText());
    }
    else
    {
        m_across->SetClueNumber(evt.GetAcrossClue(), CluePanel::CROSSING);
        m_down  ->SetClueNumber(evt.GetDownClue(), CluePanel::FOCUSED);
        m_cluePrompt->SetClue(evt.GetDownClue(), puz::DOWN,
                              m_down->GetClueText());
    }
}


void
MyFrame::OnClueFocus(wxPuzEvent & evt)
{
    m_XGridCtrl->ChangeFocusedClue(evt.GetClueNumber(), evt.GetDirection());

    m_cluePrompt->SetClue(evt.GetClueNumber(), evt.GetDirection(),
                          evt.GetClueText());

    const puz::Square * crossingStart = m_XGridCtrl->GetFocusedSquare()
        ->GetWordStart(evt.GetDirection() == puz::ACROSS ? puz::DOWN : puz::ACROSS);
    const int crossingClue = (crossingStart == NULL ?
                               0 :
                               crossingStart->GetNumber());

    if (evt.GetDirection() == puz::ACROSS)
        m_down->  SetClueNumber(crossingClue, CluePanel::CROSSING);
    else
        m_across->SetClueNumber(crossingClue, CluePanel::CROSSING);
}


void
MyFrame::OnGridLetter(wxPuzEvent & WXUNUSED(evt))
{
    // Change the save/save as button
    if (! m_isModified)
    {
        m_isModified = true;
        EnableSave();
    }
    CheckPuzzle();
}



//------------------------------------------------------------------------------
// Frame events
//------------------------------------------------------------------------------

// wxTLW::OnActivate tries to restore focus to the previously focused window.
// This usually fails (especially under wxMSW).  It seems that EVT_ACTIVATE
// is sent while IsIconized() still returns true.  Perhaps MSW does not
// allow the focus to be on a window that is not shown (sensibly so).
// Our OnActivate handler connects an idle event that trys to SetFocus()
// until it actually works.  At this point, it disconnects itself.
// Brute force.
// Unfortunately, I can't get EVT_ICONIZE to be reliable.

void
MyFrame::OnActivate(wxActivateEvent & evt)
{
    if (evt.GetActive() && ! m_isIdleConnected)
    {
        m_isIdleConnected = true;
        Connect(wxEVT_IDLE, wxIdleEventHandler(MyFrame::SetFocusOnIdle));
    }
    evt.Skip();
}

void
MyFrame::SetFocusOnIdle(wxIdleEvent & evt)
{
    if (m_winLastFocused && m_winLastFocused != wxWindow::FindFocus())
        m_winLastFocused->SetFocus();
    // We need to check again because the previous call to SetFocus()
    // isn't guaranteed to work.
    if (! m_winLastFocused || m_winLastFocused == wxWindow::FindFocus())
    {
        m_isIdleConnected = false;
        Disconnect(wxEVT_IDLE, wxIdleEventHandler(MyFrame::SetFocusOnIdle));
    }
    evt.Skip();
}


void
MyFrame::OnAppActivate()
{
    if (m_toolMgr.IsChecked(ID_TIMER))
    {
        wxLogDebug(_T("Starting timer"));
        m_timer.Start();
        m_XGridCtrl->SetPaused(false);
    }
}


void
MyFrame::OnAppDeactivate()
{
    if (m_toolMgr.IsChecked(ID_TIMER))
    {
        wxLogDebug(_T("Stopping timer"));
        m_timer.Stop();
        m_XGridCtrl->SetPaused(true);
    }
}



void
MyFrame::OnClose(wxCloseEvent & evt)
{
    if (ClosePuzzle() || ! evt.CanVeto())
    {
#ifdef XWORD_USE_LUA
        LuaUninit();
#endif
        SaveConfig();
        Hide();
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
                 wxString::Format(_T("Debug: %s"), title.c_str()),
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

    puz::Grid * grid = m_XGridCtrl->GetGrid();

    str << _T("Grid size (w x h): ") << _T("\n")
        << (int)grid->GetWidth() << _T(" x ")
        << (int)grid->GetHeight() << _T("\n");

    str << _T("\n");


    str << _T("Solution:") << _T("\n");

    for (puz::Square * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << static_cast<wxChar>(square->GetPlainSolution());
        if (square->IsLast(puz::ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("User grid:") << _T("\n");

    for (puz::Square * square = grid->First();
         square != NULL;
         square = square->Next())
    {
        str << static_cast<wxChar>(square->GetPlainText());
        if (square->IsLast(puz::ACROSS))
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
    for (puz::Square * square = grid->First();
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
                str << puz2wx(square->GetText());
        }
        else
            str << _T(" ");
        str << _T("]");

        if (square->IsLast(puz::ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("Solution rebus:") <<_T("\n");
    for (puz::Square * square = grid->First();
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
                str << puz2wx(square->GetSolution());
        }
        else
            str << _T(" ");
        str << _T("]");

        if (square->IsLast(puz::ACROSS))
            str << _T("\n");
    }

    str << _T("\n");

    str << _T("Unrecognized sections:") <<_T("\n");

    std::vector<puz::Puzzle::section>::iterator it;
    for (it  = m_puz.m_extraSections.begin();
         it != m_puz.m_extraSections.end();
         ++it)
    {
        str << puz2wx(it->name)
            << wxString::Format(_T("  (length = %d)"), it->data.size())
            << _T("\n");
    }

    ShowDebugDialog(_T("Current puzzle status"), str);
}


class UnscrambleDialog : public wxDialog
{
public:
    UnscrambleDialog(wxWindow * parent)
        : wxDialog(parent, wxID_ANY, _T("Unscrambling"), wxDefaultPosition)
    {
        m_elapsedTime = 0;

        wxSizer * sizer = new wxFlexGridSizer(0,2,5,5);
        m_text = new wxStaticText(this, wxID_ANY, _T("0000"));
        sizer->Add(new wxStaticText(this, wxID_ANY, _T("Trying key:  ")));
        sizer->Add(m_text);

        m_time = new wxStaticText(this, wxID_ANY, _T("00:00"));
        sizer->Add(new wxStaticText(this, wxID_ANY, _T("Time (seconds):  ")));
        sizer->Add(m_time);

        wxBoxSizer * outersizer = new wxBoxSizer(wxVERTICAL);
        outersizer->Add(sizer, 0, wxALL, 10);
        SetSizerAndFit(outersizer);

        Connect(wxEVT_TIMER, wxTimerEventHandler(UnscrambleDialog::OnTimer));
        m_timer = new wxTimer(this);
    }

    void SetKey(unsigned short key)
    {
        m_text->SetLabel(wxString::Format(_T("%d"), key));
    }

    void StartTimer() { m_timer->Start(1000); }
    void StopTimer()  { m_timer->Stop(); }

private:
    wxStaticText * m_text;
    wxStaticText * m_time;
    int m_elapsedTime;
    wxTimer * m_timer;

    void OnTimer(wxTimerEvent & WXUNUSED(evt))
    {
        ++m_elapsedTime;
        int minutes = m_elapsedTime / 60;
        int seconds = m_elapsedTime - minutes * 60;
        m_time->SetLabel(wxString::Format(_T("%02d:%02d"), minutes, seconds));
    }
};

void
MyFrame::OnBruteForceUnscramble(wxCommandEvent & WXUNUSED(evt))
{
    if (! m_puz.IsScrambled())
    {
        wxMessageBox(_T("Puzzle is not scrambled"),
                     XWORD_APP_NAME _T(" Debug"),
                     wxOK);
        return;
    }

    UnscrambleDialog * dlg = new UnscrambleDialog(this);
    dlg->Show();
    puz::Scrambler scrambler(m_puz.m_grid);
    unsigned short key = 0;
    wxStopWatch sw;
    dlg->StartTimer();
    for (unsigned short i = 1000; i <= 9999; ++i)
    {
        wxTheApp->Yield(); // Don't block the GUI.
        dlg->SetKey(i);
        if (scrambler.UnscrambleSolution(i))
        {
            key = i;
            break;
        }
    }
    // If we don't stop the timer, it might try to send another event, which would
    // cause a seg fault once the dialog has been destroyed.  (At least, under
    // wxGTK).
    dlg->StopTimer();
    dlg->Destroy();

    if (key == 0)
    {
        wxMessageBox(wxString::Format(
                        _T("Unscrambling failed!\n")
                        _T("Elapsed time: %f seconds"), sw.Time() / 1000.),
                    XWORD_APP_NAME _T(" Debug"),
                     wxOK);
    }
    else
    {
        wxMessageBox(wxString::Format(
                        _T("Unscrambling succeeded!\n")
                        _T("Key: %d\n")
                        _T("Elapsed time: %f seconds"), key, sw.Time() / 1000.),
                     XWORD_APP_NAME _T(" Debug"),
                     wxOK);

        // Let the rest of the frame know that we did it.
        m_toolMgr.Enable(ID_SCRAMBLE,   true);
        m_toolMgr.Enable(ID_UNSCRAMBLE, false);

        EnableCheck(true);
        EnableReveal(true);
    }
}


#endif // __WXDEBUG__