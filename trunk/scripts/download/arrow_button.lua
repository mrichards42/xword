local bmp = require 'download.bmp'

local function get_collapsed()
    return bmp.arrows:GetSubBitmap(wx.wxRect(0, 0, 10, 10))
end

local function get_expanded()
    return bmp.arrows:GetSubBitmap(wx.wxRect(10, 0, 10, 10))
end


-- Events: We have to reuse other event IDs, because wxLua doesn't have a
-- custom event mechanism
wx.wxEVT_ARROW_CLICKED = wx.wxEVT_COMMAND_BUTTON_CLICKED

local function ArrowButton(parent, id)
    local ctrl = wx.wxStaticBitmap(parent, id or wx.wxID_ANY, get_collapsed())
    ctrl.collapsed = true
    ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))

    function ctrl:IsCollapsed()
        return self.collapsed
    end

    function ctrl:IsExpanded()
        return not self.collapsed
    end

    function ctrl:Collapse()
        self.collapsed = true
        self:SetBitmap(get_collapsed())
        wx.wxPostEvent(
            self:GetEventHandler(),
            wx.wxCommandEvent(wx.wxEVT_ARROW_CLICKED, self:GetId())
        )
    end

    function ctrl:Expand()
        self.collapsed = false
        self:SetBitmap(get_expanded())
        wx.wxPostEvent(
            self:GetEventHandler(),
            wx.wxCommandEvent(wx.wxEVT_ARROW_CLICKED, self:GetId())
        )
    end

    function ctrl:Toggle()
        if self.collapsed then
            self:Expand()
        else
            self:Collapse()
        end
    end

    ctrl:Connect(wx.wxEVT_LEFT_DOWN, function(evt) ctrl:Toggle() end)

    return ctrl
end

return ArrowButton
