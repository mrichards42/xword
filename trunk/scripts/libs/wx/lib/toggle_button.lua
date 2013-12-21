--- A `BmpButton` toggling between two bitmaps.

--- Class BmpToggleButton
-- @section class

--- A BmpToggleButton.
-- @param parent Parent window
-- @param id Window id
-- @param collapsed The collapsed bitmap
-- @param expanded The expanded bitmap
-- @function BmpToggleButton
return function(parent, id, collapsed, expanded)
    local self = wx.wxStaticBitmap(parent, id, collapsed)

    local cursor = wx.wxCursor(wx.wxCURSOR_HAND)
    self:SetCursor(cursor)
    cursor:delete()

    local is_collapsed = true

    --- Is this in a collapsed state?
    -- @return true/false
    function self:IsCollapsed()
        return is_collapsed
    end

    --- Is this in an expanded state?
    -- @return true/false
    function self:IsExpanded()
        return not is_collapsed
    end

    --- Show the collapsed bitmap.
    function self:Collapse()
        is_collapsed = true
        self:SetBitmap(collapsed)
    end

    --- Show the expanded bitmap.
    function self:Expand()
        is_collapsed = false
        self:SetBitmap(expanded)
    end

    --- Toggle between Expand and Collapse.
    function self:Toggle()
        if is_collapsed then
            self:Expand()
        else
            self:Collapse()
        end
    end

    -- Toggle and send event
    self:Connect(wx.wxEVT_LEFT_UP, function(evt)
        self:Toggle()
        self:GetEventHandler():ProcessEvent(
            wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self:GetId())
        )
        evt:Skip()
    end)

    -- Stop flickering
    self:Connect(wx.wxEVT_ERASE_BACKGROUND, function() end)

    -- Delete bitmaps on destroy
    self:Connect(self:GetId(), wx.wxEVT_DESTROY, function(evt)
        collapsed:delete()
        expanded:delete()
        evt:Skip()
    end)

    return self
end
