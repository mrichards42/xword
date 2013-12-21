--- A wxBitmapButton with no border.
-- Uses a hand cursor.
-- Sends wxEVT_COMMAND_BUTTON_CLICKED events.

--- Class BmpButton
-- @section class

--- A BmpButton.
-- @param parent Parent window
-- @param id Window id
-- @param bmp The bitmap
-- @return a wxStaticBitmap formatted to use as a button
-- @function BmpButton
return function(parent, id, bmp)
    local self = wx.wxStaticBitmap(parent, id or wx.wxID_ANY, bmp)

    self.bmp = bmp
    self.disabled = false

    local cursor = wx.wxCursor(wx.wxCURSOR_HAND)
    self:SetCursor(cursor)
    cursor:delete()

    local setbitmap = self.SetBitmap
    --- Delete the old bitmap when a new one is set.
    function self:SetBitmap(bmp)
        self.bmp:delete()
        self.bmp = bmp
        setbitmap(self, bmp)
    end

    local enable = self.Enable
    -- Show a grayed-out bitmap on disable.
    function self:Enable(doit)
        if doit == nil then doit = true end
        if doit then
            setbitmap(self, self.bmp)
        else
            -- Create and set a disabled bitmap
            if not self.disabled then
                self.disabled = wx.wxBitmap(bmp:ConvertToImage():ConvertToGreyscale())
            end
            setbitmap(self, self.disabled)
        end
        enable(self, doit)
    end

    function self:Disable()
        self:Enable(false)
    end

    -- Send BUTTON_CLICKED event on click
    self:Connect(wx.wxEVT_LEFT_UP, function()
        self:GetEventHandler():ProcessEvent(
            wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self:GetId())
        )
    end)

    -- Stop flickering
    self:Connect(wx.wxEVT_ERASE_BACKGROUND, function() end)

    -- Delete bitmaps on destroy
    self:Connect(self:GetId(), wx.wxEVT_DESTROY, function(evt)
        self.bmp:delete()
        if self.disabled then self.disabled:delete() end
        evt:Skip()
    end)

    return self
end
