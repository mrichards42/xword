local tablex = require 'pl.tablex'
local join = require 'xword.pkgmgr.join'
local serialize = require 'serialize'
local split = require 'pl.stringx'.split

local P = xword.pkgmgr
require 'xword.pkgmgr.updater'
require 'xword.pkgmgr.updater.dialog'

-- Controls
local ButtonSizer = require 'xword.widgets.button'
local CustomListCtrl = require 'xword.widgets.customlist'
local SizedTextCtrl = require 'xword.widgets.sizedtext'

local function ShowRestartDialog()
    -- We can't actually restart from lua because the order that closing
    -- events, lua unint, and destructors are called results in a crash.
    xword.Message("XWord must be restarted to make this change.")
end

-- ---------------------------------------------------------------------------
-- PackagePanel
-- ---------------------------------------------------------------------------
local function PackagePanel(parent, itemFunc, fields, buttons)
    -- itemFunc is a function with the signature:
    -- function(parent, pkg) -> wx.wxWindow

    -- Arguments
    if type(parent) == 'table' then
        local opts = parent or {}

        parent = opts.parent or opts[1]
        itemFunc = options.item or opts[2]
        fields = opts.fields or opts[3]
        buttons = opts.buttons or opts[4]
    end

    -- Windows
    local panel = wx.wxPanel(parent, -1)

    -- Adding a border to CustomListCtrl either doesn't work, or adds weird
    -- flicker.  Also, for some reason wx.wxBORDER_DOUBLE isn't supposed to
    -- be used (and it especially seems to have flicker problems), and
    -- that's approximately the border that controls should have, so we have
    -- to fake BORDER_DOUBLE by using BORDER_SIMPLE and a 1px margin around
    -- the list in a sizer.
    local listWrapper = wx.wxPanel(panel, -1, wx.wxDefaultPosition,
                                   wx.wxDefaultSize, wx.wxBORDER_SIMPLE)
    panel.list = CustomListCtrl(listWrapper)

    -- Buttons
    local button_sizer, button_list
    if buttons then
        button_sizer, button_list = ButtonSizer(panel, buttons)
    end
    panel.buttons = button_list or {}

    -- Set the data of the list given a table of packages
    panel.packages = {}
    function panel:SetData(packages)
        self.packages = {}
        self.list:Clear()
        for _, pkg in ipairs(packages) do
            self:Append(pkg)
        end
    end

    function panel:Append(pkg)
        -- Check to make sure the package has the requried fields
        local has_fields = true
        for _, field in ipairs(fields or {}) do
            if pkg[field] == nil then
                has_fields = false
                break
            end
        end
        if has_fields then
            self.list:Append(itemFunc(self.list, pkg))
            table.insert(self.packages, pkg)
        end
    end

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local listSizer = wx.wxBoxSizer(wx.wxVERTICAL)
        listSizer:Add(panel.list, 1, wx.wxEXPAND + wx.wxALL, 2)
        listWrapper:SetSizer(listSizer)
    sizer:Add(listWrapper, 1, wx.wxEXPAND + wx.wxALL, 5)
    if button_sizer then
        sizer:Add(button_sizer, 0, wx.wxEXPAND + wx.wxALL, 5)
    end
    panel:SetSizer(sizer)

    return panel
end


-- ---------------------------------------------------------------------------
-- List items
-- ---------------------------------------------------------------------------

