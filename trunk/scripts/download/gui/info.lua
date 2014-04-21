local _R = string.match(..., '^.+%.') -- Relative require

local bmp = require(_R .. 'bmp')
local TextButton = require 'wx.lib.text_button'

-- A wxInfoBar subclass to use with wxFB
return function(parent)
    local self = wx.wxInfoBar(parent, wx.wxID_ANY)
    local sizer = self:GetSizer()

    -- Place a border on the text item and remove the border from the icon item
    sizer:GetItem(0):SetFlag(wx.wxALIGN_CENTER + wx.wxTOP + wx.wxLEFT + wx.wxBOTTOM)
    local text_item = sizer:GetItem(1)
    text_item:SetFlag(wx.wxALIGN_CENTER + wx.wxALL + wx.wxEXPAND)
    text_item:SetBorder(5)

    -- Don't let the text write over the close button
    text_item:SetProportion(1)
    text_item:GetWindow():SetWindowStyle(8) -- wxST_ELLIPSIZE_MIDDLE
    sizer:GetItem(2):SetProportion(0) -- Don't stretch this spacer

    -- Replace the ugly red X button
    local button = sizer:GetItem(3):GetWindow():DynamicCast('wxBitmapButton')
    local normal = bmp.sub.close(0, 0, 16, 16)
    button:SetBitmapLabel(normal)
    normal:delete()
    local hover = bmp.sub.close(16, 0, 16, 16)
    button:SetBitmapHover(hover)
    hover:delete()
    local pressed = bmp.sub.close(32, 0, 16, 16)
    button:SetBitmapSelected(pressed)
    pressed:delete()

    -- Add a hyperlink button to the right side of the info bar
    self.link = TextButton(self)
    sizer:Insert(3, self.link, 0, wx.wxALIGN_CENTER + wx.wxALL, 5)

    return self
end