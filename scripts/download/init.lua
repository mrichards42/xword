-- ============================================================================
-- Puzzle download
--     A quite complicated XWord add-on that downloads puzzles from a variety
--     of sources.

--     This add-on is implemented as a package and contains many files and even
--     a sub-project (layouts).
--     Add-ons with multiple files must be placed into their own directory.
--     An init.lua script is the only script that will be called to initialize
--     the add-on.
-- ============================================================================


-- Create the download table
download = {}

-- Add the download modules
require 'download.defs'
require 'download.dialog'
require 'download.savestate'
require 'download.index'

-- ----------------------------------------------------------------------------
-- Configuration
-- ----------------------------------------------------------------------------
require 'download.defaultconfig'
pcall(dofile, xword.GetConfigDir()..'/download/config.lua') -- Use default config if this fails

require 'download.defaultsources'
pcall(dofile, xword.GetConfigDir()..'/download/sources.lua') -- Use default sources if this fails

-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles',
        function(evt)
            download.GetDialog():Show()
        end
    )

    -- Save configuration and sources when the frame closes
    xword.frame:Connect(wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            if download.database then
                download.database.close()
            end
            download.savestate()
            evt:Skip()
        end
    )

    -- Start indexing downloaded puzzles
    --download.index.start()
end

init()
