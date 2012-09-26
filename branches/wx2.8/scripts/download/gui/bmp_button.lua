-- A bitmap button with no border
function BmpButton(parent, id, bmp)
    local ctrl = wx.wxStaticBitmap(parent, id or wx.wxID_ANY, bmp)

    ctrl.bmp = bmp
    ctrl.disabled = wx.wxBitmap(bmp:ConvertToImage():ConvertToGreyscale())

    ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))

    local enable = ctrl.Enable
    function ctrl:Enable(doit)
        if doit == nil then doit = true end
        if doit then
            ctrl:SetBitmap(ctrl.bmp)
        else
            ctrl:SetBitmap(ctrl.disabled)
        end
        enable(ctrl, doit)
    end

    function ctrl:Disable()
        ctrl:Enable(false)
    end

    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function(evt)
                ctrl:GetEventHandler():ProcessEvent(
                wx.wxCommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, ctrl:GetId()))
        end)

    return ctrl
end

return BmpButton
