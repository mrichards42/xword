local P = download
local layout = P.layout

-- ============================================================================
-- View Menu
-- ============================================================================

-- Create a menu for switching layouts
function layout.createViewMenu()
    local dlg = assert(P.dlg)
    local menu = wx.wxMenu()
        for _, l in ipairs(layout.layouts) do
            local item = menu:AppendRadioItem(wx.wxID_ANY, l.name)
            dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
                function(evt)
                    layout.setLayout(l.func, nil, unpack(l.args))
                end)
        end
    return menu
end

-- ============================================================================
-- Download menu
-- ============================================================================
require 'download.layout.callbacks'

-- Download submenus:
-- menus['menu title'].menu = the wxMenu
-- menus['menu title']['submenu title'].menu = a submenu
local menus = {}

local function getDownloadMenus()
    return {
        { "All",     function() return true end }, 
        { "Missing", function(dl) return not dl.fileexists end },
    }
end

-- Create a menu for special download targets (All, all undownloaded, etc.)
function layout.createDownloadMenu()
    local dlg = assert(P.dlg)
    local menu = wx.wxMenu()

    -- Menu structure looks like this:
        -- All
            -- Download All
            -- Puzzles
                -- PuzzleName1
                -- PuzzleName2
            -- Dates
                -- Today
                -- Yesterday
        -- Missing
            -- Download All
            -- Puzzles
                -- PuzzleName1
                -- PuzzleName2
            -- Dates
                -- Today
                -- Yesterday

    for _, menudesc in ipairs(getDownloadMenus()) do
        local title, func = unpack(menudesc)
        local submenu = wx.wxMenu()
        menus[title] = {}
        menus[title].menu = submenu

        -- Download all
        local item = submenu:Append(wx.wxID_ANY, "Download "..title)
        dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
            layout.downloadAll(func))

        -- Puzzles submenu
        local puzmenu = wx.wxMenu()
        menus[title]['Puzzles'] = {} 
        menus[title]['Puzzles'].menu = puzmenu
        -- This will be filled in each time the layout is changed
        submenu:Append(wx.wxID_ANY, 'Puzzles', puzmenu)

        -- Dates submenu
        local datemenu = wx.wxMenu()
        menus[title]['Dates'] = {}
        menus[title]['Dates'].menu = datemenu
        -- This will be filled in each time the layout is changed
        submenu:Append(wx.wxID_ANY, 'Dates', datemenu)

        menu:Append(wx.wxID_ANY, title, submenu)
    end
    return menu
end

-- Destroy all entries in the menu
local function clearMenu(menu)
    local child = menu.MenuItems.First
    while child do
        local item = child.Data:DynamicCast("wxMenuItem")
        -- Get the next node *before* we destroy the wxMenuItem
        child = child.Next
        menu:Destroy(item)
    end
end


-- Update the various submenus in the Download menu
function layout.updateMenus()
    local dlg = assert(P.dlg)
    for _, menudesc in ipairs(getDownloadMenus()) do
        local title, func = unpack(menudesc)
        local submenu = menus[title].menu

        -- Puzzles submenu
        local puzmenu = menus[title]['Puzzles'].menu
        clearMenu(puzmenu)
        -- Sort the puzzle names
        local names = {}
        for name, _ in pairs(P.dlg.downloads) do table.insert(names, name) end
        table.sort(names)
        for _, name, dates in ipairs(names) do
            local item = puzmenu:Append(wx.wxID_ANY, name)
            dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
                layout.downloadAllPuzzles(name, func))
        end

        -- Dates submenu
        local datemenu = menus[title]['Dates'].menu
        clearMenu(datemenu)
        -- Sort the puzzle dates
        local dates = {}
        for _, puzdates in pairs(P.dlg.downloads) do
            for d, _ in pairs(puzdates) do
                table.insert(dates, d)
            end
        end
        -- Newest first
        table.sort(dates, function(a,b) return a > b end)
        -- Remove the duplicates
        for i = #dates,1,-1 do
            if dates[i] == dates[i-1] then
                table.remove(dates, i)
            end
        end

        for _, d in ipairs(dates) do
            local item = datemenu:Append(wx.wxID_ANY, d:fmt(P.dateformat))
            dlg:Connect(item:GetId(), wx.wxEVT_COMMAND_MENU_SELECTED,
                layout.downloadAllDates(d, func))
        end

    end
end
