-- ============================================================================
-- The main download dialog
--     The dialog maintains a table of currently active downloads.  Before
--     application exit, the dialog terminates all active download threads.
-- ============================================================================

require 'mtask'
require 'download.ctrl'
require 'download.layout'
require 'download.dltable'
require 'date'

assert(download, 'Must load download package first')

-- Integration with 'import' plugin for converting downloaded puzzles
if not pcall(require, 'import') then
    import = {}
    import.handlers = {}
end

-- Easier access to download package
local P = download


-- ----------------------------------------------------------------------------
-- The download dialog
-- ----------------------------------------------------------------------------

function P.GetDialog()
    -- Only allow one download dialog per session
    if P.dlg then return P.dlg end

    local dlg = wx.wxFrame(
        xword.frame, wx.wxID_ANY, "Puzzle Download",
        wx.wxDefaultPosition, wx.wxSize(unpack(P.dlgsize))
    )
    P.dlg = dlg

    -- Controls
    dlg.scroller = wx.wxScrolledWindow(dlg, wx.wxID_ANY)
    dlg.panel = wx.wxPanel(dlg.scroller, wx.wxID_ANY)
    dlg.scroller:SetScrollRate(10,10)
    dlg.scroller:Scroll(0,0)

    -- Menu
    local menubar = wx.wxMenuBar()
        menubar:Append(P.layout.createViewMenu(), "View")
        menubar:Append(P.layout.createDownloadMenu(), "Download")
    dlg:SetMenuBar(menubar)


    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local panelSizer = wx.wxBoxSizer(wx.wxVERTICAL)
            panelSizer:Add(dlg.panel, 1, wx.wxEXPAND + wx.wxALL, 10)
        dlg.scroller:SetSizer(panelSizer)
        sizer:Add(dlg.scroller, 1, wx.wxEXPAND, 0)
    dlg:SetSizer(sizer)


    -- Special hash table for downlod ctrls:
    -- This table uses puzzle name/source and date as keys.
    -- e.g. dlg.downloads['source name'][date(yyyy, m, d)]
    dlg.downloads = download.dltable()

    dlg.alldownloads = {} -- An array of all downloads
    dlg.activedownloads = {} -- An array of currently running downloads

    -- ------------------------------------------------------------------------
    -- Dialog closing: Make sure no download threads are running
    -- ------------------------------------------------------------------------

    function dlg.AbortDownloads()
        -- Sometimes all the downloads aren't cleaned up from the
        -- activedownloads list.  I imagine it's a matter of timing with
        -- threads starting and ending, but I'm not sure why.  In any case,
        -- we can't call task.post() with nil as the task_id, so we'll clean
        -- up not really active downloads here.
        print 'Abort downloads'
        for i, dl in ipairs(dlg.activedownloads) do
            assert(dl.task_id)
            -- The downloadCtrl will call NotifyDownloadEnd() when this
            -- thread really aborts.
            task.post(dl.task_id, 'blah', P.DL_ABORT)
        end
    end

    -- Make sure the threads are killed before we close the dialog. We will
    -- call dlg:Close() again once the threads are killed.
    dlg:Connect(dlg:GetId(), wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            if #dlg.activedownloads == 0 then
                print 'destroying dialog'
                -- Save the dialog size
                P.dlgsize = { dlg.Size.Width, dlg.Size.Height }
                dlg:Destroy()
                -- Make this nil so the frame close evt handler knows the
                -- dlg is destroyed.
                dlg = nil
                P.dlg = nil
            else -- We have threads running
                print(string.format('%d threads are running . . . aborting them', #dlg.activedownloads))
                dlg:Hide()
                dlg.OnDownloadEnd(function() dlg:Close() end)
                dlg.AbortDownloads()
                evt:Veto()
            end
        end
    )


    -- ------------------------------------------------------------------------
    -- Download starting and ending
    -- ------------------------------------------------------------------------

    -- These functions should be called from the downloadCtrls to notify the
    -- dialog when downloads start and finish.
    function dlg.NotifyDownloadStart(dl)
        print("Download start "..dl.filename)
        table.insert(dlg.activedownloads, dl)
        dlg.panel:GetSizer():Layout()
        dlg.panel:FitInside()
    end

    function dlg.NotifyDownloadEnd(dl)
        for i, ctrl in ipairs(dlg.activedownloads) do
            if ctrl == dl then
                print("Download end "..dl.filename)
                table.remove(dlg.activedownloads, i)
                break
            end
        end
        dlg.checkAllDownloadsCompleted()
    end

    -- Table to hold functions that get called when all downloads have finished
    dlg.completion_events = {}
    function dlg.OnDownloadEnd(func)
        table.insert(dlg.completion_events, func)
    end

        -- If all downloads have completed, check to see if someone has
        -- attached a download completion event.
    function dlg.checkAllDownloadsCompleted()
        if #dlg.activedownloads == 0 then
            -- It's possible that the dialog might be destroyed after
            -- calling func(), so make sure the dialog still exists each
            -- time.
            while dlg do
                local func = table.remove(dlg.completion_events)
                if not func then break end
                func()
            end
        end
    end

    P.layout.setLayout('Grid')
    return dlg
end


-- Hijack the xword Frame's close method so we can clean up our threads
xword.frame:Connect(wx.wxEVT_CLOSE_WINDOW,
    function(evt)
        -- We can't use Disconnect to remove a single function (in wxLua)
        -- so just make this event handler irrelevant if the dlg is
        -- closed.
        if not P.dlg then evt:Skip() return end

        -- Let the frame's event handling pass through if we have already
        -- killed all the threads.
        if #P.dlg.activedownloads == 0 then
            evt:Skip()
        else
            -- We still have threads;
            -- kill them before we let the frame close.
            wx.wxBusyInfo('Ending downloads . . . please wait', xword.frame)
            P.dlg.OnDownloadEnd(function() dlg:Close() xword.frame:Close() end)
            P.dlg.AbortDownloads()
            evt:Veto()
        end
    end
)
