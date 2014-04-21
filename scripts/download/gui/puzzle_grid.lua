-- A Grid of puzzles

local _R = mod_path(...)
local _RR = mod_path(..., 2)

local tablex = require 'pl.tablex'

local stats = require(_RR .. 'stats')
local config = require(_RR .. 'config')

local PuzzlePopup = require(_R .. 'puzzle_popup')

--- A grid of text items.
local function TextGrid(parent, x_gap, y_gap)
    local self = wx.wxPanel(parent, wx.wxID_ANY)
    self.data_grid = {} -- data_grid[col][row] = data
    self.data_item = {} -- data_item[data] = {col, row, text, [data]}
    self.item_text = {} -- { {col, row, text, [data]} }
    self.item_width, self.item_height = 0, 0
    x_gap, y_gap = x_gap or 5, y_gap or 5

    --- Add an item or static text to the panel.
    -- @param text The text to display
    -- @param col the column (0-indexed)
    -- @param row the row (0-indexed)
    -- @param[opt] data Custom item data for hyperlinked text.
    function self:Add(text, col, row, data)
        local item = {col, row, text, data}
        table.insert(self.item_text, item)
        if data then
            if not self.data_grid[col] then self.data_grid[col] = {} end
            self.data_grid[col][row] = data
            self.data_item[data] = item
        end
        -- Figure out the item size
        local w, h = self:GetTextExtent(text, self:get_style(data or text))
        if w > self.item_width then self.item_width = w end
        if h > self.item_height then self.item_height = h end
    end

    --- Get the item rectangle.
    -- @return x, y, w, h
    local function get_rect(col, row)
        return col * (self.item_width + x_gap),
               row * (self.item_height + y_gap),
               self.item_width,
               self.item_height
    end
    
    --- Override wxWindow::Fit to set a better min size
    function self:Fit()
        -- Find the bottom-right most item
        local max_col, max_row = 0, 0
        for _, item in ipairs(self.item_text) do
            local col, row = unpack(item)
            if col > max_col then max_col = col end
            if row > max_row then max_row = row end
        end
        -- Calculate its rectangle
        local x, y, w, h = get_rect(max_col, max_row)
        -- Set our min size
        self:SetMinSize(wx.wxSize(x + w, y + h))
    end

    --- Override this to customize styles.
    -- @param item Item data, text, or nil.
    -- @return wxFont, wxColour
    function self:get_style()
        return self:GetFont(), wx.wxBLACK
    end

    --- Draw the item.
    -- @param item {col, row, text}
    -- @param dc The dc.
    --   If so, don't erase the item background
    local function draw_item(item, dc)
        -- Get text and rect
        local col, row, text, data = unpack(item)
        local x, y, w, h = get_rect(col, row)
        -- Set the style
        local font, color = self:get_style(data or text)
        dc:SetFont(font)
        dc:SetTextForeground(color)
        -- Draw the text
        dc:DrawLabel(text, wx.wxRect(x, y, w, h), wx.wxALIGN_CENTER)
    end

    --- Redraw just these items
    -- @param data Item data
    function self:RefreshItem(data)
        local item = self.data_item[data]
        if item then
            self:Refresh(true, wx.wxRect(get_rect(unpack(item))))
        end
    end

    self:Connect(wx.wxEVT_PAINT, function(evt)
        local dc = wx.wxPaintDC(self)
        for _, item in ipairs(self.item_text) do
            draw_item(item, dc)
        end
        dc:delete()
    end)

    --- Find the item at this position.
    -- @param pos A wxPosition
    -- @return Item data or nil
    function self:HitTest(pos)
        local x, y = pos:GetXY()
        local col = math.floor(x / (self.item_width + x_gap))
        local row = math.floor(y / (self.item_height + y_gap))
        local temp = self.data_grid[col]
        return temp and temp[row]
    end

    local active_item
    --- Is this item data the under the cursor?
    -- @param item The item data
    -- @return true/false
    function self:IsActive(item)
        return active_item == item
    end

    --- Return the item data under the cursor.
    function self:GetActiveItem()
        return active_item
    end

    --- Override this for custom hover behavior.
    -- @param data The newly active item data.  Can be nil.
    -- @param prev The previously active item data.  Can be nil.
    function self:OnHover() end

    -- The private OnHover handler.
    local function on_hover(item, prev)
        -- Refresh both items (does nothing if arg is nil)
        self:RefreshItem(item)
        self:RefreshItem(prev)
        -- Change the cursor
        if item then
            if not prev then
                -- Only need to change cursor if there was no previous item
                self:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))
            end
        else -- No item
            self:SetCursor(wx.wxCursor(wx.wxCURSOR_ARROW))
        end
        -- Process events
        wx.wxGetApp():Yield(true) -- Don't call recursively
        -- Pass to subclass
        self:OnHover(item, prev)
    end

    -- Set active_item on motion
    self:Connect(wx.wxEVT_MOTION, function(evt)
        local data = self:HitTest(evt:GetPosition())
        if data ~= active_item then
            -- Set the new active item
            local prev = active_item
            active_item = data
            -- Do something with the result
            on_hover(data, prev)
        end
        evt:Skip()
    end)

    -- nil avtive_data on evt_leave
    self:Connect(wx.wxEVT_LEAVE_WINDOW, function(evt)
        if active_item then
            local prev = active_item
            active_item = nil
            -- Call this to notify user code that there is no active item
            on_hover(active_item, prev)
        end
        evt:Skip()
    end)

    -- Clean up data on destroy
    self:Connect(self:GetId(), wx.wxEVT_DESTROY, function(evt)
        self.data_grid = nil
        self.data_item = nil
        self.item_text = nil
        self.item_width = nil
        self.item_height = nil
        evt:Skip()
    end)

    return self
