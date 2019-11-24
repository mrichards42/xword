/////////////////////////////////////////////////////////////////////////////
// Name:        wxluasetup.h
// Purpose:     Control what wxLua bindings for wxWidgets are built
// Author:      John Labenski
// Created:     1/10/2008
// Copyright:   (c) 2008 John Labenski
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __WXLUA_SETUP__
#define __WXLUA_SETUP__


// Enable or disable single or small groups of classes, see bindings/*.i

// This file is separate from the wxbind includes to allow you to modify it
// or put a -Iother/path/to/wxluasetup/ to allow your own wxluasetup file
// to be included since wxLua only includes it as "#include "wxluasetup.h"
// without any path.
#define wxLUA_USE_Geometry                      1
#define wxLUA_USE_MDI                           0
#define wxLUA_USE_wxAboutDialog                 0
#define wxLUA_USE_wxAcceleratorTable            0
#define wxLUA_USE_wxAnimation                   0
#define wxLUA_USE_wxApp                         1
#define wxLUA_USE_wxArrayInt                    1
#define wxLUA_USE_wxArrayString                 1
#define wxLUA_USE_wxArtProvider                 0
#define wxLUA_USE_wxAUI                         0
#define wxLUA_USE_wxBitmap                      1
#define wxLUA_USE_wxBitmapComboBox              0
#define wxLUA_USE_wxBitmapButton                0
#define wxLUA_USE_wxBrushList                   1
#define wxLUA_USE_wxBusyCursor                  0
#define wxLUA_USE_wxBusyInfo                    0
#define wxLUA_USE_wxButton                      1
#define wxLUA_USE_wxCalendarCtrl                0
#define wxLUA_USE_wxCaret                       0
#define wxLUA_USE_wxCheckBox                    1
#define wxLUA_USE_wxCheckListBox                1
#define wxLUA_USE_wxChoice                      1
#define wxLUA_USE_wxClassInfo                   1
#define wxLUA_USE_wxClipboard                   0
#define wxLUA_USE_wxCollapsiblePane             0
#define wxLUA_USE_wxColourDialog                0
#define wxLUA_USE_wxColourPenBrush              0
#define wxLUA_USE_wxColourPickerCtrl            0
#define wxLUA_USE_wxComboBox                    1
#define wxLUA_USE_wxCommandProcessor            0
#define wxLUA_USE_wxConfig                      0
#define wxLUA_USE_wxCursor                      0
#define wxLUA_USE_wxCriticalSection             0
#define wxLUA_USE_wxCriticalSectionLocker       0
#define wxLUA_USE_wxDataObject                  0
#define wxLUA_USE_wxDatePickerCtrl              0
#define wxLUA_USE_wxTimePickerCtrl              0
#define wxLUA_USE_wxDateSpan                    0
#define wxLUA_USE_wxDateTime                    1
#define wxLUA_USE_wxDateTimeHolidayAuthority    0
#define wxLUA_USE_wxDC                          1
#define wxLUA_USE_wxDialog                      1
#define wxLUA_USE_wxDir                         1
#define wxLUA_USE_wxDirDialog                   1
#define wxLUA_USE_wxDirPickerCtrl               1
#define wxLUA_USE_wxDisplay                     0
#define wxLUA_USE_wxDragDrop                    0
#define wxLUA_USE_wxDynamicLibrary              0
#define wxLUA_USE_wxFile                        0
#define wxLUA_USE_wxFileDialog                  1
#define wxLUA_USE_wxFileHistory                 0
#define wxLUA_USE_wxFileName                    1
#define wxLUA_USE_wxFilePickerCtrl              1
#define wxLUA_USE_wxFindReplaceDialog           0
#define wxLUA_USE_wxFont                        1
#define wxLUA_USE_wxFontDialog                  1
#define wxLUA_USE_wxFontEnumerator              0
#define wxLUA_USE_wxFontList                    0
#define wxLUA_USE_wxFontMapper                  0
#define wxLUA_USE_wxFontPickerCtrl              1
#define wxLUA_USE_wxFrame                       1
#define wxLUA_USE_wxGauge                       0
#define wxLUA_USE_wxGenericDirCtrl              0
#define wxLUA_USE_wxGenericValidator            0
#define wxLUA_USE_wxGLCanvas                    0 // must link to lib, also wxUSE_GLCANVAS
#define wxLUA_USE_wxGrid                        0
#define wxLUA_USE_wxHashTable                   0
#define wxLUA_USE_wxHelpController              0
#define wxLUA_USE_wxHTML                        0
#define wxLUA_USE_wxHtmlHelpController          0
#define wxLUA_USE_wxHyperlinkCtrl               0
#define wxLUA_USE_wxIcon                        1
#define wxLUA_USE_wxID_XXX                      1
#define wxLUA_USE_wxImage                       1
#define wxLUA_USE_wxImageList                   1
#define wxLUA_USE_wxJoystick                    0
#define wxLUA_USE_wxLayoutConstraints           0
#define wxLUA_USE_wxList                        1
#define wxLUA_USE_wxListBox                     1
#define wxLUA_USE_wxListCtrl                    1
#define wxLUA_USE_wxLog                         0
#define wxLUA_USE_wxLogWindow                   0
#define wxLUA_USE_wxLuaHtmlWindow               0
#define wxLUA_USE_wxLuaPrintout                 0
#define wxLUA_USE_wxMask                        0
#define wxLUA_USE_wxMediaCtrl                   0 // must link to lib, also wxUSE_MEDIACTRL
#define wxLUA_USE_wxMenu                        1
#define wxLUA_USE_wxMessageDialog               1
#define wxLUA_USE_wxMetafile                    0
#define wxLUA_USE_wxMiniFrame                   0
#define wxLUA_USE_wxMultiChoiceDialog           0
#define wxLUA_USE_wxNotebook                    1
#define wxLUA_USE_wxObject                      1
#define wxLUA_USE_wxPicker                      1
#define wxLUA_USE_wxPalette                     0
#define wxLUA_USE_wxPenList                     1
#define wxLUA_USE_wxPointSizeRect               1
#define wxLUA_USE_wxPrint                       0
#define wxLUA_USE_wxProcess                     0
#define wxLUA_USE_wxProgressDialog              0
#define wxLUA_USE_wxRadioBox                    1
#define wxLUA_USE_wxRadioButton                 1
#define wxLUA_USE_wxRegEx                       0
#define wxLUA_USE_wxRegion                      0
#define wxLUA_USE_wxRenderer                    0
#define wxLUA_USE_wxRichText                    0 // NOT FINISHED or WORKING
#define wxLUA_USE_wxSashWindow                  0
#define wxLUA_USE_wxScrollBar                   1
#define wxLUA_USE_wxScrolledWindow              1
#define wxLUA_USE_wxSingleChoiceDialog          1
#define wxLUA_USE_wxSizer                       1
#define wxLUA_USE_wxSlider                      1
#define wxLUA_USE_wxSocket                      0
#define wxLUA_USE_wxSpinButton                  1
#define wxLUA_USE_wxSpinCtrl                    1
#define wxLUA_USE_wxSpinCtrlDouble              1
#define wxLUA_USE_wxSplashScreen                0
#define wxLUA_USE_wxSplitterWindow              0
#define wxLUA_USE_wxStandardPaths               0
#define wxLUA_USE_wxStaticBitmap                1
#define wxLUA_USE_wxStaticBox                   1
#define wxLUA_USE_wxStaticLine                  1
#define wxLUA_USE_wxStaticText                  1
#define wxLUA_USE_wxStatusBar                   1
#define wxLUA_USE_wxStopWatch                   0
#define wxLUA_USE_wxStringList                  1
#define wxLUA_USE_wxSystemOptions               1
#define wxLUA_USE_wxSystemSettings              1
#define wxLUA_USE_wxTabCtrl                     0 // deprecated MSW only control
#define wxLUA_USE_wxTaskBarIcon                 1
#define wxLUA_USE_wxTextCtrl                    1
#define wxLUA_USE_wxTextEntryDialog             0
#define wxLUA_USE_wxTextValidator               0
#define wxLUA_USE_wxTimer                       1
#define wxLUA_USE_wxTimeSpan                    0
#define wxLUA_USE_wxToggleButton                1
#define wxLUA_USE_wxToolbar                     1
#define wxLUA_USE_wxToolbook                    0
#define wxLUA_USE_wxTooltip                     0
#define wxLUA_USE_wxTreebook                    0
#define wxLUA_USE_wxTreeCtrl                    1
#define wxLUA_USE_wxTreeListCtrl                1
#define wxLUA_USE_wxValidator                   0
#define wxLUA_USE_wxWave                        0
#define wxLUA_USE_wxWindowList                  1
#define wxLUA_USE_wxWizard                      0
#define wxLUA_USE_wxXML                         1
#define wxLUA_USE_wxXRC                         0

#endif // __WXLUA_SETUP__
