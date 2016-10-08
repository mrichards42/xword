--- A replacement for wxCheckListBox.
-- Uses `wxListView` with @{mixins.listctrl.CheckListMixin|CheckListMixin} and @{mixins.listctrl.AutoWidthMixin|AutoWidthMixin}

local _R = string.match(..., '^.+%.') -- Relative require
local mixins = require(_R .. 'mixins.listctrl')

--- Class CheckListCtrl
-- @section class

local UTF8_CHECK = "\226\156\147"
local UTF8_HEAVY_CHECK = "\226\156\148"

--- A CheckListCtrl
-- @param parent Parent window
-- @param[opt=wx.wxID_ANY] id Window id
-- @param[opt="Check"] col1 First column name
-- @param[opt="Value"] col2 Second column name
-- @function CheckListCtrl
return function(parent, id, col1, col2)
    local self = wx.wxListView(parent, id or wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxLC_REPORT + wx.wxLC_SINGLE_SEL)
    mixins.CheckListMixin(self)
    mixins.AutoWidthMixin(self)

    col1 = type(col1) == 'string' and col1 or UTF8_HEAVY_CHECK
    col2 = type(col2) == 'string' and col2 or "Value"

    local w = self:GetTextExtent(col1)
    self:InsertColumn(0, col1, wx.wxLIST_FORMAT_LEFT, -1)
    self:InsertColumn(1, col2, wx.wxLIST_FORMAT_LEFT, -1)
    self:SetColumnWidth(0, w + 15)

    --- Set the name of a column
    -- @param col 0-based index
    -- @param name Name
    function self:SetColumnName(col, name)
        local item = wx.wxListItem()
        item:SetText(name)
        self:SetColumn(col, item)
    end

    -- wxListBox functions

    --- Get the index of the selected item.
    -- @return index or -1
    -- @function self:GetSelection
    self.GetSelection = self.GetFirstSelected
    --- Set the selected item.
    -- @param idx The item index or -1 to remove the selection.
    -- @function self:SetSelection
    self.SetSelection = self.Select

    -- This function already exists:
    --- Is this item selected?
    -- @param idx The index
    -- @return true/false
    -- @function self:IsSelected

    --- Get the string value of an item.
    -- @param idx Item index
    -- @return the item's value
    function self:GetString(idx)
        return self:GetItemText(idx, 1)
    end

    --- Set the string value of an item.
    -- @param idx Item index
    -- @param label the item's value
    function self:SetString(idx, label)
        return self:SetItem(idx, 1, label)
    end

    --- Delete an item.
    -- @param idx Item index
    -- @function self:Delete
    self.Delete = self.DeleteItem

    --- Delete all items.
    -- @function self:Clear
    self.Clear = self.DeleteAllItems

    --- Set all values at once.
    -- @param items A table of labels, or a table of labels and check values.
    -- @usage
    -- check_list:Set({'one', 'two', 'three'})
    -- -- Check the first value
    -- check_list:Set({{'one', true}, 'two', 'three'})
    function self:Set(items)
        self:Clear()
        for i, item in ipairs(items) do
            if type(item) ~= 'table' then item = {item, 0} end
            self:InsertItem(i, unpack(item))
        end
    end

    return self
end