end -- end function TextGrid

--- Return the config style given a stats flag.
local function get_style(flag)
    if flag == stats.MISSING then
        return config.styles.missing
    elseif flag == stats.EXISTS then
        return config.styles.downloaded
    elseif flag == stats.SOLVING then
        return config.styles.progress
    elseif flag == stats.COMPLETE then
        return config.styles.complete
    end
    return config.styles.unknown
end

--- Copy text to the clipboard.
local function copy_text(text)
    local clipBoard = wx.wxClipboard.Get()
    if clipBoard and clipBoard:Open() then
        clipBoard:SetData(wx.wxTextDataObject(tostring(text)))
        clipBoard:Flush() -- Make this available after we've exited
        clipBoard:Close()
    end
end

-- Update when stats are updated
local CTRLS = {} -- CTRLS[PuzzleGrid][filename] = puzzle
stats:connect(stats.EVT_STATS, function(filename)
    for ctrl, puzzles in pairs(CTRLS) do
        local puzzle = puzzles[filename]
        if puzzle then
            ctrl:RefreshItem(puzzle)
            return
        end
    end
end)

--- A TextGrid of puzzles
local function PuzzleGrid(parent, x_gap, y_gap)
    local self = TextGrid(parent, x_gap, y_gap)
    CTRLS[self] = {}

    -- Save puzzles in the CTRLS[self] table
    local add = self.Add
    function self:Add(text, col, row, puzzle)
        add(self, text, col, row, puzzle)
        if puzzle then
            CTRLS[self][puzzle.filename] = puzzle
        end
    end

    --- Fetch stats.
    -- @param opts A table of options: force=true to recheck puzzles.
    function self:Update(opts)
        self:Connect(wx.wxEVT_IDLE, function()
            self:Disconnect(wx.wxEVT_IDLE) -- Only do this once
            local filenames = tablex.keys(CTRLS[self])
            stats:fetch{filenames, prepend=true, force=(opts and opts.force)}
        end)
    end

    -- Show puzzles under the cursor with an underline
    function self:get_style(puzzle)
        local filename = type(puzzle) == 'table' and puzzle.filename
        if filename then
            -- Puzzle
            local style = get_style(stats.map[filename])
            if self:IsActive(puzzle) then
                -- If this puzzle is under the cursor add an underline
                local font = wx.wxFont(style.font)
                font:SetUnderlined(true)
                return font, style.color
            else
                return style.font, style.color
            end
        else -- Not a puzzle
            return config.styles.unknown.font, wx.wxColour(100,100,100)
        end
    end

    local popup
    --- Show a popup and underline on hover
    function self:OnHover(puzzle)
        -- Destroy popup
        if puzzle then
            if not popup then
                popup = PuzzlePopup(self)
                -- Notify when the popup is destroyed
                popup:Connect(popup:GetId(), wx.wxEVT_DESTROY, function(evt)
                    popup = nil
                    evt:Skip()
                end)
            end
            -- Show it
            popup:set_puzzle(puzzle)
            popup:Popup(wx.wxSHOW_EFFECT_BLEND, 100)
        elseif popup then
            popup:HideWithEffect(wx.wxSHOW_EFFECT_BLEND, 100)
            popup:Destroy()
        end
    end

    -- Open/download puzzle
    self:Connect(wx.wxEVT_LEFT_UP, function ()
        local puzzle = self:GetActiveItem()
        if puzzle then
            puzzle:open()
        end
    end)

    local context_puzzle
    -- Context menu:
    -- Copy URL
    -- Copy local filename
    -- Redownload
    local menu = wx.wxMenu()
    self:Connect(
        menu:Append(wx.wxID_ANY, "Copy URL"):GetId(),
        wx.wxEVT_COMMAND_MENU_SELECTED,
        function () copy_text(context_puzzle.url) end
    )
    self:Connect(
        menu:Append(wx.wxID_ANY, "Copy local filename"):GetId(),
        wx.wxEVT_COMMAND_MENU_SELECTED,
        function () copy_text(context_puzzle.filename) end
    )
    self:Connect(
        menu:Append(wx.wxID_ANY, "Redownload"):GetId(),
        wx.wxEVT_COMMAND_MENU_SELECTED,
        function () context_puzzle:download() end
    )

    self:Connect(wx.wxEVT_CONTEXT_MENU, function()
        context_puzzle = self:GetActiveItem()
        if context_puzzle then
            self:PopupMenu(menu)
        end
    end)
    
    -- Cleanup when we are destroyed
    self:Connect(self:GetId(), wx.wxEVT_DESTROY, function(evt)
        CTRLS[self] = nil
        menu:delete()
        evt:Skip()
    end)

    self:Update()
    return self
end -- end function PuzzleGrid

return PuzzleGrid