-- ============================================================================
-- utils.lua
--     Various handy wxLua functions
-- ============================================================================

if xword.frame then
    -- Find a menu / submenu given a path
    -- All submenus will be created if they do not exist
    -- path is a table of submenus, e.g. {'File', 'Export'}
    function xword.frame:FindOrCreateMenu(path)
        local menubar = self:GetMenuBar()
        local menuindex = menubar:FindMenu(path[1])
        local rootMenu
        if menuindex == -1 then
            -- Create the root menu
            rootMenu = wx.wxMenu()
            menubar:Append(rootMenu)
        else
            rootMenu = menubar:GetMenu(menuindex)
        end

        local submenu = rootMenu
        for i=2,#path do
            local title = path[i]
            local id = submenu:FindItem(title)
            -- Create the submenu if it doesn't exist
            if id == -1 then
                local newmenu = wx.wxMenu()
                submenu:Append(wx.wxID_ANY, title, newmenu)
                submenu = newmenu
            else
                -- This will fail if there is already an item with this title
                -- that is not a submenu
                submenu = submenu:FindItem(id):GetSubMenu()
            end
        end
        return submenu
    end

    -- Add a menu item to the current frame.
    -- path:   a table of submenus e.g. {'menu', 'submenu'}
    -- label:  the label for the new item
    -- func:   the function to bind to this menu item (or nil)
    function xword.frame:AddMenuItem(path, label, func)
        -- Find and/or create the submenus
        local menu = self:FindOrCreateMenu(path)
        -- Add the menu item
        local item = menu:Append(wx.wxID_ANY, label)
        if func then
            self:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED, func)
        end
        return item
    end
end