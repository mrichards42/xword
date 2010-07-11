-- ===========================================================================
-- XWord package manager
-- ===========================================================================

local P = {}

local join = require 'pl.path'.join
xword.packagesfile = join(xword.configdir, 'packages.lua')

function P.GetPackagesTable()
    -- Load the packages table
    require 'safe_exec'
    return safe_dofile(xword.packagesfile)
end

P.loaded = {}
function P.LoadPlugins()
    -- Load each script in the scripts part of the packages table
    local packages = P.GetPackagesTable()
    if packages and packages.scripts then
        for _, script in ipairs(packages.scripts) do
            if script.packagename then
                if not script.disabled then
                    P.loaded[script.name.." "..script.version] = true
                    require(script.packagename)
                else
                    P.loaded[script.name.." "..script.version] = false
                end
            end
        end
    end
end


-- Add the package dialog to a menu
if xword.frame then
    xword.frame:AddMenuItem({"Tools"}, "Package Manager", function(evt)
        require 'xword.pkgmgr.dialog'
        P.dlg = P.PackageDialog()
        P.dlg:ShowModal()
    end)
end

xword.pkgmgr = P
return P