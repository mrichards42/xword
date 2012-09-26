-- An html-like hyperlink button
local function TextButton(parent, id, label, pos, size, style)
    local ctrl = wx.wxStaticText(parent, id or wx.wxID_ANY, label,
                                 pos or wx.wxDefaultPosition,
                                 size or wx.wxDefaultSize,
                                 style or 0)
    ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))

    -- Show underline on hover
    ctrl:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            local font = ctrl.Font
            font:SetUnderlined(true)
            ctrl.Font = font
            evt:Skip()
        end)

    ctrl:Connect(wx.wxEVT_LEAVE_WINDOW,
        function (evt)
            local font = ctrl.Font
            font:SetUnderlined(false)
            ctrl.Font = font
            evt:Skip()
        end)

    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function(evt)
            ctrl:GetEventHandler():ProcessEvent(
            wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, ctrl:GetId()))
        end)

    return ctrl
end

return TextButton
