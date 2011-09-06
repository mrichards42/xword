-- ============================================================================
-- XWord additions
--     This package is loaded by XWord to initialize lua extensions and
--     add-ons
-- ============================================================================

require 'luapuz'

xword.frame = xword.GetFrame()

-- Cleanup functions
xword.cleanup = {}
function xword.OnCleanup(func)
    table.insert(xword.cleanup, func)
end

-- Require the xword packages
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
