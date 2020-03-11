// This file is part of XWord
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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
#include "dialogs/LayoutDialog.hpp"
#include "dialogs/PreferencesDialog.hpp"
#include "dialogs/PrintDialog.hpp"
#include "dialogs/wxFB_Dialogs.h"
#include <wx/aboutdlg.h>

// Windows
#include "ClueListBox.hpp"
#include "CluePanel.hpp"
#include "MetadataCtrl.hpp"
#include "CluePrompt.hpp"
#include "XGridCtrl.hpp"
#include "MyStatusBar.hpp"
#include "NotesPanel.hpp"

#ifdef XWORD_USE_LUA
    // This is for luapuz_Load_Puzzle
#   include "../lua/luapuz/bind/luapuz_puz_Puzzle_helpers.hpp"
#endif // XWORD_USE_LUA

// For the scrambling dialogs
#include <wx/numdlg.h>

// Clipboard stuff
#include "utils/clipboard.hpp"

// Debug
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
    //wxID_DELETE,

    //wxID_EXIT,

    //wxID_ZOOM_IN,
    //wxID_ZOOM_FIT,
    //wxID_ZOOM_OUT,

    //wxID_COPY,
    ID_COPY_SQUARE,
    //wxID_PASTE,

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
    ID_CONVERT_TO_NORMAL,

    ID_ERASE_GRID,
    ID_ERASE_UNCROSSED,

    ID_REBUS_ENTRY,

    ID_SCRAMBLE,
    ID_UNSCRAMBLE,

    ID_EDIT_LAYOUT,
    ID_LOAD_LAYOUT,
    ID_SAVE_LAYOUT,

    ID_SHOW_NOTES,

    ID_TIMER,
    ID_RESET_TIMER,

    //wxID_PREFERENCES,

    ID_PAGE_SETUP,

    ID_PRINT_CUSTOM,
    ID_PRINT_BLANK,
    ID_PRINT_TWO_PAGES,
    ID_PRINT_CURRENT,
    ID_PRINT_SOLUTION,

    //wxID_HELP_CONTENTS,
    //wxID_ABOUT,
    ID_LICENSE,

#ifdef _DEBUG

    ID_DUMP_STATUS,
    ID_DUMP_LAYOUT,
    ID_FORCE_UNSCRAMBLE,
#endif // _DEBUG

    // Timers
    ID_CLOCK_TIMER,
    ID_AUTOSAVE_TIMER,

    // This should be last so that we can have an unlimited file history
    ID_FILE_HISTORY_MENU,
    ID_FILE_HISTORY_1
};



BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_TIMER          (ID_CLOCK_TIMER,             MyFrame::OnTimerNotify)
    EVT_TIMER          (ID_AUTOSAVE_TIMER,          MyFrame::OnAutoSaveNotify)

    EVT_PUZ_GRID_FOCUS (                      MyFrame::OnGridFocus)
    EVT_PUZ_CLUE_FOCUS (                      MyFrame::OnClueFocus)
    EVT_PUZ_LETTER     (                      MyFrame::OnGridLetter)

    EVT_CLOSE          (                      MyFrame::OnClose)

    EVT_HTML_LINK_CLICKED(wxID_ANY,           MyFrame::OnLinkClicked)

    EVT_AUI_PANE_CLOSE (                      MyFrame::OnPaneClose)

    EVT_UPDATE_UI      (ID_SHOW_NOTES,        MyFrame::OnUpdateUI)
    EVT_UPDATE_UI      (ID_REBUS_ENTRY,       MyFrame::OnUpdateUI)
    EVT_UPDATE_UI      (ID_EDIT_LAYOUT,       MyFrame::OnUpdateUI)

    EVT_MENU_RANGE     (ID_FILE_HISTORY_1,
                        ID_FILE_HISTORY_1 + 10, MyFrame::OnOpenRecentPuzzle)

    EVT_ICONIZE        (                      MyFrame::OnIconize)
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

        { wxID_DELETE,       wxITEM_NORMAL, _T("&Delete Puzzle"), _T("delete"), NULL,
                     _handler(MyFrame::OnDeletePuzzle) },

        { wxID_CLOSE,       wxITEM_NORMAL, _T("&Close\tCtrl+W"), NULL, NULL,
                     _handler(MyFrame::OnClosePuzzle) },

        { wxID_PREFERENCES, wxITEM_NORMAL, _T("Preferences..."), NULL, NULL,
                     _handler(MyFrame::OnPreferences) },

        { ID_PAGE_SETUP,     wxITEM_NORMAL, _T("Page Setup..."), NULL, NULL,
                     _handler(MyFrame::OnPageSetup) },

        { ID_PRINT_BLANK,    wxITEM_NORMAL, _T("&Blank Grid\tCtrl+P"), NULL, NULL,
                     _handler(MyFrame::OnPrintBlank) },

        { ID_PRINT_TWO_PAGES,  wxITEM_NORMAL, _T("Blank Grid (&Two Pages)"), NULL, NULL,
                     _handler(MyFrame::OnPrintTwoPages) },

        { ID_PRINT_CURRENT,  wxITEM_NORMAL, _T("&Current Progress"), NULL, NULL,
                     _handler(MyFrame::OnPrintCurrent) },

        { ID_PRINT_SOLUTION,  wxITEM_NORMAL, _T("&Solution"), NULL, NULL,
                     _handler(MyFrame::OnPrintSolution) },

        { ID_PRINT_CUSTOM,  wxITEM_NORMAL, _T("&Custom\tCtrl+Shift+P"), NULL, NULL,
                     _handler(MyFrame::OnPrintCustom) },

        { wxID_EXIT,         wxITEM_NORMAL, _T("&Quit\tCtrl+Q"), NULL, NULL,
                     _handler(MyFrame::OnQuit) },

        { wxID_COPY,       wxITEM_NORMAL, _T("&Copy\tCtrl+C"), NULL, NULL,
                     _handler(MyFrame::OnCopy) },

        { ID_COPY_SQUARE,       wxITEM_NORMAL, _T("Copy &Square"), NULL, NULL,
                     _handler(MyFrame::OnCopySquare) },

        { wxID_PASTE,       wxITEM_NORMAL, _T("&Paste\tCtrl+V"), NULL, NULL,
                     _handler(MyFrame::OnPaste) },

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

        { ID_REVEAL_SELECTION, wxITEM_NORMAL, _T("Reveal &Selection..."), NULL, NULL,
                   _handler(MyFrame::OnRevealSelection) },

        { ID_REVEAL_GRID,      wxITEM_NORMAL, _T("Reveal &Grid"), NULL, NULL,
                   _handler(MyFrame::OnRevealGrid) },

        { ID_CONVERT_TO_NORMAL, wxITEM_NORMAL, _T("Convert to &Normal Puzzle"), NULL, NULL,
                   _handler(MyFrame::OnConvertToNormal) },

        { ID_ERASE_GRID,       wxITEM_NORMAL, _T("Erase &Grid"), NULL, NULL,
                   _handler(MyFrame::OnEraseGrid) },

        { ID_ERASE_UNCROSSED,  wxITEM_NORMAL, _T("Erase &Uncrossed Letters\tCtrl+Shift+D"), NULL, NULL,
                   _handler(MyFrame::OnEraseUncrossed) },

        { ID_REBUS_ENTRY,  wxITEM_CHECK, _T("Enter Multiple Letters\tCtrl+R"), _T("rebus"), NULL,
                   _handler(MyFrame::OnRebusEntry) },

        { ID_EDIT_LAYOUT, wxITEM_CHECK,  _T("&Edit Layout"), _T("layout"), NULL,
                   _handler(MyFrame::OnEditLayout) },

        { ID_LOAD_LAYOUT,  wxITEM_NORMAL, _T("&Load Layout"), NULL, NULL,
                   _handler(MyFrame::OnLoadLayout) },

        { ID_SAVE_LAYOUT,  wxITEM_NORMAL, _T("&Save Layout"), NULL, NULL,
                   _handler(MyFrame::OnSaveLayout) },

        { ID_SHOW_NOTES,   wxITEM_CHECK,  _T("&Notes\tCtrl+N"), _T("notes"), NULL,
                   _handler(MyFrame::OnShowNotes) },


        { ID_TIMER,          wxITEM_CHECK, _T("&Start/Pause\tCtrl+T"), _T("timer"), NULL,
                   _handler(MyFrame::OnTimer) },
        { ID_RESET_TIMER,    wxITEM_NORMAL, _T("&Reset"), NULL, NULL,
                   _handler(MyFrame::OnResetTimer) },

#ifdef __WXMSW__
        { wxID_HELP_CONTENTS, wxITEM_NORMAL, _T("&Help Contents\tF1"), NULL, NULL,
                   _handler(MyFrame::OnHelp) },
#endif // __WXMSW__

        { wxID_ABOUT, wxITEM_NORMAL, _T("&About ") XWORD_APP_NAME _T("..."), NULL, NULL,
                   _handler(MyFrame::OnAbout) },

        { ID_LICENSE, wxITEM_NORMAL, _T("&License..."), NULL, NULL,
                   _handler(MyFrame::OnLicense) },

