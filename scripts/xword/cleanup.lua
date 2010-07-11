-- ============================================================================
-- XWord cleanup
--     This script is run by XWord just before lua is closed.
-- ============================================================================

for i=#xword.cleanup,1,-1 do
    local func = xword.cleanup[i]
    table.remove(xword.cleanup, i)
    func()
end
