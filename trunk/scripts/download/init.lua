-- ============================================================================
-- Puzzle download
--     A complex XWord add-on that downloads puzzles from a variety
--     of sources.

--     This add-on is implemented as a package and contains many files and even
--     sub-projects (layouts, images).
--     Add-ons with multiple files must be placed into their own directory.
--     An init.lua script is the only script that will be called to initialize
--     the add-on.
-- ============================================================================


-- Create the download table
download = {}

-- ----------------------------------------------------------------------------
-- Configuration
-- ----------------------------------------------------------------------------
require 'download.defaultconfig'
pcall(dofile, xword.configdir..'/download/config.lua') -- Use default config if this fails

require 'download.defaultsources'
pcall(dofile, xword.configdir..'/download/sources.lua') -- Use default sources if this fails

-- ----------------------------------------------------------------------------
-- init / cleanup
-- ----------------------------------------------------------------------------
download.__init_funcs = {}
function download.onInit(func)
    table.insert(download.__init_funcs, func)
end
function download.doInit()
    for _, func in ipairs(download.__init_funcs) do
        func()
    end
end

download.__close_funcs = {}
function download.onClose(func)
    table.insert(download.__close_funcs, func)
end
function download.doClose()
    for _, func in ipairs(download.__close_funcs) do
        func()
    end
    collectgarbage()
end

-- Add the download modules
require 'download.messages'
if task.id() == 1 then
    require 'download.dialog'
    require 'download.savestate'
end

-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles\tCtrl+D',
        function(evt)
            local dlg = download.GetDialog()
            dlg:Raise()
            dlg:Show()
        end
    )
end

local function uninit()
    xword.frame:RemoveMenuItem('Tools', 'Download puzzles')
    -- Save configuration and sources
    download.savestate()
end

return { init, uninit }
