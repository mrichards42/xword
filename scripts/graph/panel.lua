local tablex = require 'pl.tablex'
local get_ticks = require 'graph.ticks'

local function round(n)
    return math.floor(.5 + n)
end

-- Return minimum and maximum
local function minmax(t)
    local min, max = t[1], t[1]
    for i=2,#t do
        local value = t[i]
        if value < min then
            min = value
        elseif value > max then
            max = value
        end
    end
    return min, max
end

-- Format a number as a nice string
local function nicetostr(n, max_decimals)
    if math.floor(n) == n then
        return tostring(n)
    else
        return string.format('%0.' .. (max_decimals or 2) .. 'f', n)
    end
end

-- ---------------------------------------------------------------------------
-- Graph panel
-- ---------------------------------------------------------------------------
local valid_chart_types = {'line', 'stacked'}
local valid_chart_scales = {'linear', 'percent'}
local function GraphPanel(parent)
    local panel = wx.wxWindow(parent, wx.wxID_ANY)

    -- ------------------------------------------------------------------------
    -- Public functions
    -- ------------------------------------------------------------------------

    local chart_type = 'line'
    local chart_scale = 'linear'

    local x_label, y_label = "", ""
    local x_ticks, y_ticks = {}, {}
    local x_format, y_format = nicetostr, nicetostr

    -- Data
    local x_values = {} -- { x1, x2, x3, ... }
    local y_values = {} -- { { label = 'label', data = {y1, ...}, color = color}, ... }
    local y_min, y_max, x_min, x_max -- Data bounds

    -- Scale and bounds
    local x_scale, y_scale -- DC scale
    local left, right, top, bottom, width, height -- DC bounds

    -- X Axis
    -- ------
    function panel:SetXLabel(label)
        x_label = label or ''
    end

    function panel:SetXValues(points)
        tablex.icopy(x_values, points)
        -- set min/max
        x_min, x_max = minmax(x_values)
        tablex.icopy(x_ticks, get_ticks(x_min, x_max, 8, x_format))
    end

    function panel:GetXValues()
        return x_values
    end

    function panel:SetXFormat(func)
        x_format = func or nicetostr
    end

    -- Y Axis
    -- ------
    function panel:SetYLabel(label)
        y_label = label or ''
    end

    function panel:ClearY()
        y_min, y_max = nil, nil
        tablex.clear(y_values)
        y_format = nicetostr
    end

    function panel:SetYFormat(func)
        y_format = func or nicetostr
    end

    -- Y axis values will change when the chart type changes or when a
    -- new series is added
    local function update_y_values()
        -- Update range
        if panel:GetChartType() ~= 'stacked' then -- normal
            y_min = math.min(unpack(tablex.map('[]', y_values, 'min')))
            y_max = math.max(unpack(tablex.map('[]', y_values, 'max')))
        else -- stacked
            y_min = math.min(unpack(tablex.map('[]', y_values, 'min')))
            y_max = math.max(unpack(
                tablex.mapn(
                    function (...) return tablex.reduce('+', {...}) end,
                    unpack(tablex.map('[]', y_values, 'data'))
                )
            ))
        end

        -- Update tick marks
        if panel:GetChartScale() == 'percent' then
            tablex.icopy(y_ticks, get_ticks(y_min, y_max, 8, 'percent'))
        else
            tablex.icopy(y_ticks, get_ticks(y_min, y_max, 8, y_format))
        end
    end

    function panel:AddYValues(points, color, label)
        -- get min/max
        local min, max = minmax(points)
        -- insert into y_values
        table.insert(y_values, {
            data = tablex.copy(points),
            color = color or wx.wxBLACK,
            label = label or 'Series ' .. (#y_values + 1),
            min = min,
            max = max
        })
        update_y_values()
        return y_values[#y_values]
    end

    panel.AddSeries = panel.AddYValues -- More friendly alias

    function panel:GetYValues(label)
        for _, series in ipairs(y_values) do
            if series.label == label then
                return series
            end
        end
    end

    panel.GetSeries = panel.GetYValues

    -- Chart type and scale
    -- --------------------
    function panel:SetChartType(t)
        -- Valid types
        for _, v in ipairs(valid_chart_types) do
            if t == v then
                chart_type = v
                update_y_values()
                return true
            end
        end
        return false -- not a valid chart type
    end

    function panel:GetChartType()
        return chart_type
    end

    function panel:SetChartScale(s)
        -- Valid types
        for _, v in ipairs(valid_chart_scales) do
            if s == v then
                chart_scale = v
                update_y_values()
                return true
            end
        end
        return false -- not a valid chart scale
    end

    function panel:GetChartScale()
        return chart_scale
    end

    -- ------------------------------------------------------------------------
    -- Plotting functions
    -- ------------------------------------------------------------------------

    -- Transform a point from chart coordinates to dc coordinates
    -- using the current scale (and flipping the y axis)
    local function scale_point(x, y)
        return round((x - x_min) * x_scale) + left,
               round(height - (y - y_min) * y_scale) + top
    end

    -- Transform a point from dc coordinates to chart coordinates
    local function unscale_point(x, y)
        return (x - left) / x_scale + x_min,
               (-y + top + height) / y_scale + y_min
    end

    local function draw_plot(dc, x, y, w, h)
        -- Plotting functions
        -- ------------------
        -- These functions scale and plot lines using chart coordinates

        -- plot_line({x1, x2, x3, x4, ...}, {y1, y2, y3, y4, ...}, color, true/false)
        local function plot_line(x_list, y_list, color, fill)
            color = color or dc.Pen.Colour
            dc:SetPen(wx.wxPen(color, fill and 1 or 2, wx.wxSOLID))
            dc:SetBrush(wx.wxBrush(color, wx.wxSOLID))
            for i=2,math.min(#x_list, #y_list) do
                local x1, y1 = scale_point(x_list[i-1], y_list[i-1])
                local x2, y2 = scale_point(x_list[i], y_list[i])
                dc:DrawLine(x1, y1, x2, y2)
                if fill and x1 ~= x2 then
                    -- There are no good ways of filling a polygon in wxlua,
                    -- so we'll have to use this hack
                    for y1=y1,y2 do
                        dc:DrawLine(x1, y1, x2, y2)
                    end
                    for y2=y2,y1 do
                        dc:DrawLine(x1, y1, x2, y2)
                    end
                    dc:DrawRectangle(x1, y1, x2-x1, bottom-y1)
                end
            end
        end

        -- plot_points({ {x1, y1}, {x2, y2}, {x3, y3}, ...}, pen)
        local function plot_points(points, pen)
            if pen then
                dc:SetPen(pen)
            end
            for i=2,#points do
                local x1, y1 = scale_point(unpack(points[i]))
                local x2, y2 = scale_point(unpack(points[i-1]))
                dc:DrawLine(x1, y1, x2, y2)
            end
        end

        -- Size and Scale
        -- --------------
        -- Add room for labels
        local label_size = 2 * dc:GetCharHeight()
        local tick_size = 10
        -- Vertical space of X Axis ticks and tick labels
        local y_space = label_size + tick_size + dc:GetCharHeight()
        -- Horizontal space of Y Axis ticks and tick labels
        local y_tick_label_size = math.max(unpack(tablex.map(function (tick) return dc:GetTextExtent(tick.label) end, y_ticks)))
        local x_space = label_size + tick_size + y_tick_label_size
        -- Adjust bounds
        left = x + x_space
        top = y
        width = w - x_space
        height = h - y_space
        right = left + width
        bottom = top + height
        -- Adjust scale
        x_scale = width / (x_max - x_min)
        y_scale = height / (y_max - y_min)

        -- Plot the Chart
        -- --------------
        -- Chart lines
        if panel:GetChartType() ~= 'stacked' then -- normal
            for _, series in ipairs(y_values) do
                plot_line(x_values, series.data, series.color)
            end
        else -- stacked
            -- Stack the values and plot in reverse order (so that smaller
            -- values are not covered)
            local plots = {}
            local baseline = tablex.new(#x_values, 0)
            for _, series in ipairs(y_values) do
                baseline = tablex.imap2('+', baseline, series.data)
                -- Plot in reverse order
                table.insert(plots, 1, tablex.copy(series))
                plots[1].data = baseline
            end
            for _, series in ipairs(plots) do
                plot_line(x_values, series.data, series.color, true)
            end
        end

        -- Chart bounding box
        plot_points({{x_min, y_min}, {x_max, y_min}, {x_max, y_max},
                    {x_min, y_max}, {x_min, y_min}},
                    wx.wxBLACK_PEN)

        -- X Axis ticks
        for _, tick in ipairs(x_ticks) do
            local line_end = y_min - (tick_size / y_scale) -- unscale distance
            plot_points({{tick.value, y_min}, {tick.value, line_end}})
            dc:DrawLabel(tick.label,
                wx.wxRect(scale_point(tick.value, line_end)),
                wx.wxALIGN_CENTER_HORIZONTAL + wx.wxALIGN_TOP
            )
        end

        -- Y Axis ticks
        for _, tick in ipairs(y_ticks) do
            local line_end = x_min - (tick_size / x_scale) -- unscale distance
            plot_points({{x_min, tick.value}, {line_end, tick.value}})
            dc:DrawLabel(tick.label,
                wx.wxRect(scale_point(line_end, tick.value)),
                wx.wxALIGN_CENTER_VERTICAL + wx.wxALIGN_RIGHT
            )
        end

        -- X Axis label
        local w, h = dc:GetTextExtent(x_label)
        dc:DrawText(x_label,
                    (left + right - w) / 2,
                    bottom + y_space - (label_size + h)/2)

        -- Y Axis label
        local w, h = dc:GetTextExtent(y_label)
        dc:DrawRotatedText(y_label,
                           left - x_space + label_size/2 - h,
                           (top + bottom + w) / 2,
                           90)
    end

    -- ------------------------------------------------------------------------
    -- Events
    -- ------------------------------------------------------------------------
    local last_point

    panel:Connect(wx.wxEVT_PAINT, function (evt)
        local dc = wx.wxBufferedPaintDC(panel) 
        dc:SetBackground(wx.wxWHITE_BRUSH)
        dc:SetFont(panel:GetFont())
        dc:Clear()
        local size = panel.Size
        local border = 10
        draw_plot(dc, border, border, size.Width - 2 * border, size.Height - 2 * border)
        -- Cleanup
        dc:delete()
        last_point = nil
    end)

    panel:Connect(wx.wxEVT_SIZE, function (evt)
        evt:Skip()
        panel:Refresh()
    end)

    panel:Connect(wx.wxEVT_LEFT_UP, function (evt)
        evt:Skip()
        if panel:GetChartType() == 'line' then
            panel:SetChartType('stacked')
        else
            panel:SetChartType('line')
        end
        panel:Refresh()
    end)

    -- Point tracking
    -- --------------
    -- Return x, y, series_label or nil
    function panel:HitTest(x, y)
        local is_stacked = panel:GetChartType() == 'stacked'
        -- Find a point within <tolerance> pixels
        local tolerance = 10
        local x1, y1 = unscale_point(x - tolerance, y + tolerance)
        local x2, y2 = unscale_point(x + tolerance, y - tolerance)
        local x, y = unscale_point(x, y)
        -- Find the x-values that are within the range
        local best
        for i, series_x in ipairs(x_values) do
            if series_x >= x1 then
                if series_x > x2 then
                    break
                end
                if not is_stacked then -- normal
                    -- Look for series with y values within the range
                    local series_y
                    for _, series in ipairs(y_values) do
                        series_y = series.data[i]
                        if series_y >= y1 and series_y <= y2 then
                            -- Compute the distance between this data point and
                            -- the supplied x, y coordinates
                            local distance = math.sqrt((series_y - y)^2 +
                                                       (series_x - x)^2)
                            -- Track the closest point
                            if not best or distance < best.distance then
                                best = { label = series.label,
                                         distance = distance,
                                         x = series_x,
                                         y = series_y }
                            end
                        end
                    end
                else -- stacked
                    -- Look for series with y values within the range
                    local series_y = 0
                    for _, series in ipairs(y_values) do
                        series_y = series_y + series.data[i]
                        if y <= series_y then
                            -- Compute the distance between this data point and
                            -- the supplied x, y coordinates
                            local distance = math.sqrt((series_y - y)^2 +
                                                       (series_x - x)^2)
                            -- Track the closest point
                            if not best or distance < best.distance then
                                best = { label = series.label,
                                         distance = distance,
                                         x = series_x,
                                         y = series_y,
                                         -- Also pass the previous y point
                                         y2 = series_y - series.data[i] }
                            end
                        end
                    end
                end
            end
        end
        if best then
            return best.label, best.x, best.y, best.y2
        end
    end

    local function draw_cursor(dc)
        dc:SetLogicalFunction(wx.wxINVERT)
        if not last_point then
            return
        elseif #last_point == 2 then -- circle
            dc:DrawCircle(last_point[1], last_point[2], 3)
        elseif #last_point == 4 then -- line
            dc:DrawRectangle(last_point[1] - 1, last_point[2],
                             3, last_point[4] - last_point[2])
        end
    end

    local function erase_cursor(dc)
        draw_cursor(dc)
        last_point = nil
    end

    panel:Connect(wx.wxEVT_MOTION, function (evt)
        evt:Skip()
        local dc = wx.wxClientDC(panel)
        dc:SetLogicalFunction(wx.wxINVERT)
        erase_cursor(dc)
        local label, x, y, y2 = panel:HitTest(evt.X, evt.Y)
        if label and x and y then
            if panel:GetChartType() ~= 'stacked' then -- normal
                last_point = {scale_point(x, y)}
                draw_cursor(dc)
            else -- stacked
                -- Find the y values for this area
                local x1, y1 = scale_point(x, y)
                local x2, y2 = scale_point(x, y2)
                last_point = {x1, y1, x2, y2}
                draw_cursor(dc)
            end
            panel:SetToolTip(string.format(
                "%s (%s, %s)", label, x_format(x), y_format(math.abs(y - (y2 or 0)))
            ))
        else
            panel:SetToolTip("")
        end
        dc:delete()
    end)

    panel:Connect(wx.wxEVT_LEAVE_WINDOW, function (evt)
        evt:Skip()
        local dc = wx.wxClientDC(panel)
        erase_cursor(dc)
        panel:SetToolTip("")
        dc:delete()
    end)

    return panel
end

return GraphPanel