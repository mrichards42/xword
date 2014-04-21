local _R = mod_path(...)
local _RR = mod_path(..., 2)

local date = require 'date'
local tablex = require 'pl.tablex'

local sources = require(_RR .. 'sources')
local config = require(_RR .. 'config')
local mgr = require(_RR .. 'manager')
local stats = require(_RR .. 'stats')

local wxfb = require(_R .. 'wxFB')
local DownloadHeading = require(_R .. 'heading')
local DownloadList = require(_R .. 'list')

-- Date conversion functions
local function date_to_wx(d)
    return wx.wxDateTimeFromDMY(d:getday(), d:getmonth()-1, d:getyear())
end

local function wx_to_date(d)
    return date(d:Format("%m/%d/%Y"))  
end

-- Dialog
local function DownloadDialog(parent)
    local self = wxfb.DownloadDialog(parent or wx.NULL)
    self.label:SetWindowStyle(wx.wxALIGN_CENTER + wx.wxST_NO_AUTORESIZE)
    -- Using wxBORDER_DOUBLE doesn't seem to work in the wxFB constructor
    self.scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)

    -- Headings for each puzzle source
    self.headings = {}

    -- -----------------------------------------------------------------------
    -- Data access

    function self:GetKind()
        return self.kind:GetStringSelection():lower()
    end

    function self:GetDates()
        local start_date = wx_to_date(self.date:GetValue())
        local end_date
        local kind = self:GetKind()
        if kind == 'day' then
            end_date = start_date:copy()
        elseif kind == 'week' then
            start_date:setisoweekday(1) -- Set to monday
            end_date = start_date:copy():adddays(6) -- Add a week
        elseif kind == 'month' then
            start_date:setday(1)
            end_date = start_date:copy():addmonths(1):adddays(-1) -- last day
        end
        return start_date, end_date
    end

    -- -----------------------------------------------------------------------
    -- Update functions

    -- Update the control display based on kind
    function self:UpdateControls()
        -- Show/hide controls
        local kind = self:GetKind()
        local is_day = kind == 'day'
        local sizer = self.prev_week:GetContainingSizer()
        sizer:Show(self.prev_week, is_day)
        sizer:Show(self.next_week, is_day)
        self:Layout()
        -- Change labels
        self.prev:SetToolTip("Previous " .. kind)
        self.next:SetToolTip("Next " .. kind)
    end

    -- Show/Hide/Add/Remove headers, and update headers with current dates
    local prev_dates = {}
    function self:UpdatePuzzles()
        -- Have the dates changed since the last run?
        local start_date, end_date = self:GetDates()
        if prev_dates.start_date == start_date and prev_dates.end_date == end_date then
            return
        end
        prev_dates.start_date, prev_dates.end_date = start_date, end_date
        -- Change Label
        local kind = self:GetKind()
        if kind == 'day' then
            self.label:SetLabel(start_date:fmt("%A %b %d, %Y"))
        elseif kind == 'week' then
            self.label:SetLabel(start_date:fmt("Week of %b %d, %Y"))
        elseif kind == 'month' then
            self.label:SetLabel(start_date:fmt("%B %Y"))
        end
        -- Add/Remove headings
        local sizer = self.scroller:GetSizer()
        for _, source in sources:iterall() do
            local name = source.name
            local heading = self.headings[name]
            if source.disabled then
                -- Delete disabled sources
                if heading then
                    self.headings[name] = nil
                    sizer:Detach(heading)
                    heading:Destroy()
                    heading = nil
                end
            elseif not heading then
                -- Add missing sources
                heading = DownloadHeading(self.scroller, source)
                self.headings[name] = heading
                sizer:Add(heading, 0, wx.wxEXPAND)
            end
            -- Update headings
            if heading then
                heading:UpdatePuzzles(start_date, end_date, self:GetKind())
                sizer:Show(heading, source:has_puzzle(start_date, end_date))
            end
        end
        sizer:Layout()
        self.scroller:FitInside()
    end

    -- -----------------------------------------------------------------------
    -- Download Count/Queue
    local download_list = DownloadList(self.splitter)
    download_list:Hide()

    function self.UpdateDownloadCount()
        local sizer = self.kind:GetContainingSizer()
        -- Error count
        self.error_count:SetLabel(tostring(mgr.error_count))
        sizer:Show(self.error_count:GetContainingSizer(), mgr.error_count > 0)
        -- Queue count
        self.queue_count:SetLabel(
            mgr.queue:length() > 0 and tostring(mgr.queue:length()) or "Done")
        sizer:Layout()
    end

    mgr:connect(mgr.EVT_DOWNLOAD_END, self.UpdateDownloadCount)
    mgr:connect(mgr.EVT_CLEAR_HISTORY, self.UpdateDownloadCount)
    mgr:connect(mgr.EVT_QUEUE_UPDATED, self.UpdateDownloadCount)

    -- Show/hide the QueuePanel
    function self:ShowQueuePanel(show)
        if show == nil then show = true end
        if show == self.splitter:IsSplit() then return end
        if show then
            self:SetSize(self:GetSize():GetWidth() * 2, -1)
            self.splitter:SplitVertically(self.panel, download_list)
        else
            self:SetSize(self:GetSize():GetWidth() / 2, -1)
            self.splitter:Unsplit()
        end
    end

    -- -----------------------------------------------------------------------
    -- Events

    self.OnDateChanged = self.UpdatePuzzles

    function self:OnKindChanged()
        self:UpdateControls()
        self:UpdatePuzzles()
    end

    -- Generic function for date arrow events
    local function date_evt(amount, kind)
        return function (evt)
            local d = wx_to_date(self.date:GetValue())
            local kind = kind or self:GetKind()
            if kind == 'day' then
                d:adddays(amount)
            elseif kind == 'week' then
                d:adddays(amount * 7)
            elseif kind == 'month' then
                d:addmonths(amount)
            end
            self.date:SetValue(date_to_wx(d)) -- No event will be sent ...
            self:UpdatePuzzles() -- ... so update here
        end
    end

    self.OnPrevWeek = date_evt(-1, 'week')
    self.OnPrev = date_evt(-1)
    self.OnNext = date_evt(1)
    self.OnNextWeek = date_evt(1, 'week')

    function self:OnClose(evt)
        evt:Skip()
    end

    function self:OnDownloadMissing(evt)
        local start_date, end_date = self:GetDates()
        -- Make sure end_date isn't after today
        local today = date():sethours(0,0,0)
        if start_date < today and end_date > today then
            end_date = today
        end
        -- Download
        sources:download(start_date, end_date)
    end

    function self:OnQueueClick(evt)
        self:ShowQueuePanel()
    end

    function self:OnErrorClick(evt)
        self:ShowQueuePanel()
    end

    -- -----------------------------------------------------------------------
    -- Setup and return

    if config.default_view == 'day' then
        self.kind:SetSelection(0)
    elseif config.default_view == 'week' then
        self.kind:SetSelection(1)
    else
        self.kind:SetSelection(2)
    end
    self:UpdateControls()
    self:UpdatePuzzles()
    self:UpdateDownloadCount()

    return self
end

return DownloadDialog
