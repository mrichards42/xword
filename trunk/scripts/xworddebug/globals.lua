-- ============================================================================
-- globals.lua
--     A wxLua script that displays a tree control with all lua globals.
--     Note that everything must be declared local in order to avoid polluting
--     the globals table.
-- ============================================================================

local function sortedKeys(t)
    local ret = {}
    for k,v in pairs(t) do table.insert(ret, k) end
    pcall(table.sort, ret)
    return ret
end

local function tableTree(parent, t)
    local tree = wx.wxTreeCtrl(
        parent, wx.wxID_ANY,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTR_DEFAULT_STYLE
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
        local function add_item(k, v)
            local is_self = (v == t)
            local text = k .. ' ' .. string.rep(' ', 25 - #k) .. tostring(is_self and '(self)' or v)
            local item = tree:AppendItem(parent, text)
            -- Add the value to the tree's data table
            setData(item, v)
            -- Let the wxTreeCtrl know that tables will have children
            if not is_self and type(v) == 'table' or (type(v) == 'userdata' and getmetatable(v)) then
                tree:SetItemHasChildren(item, true)
            end
        end
        -- Add the metatable:
        local meta = getmetatable(t)
        if meta then
            add_item('[metatable]', meta)
        end
        if type(t) == 'table' then
            for i, k in ipairs(sortedKeys(t)) do
                local v = t[k]
                add_item(tostring(k), v)
            end
        end
    end
    -- Add the root
    local root_id = tree:AddRoot(tostring(t))
    setData(root_id, t)
    tree:SetItemHasChildren(root_id, true)

    -- Connect an event for tree item expansion.
    -- Only items that represent tables are expandable.
    -- The table is reloaded every time its item is expanded, so that
    -- data refreshes itself
    tree:Connect(wx.wxEVT_COMMAND_TREE_ITEM_EXPANDING,
        function(evt)
            local item = evt:GetItem()
            local value = getData(item)
            local text  = getText(item)
            if value and type(value) == 'table' or type(value) == 'userdata' then
                tree:DeleteChildren(item)
                tree:SetItemHasChildren(item, true)
                addTable(item, value)
            end
        end)

    return tree
end


function tableDialog(t, name)
    local dlg = wx.wxDialog(
        xword.frame, wx.wxID_ANY, name or "Table: "..tostring(t),
        wx.wxDefaultPosition, wx.wxSize(600,400),
        wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )

    local tree = tableTree(dlg, t)

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        sizer:Add(tree, 1, wx.wxEXPAND + wx.wxALL, 5)
    dlg:SetSizer(sizer)

    return dlg
end

local function globalsDialog()
    return tableDialog(_G)
end


local function init()
    xword.frame:AddMenuItem({'Debug'}, 'Globals',
        function(evt)
            local dlg = globalsDialog()
            dlg:Show()
        end
    )
end

init()