#ifdef _DEBUG
        { ID_DUMP_STATUS,   wxITEM_NORMAL, _T("Dump status"), NULL, NULL,
                   _handler(MyFrame::OnDumpStatus) },

        { ID_DUMP_LAYOUT,   wxITEM_NORMAL, _T("Dump layout"), NULL, NULL,
                   _handler(MyFrame::OnDumpLayout) },

        { ID_FORCE_UNSCRAMBLE, wxITEM_NORMAL, _T("Brute force unscramble"), NULL, NULL,
                   _handler(MyFrame::OnBruteForceUnscramble) },


#endif // _DEBUG

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
      m_timer(this, ID_CLOCK_TIMER),
      m_isTimerRunning(false),
      m_autoStartTimer(false),
      m_autoSaveTimer(this, ID_AUTOSAVE_TIMER),
      m_autoSaveInterval(0),
      m_showCompletionStatus(true),
      m_mgr(),
      m_fileHistory(10, ID_FILE_HISTORY_1)
{
#if 0
#ifdef _DEBUG
    // Debug window
    wxTextCtrl * logctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxTE_MULTILINE);
    logctrl->SetFont(wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Consolas")));
    new wxLogChain(new wxLogTextCtrl(logctrl));
    m_mgr.AddPane(logctrl,
                  wxAuiPaneInfo()
                  .BestSize(350, 300)
                  .CloseButton()
                  .Layer(900)
                  .Left()
                  .Caption(_T("Logger"))
                  .Name(_T("Logger")) );
#endif // _DEBUG
#endif // 0
    // Set the initial timer amount
    m_timer.Start(1000);
    m_timer.Stop();

    wxLogDebug("Creating Frame");

    SetDropTarget(new XWordFileDropTarget(this));

    SetupToolManager();  // This will check for image files
    ManageTools();

    CreateWindows();

    SetupWindowManager();
    ManageWindows();

    LoadConfig();
    // Save window state on size
    Connect(wxEVT_SIZE, wxSizeEventHandler(MyFrame::OnSize));

#if defined(__WXMSW__) && !defined(__WXPM__)
    SetIcon(wxIcon(_T("aa_main_icon")));
#else
    SetIcon(wxIcon(xword_xpm));
#endif // __WXMSW__ && ! __WXPM__

    LoadLayout("(Previous)");

    // Show a blank puzzle
    ShowPuzzle();
}


MyFrame::~MyFrame()
{
    wxGetApp().GetConfigManager().RemoveCallbacks(this);

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
    // Multimap from file type description to file extensions for that type
    std::multimap<wxString, wxString> types;
    // Set of all supported extensions across all file types
    // Note that some extensions may be common across multiple file types (e.g. .xml).
    std::set<wxString> extensions;

    // Native formats.
    types.insert(std::pair<wxString, wxString>("Across Lite Format", "puz"));
    extensions.insert("puz");
    types.insert(std::pair<wxString, wxString>("XPF", "xml"));
    extensions.insert("xml");
    types.insert(std::pair<wxString, wxString>("JPZ", "jpz"));
    extensions.insert("jpz");
    types.insert(std::pair<wxString, wxString>("ipuz", "ipuz"));
    extensions.insert("ipuz");

#if XWORD_USE_LUA
    // Also include formats supported by the import plugin by reading them from the imports.handler
    // table.
    wxLuaState& wxL = wxGetApp().GetwxLuaState();
    lua_State* L = wxL.GetLuaState();
    lua_getglobal(L, "import");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "handlers");
        if (lua_istable(L, -1)) {
            // Loop over the table; each entry should have a "desc" description and "ext" extension
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                if (lua_istable(L, -1)) {
                    std::string desc;
                    std::string ext;
                    lua_getfield(L, -1, "desc");
                    if (!lua_isnil(L, -1))
                        desc = luaL_checkstring(L, -1);
                    lua_pop(L, 1);
                    lua_getfield(L, -1, "ext");
                    if (!lua_isnil(L, -1))
                        ext = luaL_checkstring(L, -1);
                    lua_pop(L, 1);
                    if (!desc.empty() && !ext.empty()) {
                        types.insert(std::pair<wxString, wxString>(desc, ext));
                        extensions.insert(ext);
                    }
                }
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1); // Pop the handlers table
    }
    lua_pop(L, 1); // Pop the import table
#endif

    // Generate the list of all supported extensions and add that entry as the first/default.
    wxString allExtensions = "";
    for (std::set<wxString>::iterator it = extensions.begin(); it != extensions.end(); ++it) {
        allExtensions += "*." + *it;
        if (std::next(it) != extensions.end())
            allExtensions += ";";
    }
    wxString loadTypeString =
            wxString::Format("Supported Types (%s)|%s|", allExtensions, allExtensions);

    // Add an entry for each format, including all extensions in that format.
    std::pair<std::multimap<wxString, wxString>::iterator,
            std::multimap<wxString, wxString>::iterator> range;
    for (std::multimap<wxString, wxString>::iterator it = types.begin();
            it != types.end();
            it = range.second) {
        range = types.equal_range(it->first);
        wxString typeExtensions = "";
        for (std::multimap<wxString, wxString>::iterator typeIt = range.first;
                typeIt != range.second;
                ++typeIt) {
            typeExtensions += "*." + typeIt->second;
            if (std::next(typeIt) != range.second)
                typeExtensions += ";";
        }
        loadTypeString +=
                wxString::Format("%s (%s)|%s|", it->first, typeExtensions, typeExtensions);
    }

    // Add an entry for all file types, in case a supported file has an unexpected extension.
    loadTypeString += "All Files (*.*)|*.*";

    return loadTypeString;
}

wxString
MyFrame::GetSaveTypeString()
{
    return _T("Across Lite Format (*.puz)|*.puz")
           _T("|")
           _T("XPF (*.xml)|*.xml")
           _T("|")
           _T("JPZ (*.jpz)|*.jpz");
}

bool
MyFrame::LoadPuzzle(const wxString & filename)
{
    // Load a puzzle without a handler
#if XWORD_USE_LUA
    // If we dont' have a handler, use import.load
    lua_State * L = wxGetApp().GetwxLuaState().GetLuaState();

    // Push import.load function for luapuz_Load_Puzzle.
    lua_getglobal(L, "import");
    if (! lua_istable(L, -1))
        lua_pop(L, 1);
    else
    {
        lua_getfield(L, -1, "load");
        if (! lua_isfunction(L, -1))
            lua_pop(L, 1);
        else
        {
            // Create the puz::Puzzle file handler
            puz::Puzzle::FileHandlerDesc desc;
            desc.data = L;
            desc.handler = luapuz_Load_Puzzle;

            bool ret = LoadPuzzle(filename, &desc);

            lua_pop(L, 2);
            return ret;
        }
    }
#endif
    // Otherwise, use the default behavior
    return LoadPuzzle(filename, NULL);
}

bool
MyFrame::LoadPuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
{
    Freeze();
    if (! ClosePuzzle(true, false)) // Prompt for save, don't update
    {
        Thaw();
        return false;
    }

    wxStopWatch sw;

    try
    {
        m_isModified = false;
        m_puz.Load(wx2file(filename), handler);
        // Mark the theme squares
        // call import.mark_theme_squares() from lua
    #if XWORD_USE_LUA
        wxLuaState & wxL = wxGetApp().GetwxLuaState();
        lua_State * L = wxL.GetLuaState();
        lua_getglobal(L, "import");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "mark_theme_squares");
            if (! lua_isfunction(L, -1))
                lua_pop(L, 1);
            else
            {
                luapuz_pushPuzzle(L, &m_puz);
                wxL.LuaPCall(1, 0);
            }
        }
        lua_pop(L, 1); // Pop the import table
    #endif
    }
    catch (...)
    {
        m_puz.SetOk(false);
        HandlePuzException();
    }

//    LoadLayout(_T("(Current)"), false);
//    RemoveLayout(_T("(Current)"));

    ShowPuzzle(false); // don't update

    wxFileName fn(filename);
    fn.Normalize();
    m_filename = fn.GetFullPath();

    if (m_puz.IsOk())
    {
        m_XGridCtrl->SetFocus();
        SetStatus(wxString::Format(_T("%s   Load time: %ld ms"),
                                   (const wxChar *)filename.c_str(),
                                   sw.Time()));
        if (m_autoStartTimer)
            StartTimer();
        CheckPuzzle();
        m_fileHistory.AddFileToHistory(m_filename);
    }
    else
    {
        SetStatus(_T("No file loaded"));
    }

    m_mgr.Update();
    Thaw();
    return m_puz.IsOk();
}


void
MyFrame::DoSavePuzzle(const wxString & filename,
                      const puz::Puzzle::FileHandlerDesc * handler)
{
    // We can't save notes now that notes are XHTML.
    //m_puz.m_notes = wx2puz(m_notes->GetValue());
    m_puz.SetTime(m_time);
    m_puz.SetTimerRunning(IsTimerRunning());

    std::string fn = puz::encode_utf8(wx2puz(filename));
    if (! puz::Puzzle::CanSave(fn))
        fn.clear();

    if (fn.empty())
        fn = puz::encode_utf8(wx2puz(
                wxFileSelector(
                     _T("Save Puzzle As"),
                     wxEmptyString, wxEmptyString, _T("puz"),
                     GetSaveTypeString(),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT)
            ));

    if (fn.empty())
        return;

    wxStopWatch sw;

    m_puz.Save(fn, handler);
    m_filename = puz2wx(puz::decode_utf8(fn));
    m_isModified = false;

    EnableSave(false);

    SetStatus(wxString::Format(_T("%s   Save time: %ld ms"),
                               (const wxChar *)m_filename.c_str(),
                               sw.Time()));
}

