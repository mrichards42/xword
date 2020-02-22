-- ===========================================================================
-- XWord package manager
-- ===========================================================================

local P = {}
xword.pkgmgr = P

local serialize = require 'serialize'
local lfs = require 'lfs'
local join = require 'xword.pkgmgr.join'

-- Install and load functions
require 'xword.pkgmgr.install'
require 'xword.pkgmgr.load'

local function init()
    -- Check to see if we have any packages to install or uninstall
    -- (these should be dlls that have to be updated after XWord is restarted)
    do
        -- pending_install format is { name = filename, ... }
        local install = serialize.loadfile(join(xword.userdatadir, 'pending_install.lua'))
        os.remove(join(xword.userdatadir, 'pending_install.lua'))
        for name, filename in pairs(install or {}) do
            local success, err = P.install_package(filename)
            if success then
                os.remove(filename)
            else
                xword.Error("Could not install package from file:\n"..filename.."\n"..(err or "Unknown error"))
            end
        end
        require 'lfs'
        lfs.rmdir(join(xword.userdatadir, 'updates'))

        -- pending_uninstall format is { name = delete_config, ... }
        local uninstall = serialize.loadfile(join(xword.userdatadir, 'pending_uninstall.lua'))
        os.remove(join(xword.userdatadir, 'pending_uninstall.lua'))
        for packagename, delete_config in pairs(uninstall or {}) do
            P.uninstall_package(packagename, delete_config)
        end
    end

    -- Add the package dialog to the menu
    if xword.frame then
        xword.frame:AddMenuItem({"Tools"}, "Package Manager", function(evt)
            require 'xword.pkgmgr.dialog'
            P.dlg = P.PackageDialog()
            P.dlg:ShowModal()
        end)
    end

    -- Download the current package list and see if there are any updates
    require 'xword.pkgmgr.updater'
    P.updater.CheckForUpdates(function()

        -- Show a hacked version of the package dialog
        local function do_update(message, opts)
            opts.check = false
            require 'xword.pkgmgr.dialog'
            local dlg = P.PackageDialog(opts)

            -- Add a header to the dialog
            local text = wx.wxStaticText(dlg, -1, message)
            local bmp = wx.wxArtProvider.GetBitmap(
                wx.wxART_INFORMATION, wx.wxART_MESSAGE_BOX, wx.wxDefaultSize
            )
            local top = wx.wxBoxSizer(wx.wxHORIZONTAL)
            top:Add(wx.wxStaticBitmap(dlg, -1, bmp), 0, wx.wxALIGN_CENTER_VERTICAL)
            top:Add(text, 1, wx.wxALIGN_CENTER_VERTICAL + wx.wxLEFT, 10)
            dlg:GetSizer():Insert(0, top, 0, wx.wxEXPAND + wx.wxALL, 10)

            -- Add buttons to the update page
            if dlg.updates then
                local sizer = dlg.updates:GetSizer():GetItem(1):GetSizer()
                local laterButton = wx.wxButton(dlg.updates, -1, "Later")
                local ignoreButton = wx.wxButton(dlg.updates, -1, "Ignore updates")
                sizer:Add(laterButton, 0, wx.wxALL, 5)
                sizer:Add(ignoreButton, 0, wx.wxALL, 5)
                sizer:Layout()

                laterButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
                    dlg:Close()
                end)

                ignoreButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
                    -- Ignore all the update packages
                    for _, pkg in ipairs(dlg.updates.packages) do
                        pkg.ignored = true
                    end
                    dlg.updates.InstallUpdates() -- This rewrites updates.lua
                    dlg:Close()
                end)
            end

            dlg:ShowModal()
        end

        local updates = serialize.loadfile(P.updater.updates_filename) or {}

        -- Check to see if there is an update to XWord first
        if updates.xword and not updates.xword.ignored then
            if P.show_xword_update_dialog(updates) then
                return
            end
        end

        local packages = P.load_packages_info()

        -- If this is the first run of this version of XWord, show available
        -- packages.
        if xword.firstrun then
            for _, pkg in ipairs(updates) do
                if not packages[pkg.packagename] then
                    do_update("Addon packages are available:", {available=true, buttons={close=true}})
                    return
                end
            end
        end

        -- Compare the updated packages with the installed packages
        for _, pkg in ipairs(updates) do
            if not pkg.ignored then
                for _, p in ipairs(packages) do
                    if p.name == pkg.name then
                        -- We have at least one update, so show the dialog
                        if P.is_newer(pkg.version, p.version)
                           and not P.is_newer(pkg.requires, xword.version)
                        then
                            do_update("New updates are available:", {updates=true, buttons=false})
                            return
                        end
                    end
                end
            end
        end
    end)