-- A basic item, with name, description, and optional buttons
local function BaseItem(parent, pkg, buttons)
    local win = wx.wxWindow(parent, wx.wxID_ANY, wx.wxDefaultPosition,
                            wx.wxDefaultSize, wx.wxCLIP_CHILDREN)
    win.pkg = pkg
    assert(pkg.name and pkg.version)

    -- Controls
    win.name = wx.wxStaticText(win, wx.wxID_ANY, pkg.name.." "..pkg.version)
    win.name:SetFont(wx.wxFont(11, wx.wxFONTFAMILY_SWISS,
                               wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD))
    win.description = SizedTextCtrl(win, wx.wxID_ANY, pkg.description or "")

    win.buttons = {}
    for _, name in ipairs(buttons or {}) do
        local button = wx.wxButton(win, -1, name)
        -- Without this line, changing the background color of win leaves
        -- an artifact around the button.  I'm not sure why changing the
        -- FOREground color would fix this, but it seems to.
        button:SetForegroundColour(wx.wxBLACK)

        -- Remove flicker
        button:Connect(wx.wxEVT_ERASE_BACKGROUND, function() end)

        -- Add the button to the array and the hash
        table.insert(win.buttons, button)
        win.buttons[name] = button
    end

    -- Colors
    win:SetBackgroundColour(parent:GetBackgroundColour())

    -- Layout
    win:Connect(wx.wxEVT_SIZE, function(evt)
        win:Layout()
        win:Refresh()
        evt:Skip()
    end)

    win:Connect(wx.wxEVT_ST_CHANGED, function(evt)
        parent:UpdateScrollbars()
    end)

    local vsizer = wx.wxBoxSizer(wx.wxVERTICAL)
    vsizer:Add(win.name, 0, wx.wxEXPAND + wx.wxALL, 5)

    local hsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    hsizer:Add(win.description, 1, wx.wxALIGN_CENTER_VERTICAL)
    for _, name in ipairs(buttons or {}) do
        hsizer:Add(win.buttons[name], 0, wx.wxLEFT + wx.wxALIGN_TOP, 5)
    end
    vsizer:Add(hsizer, 0, wx.wxEXPAND + wx.wxBOTTOM + wx.wxLEFT + wx.wxRIGHT, 5)

    win:SetSizer(vsizer)

    return win
end


-- ---------------------------------------------------------------------------
-- Installed Packages
-- ---------------------------------------------------------------------------

-- ScriptItem, with Enable and Remove buttons
local function ScriptItem(parent, pkg)
    local win = BaseItem(parent, pkg, { "Enable", "Remove" })
    assert(pkg.packagename)

    -- Colors
    win.disabledColor = wx.wxSystemSettings.GetColour(wx.wxSYS_COLOUR_GRAYTEXT)

    -- Button events
    win.buttons.Enable:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        win.pkg.enabled = not win.pkg.enabled
        win:UpdateSettings()
        local packagename = win.pkg.packagename
        if win.pkg.enabled then -- Enable the package
            if not P.load_package(packagename) then
                win.pkg.restart = true
                ShowRestartDialog()
            end
        else -- Disable the package
            if not P.unload_package(packagename) then
                win.pkg.restart = true
                ShowRestartDialog()
            end
        end
        -- Save the change to file
        local enabled_packages = P.load_enabled_packages()
        enabled_packages[packagename] = win.pkg.enabled
        P.write_enabled_packages(enabled_packages)
    end)

    win.buttons.Remove:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        -- If this package is pending uninstallation, undo that
        if win.pkg.uninstalled then
            local filename = join(xword.userdatadir, 'pending_uninstall.lua')
            local uninstall = serialize.loadfile(filename) or {}
            uninstall[win.pkg.packagename] = nil
            serialize.dump(uninstall, filename)
            win.pkg.uninstalled = false
            win:UpdateSettings()
            return
        end
       
        -- If this is pending installation, undo that
        if win.pkg.installed then
            local filename = join(xword.userdatadir, 'pending_install.lua')
            local install = serialize.loadfile(filename) or {}
            install[win.pkg.packagename] = nil
            serialize.dump(install, filename)
            win.pkg.installed = false
            win:UpdateSettings()
            P.dlg:RefreshUpdates()
            return
        end

        local result = wx.wxMessageBox(
            "Also remove package settings?",
            "XWord Message",
            wx.wxYES_NO + wx.wxCANCEL + wx.wxICON_QUESTION
        )
        if result == wx.wxCANCEL then
            return
        end
        -- Try to unload and uninstall the package
        if not P.uninstall_package(win.pkg.packagename, result == wx.wxYES) then
            win.pkg.restart = true
            ShowRestartDialog()
            win.pkg.uninstalled = true
            win:UpdateSettings()
        else
            -- Find this package and remove it from the master list
            for i, pkg in ipairs(P.dlg.packages) do
                if pkg.name == win.pkg.name then
                    table.remove(P.dlg.packages, i)
                    break
                end
            end
            win:GetParent():Remove(win)
            P.dlg:RefreshUpdates()
        end
    end)

    -- Update the display based on the current settings
    function win:UpdateSettings()
        if self.pkg.enabled == nil then
            self.pkg.enabled = (self.pkg.enabled ~= false)
        end

        if self.pkg.uninstalled or self.pkg.installed then
            self.buttons.Enable:Enable(false)
            self.name:SetForegroundColour(self.disabledColor)
            self.description:SetForegroundColour(self.disabledColor)
            if self.pkg.uninstalled then
                self.buttons.Remove:SetLabel("Undo Remove")
                self.name:SetLabel(self.pkg.name.." "..self.pkg.version.." (uninstalled after restart)")
            else
                -- The only reason that a package isn't installed right
                -- away is if it is an update.
                self.buttons.Remove:SetLabel("Undo Update")
                self.name:SetLabel(self.pkg.name.." "..self.pkg.version.." (updated after restart)")
            end
        else
            self.buttons.Enable:Enable(true)
            self.buttons.Remove:SetLabel("Remove")
            if self.pkg.enabled then
                self.name:SetLabel(self.pkg.name.." "..self.pkg.version)
                self.name:SetForegroundColour(wx.wxBLACK)
                self.description:SetForegroundColour(wx.wxBLACK)
                self.buttons.Enable:SetLabel("Disable")
            else
                self.name:SetLabel(
                    self.pkg.name.." "..self.pkg.version.." (disabled)"
                )
                self.name:SetForegroundColour(self.disabledColor)
                self.description:SetForegroundColour(self.disabledColor)
                self.buttons.Enable:SetLabel("Enable")
            end
        end
        self:Refresh()
    end

    win:UpdateSettings()

    return win