bool
MyFrame::SavePuzzle(const wxString & filename, const puz::Puzzle::FileHandlerDesc * handler)
{
    try {
        DoSavePuzzle(filename, handler);
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
        XWordErrorMessage(this, puz2wx(puz::decode_utf8(err.what())));
    }
    catch (std::exception & err)
    {
        XWordErrorMessage(this, err.what());
    }
    catch (...)
    {
        XWordErrorMessage(this, "Unknown error.");
    }
}


bool
MyFrame::ClosePuzzle(bool prompt, bool update)
{
    // No puzzle is open
    if (! m_puz.IsOk())
        return true;

    if (prompt && m_isModified)
    {
        int ret = XWordCancelablePrompt(this, "Current Puzzle not saved.  Save before closing?");
        if (ret == wxCANCEL)
            return false;
        if (ret == wxYES)
            SavePuzzle(m_filename);
    }

    m_autoSaveTimer.Stop();
    SetStatus(_T("No file loaded"));
    m_puz.Clear();

    // As hard as we try to save wxAUI pane info for clue lists, we can't
    // save dock info, so we'll just save the current perspective here and
    // then load the current perspective when we load a new puzzle.
    SaveLayout(_T("(Current)"));

    ShowPuzzle(update);

    return true;
}


void
MyFrame::ShowPuzzle(bool update)
{
    Freeze();
    // If there is no puzzle, display a blank frame
    if (! m_puz.IsOk())
        m_puz.Clear();

    // Enable / disable tools
    if (m_puz.IsOk())
    {
        EnableTools(true);
    }
    else
    {
        EnableTools(false);
        m_cluePrompt->Clear();
        m_status->SetAlert("");
    }
    EnableSave(false);

    // Update the GUI
    ShowClues();
    ShowMetadata();
    ShowGrid();
    ShowNotes();

    // Timer
    StopTimer();
    SetTime(m_puz.GetTime());
    if (m_puz.IsTimerRunning())
        StartTimer();

    Thaw();
    if (update)
        m_mgr.Update();
}

void
MyFrame::ShowGrid()
{
    if (! m_puz.IsOk())
        m_XGridCtrl->SetPuzzle(NULL);
    else
    {
        m_XGridCtrl->SetPuzzle(&m_puz);

        const bool scrambled = m_puz.IsScrambled();
        // Enable / disable scrambling tools
        m_toolMgr.Enable(ID_SCRAMBLE,   ! scrambled);
        m_toolMgr.Enable(ID_UNSCRAMBLE, scrambled);
        const bool can_check = scrambled || ! m_puz.GetGrid().HasSolution();
        EnableCheck(! can_check);
        EnableReveal(! can_check);

        m_XGridCtrl->SetFocusedSquare(m_XGridCtrl->FirstWhite());
    }

    m_XGridCtrl->SetPaused(false);
    m_XGridCtrl->Refresh();
}


void
MyFrame::ShowClues()
{
    const bool no_clues = m_puz.GetClues().empty();
    if (no_clues) // Fake clue lists for the layout
    {
        m_puz.SetClueList(puzT("Across"), puz::ClueList(puzT("Across")));
        m_puz.SetClueList(puzT("Down"), puz::ClueList(puzT("Down")));
    }

    // Remove the old clue lists from m_clues, but keep the windows alive
    wxAuiPaneInfo * existingInfo = NULL;
    std::map<wxString, CluePanel *> old_panels; // AUI_name = CluePanel *
    {
        std::map<wxString, CluePanel *>::iterator it;
        for (it = m_clues.begin(); it != m_clues.end(); ++it)
        {
#if USE_MY_AUI_MANAGER
            wxAuiPaneInfo & info = m_mgr.FindPane(it->second);
            old_panels[info.name] = it->second;
            existingInfo = &info;
#else
            // The equivalent of FindPane()
            wxWindow * window = it->second;
            for (;;)
            {
                // We're at the top of the hierarchy.
                if (! window || window == this)
                    break;
                // Check this window
                wxAuiPaneInfo & pane = m_mgr.GetPane(window);
                if (pane.IsOk())
                    old_panels[pane.name] = it->second;
                // Next parent window
                window = window->GetParent();
            }
#endif // USE_MY_AUI_MANAGER
        }
        m_clues.clear();
    }

    // For each of the clue lists that we need, change the AUI caption of
    // an existing pane and replace the clue list contents.
    // If we have more clue lists than panels, create new panels.
    // If we have more panels than clue lists, destroy some panels.
    wxAuiPaneInfo baseInfo;
    // If we have a wxAuiPaneInfo for an existing clue pane, use it as the base
    // for new panes.
    if (existingInfo)
        baseInfo.SafeSet(*existingInfo);
    // Set some basic flags
    baseInfo.CaptionVisible(false)
            .MinSize(50,50)
            .Layer(0); // Closest to the grid
    {
        // AUI names for the clue lists are as follows:
        //  Across -> "Cluelist1"
        //  Down -> "ClueList2"
        //  Others -> "ClueList([1 or 3]..n)"
        //  if both Across and Down are present, other clue lists start at 3
        //  otherwise all clue lists start from 1
        int cluelist_id = 1; // Starting ClueList ID
        if (m_puz.HasClueList(puzT("Across")))
        {
            cluelist_id = 2;
            if (m_puz.HasClueList(puzT("Down")))
                cluelist_id = 3;
        }

        puz::Clues::iterator it;
        for (it = m_puz.GetClues().begin() ; it != m_puz.GetClues().end(); ++it)
        {
            wxString label = puz2wx(it->first);
            // Name the pane
            wxString id;
            if (label == _T("Across"))
                id = _T("ClueList1");
            else if (label == _T("Down"))
                id = _T("ClueList2");
            else
                id = wxString::Format(_T("ClueList%d"), cluelist_id++);

            // Find the pane
            wxAuiPaneInfo & info = m_mgr.GetPane(id);
            CluePanel * clues = NULL;
            if (info.IsOk())
            {
                // This pane exists, change it
                info.Caption(label);
                clues = old_panels[id];
                old_panels.erase(id);
                info.Show();
            }
            else
            {
                // This pane doesn't exist, add it.
                clues = new CluePanel(this, wxID_ANY);
                m_mgr.AddPane(clues,
                              wxAuiPaneInfo(baseInfo)
                              .Caption(label)
                              .Name(id)
                              .Show());
            }
            // Set the heading and clue list for the panel
            clues->SetHeading(label);
            clues->SetClueList(&it->second);
            if (no_clues)
                clues->ClearClueList();
            m_clues[label] = clues;
        }
    }

    // Delete and detach the left over CluePanels
    {
        std::map<wxString, CluePanel *>::iterator it;
        for (it = old_panels.begin(); it != old_panels.end(); ++it)
        {
            m_mgr.DetachPane(it->second);
            it->second->Destroy();
        }
    }

    if (no_clues)
        m_puz.GetClues().clear();

#if USE_MY_AUI_MANAGER
    // Since the set of panes / pane captions may have changed, update the menu.
    m_mgr.UpdateMenu();
#endif // USE_MY_AUI_MANAGER

    // Update the UI
    wxGetApp().GetConfigManager().Clue.Update();
}


void
MyFrame::ShowMetadata()
{
    // Set window title
    if (! m_puz.IsOk())
        SetTitle(XWORD_APP_NAME);
    else
        SetTitle(puz2wx(m_puz.GetTitle()) + _T(" - ") XWORD_APP_NAME);


    // Update the metadata panels
    typedef std::vector<wxAuiPaneInfo *> pane_vector_t;
    std::map<wxAuiDockInfo *, pane_vector_t> metadata_map;

    wxAuiPaneInfoArray & panes = m_mgr.GetAllPanes();
    for (size_t i = 0; i < panes.size(); ++i)
    {
        wxAuiPaneInfo & pane = panes.Item(i);
        MetadataCtrl * meta = wxDynamicCast(pane.window, MetadataCtrl);
        if (meta)
        {
            meta->UpdateLabel();
            if (! meta->GetPlainLabel().IsEmpty()) {
                pane.Show();
#if USE_MY_AUI_MANAGER
                wxAuiDockInfo & dock = m_mgr.FindDock(pane);
                metadata_map[&dock].push_back(&pane);
#endif // USE_MY_AUI_MANAGER
            }
            else {
                pane.Hide();
            }
        }
    }

#if USE_MY_AUI_MANAGER
    // Adjust sizes of MetadataCtrls
    std::map<wxAuiDockInfo *, pane_vector_t>::iterator it;
    for (it = metadata_map.begin(); it != metadata_map.end(); ++it)
    {
        pane_vector_t & panes = it->second;
        if (panes.size() <= 1)
            continue;
        // Calculate the total size of metadata panes in this dock
        std::vector<int> widths;
        int total_width = 0;
        int total_proportion = 0;
        for (size_t i = 0; i < panes.size(); ++i)
        {
            // This is a safe cast: we already know the type of window
            wxAuiPaneInfo & pane = *panes[i];
            MetadataCtrl * meta = (MetadataCtrl *)(pane.window);
            widths.push_back(meta->GetLayoutWidth());
            total_width += meta->GetLayoutWidth();
            total_proportion += pane.dock_proportion;
        }
        if (total_width <= 0 || total_proportion <= 0)
            continue;
        // Adjust the proportion based on calculated widths
        for (size_t i = 0; i < panes.size(); ++i)
        {
            int proportion = total_proportion * (widths[i] / (double)total_width) + 0.5;
            panes[i]->dock_proportion = std::max(proportion, 1);
        }
    }
#endif // USE_MY_AUI_MANAGER
}


