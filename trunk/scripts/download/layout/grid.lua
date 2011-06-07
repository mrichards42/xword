-- This should only be required as part of download.layout

local P = download
local layout = P.layout
local isSwapped = false

local function layoutGrid(sources, end_date)
    local panel = P.dlg.panel

    -- Dates
    local start_date = end_date:copy()
    local f = layout.filter
    f.dateSpan = { days = 7 }
    layout.subtractDateSpan(start_date, f.dateSpan)
    f.DisplayDate(string.format("%s - %s", start_date:fmt("%m/%d/%Y"), end_date:fmt("%m/%d/%Y")))

    -- ------------------------------------------------------------------------
    -- Layout functions
    -- ------------------------------------------------------------------------

    -- Allow orientation to be swapped
    local GBPosition, swap
    if isSwapped then
        GBPosition = function(row, col)
            return wx.wxGBPosition(row, col)
        end
    else
        GBPosition = function(col, row)
            return wx.wxGBPosition(row, col)
        end
    end

    local function swap()
        isSwapped = not isSwapped
        layout.setLayout("Grid (weekly)")
    end

    local sizer = wx.wxGridBagSizer(5,5)

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

    -- Dates
    local d = start_date:copy()
    local i = 1
    while d <= end_date do
        Add(layout.Header(panel, d:fmt(P.dateformat), d == end_date), {i, 0})

        -- Next iteration
        d:adddays(1)
        i = i + 1
    end

    -- Puzzles
    local dls = layout.createDownloadCtrls(panel, sources, start_date, end_date)
    for col, source in pairs(sources) do
        Add(layout.Header(panel, source.display), {0, col})

        -- Add the actual download controls to the grid
        for dl_date, dl in pairs(dls[source.display]) do
            local row = (dl_date - start_date):spandays() + 1
            Add(dl.ctrl, {row, col})
        end
    end

    panel:SetSizer(sizer)

    return end_date
end

layout.addLayout{
    -- Name
    "Grid (weekly)",
    -- Function
    layoutGrid,
    -- Filter options
    puzzle=false,
    date=true,
    weekday=false,
}
