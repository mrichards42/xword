local _R = string.match(..., '^.+%.')

local ArrowButton = require(_R .. 'arrow_button')
local TextButton = require(_R .. 'text_button')

-- A panel with a title and a collapse button
local function CollapsePanel(parent, id, label)
    local self = wx.wxPanel(parent, id or wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

    -- Header
    local header = wx.wxBoxSizer(wx.wxHORIZONTAL)

    self.arrow = ArrowButton(self, wx.wxID_ANY)
    header:Add(self.arrow, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)

    self.label = TextButton(self, wx.wxID_ANY, label)
    header:Add(self.label, 0, wx.wxALIGN_CENTER_VERTICAL)

    sizer:Add(header, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)
    self:SetSizer(sizer)

    local to_manage = {}
    local managed_items = {} -- { wxSizer = { items, ... } }
    -- Turn to_manage into managed_items
    local function make_items()
        local is_window = wx.wxClassInfo("wxWindow")
        for _, obj in ipairs(to_manage) do
            -- Get the containing sizer
            local sizer
            if obj:IsKindOf(is_window) then
                sizer = obj:GetContainingSizer()
            else -- it's a wxSizer
                sizer = obj:GetContainingWindow():GetSizer()
            end
            -- Add to managed_items
            if not managed_items[sizer] then managed_items[sizer] = {} end
            table.insert(managed_items[sizer], obj)
        end
        to_manage = nil
    end
    -- Show or Hide all but the first sizer item
    local function show_children(show)
        -- Find containing sizers
        if to_manage then make_items() end
        -- Show or hide managed items
        for sizer, items in pairs(managed_items) do
            for _, item in ipairs(items) do
                sizer:Show(item, show)
            end
            sizer:Layout()
        end
        -- Show or hide child items in the internal sizer
        local count = sizer:GetChildren():GetCount()
        if count > 1 then
            for i=1,count-1 do
                sizer:Show(i, show)
            end
            sizer:Layout()
        end
        self:GetParent():Layout()
        self:GetParent():FitInside()
    end

    -- Toggle the panel when the arrow or text is clicked
    self.arrow:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function()
        show_children(self:IsExpanded())
    end)
    
    self.label:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function ()
        self:Toggle()
    end)

    -- Public functions

    --- Add a window or sizer to the internal sizer.
    -- @param ... Arguments to wxSizer:Add
    function self:Add(...)
        sizer:Add(...):Show(self:IsExpanded())
    end

    --- Manage an existing item in a sizer
    -- @param obj A window or wxSizer
    function self:AddSibling(obj)
        table.insert(to_manage, obj)
    end

    --- Change the label.
    -- @param label String to show as a label
    function self:SetLabel(label)
        self.label:SetLabel(label)
    end

    --- Show the child panel(s).
    function self:Expand()
        self.arrow:Expand()
        show_children(true)
    end

    --- Hide the child panel(s).
    function self:Collapse()
        self.arrow:Collapse()
        show_children(true)
    end

    --- Are children shown?
    function self:IsExpanded()
        return self.arrow:IsExpanded()
    end

    --- Toggle between expanded and collapsed.
    function self:Toggle()
        self.arrow:Toggle()
        show_children(self:IsExpanded())
    end

    return self
end

return CollapsePanel