// Should this metadata be displayed as notes?
// Either has "_notes_" in the name, or is one of
// "description", or "instructions"
bool IsNotes(const puz::string_t & str)
{
    // str == "notes" is the real notepad
    // Starts with "notes_"
    if (str.compare(0, 6, puzT("notes_")) == 0)
        return true;
    // Ends with "_notes"
    int start = str.length() - 6;
    if (start >= 0 && str.compare(start, puz::string_t::npos, puzT("_notes")) == 0)
        return true;
    // Contains "-notes-"
    if (str.find(puzT("_notes_")) != puz::string_t::npos)
        return true;
    // description/instructions
    if (str == puzT("description") || str == puzT("instructions"))
        return true;
    return false;
}

// Turn a string in snake_case into title case
wxString TitleCase(const wxString & str)
{
    wxString ret(str);
    for (size_t i = 0; i < ret.size(); ++i)
    {
        if (i == 0)
        {
            ret[i] = wxToupper(str[i]);
        }
        else if (str[i] == '_' || str[i] == ' ')
        {
            ret[i] = ' ';
            ++i;
            if (i < ret.size())
                ret[i] = wxToupper(str[i]);
        }
    }
    return ret;
}

void
MyFrame::ShowNotes()
{
    wxString notes(puz2wx(m_puz.GetNotes()));
    // Notes is not just the notepad but any other metadata with
    // "note" in the name
    wxString value;
    // Add extra notes sections as table rows
    const puz::Puzzle::metamap_t & meta = m_puz.GetMetadata();
    puz::Puzzle::metamap_t::const_iterator it;
    for (it = meta.begin(); it != meta.end(); ++it)
    {
        if (IsNotes(it->first))
        {
            value.append("<tr bgcolor=#dddddd><td>");
            value.append(TitleCase(puz2wx(it->first)));
            value.append("</td></tr>");
            value.append("<tr><td>");
            value.append(puz2wx(it->second));
            value.append("</td></tr>");
        }
    }
    // Set the value of the notes pane
    if (value.empty())
    {
        if (notes.empty())
            m_notes->SetPage("[Notes]");
        else
            m_notes->SetPage(notes);
    }
    else
    {
        // If we have notes, add them as a table row as well
        wxString notes_table;
        if (! notes.empty())
        {
            notes_table.append("<tr bgcolor=#eeeeee><td>");
            notes_table.append("Notes");
            notes_table.append("</td></tr>");
            notes_table.append("<tr><td>");
            notes_table.append(notes);
            notes_table.append("</td></tr>");
        }
        m_notes->SetPage("<table>" + notes_table + value + "</table>");
    }
}

void
MyFrame::CheckPuzzle()
{
    if (!m_showCompletionStatus) {
        m_status->SetAlert("");
        return;
    }
    GridStats stats;
    m_XGridCtrl->GetStats(&stats);
    switch (stats.correct)
    {
        case CORRECT_PUZZLE:
            StopTimer();
            m_status->SetAlert(_T("The puzzle is filled correctly!"),
                wxGetApp().GetConfigManager().Status.completeColor());
            break;
        case UNCHECKABLE_PUZZLE:
            StopTimer();
            m_status->SetAlert(
                _T("The puzzle is completely filled."),
                wxGetApp().GetConfigManager().Status.uncheckableColor());
            break;
        case INCORRECT_PUZZLE:
            m_status->SetAlert(
                _T("The puzzle contains incorrect letters."),
                wxGetApp().GetConfigManager().Status.incorrectColor());
            break;
        case INCOMPLETE_PUZZLE:
        default:
            m_status->SetAlert(wxString::Format(_T("%d/%d filled (%d%%)"),
                stats.white - stats.blank, stats.white,
                int((100 * (stats.white - stats.blank) + .5) / stats.white)));
            break;
    }
}



//------------------------------------------------------------------------------
// Window setup functions
//------------------------------------------------------------------------------

void
MyFrame::CreateWindows()
{
    m_XGridCtrl = new XGridCtrl(this);
	// Windows uses the frame and all child windows as the drop target.
	// Mac seems to do this on a window-by-window basis, so we should at least
	// let the user drop a puzzle onto the grid
#ifndef __WXMSW__
	m_XGridCtrl->SetDropTarget(new XWordFileDropTarget(this));
#endif

    m_cluePrompt = new CluePrompt(this, wxID_ANY);
    m_notes      = new NotesPanel(this, wxID_ANY);

    // Metadata panels are created in LoadConfig

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
    SetStatusBarPane(-1); // Disable menu help: it's buggy

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
    m_toolMgr.Add(tb, wxID_DELETE);
    tb->AddSeparator();
    m_toolMgr.Add(tb, wxID_ZOOM_IN);
    m_toolMgr.Add(tb, wxID_ZOOM_FIT);
    m_toolMgr.Add(tb, wxID_ZOOM_OUT);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_CHECK_LETTER);
    m_toolMgr.Add(tb, ID_CHECK_WORD);
    m_toolMgr.Add(tb, ID_CHECK_GRID);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_EDIT_LAYOUT);
    m_toolMgr.Add(tb, ID_SHOW_NOTES);
    tb->AddSeparator();
    m_toolMgr.Add(tb, ID_REBUS_ENTRY);
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

    wxMenu * menu;
    wxMenu * submenu;

    // File Menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_OPEN);
        m_toolMgr.Add(menu, wxID_SAVE);
        m_toolMgr.Add(menu, wxID_SAVEAS);
        m_toolMgr.Add(menu, wxID_DELETE);
        m_toolMgr.Add(menu, wxID_CLOSE);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, wxID_PREFERENCES);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_PAGE_SETUP);
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_PRINT_BLANK);
            m_toolMgr.Add(submenu, ID_PRINT_TWO_PAGES);
            m_toolMgr.Add(submenu, ID_PRINT_CURRENT);
            m_toolMgr.Add(submenu, ID_PRINT_SOLUTION);
            m_toolMgr.Add(submenu, ID_PRINT_CUSTOM);
        menu->AppendSubMenu(submenu, _T("&Print"));
        menu->AppendSeparator();
        m_toolMgr.Add(menu, wxID_EXIT);
    mb->Append(menu, _T("&File"));

    menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_COPY);
        m_toolMgr.Add(menu, ID_COPY_SQUARE);
        m_toolMgr.Add(menu, wxID_PASTE);
    mb->Append(menu, _T("&Edit"));

    menu = new wxMenu();
        m_toolMgr.Add(menu, wxID_ZOOM_IN);
        m_toolMgr.Add(menu, wxID_ZOOM_FIT);
        m_toolMgr.Add(menu, wxID_ZOOM_OUT);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_SHOW_NOTES);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_EDIT_LAYOUT);
        m_toolMgr.Add(menu, ID_LOAD_LAYOUT);
        m_toolMgr.Add(menu, ID_SAVE_LAYOUT);
#if USE_MY_AUI_MANAGER
        wxMenu * paneMenu = new wxMenu();
        m_mgr.SetManagedMenu(paneMenu);
        menu->AppendSubMenu(paneMenu, _T("Panes"));
#endif
    mb->Append(menu, _T("&View"));

    // Solution Menu
    menu = new wxMenu();
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_CHECK_LETTER);
            m_toolMgr.Add(submenu, ID_CHECK_WORD);
            m_toolMgr.Add(submenu, ID_CHECK_SELECTION);
            m_toolMgr.Add(submenu, ID_CHECK_GRID);
        menu->AppendSubMenu(submenu, _T("&Check"));
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_REVEAL_LETTER);
            m_toolMgr.Add(submenu, ID_REVEAL_WORD);
            m_toolMgr.Add(submenu, ID_REVEAL_INCORRECT);
            m_toolMgr.Add(submenu, ID_REVEAL_INCORRECT_SELECTION);
            submenu->AppendSeparator();
            m_toolMgr.Add(submenu, ID_REVEAL_SELECTION);
            m_toolMgr.Add(submenu, ID_REVEAL_GRID);
        menu->AppendSubMenu(submenu, _T("&Reveal"));
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_ERASE_GRID);
            m_toolMgr.Add(submenu, ID_ERASE_UNCROSSED);
        menu->AppendSubMenu(submenu, _T("&Erase"));
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_REBUS_ENTRY);
        menu->AppendSeparator();
        m_toolMgr.Add(menu, ID_SCRAMBLE);
        m_toolMgr.Add(menu, ID_UNSCRAMBLE);
    mb->Append(menu, _T("&Solution"));

    // Tools Menu
    menu = new wxMenu();
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_TIMER);
            m_toolMgr.Add(submenu, ID_RESET_TIMER);
        menu->AppendSubMenu(submenu, _T("&Timer"));
        submenu = new wxMenu();
            m_toolMgr.Add(submenu, ID_CONVERT_TO_NORMAL);
        menu->AppendSubMenu(submenu, _T("&Diagramless"));
    mb->Append(menu, _T("&Tools"));

    // Help Menu
    menu = new wxMenu();
