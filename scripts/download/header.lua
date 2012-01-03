require 'date'
local bmp = require 'download.bmp'
local BmpButton = require 'download.bmp_button'
local TextButton = require 'download.text_button'

local function DownloadHeader(parent)
    local header = wx.wxPanel(parent, wx.wxID_ANY)

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    header:SetSizer(sizer)

    -- Top sizer
    local top = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(top, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    -- Previous arrows
    local prev_week = BmpButton(header, wx.wxID_ANY, bmp.prev_week)
    prev_week.ToolTip = wx.wxToolTip("Previous Week")
    top:Add(prev_week, 0, wx.wxALIGN_LEFT + wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 10)

    local prev = BmpButton(header, wx.wxID_ANY, bmp.prev)
    prev.ToolTip = wx.wxToolTip("Previous Day")
    top:Add(prev, 0, wx.wxALIGN_LEFT + wx.wxALIGN_CENTER_VERTICAL)

    -- Label
    local label = TextButton(header, wx.wxID_ANY, "",
                             wx.wxDefaultPosition, wx.wxDefaultSize,
                             wx.wxALIGN_CENTER + wx.wxST_NO_AUTORESIZE)
    label.ToolTip = wx.wxToolTip("Download Puzzles")
    label.Font = wx.wxFont(12, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD)
    label.ForegroundColour = wx.wxBLUE
    top:Add(label, 1, wx.wxALIGN_CENTER)

    -- Next arrows
    local next = BmpButton(header, wx.wxID_ANY, bmp.next)
    next.ToolTip = wx.wxToolTip("Next Day")
    top:Add(next, 0, wx.wxALIGN_RIGHT + wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 10)

    local next_week = BmpButton(header, wx.wxID_ANY, bmp.next_week)
    next_week.ToolTip = wx.wxToolTip("Next Week")
    top:Add(next_week, 0, wx.wxALIGN_RIGHT + wx.wxALIGN_CENTER_VERTICAL)

    -- Bottom sizer
    local bottom = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(bottom, 0, wx.wxEXPAND)

    -- Kind
    local kind = wx.wxChoice(header, wx.wxID_ANY,
                             wx.wxDefaultPosition, wx.wxDefaultSize,
                             { "Day", "Week", "Month", "Custom" })
    kind.Selection = 0
    bottom:Add(kind, 0, wx.wxALIGN_CENTER_VERTICAL)

    bottom:AddStretchSpacer()

    -- Start date
    local start_text = wx.wxStaticText(header, wx.wxID_ANY, "Start:")
    bottom:Add(start_text, 0, wx.wxALIGN_CENTER + wx.wxLEFT + wx.wxRIGHT, 5)

    local start_date = wx.wxDatePickerCtrl(header, wx.wxID_ANY,
        wx.wxDefaultDateTime, wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDP_DROPDOWN + wx.wxDP_SHOWCENTURY)
    bottom:Add(start_date, 0, wx.wxRIGHT, 5)

    -- End date
    local end_text = wx.wxStaticText(header, wx.wxID_ANY, "End:")
    bottom:Add(end_text, 0, wx.wxALIGN_CENTER + wx.wxLEFT + wx.wxRIGHT, 5)

    local end_date = wx.wxDatePickerCtrl(header, wx.wxID_ANY,
        wx.wxDefaultDateTime, wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDP_DROPDOWN + wx.wxDP_SHOWCENTURY)
    bottom:Add(end_date, 0, wx.wxRIGHT, 5)

    -- ------------------------------------------------------------------------
    -- Data Access
    -- ------------------------------------------------------------------------

    function header:get_kind()
        return kind:GetStringSelection():lower()
    end

    -- wxDatePickerCtrl naturally uses wxDateTime internally, but the rest
    -- of the downloader uses lua date objects.
    function header:get_start_date()
        return date(start_date.Value:Format("%m/%d/%Y"))
    end

    function header:get_end_date()
        return date(end_date.Value:Format("%m/%d/%Y"))
    end


    -- ------------------------------------------------------------------------
    -- Update date / kind
    -- ------------------------------------------------------------------------

    -- We'll update start and end dates when the user changes kind, but
    -- we need to keep track of the last start date the user selected so
    -- that the dates don't creep one way or the other.
    local real_start = start_date.Value
    local function update_dates()
        local kind = header:get_kind()
        if kind == 'day' then
            start_date.Value = real_start
            end_date.Value = real_start
            label.Label = start_date.Value:Format("%A %b %d, %Y")
        elseif kind == 'week' then
            start_date.Value = real_start:GetWeekDayInSameWeek(1)
            end_date.Value = start_date.Value:Add(wx.wxTimeSpan.Days(6))
            label.Label = start_date.Value:Format("Week of %b %d, %Y")
        elseif kind == 'month' then
            end_date.Value = real_start:GetLastMonthDay()
            start_date.Value = end_date.Value:SetDay(1)
            label.Label = start_date.Value:Format("%B %Y")
        else
            label.Label = start_date.Value:Format("%b %d, %Y") .. " - " .. end_date.Value:Format("%b %d, %Y")
        end
        parent:update_puzzles()
    end

    -- Update kind
    local function update_kind(kind)
        local kind = header:get_kind()
        -- Show or hide controls
        top:Show(prev_week, (kind == 'day'))
        top:Show(prev, (kind ~= 'custom'))
        top:Show(next, (kind ~= 'custom'))
        top:Show(next_week, (kind == 'day'))

        bottom:Show(start_text, (kind == 'custom'))
        bottom:Show(end_text, (kind == 'custom'))
        bottom:Show(end_date, (kind == 'custom'))
        -- Change labels
        if kind == 'day' then
        elseif kind == 'week' then
            prev.ToolTip = wx.wxToolTip("Previous Week")
            next.ToolTip = wx.wxToolTip("Next Week")
        elseif kind == 'month' then
            prev.ToolTip = wx.wxToolTip("Previous Month")
            next.ToolTip = wx.wxToolTip("Next Month")
        end
        header:Layout()
        update_dates()
    end

    -- ------------------------------------------------------------------------
    -- Events
    -- ------------------------------------------------------------------------

    prev_week:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function (evt)
            real_start:Subtract(wx.wxTimeSpan.Week())
            update_dates()
        end)

    prev:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function (evt)
            local kind = header:get_kind()
            if kind == 'day' then
                real_start:Subtract(wx.wxTimeSpan.Day())
            elseif kind == 'week' then
                real_start:Subtract(wx.wxTimeSpan.Week())
            elseif kind == 'month' then
                real_start:SetDay(1)
                real_start:Subtract(wx.wxTimeSpan.Days(1))
            end
            update_dates()
        end)

    next:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function (evt)
            local kind = header:get_kind()
            if kind == 'day' then
                real_start:Add(wx.wxTimeSpan.Day())
            elseif kind == 'week' then
                real_start:Add(wx.wxTimeSpan.Week())
            elseif kind == 'month' then
                real_start:SetToLastMonthDay()
                real_start:Add(wx.wxTimeSpan.Days(1))
            end
            update_dates()
        end)

    next_week:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function (evt)
            real_start:Add(wx.wxTimeSpan.Week())
            update_dates()
        end)


    kind:Connect(wx.wxEVT_COMMAND_CHOICE_SELECTED, update_kind)

    start_date:Connect(wx.wxEVT_DATE_CHANGED,
        function (evt)
            real_start = start_date.Value
            update_dates()
        end)

    end_date:Connect(wx.wxEVT_DATE_CHANGED, update_dates)

    label:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        parent:download_puzzles()
    end)

    -- Functions
    function header:set_kind(kind_)
        kind:SetStringSelection(kind_:sub(1,1):upper() .. kind_:sub(2))
        update_kind()
    end

    return header
end

return DownloadHeader
