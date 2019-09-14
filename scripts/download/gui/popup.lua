local AnimateMove = require 'wx.lib.animate'
local MOUSE_OFFSET = 10
local SCREEN_TOLERANCE = 30
local ANIMATE = false

local function PopupWindow(parent)
    -- A borderless dialog
    local self = wx.wxDialog(
        parent, wx.wxID_ANY, '', wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxFRAME_NO_TASKBAR + wx.wxFRAME_FLOAT_ON_PARENT + wx.wxWS_EX_TRANSIENT
    )

    -- wxWindow:Fit is simply SetSize(GetBestSize()), but we need to adjust
    -- both position and size
    function self:Fit()
        -- Figur out where we should anchor the window
        local screenx = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_X)
        local screeny = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_Y)
        local size = self:GetBestSize()
        local pos = wx.wxGetMousePosition()
        local offsetx, offsety = MOUSE_OFFSET, MOUSE_OFFSET
        -- Check right edge
        if pos.X + size.Width + SCREEN_TOLERANCE > screenx then
            offsetx = -size.Width - MOUSE_OFFSET
        end
        -- Check bottom edge
        if pos.Y + size.Height + SCREEN_TOLERANCE> screeny then
            offsety = -size.Height - MOUSE_OFFSET
        end
        -- Move the window (animate if already on screen)
        local Move = self:IsShown() and ANIMATE and AnimateMove or self.SetSize
        Move(self, pos.X + offsetx, pos.Y + offsety, size.Width, size.Height)
    end

    -- Remove the parent event handler and release the mouse on destroy.
    local evt_handler
    self:Connect(self:GetId(), wx.wxEVT_DESTROY, function(evt)
        evt:Skip()
        if evt_handler then
            parent:RemoveEventHandler(evt_handler)
        end
        if parent:HasCapture() then
            parent:ReleaseMouse()
        end
    end)

    --- Popup the window
    function self:Popup(effect, timeout)
        -- Move to the best position based on the current mouse pointer
        self:Fit()
        if effect and effect ~= wx.wxSHOW_EFFECT_NONE then
            self:ShowWithEffect(effect, timeout or 0)
        else
            -- On Mac, Show() causes focus to move to the popup.
            -- Unclear why this is inconsistent between platforms, but since the
            -- effect isn't supported anyway, this works fine.
            self:ShowWithoutActivating()
        end

        -- Destroy the popup when the mouse leaves the parent window.
        if evt_handler then return end

        -- Push an event handler
        local parent = self:GetParent()
        evt_handler = wx.wxEvtHandler()
        parent:PushEventHandler(evt_handler)
        -- Used for a few event handlers
        local function on_leave(evt)
            self:Destroy()
            if evt then evt:Skip() end
        end
        if wx.__WXMSW__ then
            -- The LEAVE_WINDOW event works on windows . . .
            evt_handler:Connect(parent:GetId(), wx.wxEVT_LEAVE_WINDOW, on_leave)
        else
            -- . . . but not on mac.
            -- Check to see if the cursor is within the parent rect
            evt_handler:Connect(wx.wxEVT_MOTION, function (evt)
                local rect = wx.wxRect(parent:GetSize())
                if not rect:Contains(evt:GetPosition()) then
                    on_leave()
                end
                evt:Skip()
            end)
            evt_handler:Connect(wx.wxEVT_MOUSE_CAPTURE_LOST, on_leave)
            -- Capture the mouse and handle motion events
            parent:CaptureMouse()
        end
    end

    return self
end

return PopupWindow
