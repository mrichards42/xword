
local function PopupWindow(parent, id, pos, size)
    if not pos then
        pos = wx.wxPoint(0,0)
    end
    -- Offset position by parent window
    local parent_pos = parent:GetScreenPosition()
    pos.X = pos.X + parent_pos.X
    pos.Y = pos.Y + parent_pos.Y
    -- Create a dialog without a border
    local win = wx.wxDialog(
        parent, id, '', pos or wx.wxDefaultPosition, size or wx.wxDefaultSize,
        wx.wxFRAME_NO_TASKBAR + wx.wxFRAME_FLOAT_ON_PARENT + wx.wxWS_EX_TRANSIENT 
    )
    return win
end

return PopupWindow
