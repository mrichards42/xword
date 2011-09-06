-- ============================================================================
-- The main download dialog
--     The dialog maintains a table of currently active downloads.  Before
--     application exit, the dialog terminates all active download threads.
-- ============================================================================

require 'wxtask'
require 'download.ctrl'
require 'download.layout'
require 'date'

assert(download, 'Must load download package first')

-- Easier access to download package
local P = download

-- ----------------------------------------------------------------------------
-- The download dialog
-- ----------------------------------------------------------------------------

function P.GetDialog()
    -- Only allow one download dialog per session
    if P.dlg then return P.dlg end

    P.doInit()    

    local dlg = wx.wxFrame(
        xword.frame, wx.wxID_ANY, "Puzzle Download",
        wx.wxDefaultPosition, wx.wxSize(unpack(P.dlgsize)),
        wx.wxDEFAULT_FRAME_STYLE + wx.wxCLIP_CHILDREN
    )
    P.dlg = dlg

    -- Controls
    local outerPanel = wx.wxPanel(dlg, wx.wxID_ANY)
    dlg.header = wx.wxPanel(outerPanel, wx.wxID_ANY)
    dlg.filter =wx.wxPanel(dlg.header, wx.wxID_ANY)
    P.layout.createFilter()
    dlg.scroller = wx.wxScrolledWindow(outerPanel, wx.wxID_ANY)
    dlg.panel = wx.wxPanel(dlg.scroller, wx.wxID_ANY)

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local headerSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            headerSizer:Add(dlg.filter, 1, wx.wxEXPAND)
        dlg.header:SetSizer(headerSizer)
        sizer:Add(dlg.header, 0, wx.wxEXPAND + wx.wxALL, 5)
        local scrollerSizer = wx.wxBoxSizer(wx.wxVERTICAL)
            scrollerSizer:Add(dlg.panel, 1, wx.wxEXPAND + wx.wxALL, 10)
        dlg.scroller:SetSizer(scrollerSizer)
        sizer:Add(dlg.scroller, 1, wx.wxEXPAND + wx.wxALL, 0)
    outerPanel:SetSizer(sizer)
    dlg.scroller:SetScrollRate(10,10)

     -- An array of currently running downloads
    dlg.activedownloads = {}

    -- Animation timer
    dlg.timer = wx.wxTimer(dlg)
    function dlg.OnTimer(evt)
        for _, dl in pairs(dlg.activedownloads) do
            if dl.has_ctrl() then
                dl.ctrl.OnTimer()
            end
        end
    end
    dlg:Connect(wx.wxEVT_TIMER, dlg.OnTimer)

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
            -- The downloadCtrl will call NotifyDownloadEnd() when this
            -- thread really aborts.
            task.abort(dl.task_id)
        end
    end

    -- Make sure the threads are killed before we close the dialog. We will
    -- call dlg:Close() again once the threads are killed.
    dlg:Connect(dlg:GetId(), wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            if #dlg.activedownloads == 0 then
                dlg:Destroy()
                dlg = nil
                P.dlg = nil
                P.doClose()
                collectgarbage()
            else -- We have threads running
                dlg:Hide()
                dlg.OnDownloadEnd(function() dlg:Close() end)
                dlg.AbortDownloads()
                evt:Veto()
            end
        end
    )

    -- Keep track of the size
    dlg:Connect(dlg:GetId(), wx.wxEVT_SIZE,
        function(evt)
            P.dlgsize = { dlg.Size.Width, dlg.Size.Height }
            evt:Skip()
        end
    )

    -- ------------------------------------------------------------------------
    -- Download starting and ending
    -- ------------------------------------------------------------------------

    -- These functions should be called from the downloadCtrls to notify the
    -- dialog when downloads start and finish.
    function dlg.NotifyDownloadStart(dl)
        --print("Download start "..dl.filename)
        table.insert(dlg.activedownloads, dl)
        dlg.panel:GetSizer():Layout()
        dlg.panel:FitInside()
        if not dlg.timer:IsRunning() then
            dlg.timer:Start(50)
            dlg.OnDownloadEnd(function() dlg.timer:Stop() end)
        end
    end

    function dlg.NotifyDownloadEnd(dl)
        for i, ctrl in ipairs(dlg.activedownloads) do
            if ctrl == dl then
                --print("Download end "..dl.filename)
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
    -- Do this in an idle event just in case there are windows and threads
    -- that have just been destroyed
    function dlg.docheckAllDownloadsCompleted()
        dlg:Disconnect(wx.wxEVT_IDLE)
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
    function dlg.checkAllDownloadsCompleted()
        dlg:Connect(wx.wxEVT_IDLE, dlg.docheckAllDownloadsCompleted)
    end
    
    -- Update the layout
    local state = P.filterState or {}
    P.layout.updateLayout(state.view, state.puzzle, state.date, state.weekday)

    return dlg
end

