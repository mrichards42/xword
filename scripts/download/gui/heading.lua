local _R = (string.match(..., '^.+%.') or ... .. '.')

local date = require 'date'

local wxfb = require(_R .. 'wxFB')
local PuzzleGrid = require (_R .. 'puzzle_grid')

local function MonthPanel(parent, source, year, month)
    local self = PuzzleGrid(parent)

    -- Add the labels
    for i, label in pairs({"Su", "M", "Tu", "W", "Th", "F", "Sa"}) do
        self:Add(label, i-1, 0)
    end
    -- Add the days
    local d = date(year, month, 1) -- First day of the month
    local today = date()
    local row = 1
    local col = d:getweekday() - 1 -- Find the col of the first day
    while d:getmonth() == month do
        -- Get the puzzle (or just the day number if no puzzle)
        local puzzle
        if d <= today and source:has_puzzle(d) then
            puzzle = source:get_puzzle(d)
        end
        self:Add(d:fmt('%d'), col, row, puzzle)
        -- Increment
        d:adddays(1)
        col = col + 1
        if col > 6 then
            col = 0
            row = row + 1
        end
    end
    self:Fit()
    return self
end -- end function MonthPanel

local function ListPanel(parent, puzzles, fmt)
    local self = PuzzleGrid(parent)
    for i, puzzle in ipairs(puzzles) do
        self:Add(puzzle.date:fmt(fmt or '%m/%d'), i-1, 0, puzzle)
    end
    self:Fit()
    return self
end -- end function ListPanel

-- Return a child for this heading
local function MakeChild(self, parent)
    if not parent then parent = self end
    if self.kind == 'month' then
        return MonthPanel(parent, self.source, self.start_date:getyear(), self.start_date:getmonth())
    else
        local puzzles = self.source:get_puzzles(self.start_date, self.end_date)
        local fmt = self.kind == 'week' and '%a, %d' or '%m/%d'
        return ListPanel(parent, puzzles, fmt)
    end
end -- end function MakeChild

-- ----------------------------------------------------------------------------
-- The DownloadHeading panel
-- ----------------------------------------------------------------------------
local function DownloadHeading(parent, source)
    local self = wxfb.DownloadHeading(parent)
    self.label:SetLabel(source.name)
    self:GetSizer():Hide(self.line)
    self.source = source
    self.child = false
    self.start_date = false
    self.end_date = false
    self.kind = false

    -- Update the panel when dates change
    function self:UpdatePuzzles(start_date, end_date, kind)
        self.start_date = start_date
        self.end_date = end_date
        self.kind = kind
        -- Destroy the child panel
        if self.child then
            self:GetSizer():Detach(self.child)
            self.child:Destroy()
            self.child = false
        end
        -- Alternate layout if this is just one day
        if self.kind == 'day' then
            -- Replace download_button with the puzzle
            self.child = MakeChild(self, self.download_button:GetParent())
            local sizer = self.download_button:GetContainingSizer()
            sizer:Hide(self.download_button)
            sizer:Hide(self.expand_button) -- Already expanded
            sizer:Add(self.child)
            sizer:Layout()
        else
            -- Update the puzzle count
            local today = date():sethours(0,0,0)
            if start_date <= today and end_date > today then end_date = today end
            local count, missing = source:puzzle_count(start_date, end_date)
            self.download_button:SetLabel(
                ("%d %s"):format(count, count == 1 and 'Puzzle' or 'Puzzles')
            )
            self.download_button:SetToolTip(
                ("Download %d missing %s"):format(missing, missing == 1 and 'puzzle' or 'puzzles')
            )
            -- Make sure everything is shown
            local sizer = self.download_button:GetContainingSizer()
            sizer:Show(self.download_button)
            sizer:Show(self.expand_button)
            sizer:Layout()
            -- Keep expanded if we were already expanded
            if self.expand_button:IsExpanded() then
                self:OnExpand()
            end
        end
    end

    -- Connect events
    function self:OnExpand(evt)
        local sizer = self:GetSizer()
        if not self.child then
            self.child = MakeChild(self)
            self:GetSizer():Insert(2, self.child, 0, wx.wxEXPAND + wx.wxALL, 10)
        end
        sizer:Show(self.child, self.expand_button:IsExpanded())
        sizer:Show(self.line, self.expand_button:IsExpanded())
        self:Layout()
        self:GetParent():FitInside()
    end

    function self:OnLabelClick(evt)
        if self.kind == 'day' then
            self.child:open_puzzle()
        else
            self.expand_button:Toggle()
            self:OnExpand()
        end
    end

    function self:OnDownloadMissing(evt)
        local today = date()
        source:download(
            self.start_date,
            self.end_date > today and today or self.end_date
        )
    end

    return self
end -- end function DownloadHeading

return DownloadHeading