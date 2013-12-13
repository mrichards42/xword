require 'date'
local ArrowButton = require 'download.gui.arrow_button'
local PuzzleCtrl = require 'download.puzzle_ctrl'
local TextButton = require 'download.gui.text_button'
require 'download.stats'
require 'download.config'

-- Display puzzles as a vertical list
local function ListPanel(parent, puzzle, dates)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local space = 10
    local sizer = wx.wxFlexGridSizer(0, 7, space, space)
    panel:SetSizer(sizer)

    local item_size = 0
    for _, d in ipairs(dates) do
        local text = d:fmt("%a %m/%d")
        local ctrl = PuzzleCtrl(panel, text, download.get_download_data(puzzle, d))
        item_size = math.max(item_size, ctrl.Size.Width)
        sizer:Add(ctrl, 1, wx.wxEXPAND)
    end

    -- Wrap the items
    panel:Connect(wx.wxEVT_SIZE,
        function(evt)
            local cols = (evt.Size.Width) / (item_size + space)
            if cols > 0 then
                sizer.Cols = math.floor(cols)
            end
            evt:Skip()
        end)

    return panel
end

-- Display puzzles in a calendar view
local function MonthPanel(parent, puzzle, start_date)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxGridBagSizer(10, 10)
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
            ctrl = PuzzleCtrl(panel, d:fmt("%d"), download.get_download_data(puzzle, d))
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
    panel.puzzle = puzzle

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local header = wx.wxPanel(panel, wx.wxID_ANY)
    header:SetBackgroundColour(wx.wxColour(240, 240, 240))
    sizer:Add(header, 0, wx.wxEXPAND)

    local border = wx.wxBoxSizer(wx.wxHORIZONTAL)
    header:SetSizer(border)

    local header_sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    border:Add(header_sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    -- Expand button
    local expand_button = ArrowButton(header, wx.wxID_ANY)
    header_sizer:Add(expand_button, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)

    -- Name
    local name = TextButton(header, wx.wxID_ANY, puzzle.name)
    name:SetFont(wx.wxFont(10, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD))
    header_sizer:Add(name, 1, wx.wxEXPAND + wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)

    -- Display number of puzzles with a link to download
    local download_button = TextButton(header, wx.wxID_ANY, "")
    download_button:SetToolTip("Download Missing Puzzles")
    header_sizer:Add(download_button, 0, wx.wxALIGN_CENTER_VERTICAL)

    local header_puzzle

    -- A line between this entry and the next
    local spacer = wx.wxWindow(
        panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(-1, 1))
    spacer:SetBackgroundColour(wx.wxColour(192, 192, 192))
    sizer:Add(spacer, 0, wx.wxEXPAND)

    -- ------------------------------------------------------------------------
    -- Body panel
    -- ------------------------------------------------------------------------
    -- The body panel is created when the user clicks on the arrow button
    local body
    local kind, start_date, end_date, custom_func
    panel.dates = false
    local function make_body(dates)
        panel:Freeze()
        if kind == 'week' or kind == 'custom' then
            body = ListPanel(panel, puzzle, panel:get_dates())
        elseif kind == 'month' then
            body = MonthPanel(panel, puzzle, start_date)
        end
        -- Add a border to the body
        local border = wx.wxBoxSizer(wx.wxVERTICAL)
        border:Add(body.Sizer, 1, wx.wxEXPAND + wx.wxALL, 10)
        body:SetSizer(border, false) -- don't destroy the old sizer

        sizer:Insert(1, body, 1, wx.wxEXPAND + wx.wxLEFT, 20)
        sizer:Show(body, expand_button:IsExpanded())
        panel:Thaw()
    end

    -- ------------------------------------------------------------------------
    -- Functions
    -- ------------------------------------------------------------------------
    function panel:get_dates(refresh)
        -- Cache dates
        if self.dates and not refresh then return self.dates end
        local today = date():sethours(0,0,0,0)
        -- Return a list of dates
        local function do_get_dates(start_, end_, func)
            if end_ > today then end_ = today end
            if not func then
                func = function() return true end
            end
            local dates = {}
            local d = start_:copy()
            while d <= end_ do
                if puzzle.days[d:getisoweekday()] and func(puzzle, d) then
                    table.insert(dates, d:copy())
                end
                d:adddays(1)
            end
            self.dates = dates
            return dates
        end
        if kind == 'month' then
            return do_get_dates(start_date, end_date)
        elseif kind == 'day' then
            return do_get_dates(start_date, start_date)
        elseif kind == 'week' then
            return do_get_dates(start_date, start_date:copy():adddays(6))
        else -- Custom
            return do_get_dates(start_date, end_date, custom_func)
        end
    end

    function panel:get_download_data()
        local data = {}
        for _, d in ipairs(self:get_dates()) do
            local p = download.get_download_data(puzzle, d)
            -- Don't redownload files
            if not download.puzzle_exists(p.filename) then
                table.insert(data, p)
            end
        end
        return data
    end

    function panel:set_dates(kind_, start_date_, end_date_, custom_func_)
        kind = kind_
        start_date = start_date_
        end_date = end_date_
        custom_func = custom_func_
        -- Destroy the old body
        if body then
            sizer:Detach(body)
            body:Destroy()
            body = nil
        end
        -- Compute a list of valid dates
        local dates = self:get_dates(true) -- Don't use the cache
        -- Update the label
        panel.Parent.Sizer:Show(panel, true)
        if #dates == 0 then
            panel.Parent.Sizer:Show(panel, false)
        else
            download_button.Label = string.format("Download %d puzzles", #dates)
        end
        -- Adjust the header display
        if header_puzzle then
            header_sizer:Detach(header_puzzle)
            header_puzzle:Destroy()
            header_puzzle = nil
        end
        header_sizer:Show(download_button, true)
        if #dates <= 1 and kind ~= 'month' then
            header_sizer:Show(expand_button, false)
            if #dates == 1 then
                local d = dates[1]
                header_puzzle = PuzzleCtrl(header, d:fmt("%a %m/%d"),
                                           download.get_download_data(puzzle, d))
                header_sizer:Add(header_puzzle)
                header_sizer:Show(download_button, false)
            end
        else
            header_sizer:Show(expand_button, true)
            if expand_button:IsExpanded() then
                make_body()
            end
        end
        header_sizer:Layout()
        sizer:Layout()
        -- Turn the dates into filenames
        local filenames = {}
        for _, d in ipairs(dates) do
            table.insert(filenames, download.get_filename(puzzle, d))
        end
        return filenames, expand_button:IsExpanded()
    end

    -- Update the panel to reflect our stats
    function panel:update_stats()
        local count = {}
        local dates = panel:get_dates()
        for _, d in ipairs(dates) do
            local status = download.status_map[download.get_filename(puzzle, d)]
            if status then
                count[status] = (count[status] or 0) + 1
            else
                -- Short cut . . . don't display incomplete info
                download_button.Label = string.format('%d Puzzles', #dates)
                header:Layout()
                return
            end
        end
        local text = string.format("%d Puzzles (+%d, -%d)",
            #dates, count[download.COMPLETE] or 0, count[download.MISSING] or 0)
        download_button.Label = text
        header:Layout()
    end

    -- ------------------------------------------------------------------------
    -- Events
    -- ------------------------------------------------------------------------
    -- Show or hide the body panel
    local function update_body()
        -- Lazy creation of the body
        if not body then
            make_body()
            -- Fetch the files
            local dates = panel:get_dates()
            local filenames = {}
            for _, d in ipairs(dates) do
                table.insert(filenames, download.get_filename(puzzle, d))
            end
            download.fetch_stats{filenames, prepend = true}
        end
        panel.Parent:Freeze()
        sizer:Show(body, expand_button:IsExpanded())
        panel.Parent:Layout()
        panel.Parent:FitInside()
        panel.Parent:Thaw()
    end
    expand_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        update_body()
    end)

    name:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        if header_puzzle then
            header_puzzle:open_puzzle()
        else
            expand_button:Toggle()
            update_body()
        end
    end)

    -- Download puzzles
    download_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        -- Fetch the files
        local downloads = panel:get_download_data()
        if #downloads > 0 then
            download.add_downloads(downloads)
        end
        evt:Skip()
    end)

    return panel
end

return PuzzlePanel