#ifdef __WXMSW__
        m_toolMgr.Add(menu, wxID_HELP_CONTENTS);
#endif // __WXMSW__
        m_toolMgr.Add(menu, wxID_ABOUT);
        m_toolMgr.Add(menu, ID_LICENSE);
    mb->Append(menu, _T("&Help"));

#ifdef _DEBUG

    // Debug menu
    menu = new wxMenu();
        m_toolMgr.Add(menu, ID_DUMP_STATUS);
        m_toolMgr.Add(menu, ID_DUMP_LAYOUT);
        m_toolMgr.Add(menu, ID_FORCE_UNSCRAMBLE);
    mb->Append(menu, _T("&Debug"));

#endif // _DEBUG

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


// This is an external entry point for adding Aui panes (i.e. through lua)
void
MyFrame::AddPane(wxWindow * window, const wxAuiPaneInfo & info)
{
    m_mgr.AddPane(window, info);
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
    wxAuiPaneInfo baseInfo;
    baseInfo.CaptionVisible(false)
            .CloseButton()
            .Resizable(false)
            .PaneBorder(false)
            .MinSize(15,15);

    // Give everything a name so we can save and load the layout
    m_mgr.AddPane(m_XGridCtrl,
                  wxAuiPaneInfo(baseInfo)
                  .Center()
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

    m_mgr.AddPane(m_cluePrompt,
                  wxAuiPaneInfo(baseInfo)
                  .MinSize(50, 50)
                  .Layer(2)
                  .Top()
                  .Caption(_T("Clue Prompt"))
                  .Name(_T("Clue")) );

    m_mgr.AddPane(m_notes,
                  wxAuiPaneInfo(baseInfo)
                  .Float()
                  .MinSize(50, 50)
                  .FloatingSize(250,250)
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
    wxString imagesdir = GetImagesDir();
    if (wxDirExists(imagesdir))
    {
        m_toolMgr.SetIconLocation(imagesdir);
    }
    else
    {
        XWordErrorMessage(this, wxString::Format("Cannot find images directory:\n%s", imagesdir));
        m_toolMgr.SetIconLocation(_T(""));
    }
}



void
MyFrame::EnableTools(bool enable)
{
    EnableGridSize(enable);
    EnableCheck(enable);
    EnableReveal(enable);
    EnableDiagramless(enable && m_puz.IsOk() && m_puz.IsDiagramless());

    // Tools that are only enabled or disabled when a puzzle
    // is shown or closed.  These don't have any special logic.
    m_toolMgr.Enable(wxID_COPY, enable);
    m_toolMgr.Enable(wxID_PASTE, enable);
    m_toolMgr.Enable(ID_COPY_SQUARE, enable);
    m_toolMgr.Enable(ID_UNSCRAMBLE, enable);
    m_toolMgr.Enable(ID_SCRAMBLE,   enable);
    m_toolMgr.Enable(ID_ERASE_GRID, enable);
    m_toolMgr.Enable(ID_ERASE_UNCROSSED,   enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("Solution"), _T("Erase")),
                      enable);
    m_toolMgr.Enable(ID_REBUS_ENTRY,   enable);
    m_toolMgr.Enable(wxID_SAVE,    enable);
    m_toolMgr.Enable(wxID_SAVEAS, enable);
    m_toolMgr.Enable(wxID_CLOSE, enable);
    m_toolMgr.Enable(wxID_DELETE, enable);
    m_toolMgr.Enable(ID_TIMER, enable);

    m_toolMgr.Enable(ID_PRINT_BLANK, enable);
    m_toolMgr.Enable(ID_PRINT_TWO_PAGES, enable);
    m_toolMgr.Enable(ID_PRINT_CURRENT, enable);
    m_toolMgr.Enable(ID_PRINT_SOLUTION, enable);
    m_toolMgr.Enable(ID_PRINT_CUSTOM, enable);
    m_menubar->Enable(m_menubar->FindMenuItem(_T("File"), _T("Print")),
                      enable);
}


void
MyFrame::EnableSave(bool enable)
{
    m_toolMgr.Enable(wxID_SAVE,         enable);
#if 0 // Don't swap the icons, just enable/disable
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
#endif // 0
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

void
MyFrame::EnableDiagramless(bool enable)
{
    m_toolMgr.Enable(ID_CONVERT_TO_NORMAL, enable);
    m_menubar->Enable(m_menubar->FindMenuItem("Tools", "Diagramless"), enable);
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
    if (config.Window.maximized())
        Maximize();
    else
        SetSize( config.Window.left(), config.Window.top(),
                 config.Window.width(), config.Window.height() );
    // Grid
    //-----
    if (config.Grid.fit())
    {
        m_toolMgr.Check(wxID_ZOOM_FIT);
        m_XGridCtrl->FitGrid();
    }
    m_XGridCtrl->SetBoxSize(config.Grid.boxSize());


    // Printing
    ConfigManager::Printing_t & print = config.Printing;
    g_printData->SetPaperId(static_cast<wxPaperSize>(print.paperID()));
    g_printData->SetOrientation(static_cast<wxPrintOrientation>(print.orientation()));
    g_pageSetupData->SetPaperId(g_printData->GetPaperId());

    ConfigManager::Printing_t::Margins_t & margins = print.Margins;
    g_pageSetupData->SetMarginTopLeft(wxPoint(margins.left(), margins.top()));
    g_pageSetupData->SetMarginBottomRight(wxPoint(margins.right(),
                                                  margins.bottom()));

    // Misc
    config.Timer.autoStart.AddCallback(this, &MyFrame::SetAutoStartTimer);
    config.autoSaveInterval.AddCallback(this, &MyFrame::SetAutoSaveInterval);
    config.Status.showCompletionStatus.AddCallback(this, &MyFrame::SetShowCompletionStatus);

    // File History
    config.FileHistory.saveFileHistory.AddCallback(
        this, &MyFrame::SetSaveFileHistory);
    wxFileConfig * raw_cfg = GetConfig();
    raw_cfg->SetPath(config.FileHistory.m_name);
    m_fileHistory.Load(*raw_cfg);
    raw_cfg->SetPath(_T(""));

    // CluePrompt
    // Update some deprecated formatting values
    wxString format = config.CluePrompt.displayFormat();
    format.Replace(_T("%N"), _T("%cluenumber%"));
    format.Replace(_T("%T"), _T("%clue%"));
    config.CluePrompt.displayFormat = format;

    // Load the Metadata panels
    ConfigManager::MetadataCtrls_t & metadata = config.MetadataCtrls;
    ConfigManager::MetadataCtrls_t::iterator meta;
    // Make sure we have Title, Author, and Copyright
    if (! metadata.FindChild("Title"))
        metadata.push_back("Title");
    if (! metadata.FindChild("Author"))
        metadata.push_back("Author");
    if (! metadata.FindChild("Copyright"))
        metadata.push_back("Copyright");
    // Set default values
    for (meta = metadata.begin(); meta != metadata.end(); ++meta)
    {
        if (meta->m_name == "/Metadata/Title")
            meta->displayFormat.SetDefault("%title%");
        else if (meta->m_name == "/Metadata/Copyright")
            meta->displayFormat.SetDefault(_T("%copyright%"));
        else if (meta->m_name == "/Metadata/Author")
        {
            meta->displayFormat.SetDefault("%author%");
        #if XWORD_USE_LUA
            // If we're using lua, make the author format smarter
            meta->displayFormat.SetDefault(
                "if %author% then\n"
                "  if %editor% then\n"
                "    return %author% .. ' / edited by ' .. %editor%\n"
                "  else\n"
                "    return %author%\n"
                "  end\n"
                "elseif %editor% then\n"
                "  return 'Edited by ' .. %editor%\n"
                "end");
            meta->useLua.SetDefault(true);
        #endif
        }
    }

    // Add the panels to our configuration
    for (meta = metadata.begin(); meta != metadata.end(); ++meta)
    {
        // Don't add if we already have the pane
        if (m_mgr.GetPane(meta->m_name).IsOk())
            continue;

        MetadataCtrl * ctrl = new MetadataCtrl(
            this, wxID_ANY, meta->displayFormat(),
            wxDefaultPosition, wxDefaultSize,
            meta->alignment() | (meta->useLua() ? META_USE_LUA : 0));
        // Metadata callbacks
        ctrl->SetConfig(&*meta);

        // Chop the metadata part of the name off
        wxString name;
        if (! meta->m_name.StartsWith(_T("/Metadata/"), &name))
            name = meta->m_name;
        m_mgr.AddPane(ctrl,
                      wxAuiPaneInfo().Name(meta->m_name)
                      .Caption(name).CaptionVisible(false)
                      .CloseButton().Resizable(false).PaneBorder(false)
                      .MinSize(25,25).Layer(10)
                      .Direction(name == "Copyright" ? wxAUI_DOCK_BOTTOM
                                                     : wxAUI_DOCK_TOP));
    }

    // Update the config of our controls
    config.Update();
}

void
MyFrame::SetSaveFileHistory(bool doit)
{
    // Find the recent files menu
    wxMenu * fileMenu = m_menubar->GetMenu(0);
    if (! fileMenu)
        return;
    wxMenuItem * recent = fileMenu->FindItem(ID_FILE_HISTORY_MENU);
    if (doit) // Add the menu if it doesn't exist.
    {
        if (! recent)
        {
            wxMenu * recentMenu = new wxMenu();
            fileMenu->Insert(1, new wxMenuItem(
                fileMenu, ID_FILE_HISTORY_MENU, _T("Recent Files"),
                wxEmptyString, wxITEM_NORMAL, recentMenu));
            m_fileHistory.UseMenu(recentMenu);
            m_fileHistory.AddFilesToMenu();
        }
    }
    else if (recent) // Remove the menu if it exists
    {
        m_fileHistory.RemoveMenu(recent->GetSubMenu());
        delete fileMenu->Remove(ID_FILE_HISTORY_MENU);
    }
}


void
MyFrame::SaveWindowConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();
    // Only save window position if it is not maximized
    if (! IsIconized()) // If iconized, window position and size are wrong
    {
        if (! IsMaximized())
        {
            int x, y, w, h;
            GetSize(&w, &h);
            GetPosition(&x, &y);

            // Make sure the sizes are within acceptable bounds
            if (w < 200) w = 500;
            if (h < 200) h = 500;
            // For some reason on Windows with wxWidgets 3.1.2, xleft alignment of the window
            // results in slightly negative coordinates. So only snap the window back to the
            // screen if it is significantly off screen.
            if (x < -200 || x >= wxSystemSettings::GetMetric(wxSYS_SCREEN_X)) x = 20;
            if (y < -200 || y >= wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)) y = 20;
            config.Window.width = w;
            config.Window.height = h;
            config.Window.top = y;
            config.Window.left = x;
        }
        config.Window.maximized = IsMaximized();
    }
}

