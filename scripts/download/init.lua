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
local download = download

require 'download.dialog'

local DownloadDialog = require 'download.dialog'

-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles\tCtrl+D',
        function(evt)
            local dlg = DownloadDialog()
            dlg:Raise()
            dlg:Show()
        end
    )
end

local function uninit()
    xword.frame:RemoveMenuItem('Tools', 'Download puzzles2')
    -- Save configuration and sources
end

return { init, uninit }
