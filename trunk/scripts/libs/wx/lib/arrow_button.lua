--- A BmpToggleButton toggling betweel two arrows.

local _R = string.match(..., "^.+%.") or "" -- relative require

local BmpToggleButton = require(_R..'toggle_button')

--- Class ArrowButton
-- @section class

--- A ToggleButton.
-- @param parent Parent window
-- @param[opt=wxID_ANY] id Window id
-- @function ArrowButton
return function(parent, id)
    -- Load the bitmaps
    local bmp = require(_R .. 'bmp')(_R .. 'arrows')
    local collapsed = bmp:GetSubBitmap(wx.wxRect(0,0,10,10))
    local expanded = bmp:GetSubBitmap(wx.wxRect(10,0,10,10))
    bmp:delete()
    -- Return the button
    return BmpToggleButton(parent, id or wx.wxID_ANY, collapsed, expanded)

end
