
local function PopupWindow(parent, id, pos, size)
    -- Create a dialog without a border
    local win = wx.wxDialog(
        parent, id, '', wx.wxDefaultPosition, size or wx.wxDefaultSize,
        wx.wxFRAME_NO_TASKBAR + wx.wxFRAME_FLOAT_ON_PARENT + wx.wxWS_EX_TRANSIENT 
    )

    -- The corner that we are anchored to
    local anchor = wx.wxALIGN_TOP + wx.wxALIGN_LEFT
    local fit = win.Fit
    function win:Fit()
        -- wxWindow:Fit is simply SetSize(GetBestSize()), so we adjust both
        -- position and sized here
        -- Move the window after fit based on where the window should be
        -- anchored (set in win:Popup)
        local old_size = self.Size
        local best_size = self.BestSize
        local offsetx, offsety = 0,0
        if bit.band(anchor, wx.wxALIGN_RIGHT) ~= 0 then
            offsetx = old_size.Width - best_size.Width
        end
        if bit.band(anchor, wx.wxALIGN_BOTTOM) ~= 0  then
            offsety = old_size.Height - best_size.Height
        end
        local pos = self.Position
        self:SetSize(pos.X + offsetx, pos.Y + offsety, best_size.Width, best_size.Height)
    end

    function win:Popup(at_pos)
        -- If we are given a position, use that.
        -- If we were given a position in the constructor, use that
        -- Otherwise use the current pointer coordinates

        pos = at_pos or pos
        if pos then
            self:Move(pos)
        else
            -- Move the popup off of the cursor, and make the popup fit on the screen
            local screenx = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_X)
            local screeny = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_Y)
            local size = self.Size
            local pos = wx.wxGetMousePosition()
            local offsetx, offsety = 10,10
            -- Check right edge
            if pos.X + size.Width + 30 > screenx then
                offsetx = -size.Width - 10
                anchor = anchor + wx.wxALIGN_RIGHT
            end
            -- Check bottom edge
            if pos.Y + size.Height + 30 > screeny then
                offsety = -size.Height - 10
                anchor = anchor + wx.wxALIGN_BOTTOM
            end
            self:Move(pos.X + offsetx, pos.Y + offsety)
        end

        self:Show()
        return
    end

    return win
end

return PopupWindow