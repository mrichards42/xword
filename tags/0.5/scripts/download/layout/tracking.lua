require 'download.ctrl'
require 'download.dltable'
local path = require 'pl.path'
local tablex = require 'pl.tablex'

local P = download
local layout = P.layout


-- Tracked windows
layout.tracking = {}
local t = layout.tracking


-- ---------------------------------------------------------------------------
-- Stale windows
-- ---------------------------------------------------------------------------

-- Make sure this window isn't destroyed
function layout.keep_window(window)
    t.stale[window:DynamicCast("wxWindow")] = nil
end

-- Keep track of direct children of dlg.panel
local function trackOldWindows()
    t.stale = {}
    local child = P.dlg.panel.Children.First
    while child do
        local childWindow = child.Data:DynamicCast("wxWindow")
        assert(childWindow, "Not a window: "..tostring(child.Data))
        t.stale[childWindow] = true
        child = child.Next
    end

end

-- Destroy all direct children of dlg.panel that are no longer in use
local function destroyOldWindows()
    for window, _ in pairs(t.stale) do
        if getmetatable(window) then
            window:Destroy()
        end
    end
    t.stale = nil
end



-- ---------------------------------------------------------------------------
-- DownloadCtrls
-- ---------------------------------------------------------------------------

-- Create a single download ctrl or Reparent() an existing download ctrl
-- given the source name and date.
function layout.createDownloadCtrl(parent, source, d)
    layout.checkAbort()
    local dl = P.DownloadCtrl(parent, source, d)
    if dl then
        table.insert(P.dlg.downloads, dl)
    end
    return dl
end


-- Create a list of DownloadCtrls and return them as a dltable
function layout.createDownloadCtrls(parent, sources, start_date, end_date, weekday)
    -- Make sure sources is a table of sources
    if sources.display then  -- sources is a single source
        sources = { sources }
    end
    weekday = weekday or 0

    -- Create all the matching downloads
    local dls = download.dltable()
    for _, source in ipairs(sources) do
        local d = start_date:copy()
        while d <= end_date do
            if weekday == 0 or d:getisoweekday() == weekday then
                dls[source.display][d] =
                    layout.createDownloadCtrl(parent, source, d)
            end
            d:adddays(1)
        end
    end
    return dls
end

-- ---------------------------------------------------------------------------
-- Scrolled Windows
-- ---------------------------------------------------------------------------

local function updateScroller()
    P.dlg.scroller:FitInside()
    P.dlg.scroller:Scroll(0,0)
end


-- ---------------------------------------------------------------------------
-- Headers
-- ---------------------------------------------------------------------------

-- Create and return a header
function layout.Header(parent, text, bold)
    local header = wx.wxStaticText(parent, wx.wxID_ANY, text)
    if bold then
        header:SetFont(layout.boldfont)
    else
        header:SetFont(layout.font)
    end
    return header
end



-- ---------------------------------------------------------------------------
-- Layout functions
-- ---------------------------------------------------------------------------
local database = require 'download.database'

function t.onLayoutStart()
    P.dlg.downloads = {}
    trackOldWindows()
end

function t.onLayoutEnd()
    destroyOldWindows()
    updateScroller()
end
