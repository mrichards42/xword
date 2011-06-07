-- Create a sizer of buttons given the parent and a list of strings.
-- Returns the sizer, and a list of buttons
local function ButtonSizer(parent, buttons)
    local bsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local button_list = {}
    for _, name in ipairs(buttons) do
        local b = wx.wxButton(parent, wx.wxID_ANY, name)
        table.insert(button_list, b)
        button_list[name] = b
        bsizer:Add(b, 0, wx.wxALL, 5)
    end
    return bsizer, button_list
end

return ButtonSizer