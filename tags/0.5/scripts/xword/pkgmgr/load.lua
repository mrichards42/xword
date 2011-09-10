--[[
    load.lua
    Contains plugin loading and unloading functions
]]

local join = require 'pl.path'.join
local lfs = require 'lfs'

local P = xword.pkgmgr


-- ===========================================================================
-- Load a plugin or all plugins in the package table
-- ===========================================================================

function P.load_package(name)
    -- Require the package and initialize it
    local success, result = xpcall(
        function() return require(name) end,
        debug.traceback
    )
    if not success then
        return false, result
    end

    -- Packages should return either of the following:
    -- return function init()
    -- return { function init(), function uninit() }
    if type(result) == 'function' then
        package.loaded[name] = { init = result }
    elseif type(result) == 'table' then
        if #result > 0 then
            package.loaded[name] = { init = result[1], uninit = result[2] }
        else
            package.loaded[name] = result
        end        
    end
    -- Call init
    local init = package.loaded[name].init
    if init then
        local success, err = xpcall(init, debug.traceback)
    else
        xword.logerror("Package %s must return an initialization function", name)
    end
    if not success then
        xword.Error("Package '%s' is disabled because of an error while loading the package.\nSee log for details.", name)
        xword.logerror("Package '%s' is disabled because of an error while loading the package:\n%s", name, err)
        return false
    end
    return true
end


-- Load all the packages
function P.load_packages()
    require 'serialize'
    collectgarbage('stop')
    -- Load the table of enabled/disabled packages
    local packages = P.load_enabled_packages()
    -- Walk the scripts directory and load all the packages
    for _, name in ipairs(P.get_all_scripts()) do
        if packages[name] ~= false then
            local success, err = P.load_package(name)
            -- If we can't require the package, disable it
            if not success then
                xword.Error("Package '%s' is disabled because of an error while loading the package.\nSee log for details.", name)
                xword.logerror("Package '%s' is disabled because of an error while loading the package:\n%s", name, err)
                packages[name] = false
            else
                packages[name] = true
            end
        end
    end
    -- Rewrite the enabled/disabled packages file
    P.write_enabled_packages(packages)
    collectgarbage('restart')
    collectgarbage('collect')
end

-- ===========================================================================
-- Unload a plugin
-- ===========================================================================

function P.unload_package(name)
    local funcs = package.loaded[name]
    if not funcs then -- Never loaded
        return true
    end
    if type(funcs) == "table" and funcs.uninit then
        funcs.uninit()
        return true
    end
    return false
end

-- Unload each script in the scripts part of the packages table
function P.unload_packages()
    local packages = P.packages
    for _, name in ipairs(P.get_all_scripts()) do
        P.unload_package(name)
    end
end
