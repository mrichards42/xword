-- This should only be required as part of download.layout

local P = download
local layout = P.layout

local function layoutGrid(start_date, direction)
    local dlg = assert(P.dlg)
    local panel = dlg.panel

    if not start_date then
        start_date = layout.today()
        start_date:adddays(-7)
    end
    local end_date = start_date:copy()
    end_date:adddays(7)

    -- Allow orientation to be swapped
    local GBPosition, swap
    if direction then
        GBPosition = function(row, col)
            return wx.wxGBPosition(row, col)
        end
        swap = function() layout.setLayout(layoutGrid, start_date, false) end
    else
        GBPosition = function(col, row)
            return wx.wxGBPosition(row, col)
        end
        swap = function() layout.setLayout(layoutGrid, start_date, true) end
    end


    local ALL_SOURCES = #P.sources + 2
    local ALL_DATES   = 2 + (end_date-start_date):spandays()


    -- Create the download ctrls
    layout.createDownloadCtrls(start_date, end_date)

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxGridBagSizer(5, 5)

    -- Swap button
    local btn = wx.wxBitmapButton(panel, wx.wxID_ANY, P.bmp.swap)
    sizer:Add(btn, GBPosition(0,0), wx.wxGBSpan(1, 1), wx.wxALIGN_CENTER)
    btn:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, swap)

    -- Day headers (end_Date is always first)
    local d = end_date:copy()
    local i = 1
    while d >= start_date do
        sizer:Add(layout.Header(panel, d:fmt(P.dateformat), i==1),
                  GBPosition(i, 0), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

        -- All Puzzles for a given date
        local btn = wx.wxBitmapButton(panel, wx.wxID_ANY, P.bmp.download)
        sizer:Add(btn, GBPosition(i, ALL_SOURCES), wx.wxGBSpan(1, 1), wx.wxALIGN_CENTER)
        btn:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
                    layout.downloadAllDates(d:copy()))

        -- Next iteration
        d:adddays(-1)
        i = i + 1
    end

    -- All dates header
    sizer:Add(layout.Header(panel, "All dates"),
              GBPosition(ALL_DATES, 0), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

    -- Puzzles
    for col, source in pairs(P.sources) do
        sizer:Add(layout.Header(panel, source.display),
                  GBPosition(0, col), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

        -- Add the actual download controls to the grid
        for date_tbl, dl in pairs(dlg.downloads[source.display]) do
            local row = (end_date - date(unpack(date_tbl))):spandays() + 1
            sizer:Add(dl.ctrl,
                      GBPosition(row, col),
                      wx.wxGBSpan(1,1),
                      wx.wxALIGN_CENTER)
        end

        -- All puzzles for a given source
        local btn = wx.wxBitmapButton(panel, wx.wxID_ANY, P.bmp.download)
        sizer:Add(btn,
                  GBPosition(ALL_DATES, col),
                  wx.wxGBSpan(1, 1),
                  wx.wxALIGN_CENTER)
        btn:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
                    layout.downloadAllPuzzles(source.display))
    end

    -- All puzzles header
        sizer:Add(layout.Header(panel, "All sources"),
                  GBPosition(0, ALL_SOURCES), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

    -- All puzzles period
    local btn = wx.wxBitmapButton(panel, wx.wxID_ANY, P.bmp.download)
    sizer:Add(btn,
              GBPosition(ALL_DATES, ALL_SOURCES),
              wx.wxGBSpan(1, 1),
              wx.wxALIGN_CENTER)
    btn:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
                layout.downloadAll())

    dlg.panel:SetSizerAndFit(sizer)
end


layout.addLayout("Grid", layoutGrid)
layout.addLayout("Grid2", layoutGrid, true)
