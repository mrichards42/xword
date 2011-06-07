-- ============================================================================
-- XWord cleanup
--     This script is run by XWord just before lua is closed.
-- ============================================================================

-- We're exiting, so there is no need to collect any garbage.
collectgarbage('stop')

for i=#xword.cleanup,1,-1 do
    local func = xword.cleanup[i]
    table.remove(xword.cleanup, i)
    -- Errors are strictly *not* allowed during cleanup, since an error
    -- in one cleanup function would stop cleanup altogether.
    -- This is mostly a big deal if an error occurrs before we are allowed
    -- to clean up threads, destroying pointers that the threads use before
    -- the threads end.
    local success, err = xpcall(func, debug.traceback)
    if not success then xword.logerror(err) end
end
