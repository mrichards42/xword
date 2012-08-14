-- A basic scrolled panel
local function ScrolledPanel(parent)
    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)

    function scroller:update_scrollbars()
        self.panel:Layout()
        self:FitInside()
        self:Refresh()
    end

    -- This panel preserves tab behavior
    local psizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local panel = wx.wxPanel(scroller, wx.wxID_ANY)
    psizer:Add(panel, 1, wx.wxEXPAND)
    scroller:SetSizer(psizer)

    scroller.panel = panel
    return scroller
end

return ScrolledPanel