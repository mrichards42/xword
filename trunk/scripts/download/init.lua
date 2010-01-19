-- ============================================================================
-- Puzzle download
--     A quite complicated XWord add-on that downloads puzzles from a variety
--     of sources.

--     This add-on is implemented as a package and contains several files.
--     Add-ons with multiple files must be placed into their own directory.
--     An init.lua script is the only thing that will be called to initialize
--     the add-on.
-- ============================================================================


-- Create the download table
download = {}

-- Add the download modules
require 'download.defs'
require 'download.panel'
require 'download.dialog'
require 'download.sources'

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles',
        function(evt)
            local dlg = download.GetDialog()
            dlg:Show()
        end
    )
end

init()