end


local function ScriptsPanel(parent)
    return PackagePanel(parent, ScriptItem, { "name", "packagename"} )
end


-- ---------------------------------------------------------------------------
-- Updates
-- ---------------------------------------------------------------------------

-- UpdateItem, with a checkbox for "Include this update"
local function UpdateItem(parent, pkg)
    local win = BaseItem(parent, pkg)
    assert(pkg.localversion and pkg.download)
    -- Extra windows
    win.oldVersion = wx.wxStaticText(win, -1, "Installed version: "..pkg.localversion)
    win.include = wx.wxCheckBox(win, -1, "Include Update")
    win.include:SetValue(not pkg.ignored)

    win.include:Connect(wx.wxEVT_COMMAND_CHECKBOX_CLICKED, function(evt)
        pkg.ignored = not evt:IsChecked()
    end)

    -- New layout
    -- I was having trouble with heap corruption when I just recreated
    -- the layout from scratch and called win:SetSizer(), so I'll just
    -- hack the old layout.
    local vsizer = win:GetSizer()
        vsizer:Detach(win.name)
        local topsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        topsizer:Add(win.name, 1, wx.wxEXPAND)
        topsizer:Add(win.oldVersion, 0, wx.wxEXPAND + wx.wxLEFT, 5)
    vsizer:Insert(0, topsizer, 0, wx.wxEXPAND + wx.wxALL, 5)

        local bottomsizer = vsizer:GetItem(1):GetSizer()
        bottomsizer:Add(win.include, 0, wx.wxLEFT + wx.wxALIGN_TOP, 5)

    return win
end


