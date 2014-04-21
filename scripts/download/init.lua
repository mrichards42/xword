-- ============================================================================
-- Puzzle download
--     A complex XWord add-on that downloads puzzles from a variety
--     of sources.

--     This add-on is implemented as a package and contains many files and even
--     sub-projects.
--     Add-ons with multiple files must be placed into their own directory.
--     An init.lua script is the only script that will be called to initialize
--     the add-on.
-- ============================================================================

local _R = (string.match(..., '^.+%.') or ... .. '.') -- Relative require

local function show_dialog()
    local DownloadDialog = require(_R .. 'gui.dialog')
    local dlg = DownloadDialog(xword.frame)
    dlg:SetSize(wx.wxSize(500,300))
    dlg:Raise()
    dlg:Show()
    return dlg
end


-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    local config = require(_R .. 'config')
    config.load()
    -- Add the download menu item
    xword.frame:AddMenuItem({'Tools'}, 'Download puzzles\tCtrl+D',
        function(evt) show_dialog() end
    )
    -- Check auto_download
    if config.auto_download > 0 then
        local sources = require(_R .. 'sources')
        local mgr = require(_R .. 'manager')
        -- Download puzzles we don't have yet
        local date = require 'date'
        local end_date = date()
        local start_date = end_date:copy():adddays(-config.auto_download)
        sources:download(start_date, end_date)
    end
end

local function uninit()
    xword.frame:RemoveMenuItem('Tools', 'Download puzzles')
    require(_R .. 'config').save()
    -- Reset the sources
    package.loaded[_R .. 'sources'] = nil
end

if not xword.frame then
    local config = require(_R .. 'config')
    config.load()
    local dlg = show_dialog()
    dlg:Connect(wx.wxEVT_CLOSE_WINDOW, function (evt)
        dlg:Destroy()
        evt:Skip()
    end)
end

return { init, uninit }
