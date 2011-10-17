-- ----------------------------------------------------------------------------
-- CustomListCtrl

-- A list-type control that allows arbitrary windows to be the list items.
-- If you want padding between items, you must add that to each list item.
-- ----------------------------------------------------------------------------

-- Iterate over all children in a window, recursively (depth-first)
local function ichildren(window)
    local stack = {} -- { { list = {}, i = num, last = num}, ... }
    local list = window:GetChildren()
    table.insert(stack, { list = list, i = -1, last = list:GetCount() - 1})
    local function f()
        -- Find the current list (the last in the stack)
        local current
        while true do
            current = stack[#stack]
            if not current then
                return
            end
            current.i = current.i + 1
            if current.i ~= current.last then
                break
            end
            table.remove(stack)
        end
        -- Get the window
        local win = current.list:Item(current.i):GetData():DynamicCast('wxWindow')
        -- Add the window's children to the stack
        local list = win:GetChildren()
        if list:GetCount() > 0 then
            table.insert(stack, { list = list, i = -1, last = list:GetCount() - 1})
        end
        return win
    end
    return f
end


local function CustomListCtrl(parent, id, pos, size, style)
    local list = wx.wxScrolledWindow(parent,
                                     id or -1,
                                     pos or wx.wxDefaultPosition,
                                     size or wx.wxDefaultSize,
                                     style or wx.wxVSCROLL + wx.wxHSCROLL)

    list:SetBackgroundColour(wx.wxSystemSettings.GetColour(wx.wxSYS_COLOUR_WINDOW))
    list:SetScrollRate(10, 10)

    list.highlight = -1
    list.highlightColor = wx.wxColour(195, 210, 227)
    list.windows = {}
    list.spacers = {}
    list.sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    list:SetSizer(list.sizer)

    -- Create a list spacer with <pixels> height (or 1)
    local function MakeSpacer(pixels)
        local spacer = wx.wxWindow(
            list, wx.wxID_ANY,
            wx.wxDefaultPosition, wx.wxSize(-1, pixels or 1)
        )
        spacer:SetBackgroundColour(wx.wxColour(192, 192, 192))
        return spacer
    end

    -- Return a function that highlights the given window 
    local function OnLeftDown(win)
        return function(evt)
            local old = list.windows[list.highlight] or nil
            if old ~= win then
                -- Unhighlight the previous window
                if old then
                    old:SetBackgroundColour(wx.wxSystemSettings.GetColour(wx.wxSYS_COLOUR_WINDOW))
                    old:Refresh()
                end
                list.highlight = -1
                -- Highlight this window
                if win ~= list then
                    win:SetBackgroundColour(list.highlightColor)
                    win:Refresh()
                    for i, w in ipairs(list.windows) do
                        if w == win then
                            list.highlight = i
                            break
                        end
                    end
                end
            end
            list:Refresh()
            evt:Skip()
        end
    end
    list:Connect(wx.wxEVT_LEFT_DOWN, OnLeftDown(list))

    -- Add a window to the list, return the added index
    function list:Append(window)
        self.sizer:Add(window, 0, wx.wxEXPAND)
        table.insert(self.windows, window)
        local spacer = MakeSpacer()
        self.sizer:Add(spacer, 0, wx.wxEXPAND)
        table.insert(self.spacers, spacer)
        self.sizer:Layout()
        local leftdown = OnLeftDown(window)
        window:Connect(wx.wxEVT_LEFT_DOWN, leftdown)
        window:SetBackgroundColour(wx.wxSystemSettings.GetColour(wx.wxSYS_COLOUR_WINDOW))
        -- Make all the child windows also highlight this window OnLeftDown
        for child in ichildren(window) do
            child:Connect(wx.wxEVT_LEFT_DOWN, leftdown)
        end
        return #list.windows
    end

    -- Remove and destroy a window (and its spacer)
    function list:Remove(idx)
        if type(idx) == "userdata" then
            -- Find the window in the window list
            for i, win in ipairs(self.windows) do
                if win == idx then
                    idx = i
                    break
                end
            end
        end
        local window = table.remove(self.windows, idx)
        self.sizer:Detach(window)
        window:Destroy()
        local spacer = table.remove(self.spacers, idx)
        self.sizer:Detach(spacer)
        spacer:Destroy()
        self:UpdateScrollbars()
    end

    -- Remove and destroy all windows
    function list:Clear()
        -- wxLua doesn't have wxSizer::Clear()
        for _, win in ipairs(self.windows) do
            self.sizer:Detach(win)
            win:Destroy()
        end
        self.windows = {}
        for _, spacer in ipairs(self.spacers) do
            self.sizer:Detach(spacer)
            spacer:Destroy()
        end
        self.spacers = {}
        self:UpdateScrollbars()
    end

    function list:UpdateScrollbars()
        self:Layout()
        self:FitInside()
    end

    -- Sort the list items
    -- func should be approximately func(w1, w2) return w1 < w2 end
    function list:Sort(func)
        -- Detach all the windows
        for i, window in ipairs(self.windows) do
            list.sizer:Detach(window)
        end
        table.sort(self.windows, func)
        -- Insert all the windows
        for i, window in ipairs(self.windows) do
            self.sizer:Insert(i*2-2, window, 0, wx.wxEXPAND)
        end
        self:Layout()
        self:Refresh()
    end

    return list
end

return CustomListCtrl
