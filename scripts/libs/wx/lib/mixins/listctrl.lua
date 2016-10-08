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
        item:SetMask(wx.wxLIST_MASK_FORMAT + wx.wxLIST_MASK_IMAGE)
        index = insert_item(self, item)
        if index == 0 then -- Autosize the first column
            self:SetColumnWidth(0, wx.wxLIST_AUTOSIZE)
        end
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
    self:Connect(wx.wxEVT_CHAR_HOOK, function(evt)
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
            -- Don't skip, since spacebar also sends an "activate" event
            return
        end
        evt:Skip()
    end)
end

--- AutoWidthMixin
-- @section width

--- A mixin that auto-resizes the last column.
-- @param self An existing wxListCtrl
function M.AutoWidthMixin(self)
    -- Most everything here is copied from wxPython
    -- Make the last column extend to the end of the ctrl
    local __resizeColMinWidth = nil
    local function doResize()
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


--- DragAndDropMixin
-- @section drag

-- Return the header size of a list ctrl
local function get_header_size(self)
    local rect = wx.wxRect()
    self:GetItemRect(self:GetTopItem(), rect)
    return rect.Y
end

--- A mixin that allows rearranging list items via drag and drop
-- @param self An existing wxListCtrl
function M.DragAndDropMixin(self)
    -- Sorting
    -- -------
    local order = {} -- list_idx = sort_idx

    local function SortFunc(data1, data2)
        local sort1 = order[data1]
        local sort2 = order[data2]
        if sort1 > sort2 then
            return 1
        elseif sort1 < sort2 then
            return -1
        else
            return 0
        end
    end

    local function SortList()
        self:SortItems(SortFunc, 0) -- Dummy extra argument
    end

    local function get_item_order(idx)
        return order[self:GetItemData(idx)]
    end

    local function set_item_order(idx, sort_order)
        order[self:GetItemData(idx)] = sort_order
    end
    function self:GetOrder() return order end

    -- Calculate sort order of an item
    -- @param idx The new index of the item
    -- @param[opt=false] insert
    --   If true, calculate based on where the item would be if inserted.
    --   If false, calculate based on where the item already is.
    local function get_sort_order(idx, insert)
        local last_item = self:GetItemCount() - 1
        if idx > 0 then
            if idx < last_item + (insert and 1 or 0) then
                -- Between two items: average
                local above = get_item_order(idx - 1)
                local below = get_item_order(idx + (insert and 0 or 1))
                return (above + below) / 2
            else
                -- At the end: last item + 1
                return get_item_order(idx-1) + 1
            end
        else
            if idx < last_item then
                -- At the beginning: first item - 1
                return get_item_order(0) - 1
            else
                -- This is the first item
                return 0
            end
        end
    end

    local insert_item = self.InsertItem
    --- Override wxListCtrl:InsertItem to keep track of our order
    function self:InsertItem(...)
        local idx = insert_item(self, ...)
        -- Set item data
        local list_id = #order + 1
        self:SetItemData(idx, list_id)
        -- Set initial order
        order[list_id] = get_sort_order(idx)
        return idx -- Return index for InsertItem
    end

    -- Drag Scroll
    --------------
    -- Pause between scrolls
    local SCROLL_TIMEOUT = 200

    -- When the timer fires, scroll the specified direction
    local scroll_timer = wx.wxTimer(self)
    local scroll_direction

    local header_size = 0
    -- Get the wxRect that represents the area of the list items
    local function get_list_rect()
        if header_size == 0 and self:GetItemCount() > 0 then
            header_size = get_header_size(self)
        end
        local size = self:GetSize()
        return wx.wxRect(0, header_size, size.Width, size.Height - header_size)
    end

    -- Stop scrolling
    local function end_scroll()
        scroll_timer:Stop()
        scroll_direction = nil
    end
    
    -- Set the scroll direction for the next timer
    local function check_scroll(pos)
        local rect = get_list_rect()
        rect:Deflate(0, 10)
        if pos.Y < rect.Y then
            scroll_direction = 'up'
        elseif pos.Y > rect.Bottom then
            scroll_direction = 'down'
        else
            end_scroll()
        end
        if scroll_direction then
            scroll_timer:Start(SCROLL_TIMEOUT)
        end
        rect:delete()
    end

    self:Connect(wx.wxEVT_TIMER, function(evt)
        if scroll_direction == 'up' then
            self:EnsureVisible(self:GetTopItem() - 1)
        elseif scroll_direction == 'down' then
            self:EnsureVisible(self:GetTopItem() + self:GetCountPerPage())
        else
            end_scroll()
        end
    end)

    -- Drag and drop
    -- -------------
    local drag_idx
    local last_hint
    
    -- Return the position that an item should be inserted, and the
    -- item under the cursor.
    -- If the cursor is in the top half of the item, return the item
    -- If the cursor is in the bottom half of the item, return the next item
    local function GetInsertHint(pos)
        local cursor, flags = self:HitTest(pos)
        local idx = cursor
        if cursor ~= -1 then
            local rect = wx.wxRect()
            self:GetItemRect(idx, rect)
            if (rect.Bottom - pos.Y) < (rect.Height / 2) then
                idx = idx + 1
            end
            rect:delete()
            return idx, cursor
        end
    end

    -- Draw a hint line for this item
    function self:DrawHint(dc, idx)
        if idx and idx > -1 then
            local rect = wx.wxRect()
            if idx < self:GetItemCount() then
                self:GetItemRect(idx, rect)
            else
                self:GetItemRect(idx - 1, rect)
                rect.Y = rect.Y + rect.Height
            end
            dc:DrawLine(rect.X, rect.Y, rect.Width, rect.Y)
            rect:delete()
        end
    end

    -- Start dragging
    self:Connect(wx.wxEVT_COMMAND_LIST_BEGIN_DRAG, function(evt)
        self:CaptureMouse()
        drag_idx = evt:GetIndex()
        last_hint = nil
        evt:Skip()
    end)
    
    -- Draw a hint as the cursor moves
    self:Connect(wx.wxEVT_MOTION, function(evt)
        evt:Skip()
        if not self:HasCapture() then drag_idx = nil end
        if not drag_idx then return end
        local pos = evt:GetPosition()
        local idx, cursor = GetInsertHint(pos)
        local dc = wx.wxClientDC(self)
        dc:SetLogicalFunction(wx.wxINVERT)
        -- Erase old hint
        if last_hint then
            self:DrawHint(dc, last_hint)
        end
        -- Scroll if needed (before drawing)
        check_scroll(pos)
        -- Draw new hint
        if idx then
            self:DrawHint(dc, idx)
        end
        last_hint = idx
        dc:delete()
    end)

    -- Move the item
    self:Connect(wx.wxEVT_LEFT_UP, function(evt)
        evt:Skip()
        if not self:HasCapture() then drag_idx = nil end
        if not drag_idx then return end
        self:ReleaseMouse()
        local new_idx = GetInsertHint(evt:GetPosition())
        if new_idx and new_idx ~= drag_idx then
            -- Move item and resort the list
            set_item_order(drag_idx, get_sort_order(new_idx, true))
            SortList()
        end
        drag_idx = nil
        end_scroll()
        self:Refresh()
    end)

    self:Connect(wx.wxEVT_MOUSE_CAPTURE_LOST, function(evt)
        if self:HasCapture() then
            self:ReleaseMouse()
        end
        end_scroll()
        drag_idx = nil
    end)
end

return M
