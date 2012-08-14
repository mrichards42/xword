local bmp = require 'download.gui.bmp'
BmpButton = require 'download.gui.bmp_button'

local function get_collapsed()
    return bmp.arrows:GetSubBitmap(wx.wxRect(0, 0, 10, 10))
end

local function get_expanded()
    return bmp.arrows:GetSubBitmap(wx.wxRect(10, 0, 10, 10))
end


local function ArrowButton(parent, id)
    local ctrl = BmpButton(parent, id or wx.wxID_ANY, get_collapsed())
    ctrl.collapsed = true

    function ctrl:IsCollapsed()
        return self.collapsed
    end

    function ctrl:IsExpanded()
        return not self.collapsed
    end

    function ctrl:Collapse()
        self.collapsed = true
        self:SetBitmap(get_collapsed())
    end

    function ctrl:Expand()
        self.collapsed = false
        self:SetBitmap(get_expanded())
    end

    function ctrl:Toggle()
        if self.collapsed then
            self:Expand()
        else
            self:Collapse()
        end
    end

    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function(evt)
            ctrl:Toggle()
            evt:Skip()
        end)

    return ctrl
end

return ArrowButton