end

function P.is_newer(v1, v2)
    local split = require 'pl.stringx'.split
    -- Split by '-' to determine if either version is a SNAPSHOT build.
    local v1 = split(tostring(v1) or '', '-')
    local v1_is_snapshot = v1[#v1] == 'SNAPSHOT'
    local v2 = split(tostring(v2) or '', '-')
    local v2_is_snapshot = v2[#v2] == 'SNAPSHOT'
    -- Split by '.'
    local v1 = split(v1[1], '.')
    local v2 = split(v2[1], '.')
    -- Compare successive revision numbers
    for i = 1, #v1 do
        local cmp = (tonumber(v1[i]) or 0) - (tonumber(v2[i]) or 0)
        if cmp > 0 then
            return true
        elseif cmp < 0 then
            return false
        end
    end
    if #v1 ~= #v2 then
        return #v1 > #v2
    end
    -- Versions are equal, except potentially for a -SNAPSHOT at the end of
    -- one of them. SNAPSHOT builds should be considered older.
    if v1_is_snapshot ~= v2_is_snapshot then
        return v2_is_snapshot
    end
    return false
end

function P.get_all_scripts()
    local t = {}
    local has_xworddebug -- Make sure xworddebug is loaded first
    for d in lfs.dir(xword.scriptsdir) do
        if d ~= 'xword' and d ~= 'libs'
            and d:sub(1,1) ~= '.'
            and lfs.attributes(join(xword.scriptsdir, d), 'mode') == 'directory'
        then
            if d == 'xworddebug' then
                has_xworddebug = true
            else
                table.insert(t, d)
            end
        end
    end
    if has_xworddebug then
        table.insert(t, 1, 'xworddebug')
    end
    return t
end

function P.load_enabled_packages()
    return serialize.loadfile(join(xword.configdir, 'packages.lua')) or {}
end

function P.write_enabled_packages(packages)
    local f = io.open(join(xword.configdir, 'packages.lua'), 'wb')
    if f then
        f:write("-- Set <packagename> = false to disable a package\n")
        f:write("return ")
        f:write(serialize.pprint(packages))
        f:close()
    end
end

function P.load_packages_info()
    -- Load info.lua for all packages and read enabled/disabled states
    -- from config/packages.lua
    local enabled = P.load_enabled_packages()
    local packages = {}
    for _, packagename in ipairs(P.get_all_scripts()) do
        local infofile = join(xword.scriptsdir, packagename, 'info.lua')
        local info = serialize.loadfile(infofile)
                     or { name = packagename, version = "(unknown version)" }
        info.enabled = (enabled[packagename] ~= false)
        info.packagename = packagename
        table.insert(packages, info)
        packages[packagename] = info
    end
    return packages
end

function P.show_xword_update_dialog(updates)
--if true then return false end
    if P.is_newer(updates.xword.version, xword.version) then
        -- Create the dialog
        local dlg = wx.wxMessageDialog(xword.frame,
            string.format(
                "There is a new version of XWord available (version %s).\n"
                .. "Would you like to go to the download page?",
                updates.xword.version
            ),
            "New XWord Version",
            wx.wxYES_NO + (updates.xword.changelog and wx.wxHELP or 0)
        )
        dlg:SetYesNoLabels("Download", "No")
        if has_changelog then
            dlg:SetHelpLabel("Read Changelog")
        end
        -- Show the dialog
        local result = dlg:ShowModal()
        -- Clean up
        dlg:Destroy()
        dlg = nil
        -- Process the result
        if result == wx.wxID_YES then
            wx.wxLaunchDefaultBrowser(updates.xword.download)
            return true
        elseif result == wx.wxID_HELP then
            wx.wxLaunchDefaultBrowser(updates.xword.changelog)
            return true
        else
            updates.xword.ignored = true
            serialize.dump(updates, P.updater.updates_filename)
        end
    end
end

-- xword.pkgmgr isn't actually managed as a separate package, so we need
-- to call the init function ourselves.  Likewise, an uninit function would
-- never be called, so that code must be added to xword.OnCleanup.
if xword.frame then
    init()
    xword.OnCleanup(P.unload_packages)
end

return P
