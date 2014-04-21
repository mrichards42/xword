-- Converted from python by py2lua
-- python file: wxFB.py
-- modtime: 1398002383
-- ----------------------------------

local wxfb = {} -- Table to hold classes

local require_bmp = require("wx.lib.bmp") -- require function for images

---------------------------------------------------------------------------
-- Python code generated with wxFormBuilder (version Nov  6 2013)
-- http://www.wxformbuilder.org/
--
-- PLEASE DO "NOT" EDIT THIS FILE!
---------------------------------------------------------------------------

local BmpButton = require("wx.lib.bmp_button")
local TextButton = require("wx.lib.text_button")
local ArrowButton = require("wx.lib.arrow_button")

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
    self.label:SetFont( wx.wxFont( 12, 70, 90, 92, false, "" ) )
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
    self.queue_count:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), 70, 90, 92, false, "" ) )
    self.queue_count:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.queue_count:SetToolTip( "Download queue" )
    
    queueCountSizer:Add( self.queue_count, 0, wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT, 5 )
    
    self.queueBmp = wx.wxStaticBitmap( self.panel, wx.wxID_ANY, require_bmp("download.images.down"), wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    queueCountSizer:Add( self.queueBmp, 0, wx.wxALIGN_CENTER_VERTICAL, 5 )
    
    
    bSizer3:Add( queueCountSizer, 0, wx.wxEXPAND+wx.wxALIGN_CENTER_VERTICAL+wx.wxRIGHT+wx.wxLEFT, 5 )
    
    local errorCountSizer = wx.wxBoxSizer( wx.wxHORIZONTAL )
    
    self.error_count = TextButton( self.panel, wx.wxID_ANY, "0", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.error_count:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), 70, 90, 92, false, "" ) )
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
    self.label:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), 70, 90, 92, false, "" ) )
    
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
    self.erase_history:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), 70, 90, 92, false, "" ) )
    self.erase_history:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.erase_history:SetToolTip( "Download missing puzzles" )
    
    bSizer13:Add( self.erase_history, 0, wx.wxALL, 5 )
    
    self.cancel_downloads = TextButton( self, wx.wxID_ANY, "Cancel Downloads", wx.wxDefaultPosition, wx.wxDefaultSize, 0 )
    self.cancel_downloads:SetFont( wx.wxFont( wx.wxNORMAL_FONT:GetPointSize(), 70, 90, 92, false, "" ) )
    self.cancel_downloads:SetForegroundColour( wx.wxColour( 0, 0, 255 ) )
    self.cancel_downloads:SetToolTip( "Download missing puzzles" )
    
    bSizer13:Add( self.cancel_downloads, 0, wx.wxALL, 5 )
    
    
    bSizer10:Add( bSizer13, 0, wx.wxEXPAND, 5 )
    
    self.list = wx.wxLuaListCtrl( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLC_REPORT+wx.wxLC_VIRTUAL )
    bSizer10:Add( self.list, 1, wx.wxALL+wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer10 )
    self:Layout()
    bSizer10:Fit( self )
    
    -- Connect Events
    self.erase_history:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnClearHistory(evt) end)
    self.cancel_downloads:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt) self:OnCancelDownloads(evt) end)
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

return wxfb