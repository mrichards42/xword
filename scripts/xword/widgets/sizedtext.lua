-- ---------------------------------------------------------------------------
-- SizedTextCtrl

-- A StaticTextCtrl with no width, which truncates or wraps text if it is too
-- long. If the text is too long (or is more than one line), it shows an arrow
-- that can be used to expand the text.
-- When the text is expanded or contracted, the control will fire events:
-- wxEVT_ST_CHANGED
-- ---------------------------------------------------------------------------

local split = require 'pl.stringx'.split
local join = require 'pl.path'.join


-- Events: We have to reuse other event IDs, because wxLua doesn't have a
-- custom event mechanism
wx.wxEVT_ST_CHANGED = wx.wxEVT_COMMAND_BUTTON_CLICKED

-- Styles
wx.wxST_TRUNCATE = 1
wx.wxST_WRAP = 2

-- ----------------------------------------------------------------------------
-- Text shortening functions
-- ----------------------------------------------------------------------------

-- Wrap
local function wrap_line(text, dc, width)
    local lines = {}
    local words = split(text, ' ')
    if #words == 0 then return '' end
    local this_line = ''
    repeat
        local test_line = this_line .. words[1] .. ' '
        if dc:GetTextExtent(test_line) <= width or this_line == '' then
            this_line = test_line
            table.remove(words, 1)
        else
            table.insert(lines, this_line)
            this_line = ''
        end
    until #words == 0
    if this_line ~= '' then
        table.insert(lines, this_line)
    end
    return table.concat(lines, '\n')
end

local function wrap_text(text, dc, width)
    local lines = {}
    for _, line in ipairs(split(text, '[\n\r\f]')) do
        table.insert(lines, wrap_line(line, dc, width))
    end
    return table.concat(lines, '\n')
end

-- Truncate
local function truncate_text(text, dc, width)
    local max_length = (text:find('\n') or #text + 1) - 1
    -- Start with the full string and get shorter
    local n = max_length
    local truncated = text:sub(1, max_length)
    repeat
        truncated = text:sub(1, n) .. (n < max_length and "..." or "")
        local x = dc:GetTextExtent(truncated)
        n = n - 1
    until x <= width or n <= 0
    return truncated
end

-- ----------------------------------------------------------------------------
-- The SizedTextCtrl
-- ----------------------------------------------------------------------------

-- Bitmap of the two arrows
local arrows

local function SizedTextCtrl(parent, id, text, pos, size, style)
    local ctrl = wx.wxWindow(parent,
                             id or -1,
                             pos or wx.wxDefaultPosition,
                             size or wx.wxDefaultSize,
                             style or 0)
    ctrl.text = text
    ctrl.display_text = ''
    ctrl.mode = wx.wxST_TRUNCATE
    -- This will be an empty rect if there is no arrow
    ctrl.arrowRect = wx.wxRect()

    -- Create the arrows bitmap
    if not arrows then
        arrows = wx.wxBitmap(
            join(xword.scriptsdir, 'xword', 'widgets', 'arrows.png'),
            wx.wxBITMAP_TYPE_PNG
        )
    end

    function ctrl:IsExpanded()
        return self.mode == wx.wxST_WRAP
    end

    function ctrl:IsCollapsed()
        return self.mode == wx.wxST_TRUNCATE
    end

    -- Truncate or wrap the text on size
    function ctrl:UpdateSize()
        local dc = wx.wxClientDC(self)
        dc:SetFont(self:GetFont())
        local width = dc:GetSize() - 16 -- space for a drop-down arrow
        local showArrow = false
        if self.mode == wx.wxST_TRUNCATE then
            self.display_text = truncate_text(self.text, dc, width)
            showArrow = (self.display_text ~= self.text)
            self:SetMinSize(wx.wxSize(10, dc:GetCharHeight()))
        else
            self.display_text = wrap_text(self.text, dc, width)
            -- This shouldn't return the string, just x and y . . . ?
            local str, x, y = dc:GetMultiLineTextExtent(self.display_text)
            self:SetMinSize(wx.wxSize(10, y))
            showArrow = self.display_text:find('\n') ~= nil
        end
        if showArrow then
            self.arrowRect = wx.wxRect(width + 3, 3, 10, 10)
        else
            self.arrowRect = wx.wxRect()
        end
        dc:delete()
        --ctrl:Refresh()
    end

    ctrl:Connect(wx.wxEVT_SIZE, function (evt) ctrl:UpdateSize() end)

    -- The arrow should display a pointer cursor
    ctrl:Connect(wx.wxEVT_MOTION, function(evt)
        if ctrl.arrowRect:Contains(evt:GetPosition()) then
            ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))
        else
            ctrl:SetCursor(wx.wxSTANDARD_CURSOR)
        end
        evt:Skip()
    end)

    -- LeftDown on the arrow expands/contracts the text
    ctrl:Connect(wx.wxEVT_LEFT_DOWN, function(evt)
        if ctrl.arrowRect:Contains(evt:GetPosition()) then
            if ctrl.mode == wx.wxST_TRUNCATE then
                ctrl.mode = wx.wxST_WRAP
            else
                ctrl.mode = wx.wxST_TRUNCATE
            end
            ctrl:UpdateSize()
            -- Fire the event
            wx.wxPostEvent(
                ctrl:GetEventHandler(),
                wx.wxCommandEvent(wx.wxEVT_ST_CHANGED, ctrl:GetId())
            )
        end
    end)

    -- Draw the text and arrow
    ctrl:SetBackgroundStyle(wx.wxBG_STYLE_PAINT)
    ctrl:Connect(wx.wxEVT_ERASE_BACKGROUND, function (evt) end)
    ctrl:Connect(wx.wxEVT_PAINT, function (evt)
        local dc = wx.wxAutoBufferedPaintDC(ctrl)
        dc:SetBackground(wx.wxBrush(ctrl:GetParent():GetBackgroundColour(), wx.wxSOLID))
        dc:Clear()
        dc:SetFont(ctrl:GetFont())
        dc:SetTextForeground(ctrl:GetForegroundColour())
        local width, height = dc:GetSize()
        dc:DrawLabel(ctrl.display_text, wx.wxRect(0, 0, width, height))
        -- Draw the drop-down triangle
        if not ctrl.arrowRect:IsEmpty() then
            local x, y = ctrl.arrowRect.X, ctrl.arrowRect.Y
            dc:DrawBitmap(arrows:GetSubBitmap(
                    wx.wxRect(ctrl.mode == wx.wxST_TRUNCATE and 0 or 10, 0, 10, 10)
                ), x, y, true)
        end
        dc:delete()
    end)

    return ctrl
end

return SizedTextCtrl
