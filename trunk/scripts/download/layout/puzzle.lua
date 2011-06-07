-- This should only be required as part of download.layout
local P = download
local layout = P.layout
local numPuzzles = 7

local function lastPuzzles(sources, start_date, weekday)
    local panel = P.dlg.panel

    layout.filter.dateSpan = { days = numPuzzles }
    layout.filter.DisplayDate(start_date:fmt("%m/%d/%Y"))

    -- ------------------------------------------------------------------------
    -- Layout functions
    -- ------------------------------------------------------------------------
    local sizer = wx.wxGridBagSizer(5,10)

    local row, col = 0, 0

    local function AddLine()
        if col ~= 0 then
            row = row + 2
        end
        sizer:Add(
            wx.wxStaticLine(panel, wx.wxID_ANY),
            wx.wxGBPosition(row, 0),
            wx.wxGBSpan(1, numPuzzles+1),
            wx.wxEXPAND
        )
        col = 0
        row = row + 1
    end

    local function AddSource(source)
        if col ~= 0 then
            AddLine()
        end
        sizer:Add(
            layout.Header(panel, source.display),
            wx.wxGBPosition(row, 0),
            wx.wxGBSpan(2, 1),
            wx.wxALIGN_CENTER_VERTICAL
        )
        col = 1
    end

    local function AddDownload(dl, d)
        sizer:Add(
            layout.Header(panel, d:fmt(P.dateformat)),
            wx.wxGBPosition(row, col)
        )
        sizer:Add(
            dl.ctrl,
            wx.wxGBPosition(row+1, col),
            wx.wxGBSpan(1,1),
            wx.wxALIGN_CENTER
        )
        col = col + 1
    end

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    AddLine()
    for _, source in ipairs(sources) do
        -- Does this source have any puzzles for the given weekday?
        if weekday == 0 or source.days[weekday] then
            AddSource(source)

            -- Add the actual download controls to the grid
            local d = start_date:copy()
            for i=1,numPuzzles do
                -- Find a date that matches weekday
                while not source.days[d:getisoweekday()] or (weekday ~= 0 and d:getisoweekday() ~= weekday) do
                    d:adddays(-1)
                end
                local dl = layout.createDownloadCtrl(panel, source, d)
                AddDownload(dl, d)
                d:adddays(-1)
            end
        end
    end
    AddLine()
    panel:SetSizerAndFit(sizer)
end

layout.addLayout{
    "Last "..numPuzzles.." puzzles",
    lastPuzzles,
    puzzle=false,
    date=true,
    weekday=true,
}
