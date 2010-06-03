-- ============================================================================
-- Download dialog layouts
--     This file contains utility functions for download layouts
--     The rest of this directory contains the actual layouts
-- ============================================================================

-- These functions can only be called after the download dialog has been
-- created.

require 'download.ctrl'
require 'date'

download.layout = {}

local P = download
local layout = P.layout

layout.layouts = {}

-- Return today with no time
function layout.today()
    local d = date()
    d.dayfrc = 0 -- clear the time
    return d
end

-- Register a layout with a name, and function, and arguments.
-- The function must take start_date as the first parameter.
-- Any other parameters are entirely up to the user
function layout.addLayout(name, func, ...)
    table.insert(layout.layouts, { name = name, func = func, args = arg })
    layout.layouts[name] = { name = name, func = func, args = arg }
end

require 'download.layout.callbacks'
require 'download.layout.menus'


-- Create a single download ctrl, adding it to the download dialog's
-- list of downloads if it does not already exist.
function layout.createDownloadCtrl(source, d)
    local dlg = assert(P.dlg)
    if not dlg.downloads[source.display] then
        dlg.downloads:addsource(source.display)
    end
    local dl = dlg.downloads[source.display][d]
    if not dl then -- Download does not exist so create it.
        dl = P.DownloadCtrl(dlg.panel, source, d)
        dlg.downloads[source.display][d] = dl
    end
    -- Add the download to the various download lists.
    table.insert(dlg.alldownloads, dl)
end


--[[
Create and return a list of DownloadCtrls.

Parameters:
    createDownloadCtrls([sources], [start_date], [end_date])
        sources
            <a single source> OR <a table of download sources>
            Default: all sources.
        start_date (inclusive)
            <date>
            Default: last week
        end_date (inclusive)
            <date>
            Default: today
]]
function layout.createDownloadCtrls(...)
    local dlg = assert(P.dlg)

    local sources, start_date, end_date

    -- Figure out which parameters we got

    if #arg >= 3 then
        sources, start_date, end_date = unpack(arg)
    elseif #arg == 2 then
        if arg[1].daynum then -- arg[1] is a date
            start_date, end_date = unpack(arg)
        else
            sources, start_date = unpack(arg)
        end
    elseif #arg == 1 then
        if arg[1].daynum then -- arg[1] is a date
            start_date = unpack(arg)
        else
            sources = unpack(arg)
        end
    end

    -- Defaults
    sources = sources or P.sources
    if not start_date then
        start_date = layout.today()
        start_date:adddays(-7)
    end
    end_date = end_date or layout.today()

    -- Make sure sources is a table of sources
    if sources.display then  -- sources is a single source
        sources = { sources }
    end

    -- Create all the matching downloads
    for _, source in ipairs(sources) do
        local d = start_date:copy()
        while d <= end_date do
            layout.createDownloadCtrl(source, d)
            d:adddays(1)
        end
    end
end


-- Destroy old download ctrls and remove them from dlg.downloads.
-- New ctrls are those that are present in both dlg.alldownloads and
-- dlg.downloads (i.e. they have been created since the last call to
--  layout.setLayout()).  Old downloads are only present in dlg.downloads.
function layout.cleanDownloadCtrls()
    -- Create a reverse download ctrl table { [dl] = idx }
    local ctrls = {}
    for k, v in pairs(P.dlg.alldownloads) do ctrls[v] = k end

    -- Find all the extra download ctrls
    for _, source in pairs(P.dlg.downloads) do
        for dl_date, dl in pairs(source) do
            if not ctrls[dl] then
                print("Destroying", dl.ctrl)
                dl.ctrl:Destroy()
                -- Lua reference manual says it is OK to remove a value from
                -- a hash table during iteration.
                source[dl_date] = nil
            end
        end
    end
end

-- Destroy all windows that are not attached to a sizer in the current layout.
-- This is called at the end of layout.setLayout()
function layout.cleanLayout()
    -- Clean download ctrls from dlg.downloads
    layout.cleanDownloadCtrls()

    -- Destroy any other child windows
    local child = P.dlg.panel.Children.First
    while child do
        local window = child.Data:DynamicCast("wxWindow")
        -- Get the next node *before* we destroy the window
        child = child.Next
        if not window.ContainingSizer then window:Destroy() end
    end
end


-- Change the layout, providing either a function or a layout name.
-- Handle aborting current downloads, freezing and thawing the dialog,
-- resetting the download tables, destroying child windows, etc.
function layout.setLayout(layoutFunc, start_date, ...)
    -- Find the function in the layouts table if we were given a string
    if type(layoutFunc) == "string" then
        local desc = assert(layout.layouts[layoutFunc])
        layoutFunc = desc.func
        if #arg == 0 then arg = desc.args end
    end

    if not start_date then
        start_date = layout.today()
        start_date:adddays(-7)
    end

    local dlg = assert(P.dlg)

    -- If there are active downloads, make sure they are done.
    if #dlg.activedownloads > 0 then
        if xword.Prompt(
               string.format("There are %d active downloads.\nAbort?",
                             #dlg.activedownloads))
        then
            dlg.AbortDownloads()
            -- Call this function again when all downloads have completed.
            dlg.OnDownloadEnd(
                function()
                    layout.setLayout(layoutFunc, start_date, unpack(arg))
                end)
        else
            -- Do nothing if the user answered "No"
            return
        end
    end

    dlg:Freeze()

    -- Set a dummy sizer to destroy the previous one
    dlg.panel:SetSizer(wx.wxBoxSizer(wx.wxVERTICAL))

    -- downloads owned by the dialog:
    -- (hash)  downloads["displayName"][date]
    -- (array) alldownloads[] ...
    -- We won't touch the dlg.downloads hash table in case we can reuse some
    -- of the existing download ctrls.
    dlg.alldownloads = {}
    dlg.activedownloads = {}

    -- Create the new layout
    layoutFunc(start_date, unpack(arg))

    -- Make sure the layout set a sizer or we'll have big problems
    assert(dlg.panel.Sizer)

    -- Our own version of lua's "tag and sweep" garbage collection!
    -- Clean up any windows that are not used in the current layout.  Any
    -- window that is not part of a sizer will be destroyed.
    layout.cleanLayout()

    layout.updateMenus()

    dlg.scroller:FitInside()
    dlg.scroller:Scroll(0,0)

    dlg:Thaw()
end




-- Standard fonts
layout.font = wx.wxFont(12, wx.wxFONTFAMILY_SWISS,
                        wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL)

layout.boldfont = wx.wxFont(12, wx.wxFONTFAMILY_SWISS,
                            wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD)


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


-- Require the layouts (this order determines the menu order)
require 'download.layout.grid'
require 'download.layout.puzzle'
