require 'date'
local bmp = require 'download.bmp'

local function DownloadHeader(parent)
    local header = wx.wxPanel(parent, wx.wxID_ANY)

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    header:SetSizer(sizer)

    local nav = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(nav, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    -- Previous arrow
    local prev = wx.wxBitmapButton(header, wx.wxID_ANY, bmp.left)
    nav:Add(prev, 0, wx.wxALIGN_LEFT + wx.wxRIGHT, 5)

    nav:AddStretchSpacer()

    -- Start date
    local start_text = wx.wxStaticText(header, wx.wxID_ANY, "Start:")
    nav:Add(start_text, 0, wx.wxALIGN_CENTER + wx.wxLEFT + wx.wxRIGHT, 5)

    local start_date = wx.wxDatePickerCtrl(header, wx.wxID_ANY,
        wx.wxDefaultDateTime, wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDP_DROPDOWN + wx.wxDP_SHOWCENTURY)
    nav:Add(start_date, 0, wx.wxRIGHT, 5)

    -- End date
    local end_text = wx.wxStaticText(header, wx.wxID_ANY, "End:")
    nav:Add(end_text, 0, wx.wxALIGN_CENTER + wx.wxLEFT + wx.wxRIGHT, 5)

    local end_date = wx.wxDatePickerCtrl(header, wx.wxID_ANY,
        wx.wxDefaultDateTime, wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDP_DROPDOWN + wx.wxDP_SHOWCENTURY)
    nav:Add(end_date, 0, wx.wxRIGHT, 5)

    nav:AddStretchSpacer()

    -- Next arrow
    local next = wx.wxBitmapButton(header, wx.wxID_ANY, bmp.right)
    nav:Add(next, 0, wx.wxALIGN_RIGHT)

    local hsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(hsizer, 0, wx.wxEXPAND)

    local label = wx.wxStaticText(header, wx.wxID_ANY, "")
    label:SetFont(wx.wxFont(14, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD))
    hsizer:Add(label, 1, wx.wxEXPAND)

    local kind = wx.wxChoice(header, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, { "Day", "Week", "Month", "Custom" })
    kind.Selection = 0
    hsizer:Add(kind, 0, wx.wxALIGN_CENTER_VERTICAL)

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
        -- Show or hide the nav controls
        local show = (kind == 'custom')
        nav:Show(end_text, show)
        nav:Show(end_date, show)
        nav:Show(prev, not show)
        nav:Show(next, not show)
        -- Change the label for nav controls
        if kind == 'day' then
            start_text.Label = "Day:"
        elseif kind == 'week' then
            start_text.Label = "Week of:"
        elseif kind == 'month' then
            start_text.Label = "Month of:"
        elseif kind == 'custom' then
            start_text.Label = "Start:"
        end
        nav:Layout()
        update_dates()
    end

    -- ------------------------------------------------------------------------
    -- Events
    -- ------------------------------------------------------------------------

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

    function header:set_kind(kind_)
        kind:SetStringSelection(kind_:sub(1,1):upper() .. kind_:sub(2))
        update_kind()
    end

    kind:Connect(wx.wxEVT_COMMAND_CHOICE_SELECTED, update_kind)

    start_date:Connect(wx.wxEVT_DATE_CHANGED,
        function (evt)
            real_start = start_date.Value
            update_dates()
        end)

    end_date:Connect(wx.wxEVT_DATE_CHANGED, update_dates)

    return header
end

return DownloadHeader
