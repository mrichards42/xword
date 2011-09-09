local P = download
local layout = P.layout


-- ============================================================================
-- Button callbacks
-- ============================================================================

-- These functions can be used as button callbacks to download a specified
-- subset of all download controls.
-- Callbacks like downloadAllDates or downloadAllPuzzles should iterate over
-- the dlg.downloads table when they are called because of the somewhat
-- convoluted downloadCtrl creation process.

local function startDownload(dl)
    wx.wxSafeYield() -- Prevent locking
    dl.start()
end

-- Return a callback function that will start all downloads that meet the
-- specified condition (default is all controls).
function layout.downloadAll(condition)
    local condition = condition or function() return true end
    return function()
        for _, dl in ipairs(P.dlg.downloads) do
            if condition(dl) then startDownload(dl) end
        end
    end
end