void
MyFrame::SaveConfig()
{
    ConfigManager & config = wxGetApp().GetConfigManager();
    config.AutoUpdate(false);

    // Window settings
    //----------------
    SaveWindowConfig();

    // Grid
    //-----
    config.Grid.boxSize = m_XGridCtrl->GetBoxSize();
    config.Grid.fit = m_toolMgr.IsChecked(wxID_ZOOM_FIT);

    // File History
    wxFileConfig * raw_cfg = GetConfig();
    if (! config.FileHistory.saveFileHistory())
    {
        // Erase the file history if we aren't saving it
        for (size_t i = m_fileHistory.GetCount(); i > 0; --i)
            m_fileHistory.RemoveFileFromHistory(i-1);
    }
    raw_cfg->SetPath(config.FileHistory.m_name);
    m_fileHistory.Save(*raw_cfg);
    raw_cfg->SetPath(_T(""));

    config.AutoUpdate(true);
    // The rest of the config has only changed if the user used the
    // preferences dialog, in which case the prefs dialog has already
    // written the rest of the config settings.
}


void
MyFrame::OnSize(wxSizeEvent & evt)
{
    // Save window state when we're resized
    SaveWindowConfig();
    evt.Skip();
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
        return NULL;
    else
        return m_XGridCtrl->SetFocusedSquare(square);
}

puz::Word *
MyFrame::GetFocusedWord()
{
    return m_XGridCtrl->GetFocusedWord();
}

short
MyFrame::GetFocusedDirection() const
{
    return m_XGridCtrl->GetDirection();
}

void
MyFrame::SetFocusedDirection(short direction)
{
    if (! m_XGridCtrl->IsEmpty())
        m_XGridCtrl->SetFocusedSquare(NULL, direction);
}

puz::Clue *
MyFrame::GetFocusedClue()
{
    if (! m_puz.IsDiagramless())
        return m_puz.FindClue(GetFocusedWord());
    else
    {
        std::map<wxString, CluePanel*>::iterator it;
        for (it = m_clues.begin(); it != m_clues.end(); ++it)
        {
            CluePanel * panel = it->second;
            if (panel->GetDirection() == CluePanel::FOCUSED)
                return panel->GetClue();
        }
        return NULL;
    }
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
MyFrame::OnOpenRecentPuzzle(wxCommandEvent & evt)
{
    wxString fn = m_fileHistory.GetHistoryFile(evt.GetId() - ID_FILE_HISTORY_1);
    if (! fn.empty())
        LoadPuzzle(fn);
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

void
MyFrame::OnDeletePuzzle(wxCommandEvent & WXUNUSED(evt))
{
    if (XWordPrompt(this, "Delete this file?") && ClosePuzzle(false))
        wxRemoveFile(m_filename);
}


// Edit
// -----
void
MyFrame::OnCopy(wxCommandEvent & evt)
{
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new XWordTextDataObject(GetFocusedWord()));
        wxTheClipboard->Close();
    }
}

void
MyFrame::OnCopySquare(wxCommandEvent & evt)
{
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new XWordTextDataObject(GetFocusedSquare()));
        wxTheClipboard->Close();
    }
}

void
MyFrame::OnPaste(wxCommandEvent & evt)
{
    if (wxTheClipboard->Open())
    {
        // First look for an XWord clipboard object
        WordDataObject data;
        if (wxTheClipboard->IsSupported(data.GetFormat()))
        {
            wxTheClipboard->GetData(data);
            wxLogDebug(_T("Got data"));
            wxLogDebug(data.GetText());
            wxStringTokenizer letters = data.GetTokens();
            puz::Word * word = GetFocusedWord();
            if (word)
            {
                puz::square_iterator square = word->find(GetFocusedSquare());
                for (; square != word->end() && letters.HasMoreTokens(); ++square)
                {
                    m_XGridCtrl->SetSquareText(*square, letters.GetNextToken());
                }
            }
            m_XGridCtrl->Refresh();
        }
        // Next check for a text clipboard object
        else if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            wxString letters = data.GetText();
            puz::Word * word = GetFocusedWord();
            if (word)
            {
                wxString::const_iterator letter = letters.begin();
                puz::square_iterator square = word->find(GetFocusedSquare());
                for (; square != word->end() && letter != letters.end(); ++square)
                    m_XGridCtrl->SetSquareText(*square, wx2puz(*(letter++)));
            }
            m_XGridCtrl->Refresh();
        }
        else
            wxLogDebug(_T("No data"));
        wxTheClipboard->Close();
    }
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
    if (XWordPrompt(this, "This will reveal the entire grid.  Continue?"))
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

void
MyFrame::OnConvertToNormal(wxCommandEvent & WXUNUSED(evt))
{
    wxASSERT(m_puz.IsDiagramless());

    if (! XWordPrompt(this, _T("This will convert this diagramless puzzle to a normal puzzle, ")
                            _T("revealing all black squares and overwriting any letters entered ")
                            _T("where black squares should be.  Continue?")))
        return;

    m_puz.ConvertDiagramlessToNormal();

    // Turn off the diagramless menu items
    EnableDiagramless(false);

    // Restore focus to the current square. If the current square is black or no square is selected,
    // move to the first square instead.
    puz::Square * square = m_XGridCtrl->GetFocusedSquare();
    if (square == NULL || square->IsBlack())
        square = m_XGridCtrl->FirstWhite();
    m_XGridCtrl->SetFocusedSquare(square, m_puz.FindWord(square));

    // Refresh the grid UI
    m_XGridCtrl->Refresh();

    // Send puzzle updated event
    m_XGridCtrl->SendEvent(wxEVT_PUZ_LETTER);
}

void
MyFrame::OnEraseGrid(wxCommandEvent & WXUNUSED(evt))
{
    if (! XWordPrompt(this, "This will erase all letters in the grid "
                            "and reset the timer.  Continue?"))
        return;

    const bool is_diagramless = m_puz.IsDiagramless();
    for (puz::Square * square = m_puz.GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        if (is_diagramless || square->IsWhite())
        {
            square->SetText(puzT(""));
            square->RemoveFlag(puz::FLAG_BLACK |
                               puz::FLAG_CHECK_MASK |
                               puz::FLAG_PENCIL);
        }
    }
    if (is_diagramless)
        m_puz.GetGrid().NumberGrid();
    m_XGridCtrl->SetFocusedSquare(m_XGridCtrl->FirstWhite(), puz::ACROSS);
    m_XGridCtrl->Refresh();
    SetTime(0);
    // Send puzzle updated event
    m_XGridCtrl->SendEvent(wxEVT_PUZ_LETTER);
}


