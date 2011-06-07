-- ============================================================================
-- Download dialog layouts
--     This file contains utility functions for download layouts
--     The rest of this directory contains the actual layouts
-- ============================================================================

-- These functions can only be called after the download dialog has been
-- created.

require 'date'

local P = download
P.layout = {}
local layout = P.layout

require 'download.layout.tracking'
require 'download.layout.callbacks'
require 'download.layout.filter'

-- --------------------------------------------------------------------------
-- Layout switching
-- --------------------------------------------------------------------------

layout.layouts = {}

-- Register a layout with a name, function, and required filter items.
-- e.g. layout.addLayout{"name", function, date=true, puzzle=true, weekday=false}
-- function should have the signature function(sources, date, weekday)
function layout.addLayout(opts)
    opts.name = assert(opts.name or opts[1])
    opts.func = assert(opts.func or opts[2])
    table.insert(layout.layouts, opts)
    layout.layouts[opts.name] = opts
end


-- This hack allows us to abort the current layout if the user has requested
-- a new layout during the creation of this layout.
-- Frequently during the layout (during createDownloadCtrl), we call
-- wxYield() and check __continue_layout to make sure that the layout
-- should continue.  If the layout was aborted, throw an error, which will be
-- caught by the pcall in setLayout().
-- If setLayout() is called while in the middle of another layout
-- (i.e. __continue_layout is already true), store the function arguments
-- in __next_layout and return.  The current layout function should throw
-- an error(), and the __next_layout will be called.
layout.__next_layout = false
layout.__continue_layout = false


function layout.checkAbort()
    -- Check to see if we should abort the layout
    wx.wxYield()
    if not layout.__continue_layout then
        error()
    end
end

-- Change the layout, providing either a function or a layout name.
-- Handle aborting current downloads, freezing and thawing the dialog,
-- resetting the download tables, destroying child windows, etc.
function layout.setLayout(layoutName)
    local dlg = assert(P.dlg)

    -- If there are active downloads, make sure they are done.
    if #dlg.activedownloads > 0 then
        if xword.Prompt("There are %d active downloads.\nAbort?",
                         #dlg.activedownloads)
        then
            -- Call this function again when all downloads have completed.
            dlg.OnDownloadEnd(
                function()
                    layout.setLayout(layoutName)
                end
            )
            dlg.AbortDownloads()
        end
        return
    end

    -- This will be called after we're done
    if layout.__continue_layout then
        layout.__next_layout = layoutName
        layout.__continue_layout = false
        return
    end
    
    layout.__continue_layout = true


    -- Find the layout
    local current = layout.layouts[layoutName]
    layout.current = current
    local layoutFunc = current.func

    -- Enable / disable filter items
    local f = layout.filter
    f.puzzle:Enable(current.puzzle and true or false)
    local useDate = current.date and true or false
    f.dateDisplay:Enable(useDate)
    f.prevDate:Enable(useDate)
    f.nextDate:Enable(useDate)
    f.current:Enable(useDate)
    f.weekday:Enable(current.weekday and true or false)


    dlg.panel:Freeze()

    -- Set a dummy sizer to destroy the previous one
    dlg.panel:SetSizer(wx.wxBoxSizer(wx.wxVERTICAL))

-- --[[
    -- Create the new layout
    -- pcall this so that we can thrown an error() if we want to
    layout.tracking.onLayoutStart()
    local success, err = xpcall(
        -- Function
        function ()
            layoutFunc(
                layout.getFilteredSources(),
                P.filterState.date,
                current.weekday and P.filterState.weekday or 0
            )
        end,
        -- Error handler
        debug.traceback
    )
    layout.tracking.onLayoutEnd()
    if err then
        print(err)
    end
--]]
--[[
    layout.tracking.onLayoutStart()
    layoutFunc(
        layout.getFilteredSources(),
        P.filterState.date,
        current.weekday and P.filterState.weekday or 0
    )
    layout.tracking.onLayoutEnd()
--]]
    dlg.panel:Thaw()

    -- Did this layout get aborted for another layout?
    layout.__continue_layout = false
    if layout.__next_layout then
        local l = layout.__next_layout
        layout.__next_layout = false
        layout.setLayout(l)
    else
        dlg.panel:Freeze()
        dlg.panel:Layout()
        dlg.panel:Thaw()
        dlg.panel:Refresh()
        collectgarbage()
    end
end


-- Standard fonts
download.onInit(function()
    layout.font = wx.wxFont(10, wx.wxFONTFAMILY_SWISS,
                            wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL)

    layout.boldfont = wx.wxFont(10, wx.wxFONTFAMILY_SWISS,
                                wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD)
end)

download.onClose(function()
    layout.font:delete()
    layout.font = nil
    layout.boldfont:delete()
    layout.boldfont = nil
end)


-- Require the layouts (this order determines the menu order)
require 'download.layout.grid'
require 'download.layout.month'
require 'download.layout.puzzle'
