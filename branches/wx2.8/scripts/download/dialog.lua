require 'download.puzzles'
require 'date'
local DownloadHeader = require 'download.header'
local DownloadFilter = require 'download.filter'
local PuzzlePanel = require 'download.puzzle_panel'
local Status = require 'download.status'
require 'download.stats'
local tablex = require 'pl.tablex'

local function make_puzzles(parent)
    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    scroller:SetSizer(sizer)
    local puzzle_panels = {}

    function scroller:update_puzzle_list()
        for _, w in ipairs(puzzle_panels) do
            self.Sizer:Detach(w)
            w:Destroy()
        end
        tablex.clear(puzzle_panels)
        for _, puzzle in download.puzzles:iter() do
            local p = PuzzlePanel(self, puzzle, kind, start_date, end_date)
            table.insert(puzzle_panels, p)
            sizer:Add(p, 0, wx.wxEXPAND)
        end
        self:Layout()
    end

    function scroller:set_dates(kind, start_date, end_date, custom_func)
        download.clear_stats()
        tablex.clear(download.puzzle_map)
        self:Freeze()
        local stats_filenames = {}
        for _, p in ipairs(puzzle_panels) do
            local filenames, now = p:set_dates(kind, start_date, end_date, custom_func)
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
        for _, p in ipairs(puzzle_panels) do
            download.add_downloads(p:get_download_data())
        end
    end

    scroller:update_puzzle_list()

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
    dialog:SetSizer(wx.wxBoxSizer(wx.wxHORIZONTAL))

    -- Main panel
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dialog.Sizer:Add(sizer, 1, wx.wxEXPAND)

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

    -- Filter panel
    local filter = DownloadFilter(dialog)
    dialog.Sizer:Add(filter, 0, wx.wxEXPAND)
    dialog.Sizer:Show(filter, false)

    -- Sizing
    dialog:Fit()
    dialog:SetSize(dialog.Size.Width + 50, dialog.Size.Height + 200)
    dialog.MinSize = dialog.Size

    -- Events
    dialog:Connect(wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            download.erase_stats()
            dialog:ShowFilter(false)
            evt:Skip()
        end)

    function dialog:ShowFilter(doit)
        if doit == nil then doit = true end
        if dialog.Sizer:GetItem(filter):IsShown() == doit then
            return
        end
        if doit then
            self.Sizer:Show(filter, true)
            self:Layout()
            self.MinSize = wx.wxSize(self.MinSize.Width + filter.Size.Width,
                                     self.MinSize.Height)
            local new_width = self.Size.Width + filter.Size.Width
            if new_width < wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_X) then
                self:SetSize(new_width, self.Size.Height)
            end
        else
            self.Sizer:Show(filter, false)
            self:Layout()
            self.MinSize = wx.wxSize(self.MinSize.Width - filter.Size.Width,
                                     self.MinSize.Height)
            if not self:IsMaximized() then
                self:SetSize(self.Size.Width - filter.Size.Width, self.Size.Height)
            end
        end
    end

    -- functions
    function dialog:update_puzzles()
        local kind = header:get_kind()
        local start_date = header:get_start_date()
        local end_date = header:get_end_date()
        local custom_func = filter:get_custom_func()
        puzzle_panel:set_dates(kind, start_date, end_date, custom_func)
        download.previous_view.kind = kind
        download.previous_view.start_date = start_date
        download.previous_view.end_date = end_date
    end

    function dialog:download_puzzles()
        puzzle_panel:download_puzzles()
    end

    function dialog:update_status(text)
        status:update_status(text)
    end

    function dialog:update()
        self:Freeze()
        puzzle_panel:update_puzzle_list()
        self:update_puzzles()
        self:ShowFilter(header:get_kind() == 'custom')
        self:Thaw()
    end

    if download.default_view == "previous view" then
        local v = download.previous_view
        header:set_data(v.kind, v.start_date, v.end_date)
    else
        header:set_data(download.default_view)
    end

    return dialog
end

return DownloadDialog