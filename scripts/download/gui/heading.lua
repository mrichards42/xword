local _R = (string.match(..., '^.+%.') or ... .. '.')

local date = require 'date'

local wxfb = require(_R .. 'wxFB')
local PuzzleGrid = require (_R .. 'puzzle_grid')

-- Return a child for this heading
local function MakeChild(self, parent)
    if not parent then parent = self end
    if self.kind == 'month' then
        return PuzzleGrid.Month(parent, self.source, self.start_date:getyear(), self.start_date:getmonth())
    else
        local puzzles = self.source:get_puzzles(self.start_date, self.end_date)
        local fmt = self.kind == 'week' and '%a, %d' or '%m/%d'
        return PuzzleGrid.List(parent, puzzles, fmt)
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