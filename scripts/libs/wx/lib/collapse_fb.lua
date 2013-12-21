local _R = string.match(..., '^.+%.')

local CollapsePanel = require(_R .. 'collapse')

--- Reparent all immediate wxWindows and wxWindows in sub-sizers.
local function reparent_items(sizer, parent)
    local items = s:GetChildren()
    for i=0,items:GetCount()-1 do
        local item = items:Item(i):GetData():DynamicCast("wxSizerItem")
        if item:IsWindow() then
            item:GetWindow():Reparent(parent)
        elseif item:IsSizer() then
            reparent_items(item:GetSizer(), parent)
        end
    end
end

-- Return the next wxSizerItem after the window
local function find_next(sizer, win)
    win = win:DynamicCast("wxWindow")
    local count = sizer:GetChildren():GetCount()
    for i=0,count-1 do
        local item = sizer:GetItem(i)
        -- Check if this is our window
        if item:IsWindow() and item:GetWindow() == win then
            -- Return the next item
            if i < count-1 then
                return sizer:GetItem(i + 1)
            end
        end
    end
end

--- A CollapsePanel that works with wxFormBuilder.
-- Create a wxStaticText with fbCollapsePanel as the class.
-- The item after this one in the containing sizer will be the child panel.
local function fbCollapsePanel(parent, id)
    local ctrl = CollapsePanel(parent, id, "")
    -- Redefine parent:SetSizer so we can capture the item after us
    local set_sizer = parent.SetSizer
    function parent:SetSizer(sizer)
        -- Find the item after ourself
        local item = find_next(sizer, ctrl)
        if item then
            -- Save the item info
            local p, f, b = item:GetProportion(), item:GetFlag(), item:GetBorder()
            -- Reparent
            local obj
            if item:IsWindow() then
                obj = item:GetWindow()
                obj:Reparent(ctrl)
            else -- Should be a sizer
                obj = item:GetSizer()
                reparent_items(obj, ctrl)
            end
            -- Detach and add to ourself
            sizer:Detach(obj)
            ctrl:Add(obj, p, f, b)
            ctrl:Layout()
        end
        -- Remove this method
        self.SetSizer = set_sizer
        -- Call the previous version
        self:SetSizer(sizer)
    end
    return ctrl
end

return fbCollapsePanel