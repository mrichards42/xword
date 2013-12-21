--- A hyperlink styled button.
-- Uses a hand cursor and underlines itself on hover.
-- Sends wxEVT_COMMAND_BUTTON_CLICKED events.

--- Class TextButton
-- @section class

--- A TextButton.
-- @param parent Parent window
-- @param[opt=wxID_ANY] id Window id
-- @param[opt=""] label
-- @param[opt=wxDefaultPosition] pos
-- @param[opt=wxDefaultSize] size
-- @param[opt=0] style Uses wxStaticText styles
-- @function TextButton
return function(parent, id, label, pos, size, style)
    local self = wx.wxStaticText(
        parent, id or wx.wxID_ANY, label or "",
        pos or wx.wxDefaultPosition, size or wx.wxDefaultSize, style or 0
    )

    -- Hand cursor
    local cursor = wx.wxCursor(wx.wxCURSOR_HAND)
    self:SetCursor(cursor)
    cursor:delete()

    -- Show underline on hover
    self:Connect(wx.wxEVT_ENTER_WINDOW, function (evt)
        local font = self:GetFont()
        font:SetUnderlined(true)
        self:SetFont(font)
        font:delete()
        evt:Skip()
    end)

    self:Connect(wx.wxEVT_LEAVE_WINDOW, function (evt)
        local font = self:GetFont()
        font:SetUnderlined(false)
        self:SetFont(font)
        font:delete()
        evt:Skip()
    end)

    -- Send BUTTON_CLICKED event on click
    self:Connect(wx.wxEVT_LEFT_UP, function()
        self:GetEventHandler():ProcessEvent(
            wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self:GetId())
        )
    end)

    -- Stop flickering
    self:Connect(wx.wxEVT_ERASE_BACKGROUND, function() end)

    return self
end
