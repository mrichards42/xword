local function DownloadFilter(parent)
    local filter = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    filter:SetSizer(sizer)

    -- Days of the week
    local day_sizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, filter, "Days")
    sizer:Add(day_sizer, 0)
    local day_checkboxes = {}
    local daygrid = wx.wxGridSizer(1, 0, 5, 5)
    day_sizer:Add(daygrid, 1, wx.wxEXPAND + wx.wxALL, 5)
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(filter, wx.wxID_ANY, label)
        ctrl:SetValue(true)
        table.insert(day_checkboxes, ctrl)
        daygrid:Add(ctrl, 1, wx.wxEXPAND)
    end

    local buttons = wx.wxFlexGridSizer(1, 0, 5, 5)
    day_sizer:Add(buttons)

    local select_none = wx.wxButton(filter, wx.wxID_ANY, "None")
    local select_all = wx.wxButton(filter, wx.wxID_ANY, "All")
    local weekend = wx.wxButton(filter, wx.wxID_ANY, "Weekend")
    local weekday = wx.wxButton(filter, wx.wxID_ANY, "Weekday")

    buttons:Add(select_none)
    buttons:Add(select_all)
    buttons:Add(weekend)
    buttons:Add(weekday)

    local apply_button = wx.wxButton(filter, wx.wxID_ANY, "Apply Filter")
    sizer:Add(apply_button)

    filter:Fit()

    -- ------------------------------------------------------------------------
    -- Data Access
    -- ------------------------------------------------------------------------

    function filter:get_custom_func()
        local days = {}
        for _, ctrl in ipairs(day_checkboxes) do
            table.insert(days, ctrl:GetValue())
        end
        return function(puzzle, d)
            return days[d:getisoweekday()]
        end
    end

    -- ------------------------------------------------------------------------
    -- Events
    -- ------------------------------------------------------------------------
    select_none:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        for _, ctrl in ipairs(day_checkboxes) do
            ctrl:SetValue(false)
        end
    end)

    select_all:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        for _, ctrl in ipairs(day_checkboxes) do
            ctrl:SetValue(true)
        end
    end)

    weekend:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        day_checkboxes[6].Value = true
        day_checkboxes[7].Value = true
        for i=1,5 do
            day_checkboxes[i].Value = false
        end
    end)

    weekday:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        day_checkboxes[6].Value = false
        day_checkboxes[7].Value = false
        for i=1,5 do
            day_checkboxes[i].Value = true
        end
    end)

    apply_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        parent:update_puzzles()
    end)

    return filter
end

return DownloadFilter