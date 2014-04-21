local CollapsePanel = require 'download.gui.collapse_panel'
local ScrolledPanel = require 'download.gui.scrolled_panel'

local DateFilter
local NameFilter

local function DownloadFilter(parent)
    local filter = ScrolledPanel(parent)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    filter.panel:SetSizer(sizer)

    -- The list of filters
    -- Filters are in a collapse panel
    -- constructor should take a wxWindow as a parent, and return a window
    -- with a function get_custom_func() that returns the custom function
    -- for the filter.
    local filters = {}
    local function add_filter(constructor, label)
        local panel = CollapsePanel(filter.panel, label)
        local f = constructor(panel)
        panel:SetPanel(f)
        panel:Expand()
        function panel:OnToggle()
            filter:UpdateScrollbars()
        end
        sizer:Add(panel, 0, wx.wxEXPAND)
        table.insert(filters, f)
    end

    -- Filters
    add_filter(DateFilter, "Day of the Week")
    add_filter(NameFilter, "Puzzles")

    -- Buttons
    local apply_button = wx.wxButton(filter.panel, wx.wxID_ANY, "Apply Filter")
    sizer:Add(apply_button)

    -- Set min size
    filter.panel:Fit()
    filter.MinSize = wx.wxSize(
        filter.panel.Size.Width
            + wx.wxSystemSettings.GetMetric(wx.wxSYS_VSCROLL_X)
            + 5,
        -1)
    filter:UpdateScrollbars()

    -- Custom Function
    function filter:get_custom_func()
        local functions = {}
        for _, f in ipairs(filters) do
            local func = f:get_custom_func()
            if func then
                table.insert(functions, func)
            end
        end
        if #functions > 0 then
            return function(puzzle, d)
                for _, func in ipairs(functions) do
                    if not func(puzzle, d) then
                        return false
                    end
                end
                return true
            end
        else
            return function() return true end
        end
    end

    -- Apply Event
    apply_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        parent:update_puzzles()
    end)

    return filter
end

-- ----------------------------------------------------------------------------
-- Filter by day of the week
-- ----------------------------------------------------------------------------
DateFilter = function(parent)
    local date_filter = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    date_filter:SetSizer(sizer)

    -- Checkboxes
    local day_sizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, date_filter, "Days")
    sizer:Add(day_sizer, 0)
    local checkboxes = {}
    local daygrid = wx.wxGridSizer(1, 0, 5, 5)
    day_sizer:Add(daygrid, 1, wx.wxEXPAND + wx.wxALL, 5)
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(date_filter, wx.wxID_ANY, label)
        ctrl:SetValue(true)
        table.insert(checkboxes, ctrl)
        daygrid:Add(ctrl, 1, wx.wxEXPAND)
    end

    -- Buttons
    local buttons = wx.wxFlexGridSizer(1, 0, 5, 5)
    day_sizer:Add(buttons)

    local select_none = wx.wxButton(date_filter, wx.wxID_ANY, "None")
    local select_all = wx.wxButton(date_filter, wx.wxID_ANY, "All")
    local weekend = wx.wxButton(date_filter, wx.wxID_ANY, "Weekend")
    local weekday = wx.wxButton(date_filter, wx.wxID_ANY, "Weekday")

    buttons:Add(select_none)
    buttons:Add(select_all)
    buttons:Add(weekend)
    buttons:Add(weekday)

    -- Button events
    select_none:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        for _, ctrl in ipairs(checkboxes) do
            ctrl:SetValue(false)
        end
    end)

    select_all:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        for _, ctrl in ipairs(checkboxes) do
            ctrl:SetValue(true)
        end
    end)

    weekend:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        checkboxes[6].Value = true
        checkboxes[7].Value = true
        for i=1,5 do
            checkboxes[i].Value = false
        end
    end)

    weekday:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        checkboxes[6].Value = false
        checkboxes[7].Value = false
        for i=1,5 do
            checkboxes[i].Value = true
        end
    end)

    -- Custom Function
    function date_filter:get_custom_func()
        local days = {}
        local has_custom_func = false
        for _, ctrl in ipairs(checkboxes) do
            table.insert(days, ctrl:GetValue())
            if ctrl:GetValue() == false then
                has_custom_func = true
            end
        end
        if has_custom_func then
            return function(puzzle, d)
                return days[d:getisoweekday()]
            end
        end
    end

    return date_filter
end


-- ----------------------------------------------------------------------------
-- Filter by puzzle name
-- ----------------------------------------------------------------------------
NameFilter = function(parent)
    local name_filter = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    name_filter:SetSizer(sizer)

    local checkboxes = {}
    for _, puzzle in download.puzzles:iter() do
        local ctrl = wx.wxCheckBox(name_filter, wx.wxID_ANY, puzzle.name)
        ctrl.Value = true
        table.insert(checkboxes, ctrl)
    end

    -- Add names in two columns
    local name_sizer = wx.wxFlexGridSizer(0, 2, 5, 5)
    local half = math.ceil(#checkboxes / 2)
    for i=1, half do
        name_sizer:Add(checkboxes[i])
        if #checkboxes >= i + half then
            name_sizer:Add(checkboxes[i+half])
        end
    end
    sizer:Add(name_sizer)

    -- Custom Function
    function name_filter:get_custom_func()
        local names = {}
        local has_custom_func = false
        for _, ctrl in ipairs(checkboxes) do
            names[ctrl:GetLabel()] = ctrl:GetValue()
            if ctrl:GetValue() == false then
                has_custom_func = true
            end
        end
        if has_custom_func then
            return function(puzzle, d)
                return names[puzzle.name]
            end
        end
    end

    return name_filter
end

return DownloadFilter