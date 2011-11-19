require 'download.puzzles'
require 'date'
local DownloadHeader = require 'download.header'
local PuzzlePanel = require 'download.puzzle_panel'
local Status = require 'download.status'
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
        sizer:Add(p, 0, wx.wxEXPAND)
    end

    function scroller:set_dates(kind, start_date, end_date)
        download.clear_stats()
        download.puzzle_map = {}
        self:Freeze()
        local stats_filenames = {}
        for _, p in ipairs(self.puzzles) do
            local filenames, now = p:set_dates(kind, start_date, end_date)
            for _, fn in ipairs(filenames) do
                download.puzzle_map[fn] = p
                if now then
                    table.insert(stats_filenames, 1, fn)
                else
                    table.insert(stats_filenames, fn)
                end
            end
            p:update_stats()
        end
        download.fetch_stats(stats_filenames)
        self:Layout()
        self:FitInside()
        self:Refresh()
        self:Thaw()
    end

    function scroller:download_puzzles()
        for _, p in ipairs(self.puzzles) do
            download.add_downloads(p:get_download_data())
        end
    end

    return scroller
end

download.dialog = nil
local function DownloadDialog(parent, id, title, pos, size)
    if download.dialog then return download.dialog end
    download.dialog = wx.wxDialog(
        parent or xword.frame, id or wx.wxID_ANY, title or "Downloader",
        pos or wx.wxDefaultPosition, size or wx.wxDefaultSize,
        wx.wxDEFAULT_FRAME_STYLE
    )
    local dialog = download.dialog

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

    local status = Status(panel)
    sizer:Add(status, 0, wx.wxEXPAND)

    -- Sizing
    dialog:Fit()
    dialog:SetSize(dialog.Size.Width, dialog.Size.Height + 100)
    dialog.MinSize = dialog.Size

    -- Events
    dialog:Connect(wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            dialog = nil
            download.erase_stats()
            evt:Skip()
        end)

    -- functions
    function panel:update_puzzles()
        puzzle_panel:set_dates(header:get_kind(), header:get_start_date(), header:get_end_date())
    end

    function dialog:update_status()
        status:update_status()
    end

    function panel:download_puzzles()
        puzzle_panel:download_puzzles()
    end

    header:set_kind('day')

    return dialog
end

return DownloadDialog
