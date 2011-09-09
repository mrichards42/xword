-- This should only be required as part of download.layout

local P = download
local layout = P.layout

local function layoutMonth(sources, month_date)
    local panel = P.dlg.panel
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

    -- We can only do a layout if there is only one source
    if #sources ~= 1 then
        sizer:Add(layout.Header(panel, "Select one puzzle source"), 0, wx.wxALIGN_CENTER + wx.wxALL, 10)
        return
    end

    local source = sources[1]

    -- Dates
    local f = layout.filter
    f.dateSpan = { months = 1 }
    f.DisplayDate(month_date:fmt("%B %Y"))

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------

    sizer:Add(layout.Header(panel, source.display, true), 0, wx.wxALIGN_CENTER + wx.wxALL, 10)
    sizer:Add(layout.Header(panel, month_date:fmt('%B %Y')), 0, wx.wxALIGN_CENTER + wx.wxALL, 10)

    local gridSizer = wx.wxGridBagSizer(0, 20)

    local function Add(ctrl, pos, flag, border)
        gridSizer:Add(
            ctrl,
            wx.wxGBPosition(pos[1], pos[2]), wx.wxGBSpan(1,1),
            flag or wx.wxALIGN_CENTER, border or 0
        )
    end

    -- Days of the week
    for i, label in pairs({"S", "M", "T", "W", "T", "F", "S"}) do
        Add(layout.Header(panel, label, true), {0, i-1})
    end

    -- Month numbers and downloadCtrls    
    local d = month_date:copy():setday(1)
    local today = date():sethours(0,0,0,0)
    local month = d:getmonth()

    -- Do each day of the month
    local row = 1
    while d:getmonth() == month do
        Add(layout.Header(panel, d:fmt("%d"), d == today), {row, d:getweekday()-1}, wx.wxALIGN_CENTER + wx.wxTOP, 20)
        local dl = layout.createDownloadCtrl(panel, source, d)
        if dl then
            Add(dl.ctrl,{row+1, d:getweekday()-1})
        else
            gridSizer:Add(16, 16, wx.wxGBPosition(row+1, d:getweekday()-1))
        end
        if d:getweekday() == 7 then
            row = row + 2
        end
        d:adddays(1)
    end

    sizer:Add(gridSizer, 0, wx.wxALIGN_CENTER + wx.wxALL, 10)
    local borderSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    borderSizer:Add(sizer, 1, wx.wxEXPAND + wx.wxALIGN_CENTER)
    panel:SetSizer(borderSizer)
end

layout.addLayout{
    -- Name
    "Month",
    -- Function
    layoutMonth,
    -- Filter options
    puzzle=true,
    date=true,
    weekday=false,
}