void
MyFrame::OnEraseUncrossed(wxCommandEvent & WXUNUSED(evt))
{
    // Helper function
    struct _is_uncrossed {
        _is_uncrossed(puz::Puzzle & puz) : m_puz(puz) {}

        bool operator()(puz::Square & square, puz::Word * word)
        {
            // Look for another word that contains this square.
            puz::Clues & clues = m_puz.GetClues();
            puz::Clues::iterator clues_it;
            for (clues_it = clues.begin(); clues_it != clues.end(); ++clues_it)
            {
                puz::ClueList & cluelist = clues_it->second;
                puz::ClueList::iterator clue;
                for (clue = cluelist.begin(); clue != cluelist.end(); ++clue)
                {
                    puz::Word & crossing = clue->GetWord();
                    if (&crossing == word || ! crossing.Contains(&square))
                        continue;
                    if (is_word_filled(crossing))
                        return false;
                }
            }
            // If no filled words have been found, it might be because the
            // crossing word doesn't exist.  Create a crossing word and
            // check that.
            puz::GridDirection dir = puz::IsHorizontal(word->GetDirection())
                                        ? puz::DOWN : puz::ACROSS;
            puz::Word crossing(square.GetWordStart(dir),
                               square.GetWordEnd(dir));
            // If this is the same as the given word, don't bother.
            if (crossing.front() == word->front()
                && crossing.back() == word->back())
            {
                return true;
            }
            if (is_word_filled(crossing))
                return false;
            // Otherwise this square is uncrossed
            return true;
        }

        bool is_word_filled(puz::Word & word)
        {
            // Check to see that all squares in this word are filled.
            for (puz::square_iterator square = word.begin();
                 square != word.end();
                 ++square)
            {
                if (square->IsBlank())
                    return false;
            }
            return true;
        }

        puz::Puzzle & m_puz;

    } is_uncrossed(m_puz);

    puz::Word * word = m_XGridCtrl->GetFocusedWord();
    puz::square_iterator square_it;
    for (square_it = word->begin(); square_it != word->end(); ++square_it)
    {
        puz::Square & square = *square_it;
        if (square.IsWhite() && is_uncrossed(square, word))
            m_XGridCtrl->SetSquareText(square, _T(""));
    }
    m_XGridCtrl->Refresh();
}


void
MyFrame::OnRebusEntry(wxCommandEvent & evt)
{
    if (evt.IsChecked())
        m_XGridCtrl->StartRebusEntry();
    else
        m_XGridCtrl->EndRebusEntry();
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
                    _T("Scrambling solution"),
                    0,
                    0,
                    9999);
    if (key < 0)
        return;

    if (key < 1000)
        key = 0;

    if (m_XGridCtrl->GetGrid()->ScrambleSolution(key))
    {
        XWordMessage(this, wxString::Format("Solution scrambled.  Key is %d",
                                            m_XGridCtrl->GetGrid()->GetKey()));

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
        XWordMessage(this, "Solution unscrambled!");

        CheckPuzzle();

        m_toolMgr.Enable(ID_SCRAMBLE,   true);
        m_toolMgr.Enable(ID_UNSCRAMBLE, false);

        EnableCheck(true);
        EnableReveal(true);
    }
    else
    {
        XWordMessage(this, "Wrong Key!");
    }
}



// Window Layout
//--------------
void
MyFrame::OnEditLayout(wxCommandEvent & evt)
{
#if USE_MY_AUI_MANAGER
    if (evt.IsChecked()) // Start Layout
    {
        m_mgr.StartEdit();
        m_XGridCtrl->DisconnectEvents();
    }
    else // End Layout
    {
        m_mgr.EndEdit();
        if (m_puz.IsOk())
            m_XGridCtrl->ConnectEvents();
    }
    m_mgr.Update();
#endif // USE_MY_AUI_MANAGER
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
    wxASSERT(nameArray.size() > 0);

    // Show the dialog
    LayoutDialog dlg(this,
                     _T("Choose a layout"),
                     _T("Load Layout"),
                     nameArray,
                     layoutArray);

    // If the dialog is canceled, load the current layout
    if (dlg.ShowModal() != wxID_OK)
        LoadPerspective(layoutArray.front(), true);

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
            // Set the notes bitmap depending on whether there are notes or not
            if (m_puz.GetNotes().empty())
                m_toolMgr.SetIconName(ID_SHOW_NOTES, _T("notes"));
            else
                m_toolMgr.SetIconName(ID_SHOW_NOTES, _T("notes_new"));
            break;
        case ID_REBUS_ENTRY:
            evt.Check(m_XGridCtrl->IsRebusEntry());
            break;
#if USE_MY_AUI_MANAGER
        case ID_EDIT_LAYOUT:
            evt.Check(m_mgr.IsEditing());
            break;
#endif // USE_MY_AUI_MANAGER
    }
}


// Timer
//------
void
MyFrame::StartTimer()
{
    m_timer.Start();
    m_toolMgr.Check(ID_TIMER);
    m_status->SetTime(m_time);
}

void
MyFrame::StopTimer()
{
    m_timer.Stop();
    m_toolMgr.Check(ID_TIMER, false);
    m_status->SetTime(m_time, _T(" (Paused)"));
}

void
MyFrame::OnTimer(wxCommandEvent & evt)
{
    wxASSERT(evt.IsChecked() != IsTimerRunning());
    ToggleTimer();
}

void
MyFrame::OnResetTimer(wxCommandEvent & evt)
{
    SetTime(0);
}


void
MyFrame::OnTimerNotify(wxTimerEvent & WXUNUSED(evt))
{
    // Check to see if somebody is active.
    // Calling SetPaused is already done in OnApp(De)Activate, but in case that
    // doesn't work, we'll do it here too.
    if (wxGetApp().IsActive() && !IsIconized())
    {
        SetTime(m_time+1);
        m_XGridCtrl->SetPaused(false);
    }
    else
    {
        m_XGridCtrl->SetPaused(true);
    }
}

// Status bar
//-----------
void
MyFrame::SetShowCompletionStatus(bool show)
{
    m_showCompletionStatus = show;
    if (m_puz.IsOk())
    {
        CheckPuzzle();
    }
}

// AutoSave
//---------
void
MyFrame::OnAutoSaveNotify(wxTimerEvent & WXUNUSED(evt))
{
    wxLogDebug(_T("AutoSave: %s"), m_filename.c_str());
    if (puz::Puzzle::CanSave(puz::encode_utf8(wx2puz(m_filename)))
        && wxFileName::IsFileWritable(m_filename))
    {
        try {
            DoSavePuzzle(m_filename);
        } catch (...) {
            wxLogDebug(_T("AutoSave failed"));
            SetStatus(_T("Auto save faield"));
            // If it doesn't work, don't show a message box
        }
    }
}

// Preferences
//------------
void
MyFrame::OnPreferences(wxCommandEvent & WXUNUSED(evt))
{
    PreferencesDialog::ShowDialog(this);
}



// Help
//------------

#ifdef __WXMSW__
void
MyFrame::OnHelp(wxCommandEvent & WXUNUSED(evt))
{
    wxGetApp().ShowHelp();
}
#endif // __WXMSW__

void
MyFrame::OnAbout(wxCommandEvent & WXUNUSED(evt))
{
    wxAboutDialogInfo info;
    info.SetName(XWORD_APP_NAME);

    info.SetVersion(_T(XWORD_VERSION_STRING) _T(" (") __TDATE__ _T(")"));

    info.SetCopyright(XWORD_COPYRIGHT_STRING);

    info.SetDescription(_T("https://github.com/mrichards42/xword"));
    wxAboutBox(info);
}

