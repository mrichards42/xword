--- A wxTextBox with autocomplete, intended to mimic a wxComboBox

--- Class TextCombo
-- @section class

--- A TextCombo.
-- @param parent Parent window
-- @param id Window id
-- @function TextCombo
return function(parent, id, value, pos, size, choices_, style)
    local is_readonly = bit.band(style or 0, wx.wxCB_READONLY) ~= 0
    style = bit.band(style, bit.bnot(wx.wxCB_READONLY))

    local self = wx.wxTextCtrl(
        parent, id or wx.wxID_ANY, value or "",
        pos or wx.wxDefaultPosition, size or wx.wxDefaultSize, style or 0
    )

    local choices = {}

    -- Require a valid choice?
    if is_readonly then
        self:Connect(wx.wxEVT_KILL_FOCUS, function(evt)
            evt:Skip()
            local str = self:GetValue()
            -- Check for value in choices
            for _, v in ipairs(choices) do
                if str == v then return end
            end
            -- Not valid
            self:SetValue('')
        end)
    end

    --- Set the choices for this control.
    -- Uses AutoComplete
    -- @param choices a table of string choices
    function self:SetChoices(choices_)
        choices = choices_
        self:AutoComplete(choices)
    end

    self:SetChoices(choices_)

    return self
end
