-- ===========================================================================
-- A Panel with a ListCtrl displaying packages and a TextCtrl linked to the
-- ListCtrl to display description information.
-- ===========================================================================

require 'wx.mixins.DataListCtrl'
require 'wx.mixins.CheckListCtrl'

local P = xword.pkgmgr

function P.PackagePanel(parent, colums, checkList)
    if checkList == nil then
        checkList = true
    end

    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    -- The description text ctrl
    local description = wx.wxTextCtrl(panel, wx.wxID_ANY, "",
                                      wx.wxDefaultPosition, wx.wxDefaultSize,
                                      wx.wxTE_READONLY + wx.wxTE_MULTILINE)
    panel.description = description
    description:SetBackgroundColour(panel:GetBackgroundColour())

    -- The list
    local list = wx.wxListCtrl(panel, wx.wxID_ANY, wx.wxDefaultPosition,
                               wx.wxDefaultSize, wx.wxLC_REPORT)
    if checkList then
        wx.mixins.CheckListCtrl(list)
        list:SetDefaultCheck(true)
    end
    wx.mixins.DataListCtrl(list, colums)

    panel.list = list

    -- Change the description text ctrl when a new item is selected
    local function OnItemSelected(evt)
        local txt = list.data[evt:GetIndex() + 1].description
        description:ChangeValue(txt or "")
    end
    local function OnItemDeselected(evt)
        local txt = list.data[evt:GetIndex() + 1].description
        description:ChangeValue("")
    end
    list:Connect(wx.wxEVT_COMMAND_LIST_ITEM_SELECTED, OnItemSelected)
    list:Connect(wx.wxEVT_COMMAND_LIST_ITEM_DESELECTED, OnItemDeselected)

    function panel:SetData(data)
        self.list:SetData(data)
    end

    if checkList then
        -- Process checks and check for dependencies
        local function OnCheck(evt)
            evt:Skip()
            if not panel.dependenciesField then return end
            local index = evt:GetExtraLong()
            local check = evt:IsChecked()
        end
        list:Connect(wx.wxEVT_COMMAND_CHECKBOX_CLICKED, OnCheck)

        function panel:SetDependenciesField(fieldname)
            self.dependenciesField = fieldname or false
        end
        panel:SetDependenciesField(false)
    end

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    sizer:Add(list, 2, wx.wxEXPAND + wx.wxALL, 5)
    sizer:Add(description, 1, wx.wxEXPAND + wx.wxALL, 5)
    panel:SetSizer(sizer)

    return panel
end