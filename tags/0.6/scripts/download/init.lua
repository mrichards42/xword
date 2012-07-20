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

local DownloadDialog = require 'download.dialog'

-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles\tCtrl+D',
        function(evt)
            local dlg = DownloadDialog()
            dlg:update()
            dlg:Raise()
            dlg:Show()
        end
    )
    -- Check auto_download
    if download.auto_download > 0 then
        require 'download.puzzles'
        require 'download.config'
        require 'download.download'
        require 'download.stats'
        require 'date'
        local end_date = date():sethours(0,0,0,0):adddays(1)
        local start_date = end_date:copy():adddays(-download.auto_download)
        -- Find downloads
        local downloads = {}
        local d = start_date:copy()
        while d < end_date do
            for _, puzzle in download.puzzles:iter() do
                if puzzle.days[d:getisoweekday()] then
                    local data = download.get_download_data(puzzle, d)
                    if not download.puzzle_exists(data.filename) then
                        table.insert(downloads, data)
                    end
                end
            end
            d:adddays(1)
        end
        if #downloads > 0 then
            for k,v in pairs(downloads) do print(v.filename) end
            download.add_downloads(downloads)
        end
    end
end

local function uninit()
    xword.frame:RemoveMenuItem('Tools', 'Download puzzles')
    -- Save configuration and sources
end

return { init, uninit }
