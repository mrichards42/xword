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
        for _, dl in ipairs(P.dlg.alldownloads) do
            if condition(dl) then startDownload(dl) end
        end
    end
end


-- Download all puzzles on a given date that meet a condition
function layout.downloadAllDates(target_date, condition)
    local condition = condition or function() return true end
    return function ()
        for name, dates in pairs(P.dlg.downloads) do
            for dl_date, dl in pairs(dates) do
                if target_date == dl_date then
                    if condition(dl) then startDownload(dl) end
                end
            end
        end
    end
end


-- Download all available puzzles from a given source that meet a condition
function layout.downloadAllPuzzles(target_label, condition)
    local condition = condition or function() return true end
    return function ()
        for _, dl in pairs(P.dlg.downloads[target_label]) do
            if condition(dl) then startDownload(dl) end
        end
    end
end
