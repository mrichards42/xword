-- ===========================================================================
-- A wxListCtrl with check boxes.
-- Produces wxEVT_COMMAND_CHECKBOX_CLICKED events.
-- Overrides InsertItem to set a default check box.
-- Call function list:SetDefaultCheck() to set the default for InsertItem.
-- You may not assign an image list to this listctrl.
-- ===========================================================================

local wx = wx
local type = type
module("wx.mixins")

function CheckListCtrl(list)
    -- Create check box images
    local function DrawCheckBox(list, flag, width, height)
        local bmp = wx.wxBitmap(width, height)
        local dc = wx.wxMemoryDC()
        dc:SelectObject(bmp)
        dc:SetBackground(wx.wxWHITE_BRUSH)
        dc:Clear()
        wx.wxRendererNative.Get():DrawCheckBox(list, dc,
                                               wx.wxRect(0, 0, width, height),
                                               flag)
        dc:SelectObject(wx.wxNullBitmap)
        return bmp
    end

    local images = wx.wxImageList(16, 16, true, 2)
    local unchecked = images:Add(DrawCheckBox(list, 0, 16, 16))
    local checked = images:Add(DrawCheckBox(list, wx.wxCONTROL_CHECKED, 16, 16))
    list:AssignImageList(images, wx.wxIMAGE_LIST_SMALL)

    -- Check functions for the list ctrl
    function list:IsChecked(index)
        -- Get the image index
        local item = wx.wxListItem()
        item:SetId(index)
        item:SetMask(wx.wxLIST_MASK_IMAGE)
        self:GetItem(item)
        return item:GetImage() == 1
    end

    function list:ToggleCheck(index)
        return self:Check(index, not self:IsChecked(index))
    end

    function list:Check(index, doit)
        doit = doit == false and 0 or 1
        self:SetItemImage(index, doit)
        return doit
    end

    function list:Uncheck(index)
        return list:Check(index, false)
    end

    local function sendCheckEvent(index, check)
        local evt = wx.wxCommandEvent(wx.wxEVT_COMMAND_CHECKBOX_CLICKED,
                                      list:GetId())
        evt:SetInt(check) -- Check box state: 0 = unchecked, 1 = checked
        evt:SetExtraLong(index)
        evt:SetEventObject(list)
        list:AddPendingEvent(evt)
    end

    -- Events for toggling checks: Left down on check box; spacebar
    local function OnLeftDown(evt)
        local index, flags = list:HitTest(evt:GetPosition())
        if flags == wx.wxLIST_HITTEST_ONITEMICON then
            sendCheckEvent(index, list:ToggleCheck(index))
        else
            evt:Skip()
        end
    end
    list:Connect(wx.wxEVT_LEFT_DOWN, OnLeftDown)

    local function OnKeyDown(evt)
        if evt:GetKeyCode() == wx.WXK_SPACE then
            local index = -1
            while true do
                index = list:GetNextItem(index, wx.wxLIST_NEXT_ALL,
                                                wx.wxLIST_STATE_SELECTED)
                if index == -1 then break end
                sendCheckEvent(index, list:ToggleCheck(index))
            end
        else
            evt:Skip()
        end
    end
    list:Connect(wx.wxEVT_KEY_DOWN, OnKeyDown)

    function list:SetDefaultCheck(doit)
        self.defaultCheck = doit and 1 or 0
    end
    function list:GetDefaultCheck()
        return self.defaultCheck
    end
    list:SetDefaultCheck(false)

    -- Override InsertItem to set the check box to unchecked
    function list:InsertItem(index, str, img)
        if type(index) == "userdata" then -- This is a wxListItem
            index:SetImage(list:GetDefaultCheck())
            return self:_InsertItem(index)
        end
        return self:_InsertItem(index, str, list:GetDefaultCheck())
    end

    return list
end