void
MyFrame::OnLicense(wxCommandEvent & WXUNUSED(evt))
{
    const wxString licenseText(
        XWORD_APP_NAME _T(" ") _T(XWORD_VERSION_STRING) _T("\n")
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


bool
MyFrame::Print(const PrintInfo & info, puz::Puzzle * puz, bool prompt)
{
    if (puz == NULL)
        puz = &m_puz;

    wxPrintDialogData printDialogData(*g_printData);

    wxPrinter printer(& printDialogData);
    MyPrintout printout(this, puz, info);
    if (!printer.Print(this, &printout, prompt))
    {
        if (prompt && wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox(_T("There was a problem printing.\n")
                         _T("Perhaps your current printer is not set correctly?"),
                         _T("Printing"),
                         wxOK);
        return false;
    }
    else
    {
        *g_printData = printer.GetPrintDialogData().GetPrintData();
        return true;
    }
}

void
MyFrame::PrintPreview(const PrintInfo & info, puz::Puzzle * puz)
{
    if (puz == NULL)
        puz = &m_puz;

    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(*g_printData);
    wxPrintPreview * preview = new wxPrintPreview(
        new MyPrintout(this, puz, info),
        new MyPrintout(this, puz, info),
        &printDialogData
    );
    if (! preview->Ok())
    {
        delete preview;
        wxMessageBox(_T("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _T("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
                                               _T("Print Preview"));
    frame->Centre();
    frame->Initialize();
    frame->Show();
}

void
MyFrame::OnPrintBlank(wxCommandEvent & WXUNUSED(evt))
{
    PrintInfo info;
    if (! m_puz.IsDiagramless())
        info.grid_options = XGridDrawer::DRAW_NUMBER;
    else
        info.grid_options = XGridDrawer::DRAW_BLANK_DIAGRAMLESS;
    Print(info);
}

void
MyFrame::OnPrintTwoPages(wxCommandEvent & WXUNUSED(evt))
{
    PrintInfo info;
    info.two_pages = true;
    if (! m_puz.IsDiagramless())
        info.grid_options = XGridDrawer::DRAW_NUMBER;
    else
        info.grid_options = XGridDrawer::DRAW_BLANK_DIAGRAMLESS;
    Print(info);
}

void
MyFrame::OnPrintCurrent(wxCommandEvent & WXUNUSED(evt))
{
    PrintInfo info;
    if (! m_puz.IsDiagramless())
        info.grid_options = XGridDrawer::DRAW_USER_TEXT
                            | XGridDrawer::DRAW_NUMBER;
    else
        info.grid_options = XGridDrawer::DRAW_USER_TEXT;
    Print(info);
}

void
MyFrame::OnPrintSolution(wxCommandEvent & WXUNUSED(evt))
{
    PrintInfo info;
    info.grid_options = XGridDrawer::DRAW_SOLUTION;
    info.clues = false;
    Print(info);
}

void
MyFrame::OnPrintCustom(wxCommandEvent & WXUNUSED(evt))
{
    CustomPrintDialog dlg(this);
    int code = dlg.ShowModal();
    if (code == CustomPrintDialog::ID_PRINT)
        Print(dlg.GetPrintInfo());
    else if (code == CustomPrintDialog::ID_PREVIEW)
        PrintPreview(dlg.GetPrintInfo());
}


//------------------------------------------------------------------------------
// XGridCtrl and CluePanel events
//------------------------------------------------------------------------------

void
MyFrame::OnGridFocus(wxPuzEvent & evt)
{
    // Update clue lists and clue prompt
    UpdateClues();
    evt.Skip();
}


void
MyFrame::OnClueFocus(wxPuzEvent & evt)
{
    puz::Clue * focusedClue = evt.GetClue();
    m_XGridCtrl->SetFocusedWord(&focusedClue->GetWord());
    UpdateClues();
    m_XGridCtrl->SetFocus();
    evt.Skip();
}

void
MyFrame::UpdateClues()
{
    const puz::Square * focusedSquare = GetFocusedSquare();
    const puz::Word * focusedWord = GetFocusedWord();
    const puz::Clue * focusedClue = NULL;

    if (! m_puz.IsDiagramless())
    {
        puz::Clues::const_iterator it;
        for (it = m_puz.GetClues().begin(); it != m_puz.GetClues().end(); ++it)
        {
            CluePanel * panel = m_clues[puz2wx(it->first)];
            const puz::ClueList & cluelist = it->second;
            if (focusedClue == NULL)
            {
                // Look for focusedWord in this clue list; if it is not in the list,
                // look for a word with focusedSquare.
                const puz::Clue * foundClue = NULL;
                puz::ClueList::const_iterator clues_it;
                for (clues_it = cluelist.begin(); clues_it != cluelist.end(); ++clues_it)
                {
                    const puz::Clue * clue = &*clues_it;
                    const puz::Word * word = &clue->GetWord();
                    if (word == focusedWord)
                    {
                        focusedClue = clue;
                        panel->SetClue(focusedClue, CluePanel::FOCUSED);
                        break;
                    }
                    else if (foundClue == NULL)
                        if (word->Contains(focusedSquare))
                            foundClue = clue;
                }
                if (! focusedClue)
                    panel->SetClue(foundClue, CluePanel::CROSSING);
            }
            else // We have already set focusedClue, so this will be a crossing clue.
            {
                panel->SetClue(cluelist.Find(focusedSquare), CluePanel::CROSSING);
            }
        }
    }
    else // diagramless
    {
        const short direction = GetFocusedDirection();
        const puz::GridDirection crossing_direction =
            puz::IsVertical(direction) ? puz::ACROSS : puz::DOWN;
        // The first letter in the crossing word
        const puz::Square * crossingSquare =
            focusedSquare->GetWordStart(crossing_direction);

        puz::Clues::const_iterator it;
        for (it = m_puz.GetClues().begin(); it != m_puz.GetClues().end(); ++it)
        {
            CluePanel * panel = m_clues[puz2wx(it->first)];
            const puz::ClueList & cluelist = it->second;
            puz::ClueList::const_iterator clues_it;
            // Figure out if this clue is focused or crossing
            const bool is_focused =
                (direction == puz::ACROSS &&
                 cluelist.GetTitle() == puzT("Across")) ||
                (direction == puz::DOWN &&
                 cluelist.GetTitle() == puzT("Down"));

            // Find the clue
            for (clues_it = cluelist.begin(); clues_it != cluelist.end(); ++clues_it)
            {
                const puz::Clue * clue = &*clues_it;
                if (is_focused
                    && clue->GetNumber() == focusedWord->front()->GetNumber())
                {
                    panel->SetClue(clue, CluePanel::FOCUSED);
                    break;
                }
                else if (! is_focused && crossingSquare
                         && clue->GetNumber() == crossingSquare->GetNumber())
                {
                    panel->SetClue(clue, CluePanel::CROSSING);
                    break;
                }
            }
            if (clues_it == cluelist.end())
                panel->SetClue(NULL, is_focused ? CluePanel::FOCUSED : CluePanel::CROSSING);
        }
    }
    m_cluePrompt->UpdateLabel();
}

void
MyFrame::OnGridLetter(wxPuzEvent & evt)
{
    // Change the save/save as button
    if (! m_isModified)
    {
        m_isModified = true;
        EnableSave();
    }
    CheckPuzzle();
    // Auto Save
    if (m_autoSaveInterval > 0)
    {
        wxLogDebug(_T("AutoSave in %d seconds"), m_autoSaveInterval);
        m_autoSaveTimer.Start(m_autoSaveInterval * 1000, wxTIMER_ONE_SHOT);
    }
    evt.Skip();
}



//------------------------------------------------------------------------------
// Frame events
//------------------------------------------------------------------------------

// This used to be where the timer would be started and stopped.
// Now this is handled in OnTimerNotify.
// Since OnTimerNotify only happens every second, we still display the
// "(Paused)" message here because the delay would be noticeable.
void
MyFrame::OnAppActivate()
{
    if (m_toolMgr.IsChecked(ID_TIMER))
        m_XGridCtrl->SetPaused(false);
}


void
MyFrame::OnAppDeactivate()
{
    if (m_toolMgr.IsChecked(ID_TIMER))
        m_XGridCtrl->SetPaused(true);
}



void
MyFrame::OnClose(wxCloseEvent & evt)
{
    if (ClosePuzzle() || ! evt.CanVeto())
    {
        SaveLayout(_T("(Previous)"));
        SaveConfig();
        Disconnect(wxEVT_SIZE, wxSizeEventHandler(MyFrame::OnSize));
        // Hide all the top level windows
        wxWindowList::iterator it;
        wxWindowList::iterator begin = wxTopLevelWindows.begin();
        wxWindowList::iterator end   = wxTopLevelWindows.end();
        for (it = begin; it != end; ++it)
            (*it)->Hide();

#if XWORD_USE_LUA
        // cleanup xword package
        wxGetApp().GetwxLuaState().RunFile(GetScriptsDir() + "/xword/cleanup.lua");
#endif

        // Close the clipboard
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->Flush();
            wxTheClipboard->Close();
        }
        Destroy();
        return;
    }
    else
    {
        evt.Veto();
    }
}


void
MyFrame::OnLinkClicked(wxHtmlLinkEvent & evt)
{
    wxLaunchDefaultBrowser(evt.GetLinkInfo().GetHref());
}

void
MyFrame::OnIconize(wxIconizeEvent & evt)
{
    if (evt.IsIconized()) {
        OnAppDeactivate();
    } else {
        OnAppActivate();
    }
}


//------------------------------------------------------------------------------
// Debug stuff
//------------------------------------------------------------------------------

#ifdef _DEBUG

void
MyFrame::ShowDebugDialog(const wxString & title, const wxString & str)
{
    wxDialog dlg(this, wxID_ANY,
                 wxString::Format(_T("Debug: %s"), (const wxChar *)title.c_str()),
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

    puz::Grid * grid = &m_puz.GetGrid();

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

bool isVowel(char x) {
    return x == 'A' || x == 'E' || x == 'I' || x == 'O' || x == 'U';
}

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
    puz::Scrambler scrambler(m_puz.GetGrid());
    unsigned short key = 0;
    int maxCandidateVowels = 0;
    wxStopWatch sw;
    dlg->StartTimer();
    for (unsigned short i = 1000; i <= 9999; ++i)
    {
        wxTheApp->Yield(); // Don't block the GUI.
        // Only update every 100 keys so rendering doesn't slow down the unscramble process.
        if (i % 100 == 0) {
            dlg->SetKey(i);
        }
        std::string solution = scrambler.GetUnscrambledSolution(i);
        if (!solution.empty())
        {
            // Some puzzles have more than one key which "unlocks" the puzzle, but only one key is
            // valid, whereas the rest produce gibberish. As a rough heuristic, take the key whose
            // resulting solution has the highest number of vowels.
            int candidateVowels = std::count_if(solution.begin(), solution.end(), isVowel);
            wxLogDebug(_T("Candidate key %d has %d vowels"), i, candidateVowels);
            if (candidateVowels >= maxCandidateVowels) {
                maxCandidateVowels = candidateVowels;
                key = i;
            }
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
        assert(scrambler.UnscrambleSolution(key));
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


#endif // _DEBUG
