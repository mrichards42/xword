-- This should only be required as part of download.layout

local P = download
local layout = P.layout

local function layoutGrid(start_date, direction)
    local dlg = assert(P.dlg)
    local panel = dlg.panel
    local sizer = wx.wxGridBagSizer(5,5)

    assert(start_date)
    local end_date = start_date:copy()
    end_date:adddays(7)

    layout.createDownloadCtrls(start_date, end_date)

    -- ------------------------------------------------------------------------
    -- Layout functions
    -- ------------------------------------------------------------------------

    -- Row and column indexes
    local MISSING_SOURCES = #P.sources + 2
    local MISSING_DATES   = 2 + (end_date-start_date):spandays()

    -- Allow orientation to be swapped
    local GBPosition, swap
    if direction then
        GBPosition = function(row, col)
            return wx.wxGBPosition(row, col)
        end
        swap = function()
            layout.setLayout(layoutGrid, start_date, false)
        end
    else
        GBPosition = function(col, row)
            return wx.wxGBPosition(row, col)
        end
        swap = function()
            layout.setLayout(layoutGrid, start_date, true)
        end
    end

    -- Add a window to the sizer
    local function Add(window, position, span, align)
        local position = GBPosition(unpack(position))
        local span = span or wx.wxGBSpan(1,1)
        local align = align or wx.wxALIGN_CENTER
        sizer:Add(window, position, span, align)
    end

    -- Create a button, add it, and connect it with a function
    local function AddButton(bmpName, position, func)
        local btn = wx.wxBitmapButton(panel, wx.wxID_ANY, P.bmp[bmpName])
        Add(btn, position)
        btn:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, func)
    end

    -- Callback function for download buttons
    local function dlMissing(dl) return not dl.fileexists end

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------

    -- Swap orientation button
    AddButton('swap', {0, 0}, swap)

    -- Dates (newest to oldest)
    local d = end_date:copy()
    local i = 1
    while d >= start_date do
        Add(layout.Header(panel, d:fmt(P.dateformat), i==1), {i, 0})

        -- Missing puzzles for a given date
        AddButton('download',
                  {i, MISSING_SOURCES},
                  layout.downloadAllDates(d:copy(), dlMissing))

        -- Next iteration
        d:adddays(-1)
        i = i + 1
    end

    -- Missing dates header
    Add(layout.Header(panel, "Missing dates"), {MISSING_DATES, 0})

    -- Missing puzzles header
    Add(layout.Header(panel, "Missing sources"), {0, MISSING_SOURCES})

    -- Puzzles
    for col, source in pairs(P.sources) do
        Add(layout.Header(panel, source.display), {0, col})

        -- Add the actual download controls to the grid
        for dl_date, dl in pairs(dlg.downloads[source.display]) do
            local row = (end_date - dl_date):spandays() + 1
            Add(dl.ctrl, {row, col})
        end

        -- Missing puzzles for a given source
        AddButton('download',
                  {MISSING_DATES, col},
                  layout.downloadAllPuzzles(source.display, dlMissing))

    end

    -- All missing puzzles
    AddButton('download',
              {MISSING_DATES, MISSING_SOURCES},
              layout.downloadAll(dlMissing))

    dlg.panel:SetSizerAndFit(sizer)
end


layout.addLayout("Grid", layoutGrid)
layout.addLayout("Grid2", layoutGrid, true)
