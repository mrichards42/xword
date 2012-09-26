local ArrowButton = require 'download.gui.arrow_button'
local TextButton = require 'download.gui.text_button'

-- A panel with a title and a collapse button
function CollapsePanel(parent, label)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    -- Header
    local header = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(header, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local arrow = ArrowButton(panel, wx.wxID_ANY)
    header:Add(arrow, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)
    local text = TextButton(panel, wx.wxID_ANY, label)
    header:Add(text, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Collapsing
    local inside_panel
    local function update_panel()
        if inside_panel then
            sizer:Show(inside_panel, arrow:IsExpanded())
            sizer:Layout()
            panel:OnToggle()
        end
    end

    function panel:OnToggle()
        parent.Parent:Fit()
    end

    arrow:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, update_panel)
    
    text:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        arrow:Toggle()
        update_panel()
    end)

    -- Public stuff
    panel.header = header
    panel.sizer = sizer
    function panel:SetPanel(p)
        inside_panel = p
        sizer:Add(inside_panel, 0, wx.wxEXPAND)
        update_panel()
    end

    function panel:SetLabel(label)
        text:SetLabel(label)
    end

    function panel:Expand()
        arrow:Expand()
        update_panel()
    end

    function panel:Collapse()
        arrow:Collapse()
        update_panel()
    end

    return panel
end

return CollapsePanel