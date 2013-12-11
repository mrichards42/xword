--[[
    load.lua
    Contains plugin loading and unloading functions
]]

local join = require 'pl.path'.join
local lfs = require 'lfs'
local startswith = require 'pl.stringx'.startswith

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
    else
        return false, "Package must return an initialization function"
    end
    -- Call init
    local init = package.loaded[name].init
    if init then
        local success, err = xpcall(init, debug.traceback)
        if not success then
            xword.Message(err)
            return false, err
        end
    else
        return false, "Package must return an initialization function"
    end
    return true
end


-- Load all the packages
function P.load_packages()
    require 'serialize'
    collectgarbage('stop')
    errors = {}
    -- Load the table of enabled/disabled packages
    local packages = P.load_enabled_packages()
    -- Walk the scripts directory and load all the packages
    for _, name in ipairs(P.get_all_scripts()) do
        if packages[name] ~= false then
            local success, err = P.load_package(name)
            -- If we can't require the package, disable it
            if not success then
                table.insert(errors, name)
                xword.logerror("Package '%s' is disabled because of an error while loading the package:\n%s", name, err)
                packages[name] = false
            else
                packages[name] = true
            end
        end
    end
    if #errors > 0 then
        xword.Error("The following packages are disabled because of error(s)"..
                    "while loading:\n    %s\nSee log for details.",
                    table.concat(errors, '\n    '))
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
        -- Remove all references in package.loaded
        package.loaded[name] = nil
        local namedot = name .. '.'
        for k, _ in pairs(package.loaded) do
            if startswith(k, namedot) then
                package.loaded[k] = nil
            end
        end
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
