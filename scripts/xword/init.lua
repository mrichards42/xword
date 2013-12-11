--- XWord for lua.
-- This package is loaded by XWord to initialize lua extensions.
-- @module xword

require 'luapuz'
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

-- Export some globals for all tasks
local task = require 'wxtask'
task.globals.xword = {
    scriptsdir  = xword.scriptsdir,
    imagesdir   = xword.imagesdir,
    configdir   = xword.configdir,
    userdatadir = xword.userdatadir,
    isportable  = xword.isportable,
}

-- Set task error handler
task.error_handler = xword.logerror

-- Abort all tasks on cleanup
xword.OnCleanup(function()
    while true do
        local is_done = true
        -- Try to abort each thread
        for id, _ in pairs(task.list()) do
            if id ~= 1 then
                is_done = false
                task.find(id):abort()
            end
        end
        -- Main thread is the only one left
        if is_done then return end
        -- Wait a bit, then check the list of tasks again
        task.sleep(10)
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
