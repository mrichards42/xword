local P = download
local layout = P.layout
local tablex = require 'pl.tablex'
require 'date'


function layout.addDateSpan(d, span, multiple)
    multiple = multiple or 1
    for k, v in pairs(span) do
        local func = d['add'..k]
        if func then
            func(d, v * multiple)
        end
    end
end

function layout.subtractDateSpan(d, span, multiple)
    layout.addDateSpan(d, span, multiple and -multiple or -1)
end


-- The filter is a part of the header that allows the user to alter the layout
-- Options inclue
--    * View
--    * Puzzle
--    * Date arrows
function layout.createFilter()
    local dlg = P.dlg
    local f = dlg.filter
    layout.filter = f

    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local function Add(obj, label)
        if label then
            local label_sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            label_sizer:Add(wx.wxStaticText(f, wx.wxID_ANY, label..": "), 0, wx.wxALIGN_CENTER)
            --label_sizer:AddSpacer(2)
            label_sizer:Add(obj, 0, wx.wxALIGN_CENTER)
            obj = label_sizer
        end
        sizer:Add(obj, 0, wx.wxALIGN_CENTER + wx.wxLEFT + wx.wxRIGHT, 5)
    end

    -- --------------------------------------
    -- View
    -- --------------------------------------
    f.view = wx.wxChoice(
        f, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize,
        tablex.map(function(l) return l.name end, layout.layouts)
    )
    f.view:SetSelection(0)
    f:Connect(
        f.view:GetId(),
        wx.wxEVT_COMMAND_CHOICE_SELECTED,
        function (evt) layout.updateLayout() end
    )
    Add(f.view, 'View')
    sizer:AddSpacer(10)

    -- --------------------------------------
    -- Puzzle
    -- --------------------------------------
    f.puzzle = wx.wxChoice(
        f, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize,
        tablex.map(function(src) return src.display end, P.sources)
    )
    f.puzzle:Insert("All", 0)
    f.puzzle:SetSelection(0)
    f:Connect(
        f.puzzle:GetId(),
        wx.wxEVT_COMMAND_CHOICE_SELECTED,
        function (evt) layout.updateLayout() end
    )
    Add(f.puzzle, 'Puzzle')
    sizer:AddSpacer(10)

    -- --------------------------------------
    -- Date
    -- --------------------------------------
    f.dateDisplay = wx.wxStaticText(f, wx.wxID_ANY, "")
    f.prevDate = wx.wxBitmapButton(f, wx.wxID_ANY, P.bmp.left)
    f.nextDate = wx.wxBitmapButton(f, wx.wxID_ANY, P.bmp.right)
    f.current = wx.wxButton(f, wx.wxID_ANY, "Current")
    function f.DisplayDate(text)
        f.dateDisplay:SetLabel(text)
        f:Layout()
    end

    Add(wx.wxStaticText(f, wx.wxID_ANY, "Date: "))
    Add(f.prevDate)
    Add(f.dateDisplay)
    Add(f.nextDate)
    Add(f.current)

    function f.resetDate()
        f.date = date()
        f.date:sethours(0,0,0,0) -- Erase the time component
    end
    f.resetDate()
    f.dateSpan = { days = 7 }

    function f.onPrevDate(evt)
        layout.subtractDateSpan(f.date, f.dateSpan)
        layout.updateLayout()
    end
    function f.onNextDate(evt)
        layout.addDateSpan(f.date, f.dateSpan)
        layout.updateLayout()
    end
    function f.onCurrent(evt)
        f.resetDate()
        layout.updateLayout()
    end
    f:Connect(f.prevDate:GetId(), wx.wxEVT_COMMAND_BUTTON_CLICKED, f.onPrevDate)
    f:Connect(f.nextDate:GetId(), wx.wxEVT_COMMAND_BUTTON_CLICKED, f.onNextDate)
    f:Connect(f.current:GetId(), wx.wxEVT_COMMAND_BUTTON_CLICKED, f.onCurrent)


    -- --------------------------------------
    -- Week Day
    -- --------------------------------------
    f.weekday = wx.wxChoice(
        f, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize,
        { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" }
    )
    f.weekday:Insert("All", 0)
    f.weekday:SetSelection(0)
    f:Connect(
        f.weekday:GetId(),
        wx.wxEVT_COMMAND_CHOICE_SELECTED,
        function (evt) layout.updateLayout() end
    )
    Add(f.weekday, 'Week day')
    sizer:AddSpacer(10)

    -- --------------------------------------
    -- Download missing puzzles
    -- --------------------------------------
    f.downloadMissing = wx.wxButton(f, wx.wxID_ANY, "Download missing")
    Add(f.downloadMissing)
    f:Connect(
        f.downloadMissing:GetId(),
        wx.wxEVT_COMMAND_BUTTON_CLICKED,
        layout.downloadAll(function(dl) return not dl.fileexists end)
    )

    f:SetSizer(sizer)
end

-- Filter out all downloads that don't match
function layout.getFilteredSources()
    local puzzle = layout.filter.puzzle:GetSelection()
    if puzzle == 0 or not layout.current.puzzle then
        return P.sources
    else
        return { P.sources[puzzle] }
    end
end

function layout.updateLayout(view, puzzle, d, weekday)
    local f = layout.filter
    -- Change filter settings
    if view then
        f.view:SetStringSelection(view)
    end
    if puzzle then 
        f.puzzle:SetStringSelection(puzzle)
    end
    if d then
        f.date = d
    end
    if weekday then 
        f.weekday:SetSelection(weekday)
    end

    -- Save the filter
    P.filterState = {
        view = f.view:GetStringSelection(),
        puzzle = f.puzzle:GetStringSelection(),
        date = f.date,
        weekday = f.weekday:GetSelection(),
    }

    -- Update the layout
    layout.setLayout(P.filterState.view)
end
