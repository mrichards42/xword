-- ============================================================================
-- XWord additions
--     This package is loaded by XWord to initialize lua extensions and
--     add-ons
-- ============================================================================

xword.frame = xword.GetFrame()

-- Cleanup functions
xword.cleanup = {}
function xword.OnCleanup(func)
    table.insert(xword.cleanup, func)
end

-- Inject important xword data into task.create
xword.task_exports = {
    scriptsdir  = xword.scriptsdir,
    imagesdir   = xword.imagesdir,
    configdir   = xword.configdir,
    userdatadir = xword.userdatadir,
    isportable  = xword.isportable,
}
local task_create = require 'wxtask'.create
function task.create(s, args)
    args = args or {}
    args.xword = xword.task_exports
    return task_create(s, args)
end

-- Require the xword packages
require 'xword.menu'
require 'xword.messages'

-- Load the plugins if we have a frame.
-- Otherwise this script is being called from the command line
if xword.frame then
    require 'xword.pkgmgr'.LoadPlugins()
end
