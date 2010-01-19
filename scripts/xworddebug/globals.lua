-- ============================================================================
-- globals.lua
--     A wxLua script that displays a tree control with all lua globals.
--     Note that everything must be declared local in order to avoid polluting
--     the globals table.
-- ============================================================================

local function sortedKeys(t)
    local ret = {}
    for k,v in pairs(t) do table.insert(ret, k) end
    table.sort(ret)
    return ret
end


local function globalsDialog()
    local dlg = wx.wxDialog(
        xword.frame, wx.wxID_ANY, "Lua Globals",
        wx.wxDefaultPosition, wx.wxSize(600,400),
        wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )

    local tree = wx.wxTreeCtrl(
        dlg, wx.wxID_ANY,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTR_DEFAULT_STYLE + wx.wxTR_HIDE_ROOT
    )
    tree:SetFont(wx.wxFont(8, wx.wxFONTFAMILY_MODERN,
                           wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL))

    -- A table to store the data, references by wxTreeCtrlId's
    tree.data = {}

    -- wxLua uses wxTreeItemId:GetValue() for use as a table index.
    local setData = function(item, value)
        tree.data[item:GetValue()] = value
    end

    local getData = function(item)
        return tree.data[item:GetValue()]
    end

    local getText = function(item)
        return tree:GetItemText(item)
    end

    -- Populate the tree control with a table (on demand)
    local addTable = function(parent, t)
        for i, k in ipairs(sortedKeys(t)) do
            local v = t[k]
            k = tostring(k)
            local text = k .. ' ' .. string.rep(' ', 25 - #k) .. tostring(v)
            local item = tree:AppendItem(parent, text)
            -- Add the value to the tree's data table
            setData(item, v)

            -- Let the wxTreeCtrl know that tables will eventually have
            -- children
            if type(v) == 'table' then
                tree:SetItemHasChildren(item, true)
            end
        end
    end
    -- Add the root (this won't show up because we used wx.wxTR_HIDE_ROOT)
    addTable(tree:AddRoot('_G'), _G)

    -- Connect an event for tree item expansion.
    -- Only items that represent tables are expandable.  As soon as they are
    -- expanded, the item is removed from the tree control's data table.
    tree:Connect(wx.wxEVT_COMMAND_TREE_ITEM_EXPANDING,
        function(evt)
            local item = evt:GetItem()
            local value = getData(item)
            local text  = getText(item)
            if value and type(value) == 'table' then
                addTable(item, value)
                setData(item, nil)
            end
        end)

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        sizer:Add(tree, 1, wx.wxEXPAND + wx.wxALL, 5)
    dlg:SetSizer(sizer)

    return dlg
end


local function init()
    xword.frame:AddMenuItem({'Tools', 'Debug'}, 'Globals',
        function(evt)
            local dlg = globalsDialog()
            dlg:ShowModal()
        end
    )
end

init()
