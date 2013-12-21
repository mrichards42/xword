--- wxListCtrl mixin classes

local M = {}

local function create_check_bitmap(win, flag, w, h)
    w = w or 16
    h = h or 16
    -- Create a bitmap from wxRendererNative
    local bmp = wx.wxBitmap(w, h)
    local dc = wx.wxMemoryDC()
    dc:SelectObject(bmp)
    dc:Clear()
    wx.wxRendererNative.GetDefault():DrawCheckBox(win, dc, wx.wxRect(0, 0, w, h), flag)
    dc:SelectObject(wx.wxNullBitmap)
    dc:delete()
    return bmp
end

--- CheckListMixin
-- @section check

--- A mixin that adds a check box to a wxListCtrl.
-- @param self An existing wxListCtrl
function M.CheckListMixin(self)
    -- Check icons
    self.images = wx.wxImageList(16, 16, 2)
    self.images:Add(create_check_bitmap(self, 0, 16, 16))
    self.images:Add(create_check_bitmap(self, wx.wxCONTROL_CHECKED, 16, 16))
    self:SetImageList(self.images, wx.wxIMAGE_LIST_SMALL)

    local insert_item = self.InsertItem
    --- Override wxListCtrl:InsertItem.
    -- @param index Insert before this index
    -- @param label The item label
    -- @param[opt=false] is_checked Is this item checked?
    -- @return The new item's index
    function self:InsertItem(index, label, is_checked)
        local item = wx.wxListItem()
        item:SetId(index)
        item:SetImage(is_checked and 1 or 0)
        item:SetAlign(wx.wxLIST_FORMAT_CENTRE)
        index = insert_item(self, item)
        self:SetItem(index, 1, label)
        return index
    end

    --- Is this item checked?
    -- @param index Item index
    -- @return true/false
    function self:IsChecked(index)
        local item = wx.wxListItem()
        item:SetId(index)
        item:SetMask(wx.wxLIST_MASK_IMAGE)
        item:SetColumn(0)
        self:GetItem(item)
        return item:GetImage() == 1
    end

    --- Check or uncheck this item.
    -- @param index Item index
    -- @param[opt=true] check Is this item checked?
    function self:CheckItem(index, check)
        if check == nil then check = true end
        local is_checked = self:IsChecked(index)
        if check and not is_checked then
            self:SetItemImage(index, 1)
        elseif not check and is_checked then
            self:SetItemImage(index, 0)
        end
    end

    --- Toggle this item.
    -- @param index Item index
    function self:ToggleItem(index)
        self:CheckItem(index, not self:IsChecked(index))
    end

    -- Toggle check on left down
    self:Connect(wx.wxEVT_LEFT_DOWN, function(evt)
        local index, flags = self:HitTest(evt:GetPosition())
        if flags == wx.wxLIST_HITTEST_ONITEMICON then
            self:ToggleItem(index)
        end
        evt:Skip()
    end)

    -- Toggle check on space
    self:Connect(wx.wxEVT_CHAR, function(evt)
        if evt.KeyCode == wx.WXK_SPACE then
            local item = -1
            while true do
                item = self:GetNextItem(item, wx.wxLIST_NEXT_ALL,
                                        wx.wxLIST_STATE_SELECTED)
                if item == -1 then
                    break
                end
                self:ToggleItem(item)
            end
        end
        evt:Skip()
    end)
end

--- AutoWidthMixin
-- @section check

--- A mixin that auto-resizes the last column.
-- @param self An existing wxListCtrl
function M.AutoWidthMixin(self)
    -- Most everything here is copied from wxPython
    -- Make the last column extend to the end of the ctrl
    local __resizeColMinWidth = nil
    function doResize()
        if self.Size.Height < 32 then
            return -- avoid an endless update bug when the height is small
        end
        local resizeCol = self:GetColumnCount() - 1
        if resizeCol < 0 then return end -- Nothing to resize
        if not __resizeColMinWidth then
            __resizeColMinWidth = self:GetColumnWidth(resizeCol)
        end

        -- We're showing the vertical scrollbar -> allow for scrollbar width
        -- NOTE: on GTK, the scrollbar is included in the client size, but on
        -- Windows it is not included
        local listWidth = self.ClientSize.Width
        if not wx.__WXMSW__ then
            if self:GetItemCount() > self:GetCountPerPage() then
                local scrollWidth = wx.wxSystemSettings.GetMetric(wx.wxSYS_VSCROLL_X)
                listWidth = listWidth - scrollWidth
            end
        end

        local totColWidth = 0 -- Width of all columns except last one.
        for col=0,resizeCol-1 do
            totColWidth = totColWidth + self:GetColumnWidth(col)
        end

        if totColWidth + __resizeColMinWidth > listWidth then
            -- We haven't got the width to show the last column at its minimum
            -- width -> set it to its minimum width and allow the horizontal
            -- scrollbar to show.
            self:SetColumnWidth(resizeCol, __resizeColMinWidth)
            return
        end

        -- Resize the last column to take up the remaining available space.
        self:SetColumnWidth(resizeCol, listWidth - totColWidth)
    end

    self:Connect(wx.wxEVT_SIZE, function (evt) doResize() evt:Skip() end)
    self:Connect(wx.wxEVT_COMMAND_LIST_COL_END_DRAG, function (evt) doResize() evt:Skip() end)
end

return M