local function UpdatePanel(parent)
    local updates = PackagePanel(parent, UpdateItem,
                                 { "name", "download" },
                                 { "Install updates" })
    -- Install the selected updates
    function updates.InstallUpdates()
        -- Make a list of updates that should be installed
        -- Update the updates.lua file with ignored updates
        local to_install = {}
        local update_packages = serialize.loadfile(P.updater.updates_filename)
        for _, pkg in ipairs(updates.packages) do
            if not pkg.ignored then
                table.insert(to_install, pkg)
            else
                -- Find the update in updates_file and set ignored
                for _, p in ipairs(update_packages) do
                    if p.name == pkg.name then
                        p.ignored = true
                        break
                    end
                end
            end
        end
        serialize.dump(update_packages, P.updater.updates_filename)

        if #to_install > 0 then
            local dlg = P.updater.DownloadDialog()
            dlg:Connect(wx.wxEVT_CLOSE_WINDOW, function (evt)
                P.dlg:RefreshPackages()
                P.dlg:RefreshUpdates()
                evt:Skip()
            end)
            dlg:Show()
            dlg:DoUpdate(to_install)
        end
    end

    updates.buttons[1]:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, updates.InstallUpdates)

    return updates
end

-- ---------------------------------------------------------------------------
-- Available Packages
-- ---------------------------------------------------------------------------
-- AvailableItem, with an Install button
local function AvailableItem(parent, pkg)
    local win = BaseItem(parent, pkg, { "Install" })
    assert(pkg.download)

    -- Button events
    win.buttons.Install:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        local dlg = P.updater.DownloadDialog()
        dlg:Connect(wx.wxEVT_CLOSE_WINDOW, function (evt)
            P.dlg:RefreshPackages()
            P.dlg:RefreshUpdates()
            evt:Skip()
        end)
        dlg:Show()
        dlg:DoUpdate({ win.pkg })
    end)

    return win
end

local function AvailablePanel(parent)
    return PackagePanel(parent, AvailableItem, { "name", "download"} )
end


