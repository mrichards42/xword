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

    -- Show or Hide all but the first sizer item
    local function show_children(show)
        local count = sizer:GetChildren():GetCount()
        if count > 1 then
            for i=1,count-1 do
                sizer:Show(i, show)
            end
            sizer:Layout()
            self:GetParent():Layout()
            self:GetParent():FitInside()
        end
    end

    -- Toggle the panel when the arrow or text is clicked
    self.arrow:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function()
        show_children(self:IsExpanded())
    end)
    
    self.label:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function ()
        self:Toggle()
    end)

    -- Public functions
    function self:Add(...)
        sizer:Add(...):Show(self:IsExpanded())
    end

    function self:SetLabel(label)
        self.label:SetLabel(label)
    end

    function self:Expand()
        self.arrow:Expand()
        show_children(true)
    end

    function self:Collapse()
        self.arrow:Collapse()
        show_children(true)
    end

    function self:IsExpanded()
        return self.arrow:IsExpanded()
    end

    function self:Toggle()
        self.arrow:Toggle()
        show_children(self:IsExpanded())
    end

    return self
end

return CollapsePanel