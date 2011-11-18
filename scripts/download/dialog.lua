require 'download.puzzles'
require 'date'
local DownloadHeader = require 'download.header'
local PuzzlePanel = require 'download.puzzle_panel'
local PopupWindow = require 'download.popup'
require 'download.stats'

local function make_puzzles(parent)
    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    scroller:SetSizer(sizer)
    scroller.puzzles = {}
    for _, puzzle in ipairs(puzzles) do
        local p = PuzzlePanel(scroller, puzzle, kind, start_date, end_date)
        table.insert(scroller.puzzles, p)
        sizer:Add(p, 0, wx.wxEXPAND + wx.wxALL, 5)
    end

    function scroller:set_dates(kind, start_date, end_date)
        download.clear_stats()
        self:Freeze()
        local filenames = {}
        for _, p in ipairs(self.puzzles) do
            for _, fn in ipairs(p:set_dates(kind, start_date, end_date)) do
                table.insert(filenames, fn)
            end
        end
        download.fetch_stats(filenames)
        self:Layout()
        self:FitInside()
        self:Refresh()
        self:Thaw()
    end

    return scroller
end

local function DownloadDialog(parent, id, title, pos, size)
    local dialog = wx.wxDialog(
        parent or xword.frame, id or wx.wxID_ANY, title or "Downloader",
        pos or wx.wxDefaultPosition, size or wx.wxSize(450, 450),
        wx.wxDEFAULT_FRAME_STYLE
    )

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dialog:SetSizer(sizer)

    local panel = wx.wxPanel(dialog, wx.wxID_ANY)
    panel:SetBackgroundColour(wx.wxWHITE)
    sizer:Add(panel, 1, wx.wxEXPAND)
    sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local header = DownloadHeader(panel)
    sizer:Add(header, 0, wx.wxEXPAND + wx.wxALL, 5)

    local puzzle_panel = make_puzzles(panel)
    sizer:Add(puzzle_panel, 1, wx.wxEXPAND)

    function panel:update_puzzles()
        puzzle_panel:set_dates(header:get_kind(), header:get_start_date(), header:get_end_date())
    end

    header:set_kind('day')

    return dialog
end

return DownloadDialog
