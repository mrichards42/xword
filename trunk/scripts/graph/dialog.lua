local tablex = require 'pl.tablex'
local GraphPanel = require 'graph.panel'

local function format_minutes(n)
    local minutes = math.floor(n)
    local seconds = math.floor(.5 + (n - minutes) * 60)
    if seconds == 0 then
        return tostring(minutes)
    else
        return string.format('%d:%02d', minutes, seconds)
    end
end

-- ---------------------------------------------------------------------------
-- Graph dialog
-- ---------------------------------------------------------------------------
local function ShowDialog(points)
    local dlg = wx.wxDialog(xword.frame, wx.wxID_ANY,
                            "Solving Graph",
                            wx.wxDefaultPosition, wx.wxSize(450,350),
                            wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER)

    -- The graph panel
    local panel = GraphPanel(dlg)
    panel:SetFont(wx.wxSWISS_FONT)

    -- Turn { { time = x, incorrect = y, correct = y, ... }, ... }
    -- Into { time = {x, ...}, incorrect = { y, ... }, ... }
    local resolution = 15 -- No more than this many seconds between points
    local values = {}
    for _, point in ipairs(points) do
        local x = values.time and values.time[#values.time] or 0
        while x <= point.time do
            for k,v in pairs(point) do
                if not values[k] then values[k] = {} end
                table.insert(values[k], v)
            end
            -- Adjust the x value if it is between this point and the previous
            if x + resolution <= point.time then
                values.time[#values.time] = x
            end
            x = x + resolution
        end
    end

    -- Add a series to the GraphPanel and to the legend
    local label_sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local function add_series(name, default_color, label)
        -- Add the series
        local series = panel:AddSeries(values[name],
                                       graph.config.color[name] or default_color,
                                       label)
        -- Make the legend label with color picker
        local color = wx.wxColourPickerCtrl(dlg, wx.wxID_ANY, series.color, wx.wxDefaultPosition, wx.wxSize(15, 15))
        color:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))
        color:SetToolTip("Select a color")
        -- Update the graph and the config settings when the color changes
        color:Connect(wx.wxEVT_COMMAND_COLOURPICKER_CHANGED, function (evt)
            series.color = evt.Colour
            graph.config.color[name] = series.color
            panel:Refresh()
        end)
        label_sizer:Add(color, 0, wx.wxRIGHT, 5)
        local label = wx.wxStaticText(dlg, wx.wxID_ANY, series.label)
        label_sizer:Add(label, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 20)
    end

    -- X Data
    panel:SetXLabel("Time (minutes)")
    panel:SetXValues(tablex.map('/', values.time, 60)) -- Use minutes instead of seconds
    panel:SetXFormat(format_minutes)
    -- Y Data
    panel:SetYLabel("Squares")
    add_series('correct', wx.wxGREEN, "Correct")
    add_series('incorrect', wx.wxRED, "Incorrect")
    add_series('blank', wx.wxLIGHT_GREY, "Blank")
    add_series('black', wx.wxBLACK, "Black Squares")

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    sizer:Add(panel , 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:Add(label_sizer , 0, wx.wxALIGN_CENTER + wx.wxALL, 5)
    dlg:SetSizer(sizer)
    local min_size = sizer.MinSize.Width
    sizer:SetMinSize(wx.wxSize(min_size, min_size))
    dlg:Fit()
    dlg:SetMinSize(dlg:GetSize())

    dlg:Connect(wx.wxEVT_CLOSE_WINDOW, function(evt)
        dlg:Destroy()
        evt:Skip()
    end)

    dlg:Fit()
    dlg:Center()
    dlg:ShowModal()
end

return ShowDialog