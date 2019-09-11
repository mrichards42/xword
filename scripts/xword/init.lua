--- XWord for lua.
-- This package is loaded by XWord to initialize lua extensions.
-- @module xword

require 'pl.compat' -- Add table.pack

-- table xword is already created in a C++ function

--- Location of lua scripts.
-- @field xword.scriptsdir

--- Location of userdata.
-- Config files are stored in a userdata subdir.
-- @field xword.userdatadir

--- Location of config files.
-- @field xword.configdir

--- Location of application images.
-- @field xword.imagesdir

--- Is XWord being run in portable mode?
-- All paths are different in portable mode.
-- @field xword.isportable

--- XWord's main wxFrame.
xword.frame = xword.GetFrame()

-- Cleanup functions
xword.cleanup = {}

--- Add functions to be called as XWord is closing.
-- @param func A function to be called on cleanup.
function xword.OnCleanup(func)
    table.insert(xword.cleanup, func)
end

--- Compute a relative module path.
-- @param path The path passed to require.
-- @param[opt=1] level Level up from this path.
-- @usage
-- -- At the top of a module
-- local PATH = mod_path(...)
-- local PARENT = mod_path(..., 2)
-- -- Later
-- require(PATH.."relative_module")
-- require(PARENT.."module_relative_to_parent")
function mod_path(path, level)
    for i=1,(level or 1) do
        -- Find the part of the path before the last dot
        path = path:match('^(.+)%.[^%.]+') or path
    end
    -- Add a dot so we can use this as require(PATH .. 'module')
    return path .. '.'
end

-- Setup curl
-- luacurl initialization will call curl_global_init; per
-- https://curl.haxx.se/libcurl/c/threadsafe.html, we must do this before
-- starting any other threads (as is done by the task library) to ensure it
-- completes before anything else attempts to use curl. This isn't 100%
-- sufficient - curl_global_init may call other thread-unsafe methods (e.g.
-- SSL init methods), leading to issues if anything else is using those methods.
-- But it's the best we can do here, and nothing else should be making network
-- requests except for lua plugins.
require 'luacurl'

-- Setup task library
local task = require 'task'
task.error_handler = xword.logerror
-- Abort all tasks on cleanup
xword.OnCleanup(function()
    -- Abort each thread
    for id, t in ipairs(task.list()) do
        t:abort()
    end
end)


-- Require the xword packages
require 'xword.preferences'
require 'xword.menu'
require 'xword.messages'
require 'xword.pkgmgr'

-- Prints an error message if require fails
function xword.require(name)
    local success, result = xpcall(
        function() return require(name) end,
        debug.traceback
    )
    if not success then
        xword.logerror(result)
    end
    return success and result or nil
end

-- Load the plugins if we have a frame.
-- Otherwise this script is being called from the command line
if xword.frame then
    xword.pkgmgr.load_packages()
end
