local function init()
    -- Add a "debug" submenu to the tools menu
    local importMenu = xword.frame:FindOrCreateMenu({'Tools', 'Debug'})
end

init()

-- Require the rest of the debug directory
xword.requiredir('xworddebug')