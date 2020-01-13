-- Converted from python by py2lua
-- python file: wxFB.py
-- modtime: 1571291120
-- md5sum: ecc728a0399aad746ee6608735de0837
-- ----------------------------------

local wxfb = {} -- Table to hold classes

local require_bmp = require("wx.lib.bmp") -- require function for images

---------------------------------------------------------------------------
-- Python code generated with wxFormBuilder (version Oct 10 2016)
-- http://www.wxformbuilder.org/
--
-- PLEASE DO "NOT" EDIT THIS FILE!
---------------------------------------------------------------------------

local BmpButton = require("wx.lib.bmp_button")
local TextButton = require("wx.lib.text_button")
local ArrowButton = require("wx.lib.arrow_button")
local CheckListCtrl = require("wx.lib.checklist")
local ControlGrid = require("wx.lib.ctrlgrid")

---------------------------------------------------------------------------
-- Class DownloadDialog
---------------------------------------------------------------------------

function wxfb.DownloadDialog(parent)

    local self = wx.wxDialog ( parent, wx.wxID_ANY, "Downloader", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDEFAULT_DIALOG_STYLE+wx.wxRESIZE_BORDER+wx.wxCLIP_CHILDREN )
    
    self:SetSizeHints( wx.wxDefaultSize, wx.wxDefaultSize )
    
    local bSizer5 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.splitter = wx.wxSplitterWindow( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxSP_3D )

    
    self.panel = wx.wxPanel( self.splitter, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    self.panel:SetBackgroundColour( wx.wxColour( 255, 255, 255 ) )
    
    local bSizer13 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    local bSizer1 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer2 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.prev_week = BmpButton( self.panel, wx.wxID_ANY, require_bmp("download.images.prev_week"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.prev_week:SetToolTip( "Previous Week" )
    
    bSizer2:Add( self.prev_week, 0, wx.wxRIGHT+wx.wxALIGN_CENTER_VERTICAL, 10 )
    
    self.prev = BmpButton( self.panel, wx.wxID_ANY, require_bmp("download.images.prev"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.prev:SetToolTip( "Previous Day" )
    
    bSizer2:Add( self.prev, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.label = TextButton( self.panel, wx.wxID_ANY, "Label", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.label:SetFont( wx.wxFont( 12, wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.label:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.label:SetToolTip( "Download missing puzzles" )
    
    bSizer2:Add( self.label, 1, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.next = BmpButton( self.panel, wx.wxID_ANY, require_bmp("download.images.next"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.next:SetToolTip( "Next Day" )
    
    bSizer2:Add( self.next, 0, wx.wxALIGN_CENTER_VERTICAL, 10 )
    
    self.next_week = BmpButton( self.panel, wx.wxID_ANY, require_bmp("download.images.next_week"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.next_week:SetToolTip( "Next Week" )
    
    bSizer2:Add( self.next_week, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxLEFT, 10 )
    
    
    bSizer1:Add( bSizer2, 0, wx.wxEXPAND+wx.wxALL, 5 )
    
    local bSizer3 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    local kindChoices = { "Day", "Week", "Month" }
    self.kind = wx.wxChoice( self.panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize( 80,-1 ), kindChoices, 0 )
    self.kind:SetSelection( 0 )
    bSizer3:Add( self.kind, 0, 0, 5 )
    
    
    bSizer3:Add(0, 0, 1)
    
    local queueCountSizer = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.queue_count = TextButton( self.panel, wx.wxID_ANY, "0", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.queue_count:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.queue_count:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.queue_count:SetToolTip( "Download queue" )
    
    queueCountSizer:Add( self.queue_count, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.queueBmp = wx.wxStaticBitmap( self.panel, wx.wxID_ANY, require_bmp("download.images.down"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    queueCountSizer:Add( self.queueBmp, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer3:Add( queueCountSizer, 0, wx.wxEXPAND+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    local errorCountSizer = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.error_count = TextButton( self.panel, wx.wxID_ANY, "0", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.error_count:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.error_count:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.error_count:SetToolTip( "Download missing puzzles" )
    
    errorCountSizer:Add( self.error_count, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.errorBmp = wx.wxStaticBitmap( self.panel, wx.wxID_ANY, require_bmp("download.images.error"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    errorCountSizer:Add( self.errorBmp, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer3:Add( errorCountSizer, 0, wx.wxEXPAND, 5 )
    
    
    bSizer3:Add(0, 0, 1)
    
    self.date = wx.wxDatePickerCtrl( self.panel, wx.wxID_ANY, wx.wxDefaultDateTime, wx.wxDefaultPosition, wx.wxSize( 80,-1 ), wx.wxDP_DROPDOWN+wx.wxDP_SHOWCENTURY )
    bSizer3:Add( self.date, 0, 0, 5 )
    
    
    bSizer1:Add( bSizer3, 0, wx.wxEXPAND+wx.wxBOTTOM+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    self.scroller = wx.wxScrolledWindow( self.panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDOUBLE_BORDER+wx.wxHSCROLL+wx.wxVSCROLL )
    self.scroller:SetScrollRate( 10, 10 )
    local bSizer8 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    
    self.scroller:SetSizer( bSizer8 )
    self.scroller:Layout()
    bSizer8:Fit( self.scroller )
    bSizer1:Add( self.scroller, 1, wx.wxEXPAND, 5 )
    
    
    bSizer13:Add( bSizer1, 1, wx.wxEXPAND, 5 )
    
    
    self.panel:SetSizer( bSizer13 )
    self.panel:Layout()
    bSizer13:Fit( self.panel )
    self.splitter:Initialize( self.panel )
    bSizer5:Add( self.splitter, 1, wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer5 )
    self:Layout()
    bSizer5:Fit( self )
    
    self:Centre( wx.wxBOTH )
    
    -- Connect Events
    self:Connect(wx.wxEVT_CLOSE_WINDOW, function(evt) self:OnClose(evt) end)
    self.prev_week:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnPrevWeek(evt) end)
    self.prev:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnPrev(evt) end)
    self.label:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnDownloadMissing(evt) end)
    self.next:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnNext(evt) end)
    self.next_week:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnNextWeek(evt) end)
    self.kind:Connect(wx.wxEVT_COMMAND_CHOICE_SELECTED, function(evt) self:OnKindChanged(evt) end)
    self.queue_count:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnQueueClick(evt) end)
    self.error_count:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnErrorClick(evt) end)
    self.date:Connect(wx.wxEVT_DATE_CHANGED, function(evt) self:OnDateChanged(evt) end)

    return self
end

---------------------------------------------------------------------------
-- Class DownloadHeading
---------------------------------------------------------------------------

function wxfb.DownloadHeading(parent)

    local self = wx.wxPanel ( parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize( -1,-1 ), wx.wxTAB_TRAVERSAL )
    
    local bSizer7 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.m_panel4 = wx.wxPanel( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    self.m_panel4:SetBackgroundColour( wx.wxColour( 240, 240, 240 ) )
    
    local bSizer11 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer8 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.expand_button = ArrowButton( self.m_panel4, wx.wxID_ANY, ">>", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    bSizer8:Add( self.expand_button, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.label = TextButton( self.m_panel4, wx.wxID_ANY, "Source", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.label:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    
    bSizer8:Add( self.label, 1, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.download_button = TextButton( self.m_panel4, wx.wxID_ANY, "0 Puzzles", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.download_button:SetToolTip( "Download missing puzzles" )
    
    bSizer8:Add( self.download_button, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer11:Add( bSizer8, 1, wx.wxEXPAND+wx.wxALL, 2 )
    
    
    self.m_panel4:SetSizer( bSizer11 )
    self.m_panel4:Layout()
    bSizer11:Fit( self.m_panel4 )
    bSizer7:Add( self.m_panel4, 1, wx.wxEXPAND, 5 )
    
    self.m_staticline5 = wx.wxStaticLine( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLI_HORIZONTAL )
    bSizer7:Add( self.m_staticline5, 0, wx.wxEXPAND, 5 )
    
    self.line = wx.wxStaticLine( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLI_HORIZONTAL )
    bSizer7:Add( self.line, 0, wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer7 )
    self:Layout()
    bSizer7:Fit( self )
    
    -- Connect Events
    self.expand_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnExpand(evt) end)
    self.label:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnLabelClick(evt) end)
    self.download_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnDownloadMissing(evt) end)

    return self
end

---------------------------------------------------------------------------
-- Class DownloadList
---------------------------------------------------------------------------

function wxfb.DownloadList(parent)

    local self = wx.wxPanel ( parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize( -1,-1 ), wx.wxTAB_TRAVERSAL )
    
    local bSizer10 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer13 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.label = wx.wxStaticText( self, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.label:Wrap( -1 )
    bSizer13:Add( self.label, 0, wx.wxALL, 5 )
    
    
    bSizer13:Add(0, 0, 1)
    
    self.erase_history = TextButton( self, wx.wxID_ANY, "Clear History", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.erase_history:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.erase_history:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    
    bSizer13:Add( self.erase_history, 0, wx.wxALL, 5 )
    
    self.cancel_downloads = TextButton( self, wx.wxID_ANY, "Cancel Downloads", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.cancel_downloads:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.cancel_downloads:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    
    bSizer13:Add( self.cancel_downloads, 0, wx.wxALL, 5 )
    
    self.close = TextButton( self, wx.wxID_ANY, "X", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.close:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.close:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.close:SetToolTip( "Close" )
    
    bSizer13:Add( self.close, 0, wx.wxALL, 5 )
    
    
    bSizer10:Add( bSizer13, 0, wx.wxEXPAND, 5 )
    
    self.list = wx.wxLuaListCtrl( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLC_REPORT+wx.wxLC_VIRTUAL )
    bSizer10:Add( self.list, 1, wx.wxALL+wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer10 )
    self:Layout()
    bSizer10:Fit( self )
    
    -- Connect Events
    self.erase_history:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnClearHistory(evt) end)
    self.cancel_downloads:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnCancelDownloads(evt) end)
    self.close:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnClose(evt) end)
    self.list:Connect(wx.wxEVT_COMMAND_LIST_COL_CLICK, function(evt) self:OnHeaderClick(evt) end)

    return self
end

---------------------------------------------------------------------------
-- Class PromptDialog
---------------------------------------------------------------------------

function wxfb.PromptDialog(parent)

    local self = wx.wxDialog ( parent, wx.wxID_ANY, "XWord Message", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDEFAULT_DIALOG_STYLE )
    
    self:SetSizeHints( wx.wxDefaultSize, wx.wxDefaultSize )
    
    local bSizer15 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer14 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.message = wx.wxStaticText( self, wx.wxID_ANY, "Message", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxALIGN_LEFT )
    self.message:Wrap( -1 )
    bSizer14:Add( self.message, 0, wx.wxBOTTOM, 10 )
    
    local fgSizer1 = wx.wxFlexGridSizer( 0, 2, 10, 10 )
    fgSizer1:AddGrowableCol( 1 )
    fgSizer1:SetFlexibleDirection( wx.wxBOTH )
    fgSizer1:SetNonFlexibleGrowMode( wx.wxFLEX_GROWMODE_SPECIFIED )
    
    
    bSizer14:Add( fgSizer1, 1, wx.wxEXPAND+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    self.m_staticline3 = wx.wxStaticLine( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLI_HORIZONTAL )
    bSizer14:Add( self.m_staticline3, 0, wx.wxEXPAND+wx.wxTOP+wx.wxBOTTOM, 10 )
    
    local buttons = wx.wxStdDialogButtonSizer()
    self.buttonsOK = wx.wxButton( self, wx.wxID_OK, "" )
    buttons:AddButton( self.buttonsOK )
    self.buttonsCancel = wx.wxButton( self, wx.wxID_CANCEL, "" )
    buttons:AddButton( self.buttonsCancel )
    buttons:Realize();
    
    bSizer14:Add( buttons, 0, wx.wxEXPAND+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    
    bSizer15:Add( bSizer14, 1, wx.wxEXPAND+wx.wxALL, 10 )
    
    
    self:SetSizer( bSizer15 )
    self:Layout()
    bSizer15:Fit( self )
    
    self:Centre( wx.wxBOTH )

    return self
end
---------------------------------------------------------------------------
-- Class DownloadDialog2
---------------------------------------------------------------------------

function wxfb.DownloadDialog2(parent)

    local self = wx.wxDialog ( parent, wx.wxID_ANY, "Puzzle Downloader", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDEFAULT_DIALOG_STYLE+wx.wxRESIZE_BORDER )
    
    self:SetSizeHints( wx.wxDefaultSize, wx.wxDefaultSize )
    
    local bSizer16 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.tree = wx.wxTreeCtrl( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTR_FULL_ROW_HIGHLIGHT+wx.wxTR_HIDE_ROOT+wx.wxTR_NO_BUTTONS+wx.wxTR_NO_LINES+wx.wxTR_SINGLE )
    bSizer16:Add( self.tree, 0, wx.wxALL+wx.wxEXPAND, 5 )
    
    local bSizer20 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.scroller = wx.wxScrolledWindow( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDOUBLE_BORDER+wx.wxHSCROLL+wx.wxVSCROLL )
    self.scroller:SetScrollRate( 10, 10 )
    local bSizer8 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    
    self.scroller:SetSizer( bSizer8 )
    self.scroller:Layout()
    bSizer8:Fit( self.scroller )
    bSizer20:Add( self.scroller, 1, wx.wxEXPAND +wx.wxALL, 5 )
    
    
    bSizer16:Add( bSizer20, 1, wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer16 )
    self:Layout()
    bSizer16:Fit( self )
    
    self:Centre( wx.wxBOTH )
    
    -- Connect Events
    self.tree:Connect(wx.wxEVT_COMMAND_TREE_SEL_CHANGED, function(evt) self:OnTreeSelect(evt) end)

    return self
end

---------------------------------------------------------------------------
-- Class PreferencesPanel
---------------------------------------------------------------------------

function wxfb.PreferencesPanel(parent)

    local self = wx.wxPanel ( parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize( 500,300 ), wx.wxTAB_TRAVERSAL )
    
    local bSizer19 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer231 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.source_list = CheckListCtrl( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLC_REPORT+wx.wxLC_SINGLE_SEL )
    bSizer231:Add( self.source_list, 1, wx.wxALL+wx.wxEXPAND, 5 )
    
    local bSizer24 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.new_button = wx.wxButton( self, wx.wxID_ANY, "&New", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    bSizer24:Add( self.new_button, 0, wx.wxALL, 5 )
    
    self.edit_button = wx.wxButton( self, wx.wxID_ANY, "&Edit", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    bSizer24:Add( self.edit_button, 0, wx.wxALL, 5 )
    
    self.remove_button = wx.wxButton( self, wx.wxID_ANY, "&Remove", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    bSizer24:Add( self.remove_button, 0, wx.wxALL, 5 )
    
    
    bSizer231:Add( bSizer24, 0, wx.wxEXPAND, 5 )
    
    
    bSizer19:Add( bSizer231, 1, wx.wxEXPAND, 5 )
    
    local bSizer20 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer21 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.m_staticText3 = wx.wxStaticText( self, wx.wxID_ANY, "Puzzle directory", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText3:Wrap( -1 )
    bSizer21:Add( self.m_staticText3, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.puzzle_directory = wx.wxDirPickerCtrl( self, wx.wxID_ANY, "", "Select a folder", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDIRP_DEFAULT_STYLE+wx.wxDIRP_DIR_MUST_EXIST )
    bSizer21:Add( self.puzzle_directory, 1, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer20:Add( bSizer21, 0, wx.wxEXPAND, 5 )
    
    local bSizer23 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.separate_directories = wx.wxCheckBox( self, wx.wxID_ANY, "Download to separate directories (by puzzle)", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.separate_directories:SetValue(true) 
    bSizer23:Add( self.separate_directories, 0, wx.wxALL+wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer23:Add(0, 0, 1)
    
    self.m_staticText4 = wx.wxStaticText( self, wx.wxID_ANY, "Auto download", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText4:Wrap( -1 )
    bSizer23:Add( self.m_staticText4, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.auto_download = wx.wxSpinCtrl( self, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxSize( 50,-1 ), wx.wxSP_ARROW_KEYS, 0, 10, 0 )
    bSizer23:Add( self.auto_download, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    self.m_staticText6 = wx.wxStaticText( self, wx.wxID_ANY, "days of puzzles", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText6:Wrap( -1 )
    bSizer23:Add( self.m_staticText6, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer20:Add( bSizer23, 0, wx.wxEXPAND+wx.wxTOP, 5 )
    
    
    bSizer19:Add( bSizer20, 0, wx.wxEXPAND+wx.wxALL, 5 )
    
    
    self:SetSizer( bSizer19 )
    self:Layout()
    
    -- Connect Events
    self.source_list:Connect(wx.wxEVT_COMMAND_LIST_ITEM_ACTIVATED, function(evt) self:OnEditSource(evt) end)
    self.source_list:Connect(wx.wxEVT_COMMAND_LIST_ITEM_SELECTED, function(evt) self:OnSourceSelected(evt) end)
    self.new_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnNewSource(evt) end)
    self.edit_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnEditSource(evt) end)
    self.remove_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnRemoveSource(evt) end)

    return self
end

---------------------------------------------------------------------------
-- Class PuzzleSourceDialog
---------------------------------------------------------------------------

function wxfb.PuzzleSourceDialog(parent)

    local self = wx.wxDialog ( parent, wx.wxID_ANY, "Edit Puzzle Source", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDEFAULT_DIALOG_STYLE )
    
    self:SetSizeHints( wx.wxDefaultSize, wx.wxDefaultSize )
    
    local bSizer25 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.notebook = wx.wxNotebook( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_panel3 = wx.wxPanel( self.notebook, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    local bSizer26 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local fgSizer3 = wx.wxFlexGridSizer( 0, 2, 5, 5 )
    fgSizer3:AddGrowableCol( 1 )
    fgSizer3:SetFlexibleDirection( wx.wxBOTH )
    fgSizer3:SetNonFlexibleGrowMode( wx.wxFLEX_GROWMODE_SPECIFIED )
    
    self.m_staticText6 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Name:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText6:Wrap( -1 )
    fgSizer3:Add( self.m_staticText6, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.name = wx.wxTextCtrl( self.m_panel3, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    fgSizer3:Add( self.name, 0, wx.wxEXPAND, 5 )
    
    self.m_staticText7 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "URL:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText7:Wrap( -1 )
    fgSizer3:Add( self.m_staticText7, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.url = wx.wxTextCtrl( self.m_panel3, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.url:SetToolTip( "Full puzzle URL.  Use date format codes." )
    
    fgSizer3:Add( self.url, 0, wx.wxEXPAND, 5 )
    
    self.m_staticText8 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Local filename:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText8:Wrap( -1 )
    fgSizer3:Add( self.m_staticText8, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.filename = wx.wxTextCtrl( self.m_panel3, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.filename:SetToolTip( "Filename of the downloaded puzzle.  Use date format codes." )
    
    fgSizer3:Add( self.filename, 0, wx.wxEXPAND, 5 )
    
    self.m_staticText81 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Puzzle directory:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText81:Wrap( -1 )
    fgSizer3:Add( self.m_staticText81, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.directoryname = wx.wxTextCtrl( self.m_panel3, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.directoryname:SetToolTip( "Name of the subdirectory to save puzzles in.  Defaults to the source name." )
    
    fgSizer3:Add( self.directoryname, 0, wx.wxEXPAND, 5 )
    
    
    bSizer26:Add( fgSizer3, 1, wx.wxEXPAND+wx.wxTOP+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    local sbSizer1 = wx.wxStaticBoxSizer( wx.wxStaticBox( self.m_panel3, wx.wxID_ANY, "Days available" ), wx.wxHORIZONTAL )
    
    self.day1 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Mon", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day1, 0, wx.wxALL, 5 )
    
    self.day2 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Tue", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day2, 0, wx.wxALL, 5 )
    
    self.day3 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Wed", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day3, 0, wx.wxALL, 5 )
    
    self.day4 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Thu", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day4, 0, wx.wxALL, 5 )
    
    self.day5 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Fri", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day5, 0, wx.wxALL, 5 )
    
    self.day6 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Sat", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day6, 0, wx.wxALL, 5 )
    
    self.day7 = wx.wxCheckBox( sbSizer1:GetStaticBox(), wx.wxID_ANY, "Sun", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    sbSizer1:Add( self.day7, 0, wx.wxALL, 5 )
    
    
    bSizer26:Add( sbSizer1, 0, wx.wxEXPAND+wx.wxALL, 5 )
    
    self.m_staticText122 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Date format codes for puzzle URL and filename:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText122:Wrap( -1 )
    self.m_staticText122:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL, false, "" ) )
    
    bSizer26:Add( self.m_staticText122, 0, wx.wxTOP+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    local bSizer27 = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.m_staticText121 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "%Y\n%y\n%m\n%d", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText121:Wrap( -1 )
    bSizer27:Add( self.m_staticText121, 0, wx.wxALL, 5 )
    
    self.m_staticText12 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Year (4-digit)\nYear (2-digit)\nMonth\nDay", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText12:Wrap( -1 )
    bSizer27:Add( self.m_staticText12, 0, wx.wxEXPAND+wx.wxALL, 5 )
    
    
    bSizer27:Add(30, 0, 1)
    
    self.m_staticText123 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "%A\n%a\n%B\n%b", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText123:Wrap( -1 )
    bSizer27:Add( self.m_staticText123, 0, wx.wxALL, 5 )
    
    self.m_staticText1231 = wx.wxStaticText( self.m_panel3, wx.wxID_ANY, "Weekday name (full)\nWeekday name (abbr)\nMonth name (full)\nMonth name (abbr)", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText1231:Wrap( -1 )
    bSizer27:Add( self.m_staticText1231, 0, wx.wxALL, 5 )
    
    
    bSizer26:Add( bSizer27, 0, wx.wxALIGN_CENTER_HORIZONTAL, 5 )
    
    
    self.m_panel3:SetSizer( bSizer26 )
    self.m_panel3:Layout()
    bSizer26:Fit( self.m_panel3 )
    self.notebook:AddPage( self.m_panel3, "Basic Info", true )
    self.m_panel4 = wx.wxPanel( self.notebook, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    local bSizer28 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    local bSizer29 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.auth_required = wx.wxCheckBox( self.m_panel4, wx.wxID_ANY, "Authentication required", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    bSizer29:Add( self.auth_required, 0, wx.wxTOP+wx.wxBOTTOM, 5 )
    
    local fgSizer31 = wx.wxFlexGridSizer( 0, 2, 5, 5 )
    fgSizer31:AddGrowableCol( 1 )
    fgSizer31:SetFlexibleDirection( wx.wxBOTH )
    fgSizer31:SetNonFlexibleGrowMode( wx.wxFLEX_GROWMODE_SPECIFIED )
    
    self.m_staticText61 = wx.wxStaticText( self.m_panel4, wx.wxID_ANY, "Login URL:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText61:Wrap( -1 )
    fgSizer31:Add( self.m_staticText61, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.auth_url = wx.wxTextCtrl( self.m_panel4, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.auth_url:SetToolTip( "URL that contains an HTML login form." )
    
    fgSizer31:Add( self.auth_url, 0, wx.wxEXPAND, 5 )
    
    self.m_staticText71 = wx.wxStaticText( self.m_panel4, wx.wxID_ANY, "Username field:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText71:Wrap( -1 )
    fgSizer31:Add( self.m_staticText71, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.auth_user_id = wx.wxTextCtrl( self.m_panel4, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.auth_user_id:SetToolTip( "HTML name attribute of the username input field." )
    
    fgSizer31:Add( self.auth_user_id, 0, wx.wxEXPAND, 5 )
    
    self.m_staticText82 = wx.wxStaticText( self.m_panel4, wx.wxID_ANY, "Password field:", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText82:Wrap( -1 )
    fgSizer31:Add( self.m_staticText82, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    self.auth_password_id = wx.wxTextCtrl( self.m_panel4, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.auth_password_id:SetToolTip( "HTML name attribute of the password input field." )
    
    fgSizer31:Add( self.auth_password_id, 0, wx.wxEXPAND, 5 )
    
    
    bSizer29:Add( fgSizer31, 1, wx.wxEXPAND+wx.wxLEFT, 15 )
    
    self.m_staticText37 = wx.wxStaticText( self.m_panel4, wx.wxID_ANY, "Note: Authentication may not work for every source.  Login is attempted via an HTML form.  Any cookies set after a successful login are saved and used to download puzzles.", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText37:Wrap( 325 )
    self.m_staticText37:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_ITALIC, wx.wxFONTWEIGHT_NORMAL, false, "" ) )
    
    bSizer29:Add( self.m_staticText37, 0, wx.wxALL+wx.wxALIGN_CENTER_HORIZONTAL, 5 )
    
    
    bSizer28:Add( bSizer29, 1, wx.wxEXPAND+wx.wxALL, 5 )
    
    
    self.m_panel4:SetSizer( bSizer28 )
    self.m_panel4:Layout()
    bSizer28:Fit( self.m_panel4 )
    self.notebook:AddPage( self.m_panel4, "Authentication", false )
    self.m_panel6 = wx.wxPanel( self.notebook, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    local bSizer30 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.custom_func_message = wx.wxStaticText( self.m_panel6, wx.wxID_ANY, "This source uses a custom download function!  Not all changes you make in this dialog may have the desired effect.  Click to view the function.", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxALIGN_CENTRE )
    self.custom_func_message:Wrap( 250 )
    self.custom_func_message:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    self.custom_func_message:SetBackgroundColour( wx.wxColour( 255, 255, 0 ) )
    self.custom_func_message:Hide()
    
    bSizer30:Add( self.custom_func_message, 0, wx.wxALL+wx.wxEXPAND, 5 )
    
    self.not_puzzle = wx.wxCheckBox( self.m_panel6, wx.wxID_ANY, "Skip puzzle validation", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.not_puzzle:SetToolTip( "Check to allow downloading files that are not XWord puzzles (e.g. PDFs)" )
    
    bSizer30:Add( self.not_puzzle, 0, wx.wxALL, 5 )
    
    self.m_staticText22 = wx.wxStaticText( self.m_panel6, wx.wxID_ANY, "TODO: cURL options", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText22:Wrap( -1 )
    bSizer30:Add( self.m_staticText22, 0, wx.wxALL, 5 )
    
    self.curl_opts = ControlGrid( self.m_panel6, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    bSizer30:Add( self.curl_opts, 1, wx.wxEXPAND +wx.wxALL, 5 )
    
    
    self.m_panel6:SetSizer( bSizer30 )
    self.m_panel6:Layout()
    bSizer30:Fit( self.m_panel6 )
    self.notebook:AddPage( self.m_panel6, "Advanced", false )
    self.custom_func_panel = wx.wxPanel( self.notebook, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    local bSizer34 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.m_staticText21 = wx.wxStaticText( self.custom_func_panel, wx.wxID_ANY, "Don't edit this function unless you know what you're doing.", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText21:Wrap( -1 )
    self.m_staticText21:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_DEFAULT, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD, false, "" ) )
    
    bSizer34:Add( self.m_staticText21, 0, wx.wxALL, 5 )
    
    self.m_panel8 = wx.wxPanel( self.custom_func_panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxDOUBLE_BORDER )
    local bSizer32 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.m_staticText40 = wx.wxStaticText( self.m_panel8, wx.wxID_ANY, "function(puzzle)", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.m_staticText40:Wrap( -1 )
    self.m_staticText40:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_TELETYPE, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL, false, "" ) )
    self.m_staticText40:SetForegroundColour( wx.wxSystemSettings.GetColour( wx.wxSYS_COLOUR_GRAYTEXT ) )
    
    bSizer32:Add( self.m_staticText40, 0, wx.wxEXPAND, 5 )
    
    self.func = wx.wxTextCtrl( self.m_panel8, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTE_DONTWRAP+wx.wxTE_MULTILINE+wx.wxNO_BORDER )
    self.func:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), wx.wxFONTFAMILY_TELETYPE, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL, false, "" ) )
    
    bSizer32:Add( self.func, 1, wx.wxEXPAND, 5 )
    
    
    self.m_panel8:SetSizer( bSizer32 )
    self.m_panel8:Layout()
    bSizer32:Fit( self.m_panel8 )
    bSizer34:Add( self.m_panel8, 1, wx.wxEXPAND +wx.wxALL, 5 )
    
    
    self.custom_func_panel:SetSizer( bSizer34 )
    self.custom_func_panel:Layout()
    bSizer34:Fit( self.custom_func_panel )
    self.notebook:AddPage( self.custom_func_panel, "Custom Function", false )
    
    bSizer25:Add( self.notebook, 1, wx.wxEXPAND+wx.wxALL, 5 )
    
    local m_sdbSizer2 = wx.wxStdDialogButtonSizer()
    self.m_sdbSizer2OK = wx.wxButton( self, wx.wxID_OK, "" )
    m_sdbSizer2:AddButton( self.m_sdbSizer2OK )
    self.m_sdbSizer2Cancel = wx.wxButton( self, wx.wxID_CANCEL, "" )
    m_sdbSizer2:AddButton( self.m_sdbSizer2Cancel )
    m_sdbSizer2:Realize();
    
    bSizer25:Add( m_sdbSizer2, 0, wx.wxEXPAND+wx.wxALL, 5 )
    
    
    self:SetSizer( bSizer25 )
    self:Layout()
    bSizer25:Fit( self )
    
    self:Centre( wx.wxBOTH )
    
    -- Connect Events
    self.name:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(evt) self:OnNameChanged(evt) end)
    self.directoryname:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(evt) self:OnDirectoryChanged(evt) end)
    self.auth_required:Connect(wx.wxEVT_COMMAND_CHECKBOX_CLICKED, function(evt) self:OnAuthRequired(evt) end)
    self.custom_func_message:Connect(wx.wxEVT_LEFT_UP, function(evt) self:OnShowCustomFunction(evt) end)
    self.m_sdbSizer2OK:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnOK(evt) end)

    return self
end

return wxfb