-- ---------------------------------------------------------------------------
-- The Package Manager dialog
-- ---------------------------------------------------------------------------
function P.PackageDialog(opts)
    if P.dlg then
        p.dlg:Show()
        return false
    end

    -- Dialog options
    if opts == nil then
        opts = {}
    end
    if opts.scripts == nil and opts.updates == nil and opts.available == nil then
        opts.scripts = true
        opts.updates = true
        opts.available = true
    end
    if opts.check == nil then
        opts.check = true
    end
    if opts.parent == nil then
        opts.parent = xword.frame or wx.NULL
    end
    if opts.buttons == nil or opts.buttons == true then
        opts.buttons = { close = true, check = true }
    end

    local dlg = wx.wxDialog(opts.parent, wx.wxID_ANY,
                            "Package Manager",
                            wx.wxDefaultPosition, wx.wxSize(450,350),
                            wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER)
    P.dlg = dlg

    -- Count the number of pages to see if we need a notebook
    local num_pages = (opts.scripts and 1 or 0)
                      + (opts.updates and 1 or 0)
                      + (opts.available and 1 or 0)

    local panel_parent = dlg
    local main_panel = nil
    if num_pages > 1 then
        dlg.notebook = wx.wxNotebook(dlg, wx.wxID_ANY)
        panel_parent = dlg.notebook
        main_panel = dlg.notebook
    else
        dlg.notebook = false
    end

    -- Scripts
    dlg.scripts = opts.scripts and ScriptsPanel(panel_parent) or false

    -- Updates
    dlg.updates = opts.updates and UpdatePanel(panel_parent) or false

    -- Available
    dlg.available = opts.available and AvailablePanel(panel_parent) or false


    -- Add notebook pages
    if dlg.notebook then
        if dlg.scripts then
            dlg.notebook:AddPage(dlg.scripts, "Installed Packages")
        end
        if dlg.updates then
            dlg.notebook:AddPage(dlg.updates, "Updates")
        end
        if dlg.available then
            dlg.notebook:AddPage(dlg.available, "Available Packages")
        end
    else
        main_panel = dlg.scripts or dlg.updates or dlg.available
    end


    -- Load scripts info from files
    function dlg:RefreshPackages()
        -- Load info.lua for all packages and read enabled/disabled states
        -- from config/packages.lua
        self.packages = P.load_packages_info()
        local uninstalled = serialize.loadfile(join(xword.userdatadir, 'pending_uninstall.lua')) or {}
        local installed = serialize.loadfile(join(xword.userdatadir, 'pending_install.lua')) or {}
        for packagename, value in pairs(uninstalled) do
            self.packages[packagename].uninstalled = value
        end
        for packagename, value in pairs(installed) do
            self.packages[packagename].installed = value
        end
        if self.scripts then
            self.scripts:SetData(self.packages)
        end
    end

    -- Load updates and available packages
    function dlg:RefreshUpdates(check_xword)
        -- Make new_packages and update_packages table, with packages that
        -- have not been installed, and packages that could be updated.
        -- If the version of the package in the updates table has already been
        -- seen by the user and was not updated, it should have already been
        -- marked with ignored = true.
        local updates = serialize.loadfile(P.updater.updates_filename) or {}
        -- Check to see if there is an update for XWord first
        if check_xword and updates.xword -- don't check ignored here
            and P.is_newer(updates.xword.version, xword.version)
        then
            if xword.Prompt(
                "There is a new version of XWord available (version %s).\n"
                .."Would you like to go to the download page?",
                updates.xword.version
            ) then
                dlg:Close()
                wx.wxLaunchDefaultBrowser(updates.xword.download)
                return
            else
                updates.xword.ignored = true
                serialize.dump(updates, P.updater.updates_filename)
            end
        end

        local new_packages = {}
        local update_packages = {}
        for _, pkg in ipairs(updates) do
            local local_pkg
            for i, p in ipairs(self.packages) do
                if p.name == pkg.name then
                    local_pkg = p
                    break
                end
            end
            if not P.is_newer(pkg.requires, xword.version) then
                if not local_pkg then
                    pkg.localversion = "New"
                    table.insert(new_packages, pkg)
                elseif P.is_newer(pkg.version, local_pkg.version)
                        and not local_pkg.installed and local_pkg.enabled ~= false
                then
                    pkg.localversion = local_pkg.version
                    table.insert(update_packages, pkg)
                end
            end
        end
        if self.updates then
            self.updates:SetData(update_packages)
        end
        if self.available then
            self.available:SetData(new_packages)
        end
    end

    dlg:RefreshPackages()
    dlg:RefreshUpdates()

    -- The buttons
    local buttons
    if opts.buttons then
        buttons = wx.wxStdDialogButtonSizer()

        if opts.buttons.close then
            local close = wx.wxButton(dlg, wx.wxID_OK, "Close")
            close:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
                dlg:Close()
            end)
            buttons:SetAffirmativeButton(close)
        end

        if opts.buttons.check then
            local checkbutton = wx.wxButton(dlg, wx.wxID_ANY, "Check for updates")
            checkbutton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
                P.updater.CheckForUpdates(function ()
                    dlg:RefreshUpdates(true) -- Check for XWord update
                    if dlg.notebook and dlg.scripts then -- Select the update page
                        if dlg.notebook:GetPage(dlg.notebook:GetSelection()).Id == dlg.scripts.Id then
                            dlg.notebook:SetSelection(1)
                        end
                    end
                end)
                evt:Skip()
            end)
            buttons:Add(checkbutton, 0)
        end
        buttons:Realize()
    end

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    sizer:Add(main_panel, 1, wx.wxEXPAND + wx.wxALL, 5)
    if buttons then
        sizer:Add(buttons, 0, wx.wxEXPAND + wx.wxALL, 5)
    end
    dlg:SetSizer(sizer)

    dlg:Center()

    if opts.check ~= false then
        P.updater.CheckForUpdates(function() if P.dlg then P.dlg:RefreshUpdates() end end)
    end

    dlg:Connect(wx.wxEVT_CLOSE_WINDOW, function(evt)
        evt:Skip()
        dlg:Destroy()
        P.dlg = nil
    end)

    return dlg
end
