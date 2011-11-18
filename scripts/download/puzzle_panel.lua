require 'date'
local ArrowButton = require 'download.arrow_button'
local PuzzleCtrl = require 'download.puzzle_ctrl'
local join = require 'pl.path'.join
require 'download.stats'

local function get_url(puzzle, d)
    return d:fmt(puzzle.url)
end

local function get_filename(puzzle, d)
    return join(download_dir, puzzle.prefix .. d:fmt(download_fmt) .. '.' .. puzzle.ext)
end


-- Display puzzles as a vertical list
local function ListPanel(parent, puzzle, start_date, end_date)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local d = start_date:copy()
    local today = date():sethours(0,0,0,0)
    while d <= end_date do
        local ctrl
        if puzzle.days[d:getisoweekday()] and d <= today then
            ctrl = PuzzleCtrl(panel, d:fmt("%x"), get_url(puzzle, d), get_filename(puzzle, d))
        else
            ctrl = wx.wxStaticText(panel, wx.wxID_ANY, d:fmt("%x"))
            ctrl:SetForegroundColour(wx.wxLIGHT_GREY)
        end
        sizer:Add(ctrl)
        d:adddays(1)
    end
    return panel
end

-- Display puzzles in a calendar view
local function MonthPanel(parent, puzzle, start_date)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxGridBagSizer(5, 5)
    panel:SetSizer(sizer)

    local function Add(ctrl, pos, flag, border)
        sizer:Add(
            ctrl, wx.wxGBPosition(pos[1], pos[2]), wx.wxGBSpan(1,1),
            flag or wx.wxALIGN_CENTER, border or 0)
    end

    -- Days of the week
    for i, label in pairs({"Su", "M", "Tu", "W", "Th", "F", "Sa"}) do
        Add(wx.wxStaticText(panel, wx.wxID_ANY, label), {0, i-1})
    end

    -- Month numbers and downloadCtrls    
    local d = start_date:copy():setday(1)
    local month = d:getmonth()
    local today = date():sethours(0,0,0,0)

    -- Do each day of the month
    local row = 1
    while d:getmonth() == month do
        local ctrl
        if puzzle.days[d:getisoweekday()] and d <= today then
            ctrl = PuzzleCtrl(panel, d:fmt("%d"), get_url(puzzle, d), get_filename(puzzle, d))
        else
            ctrl = wx.wxStaticText(panel, wx.wxID_ANY, d:fmt("%d"))
            ctrl:SetForegroundColour(wx.wxLIGHT_GREY)
        end
        Add(ctrl, {row, d:getweekday()-1}, wx.wxALIGN_CENTER)
        if d:getweekday() == 7 then
            row = row + 1
        end
        d:adddays(1)
    end
    return panel
end


local function PuzzlePanel(parent, puzzle)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local header = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(header, 0, wx.wxEXPAND)

    -- Expand button
    local button = ArrowButton(panel, wx.wxID_ANY)
    header:Add(button, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)

    -- Name
    local name = wx.wxStaticText(panel, wx.wxID_ANY, puzzle.name)
    name:SetFont(wx.wxFont(10, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD))
    header:Add(name, 1, wx.wxEXPAND + wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)

    -- Number of puzzles
    local puzzles = wx.wxStaticText(panel, wx.wxID_ANY, "(7 puzzles)")
    header:Add(puzzles, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- A line between this entry and the next
    local spacer = wx.wxWindow(
        panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(-1, 1))
    spacer:SetBackgroundColour(wx.wxColour(192, 192, 192))
    sizer:Add(spacer, 0, wx.wxEXPAND + wx.wxTOP + wx.wxBOTTOM, 5)

    -- ------------------------------------------------------------------------
    -- Body panel
    -- ------------------------------------------------------------------------
    -- The body panel is created when the user clicks on the arrow button
    local body
    local kind, start_date, end_date
    local function make_body()
        panel:Freeze()
        if kind == 'week' or kind == 'custom' then
            body = ListPanel(panel, puzzle,
                             start_date,
                             end_date or start_date:copy():adddays(6))
        elseif kind == 'month' then
            body = MonthPanel(panel, puzzle, start_date)
        end
        sizer:Insert(1, body, 0, wx.wxEXPAND + wx.wxLEFT, 20)
        sizer:Show(1, button:IsExpanded())
        panel:Thaw()
    end

    local function get_filenames()
        -- Return a list of filenames
        local filenames = {}
        local today = date():sethours(0,0,0,0)
        if kind == 'month' then
            local d = start_date:copy():setday(1)
            local month = d:getmonth()
            while d:getmonth() == month and d <= today do
                if puzzle.days[d:getisoweekday()] then
                    table.insert(filenames, get_filename(puzzle, d))
                end
                d:adddays(1)
            end
        elseif kind == 'day' then
            if puzzle.days[start_date:getisoweekday()] then
                table.insert(filenames, get_filename(puzzle, start_date))
            end
        else
            local d = start_date:copy()
            local end_ = end_date or start_date:copy():adddays(7)
            while d <= end_ and d <= today do
                if puzzle.days[d:getisoweekday()] then
                    table.insert(filenames, get_filename(puzzle, d))
                end
                d:adddays(1)
            end
        end
        return filenames
    end

    function panel:set_dates(kind_, start_date_, end_date_)
        kind = kind_
        start_date = start_date_
        end_date = end_date_
        local filenames = get_filenames()
        if #filenames == 1 then
            puzzles.Label = "(1 puzzle)"
        else
            puzzles.Label = string.format("(%d puzzles)", #filenames)
        end
        -- Destroy the old body
        if body then
            sizer:Detach(body)
            body:Destroy()
            body = nil
        end
        -- Figure out if we should show the arrow
        if #filenames <= 1 then
            header:Show(button, false)
        else
            header:Show(button, true)
            if button:IsExpanded() then
                make_body()
            end
        end
        header:Layout()
        sizer:Layout()
        return filenames
    end

    -- Show / hide the body
    panel:Connect(wx.wxEVT_ARROW_CLICKED,
        function(evt)
            if kind == 'day' then
                return
            end
            -- Lazy creation of the body
            if not body then
                make_body()
            end
            sizer:Show(1, button:IsExpanded())
            panel.Parent:Layout()
            panel.Parent:FitInside()
            panel.Parent:Refresh()
        end)

    return panel
end

return PuzzlePanel