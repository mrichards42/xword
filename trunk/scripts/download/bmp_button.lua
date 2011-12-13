-- A bitmap button with no border
local function BmpButton(parent, id, bmp)
    local ctrl = wx.wxStaticBitmap(parent, id or wx.wxID_ANY, bmp)

    ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))

    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function(evt)
                ctrl:GetEventHandler():ProcessEvent(
                wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, ctrl:GetId()))
        end)

    return ctrl
end

return BmpButton
