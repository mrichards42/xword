--- A grid similar to wxListCtrl that contains controls

local _R = string.match(..., '^.+%.') -- Relative require
local TextButton = require(_R .. 'text_button')

--- Class ControlGrid
-- @section class

--- A ControlGrid.
-- @param parent Parent window
-- @param id Window id
-- @function ControlGrid
return function(parent, id)
    -- The scroller
    local self = wx.wxScrolledWindow(parent, id or wx.wxID_ANY)
    self:SetWindowStyle(wx.wxBORDER_SIMPLE)
    -- Internal list panel
    local panel = wx.wxPanel(self, wx.wxID_ANY)
    self:SetTargetWindow(panel)
    -- Main list sizer
    local sizer = wx.wxFlexGridSizer(0, 0, 0, 0)
    panel:SetSizer(sizer)

    -- List data
    local col_data = {}
    local ctrls = {}

    -- Auto add (see Auto Add section)
    local auto_add = false

    -- List Header
    -- -----------
    local renderer = wx.wxRendererNative.GetDefault()
    local header_height = renderer:GetHeaderButtonHeight(self)

    -- Move list panel OnSize
    self:Connect(wx.wxEVT_SIZE, function(evt)
        local size = self:GetClientSize()
        panel:SetSize(0, header_height, size.Width, size.Height - header_height)
        size:delete()
        evt:Skip()
    end)
    
    -- Eliminate flicker
    self:Connect(wx.wxEVT_ERASE_BACKGROUND, function(evt) end)

    -- Draw the header using RendererNative
    self:Connect(wx.wxEVT_PAINT, function(evt)
        local dc = wx.wxPaintDC(self)
        local x, y = self:CalcScrolledPosition(0,0)
        y = 0
        for col, data in ipairs(col_data) do
            local params = wx.wxHeaderButtonParams()
            params.m_labelText = data.label
            local ctrl = self:GetCtrl(0, col-1)
            local size
            if auto_add and col == #col_data then
                -- Last column (delete col) takes the rest of the space.
                size = self:GetSize()
                size.Width = size.Width - x
            elseif ctrl then
                -- Ctrl size
                size = ctrl:GetSize()
            else
                -- No items in the control
                -- Use ctrl width / number of columns for the column width
                size = self:GetSize()
                size.Width = size.Width / #col_data
            end
            local rect = wx.wxRect(x, y, size.Width, header_height)
            renderer:DrawHeaderButton(self, dc, rect, 0, wx.wxHDR_SORT_ICON_NONE, params)
            x = x + size.Width
            size:delete()
            rect:delete()
            params:delete()
        end
        dc:delete()
    end)

    -- Refresh OnScroll
    local function OnScroll(evt)
        if evt:GetOrientation() == wx.wxHORIZONTAL then
            self:Refresh()
        end
    end
    self:Connect(wx.wxEVT_SCROLLWIN_TOP, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_BOTTOM, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_LINEUP, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_LINEDOWN, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_PAGEUP, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_PAGEDOWN, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_THUMBTRACK, OnScroll)
    self:Connect(wx.wxEVT_SCROLLWIN_THUMBRELEASE, OnScroll)
    panel:Connect(wx.wxEVT_CHILD_FOCUS, function(evt) self:Refresh() evt:Skip() end)

    -- Get/Set
    -- -------

    local function _set_value(col, ctrl, value)
        local func = ctrl['Set' .. col_data[col+1].key]
        func(ctrl, value)
    end

    local function _get_value(col, ctrl)
        local func = ctrl['Get' .. col_data[col+1].key]
        return func(ctrl)
    end

    --- Get the control at the specified row and column
    -- @param row (0-based)
    -- @param col (0-based)
    function self:GetCtrl(row, col)
        local ctrl_row = ctrls[row+1]
        if ctrl_row then
            return ctrl_row[col+1]
        end
    end

    --- Get the value in the control at the specified row and column
    -- @param row
    -- @param col
    function self:GetValue(row, col)
        local ctrl = self:GetCtrl(row, col)
        if ctrl then
            return _get_value(col, ctrl)
        end
    end

    --- Return a table of all data
    function self:GetData()
        local data = {}
        local last_row = #ctrls - 1 - (auto_add and 1 or 0)
        local last_col = #col_data - 1 - (auto_add and 1 or 0)
        for row = 0, last_row do
            local r = {}
            for col = 0, last_col do
                table.insert(r, self:GetValue(row, col))
            end
            if #r > 0 then table.insert(data, r) end
        end
        return data
    end

    -- Auto Add
    -- --------

    local function _add_auto_row()
        auto_add = false -- let self:Insert actually put this row at the end
        -- Add a blank row
        self:Insert(#ctrls)
        -- Add a new blank row if the value on this row is changed
        local _ctrl_value
        for i, ctrl in ipairs(ctrls[#ctrls]) do
            local handler = wx.wxEvtHandler()
            handler:Connect(wx.wxEVT_SET_FOCUS, function(evt)
                evt:Skip()
                _ctrl_value = _get_value(i, ctrl)
            end)
            handler:Connect(wx.wxEVT_KILL_FOCUS, function(evt)
                evt:Skip()
                local v = _get_value(i, ctrl)
                if v and v ~= "" and v ~= _ctrl_value then
                    -- Disconnect events and add a new row
                    ctrl:RemoveEventHandler(handler)
                    _add_auto_row()
                end
            end)
            handler:Connect(wx.wxEVT_DESTROY, function(evt)
                ctrl:RemoveEventHandler(handler)
            end)
            ctrl:PushEventHandler(handler)
        end
        auto_add = true
    end

    --- Enable/disalbe auto-add.
    -- Auto-add keeps an empty row at the bottom of the grid.
    -- When a new entry is added, create a new auto-add row
    function self:SetAutoAdd(doit)
        if auto_add and not doit then
            -- Remove the old blank row
            self:Remove(#ctrls - 1)
        elseif not auto_add and doit then
            -- Add a delete column
            local function NewTextButton(parent)
                local ctrl = TextButton(parent, wx.wxID_ANY, 'X')
                -- Delete the row on click
                ctrl:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
                    -- Find this ctrl in the ctrls table
                    for row, ctrl_row in ipairs(ctrls) do
                        c = ctrl_row[#ctrl_row]
                        -- Don't delete the auto_add row
                        if c == ctrl and (not auto_add or row < #ctrls) then
                            self:Remove(row-1) -- 0-based
                            return
                        end
                    end
                end)
                return ctrl
            end
            self:AddColumn("", NewTextButton, 'Label', 1, wx.wxALIGN_CENTER)
            -- Add the blank row
            _add_auto_row()
        end
        auto_add = doit
    end

    -- Main API
    -- --------

    --- Add a column to the grid.
    -- @param label
    -- @param constructor a function taking the parent window
    -- @param[opt='Value'] value_key The title of Get/Set functions
    --    e.g. [Get/Set]Value, [Get/Set]StringSelection, etc.
    -- @param[opt] proportion proportion for growable cols
    -- @param[opt=wx.wxEXPAND] flags sizer flags
    function self:AddColumn(label, constructor, value_key, proportion, flags)
        local pos = #col_data + (auto_add and 0 or 1)
        table.insert(col_data, pos, {label=label, constructor=constructor, key=value_key or 'Value', flags=flags or wx.wxEXPAND})
        sizer:SetCols(#col_data)
        if proportion then
            sizer:AddGrowableCol(#col_data - 1, proportion)
        end
    end

    --- Add a row to the grid
    -- @param ... Values for each column
    function self:Append(...)
        -- Ensure that the auto-add row is always last
        self:Insert(#ctrls, ...)
    end

    local function _update_scrollbars()
        panel:Layout()
        local vsize = sizer:GetMinSize()
        vsize:SetHeight(vsize:GetHeight() + header_height)
        panel:SetVirtualSize(vsize)
        vsize:delete()
        self:FitInside()
    end

    --- Insert a row in the grid before idx
    -- @param idx Insert before this index (0-based)
    -- @param ... Values for each column
    function self:Insert(idx, ...)
        -- Ensure that the auto-add row is always last
        if auto_add and idx >= #ctrls then
            idx = #ctrls - 1
        end
        local sizer_idx = idx * sizer:GetCols()
        local values = {...}
        local ctrl_row = {}
        local next_ctrl = idx+1 <= #ctrls and ctrls[idx+1][1] -- tab order
        sizer:SetRows(sizer:GetRows() + 1)
        for col, data in ipairs(col_data) do
            local ctrl = data.constructor(panel, wx.wxID_ANY)
            table.insert(ctrl_row, ctrl)
            local value = values[col]
            if value then _set_value(col-1, ctrl, value) end
            sizer:Insert(sizer_idx + col - 1, ctrl, 0,  data.flags, 1)
            if next_ctrl then
                ctrl:MoveBeforeInTabOrder(next_ctrl)
            end
        end
        table.insert(ctrls, idx + 1, ctrl_row)
        -- Layout and adjust scrollbars
        _update_scrollbars()
        -- Adjust scroll rate to one row
        self:SetScrollRate(5, sizer:GetSize():GetHeight() / #ctrls)
    end

    --- Remove a row
    -- @param idx (0-based)
    function self:Remove(idx)
        for _, ctrl in ipairs(ctrls[idx+1]) do
            sizer:Detach(ctrl)
            ctrl:Destroy()
        end
        sizer:SetRows(sizer:GetRows() - 1)
        _update_scrollbars()
        table.remove(ctrls, idx+1)
    end

    return self
end
