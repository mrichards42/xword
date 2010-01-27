-- ============================================================================
-- The main download dialog
--     The dialog maintains a table of currently active downloads.  Before
--     application exit, the dialog terminates all active download threads.
-- ============================================================================


require 'mtask'
require 'download.ctrl'
require 'download.layout'
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
    local setLayout
    local layoutGrid
    local layoutByPuzzle
    -- Only allow one download dialog per session
    if P.dlg then return P.dlg end

    local dlg = wx.wxFrame(
        xword.frame, wx.wxID_ANY, "Puzzle Download",
        wx.wxDefaultPosition, wx.wxDefaultSize
        --wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )
    P.dlg = dlg

    -- Controls
    dlg.scroller = wx.wxScrolledWindow(dlg, wx.wxID_ANY)
    dlg.panel = wx.wxPanel(dlg.scroller, wx.wxID_ANY)
    dlg.scroller:SetScrollRate(10,10)
    dlg.scroller:Scroll(0,0)

    -- Menu
    local menubar = wx.wxMenuBar()
        menubar:Append(P.layout.createMenu(), "View")
    dlg:SetMenuBar(menubar)


    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local panelSizer = wx.wxBoxSizer(wx.wxVERTICAL)
            panelSizer:Add(dlg.panel, 1, wx.wxEXPAND + wx.wxALL, 10)
        dlg.scroller:SetSizer(panelSizer)
        sizer:Add(dlg.scroller, 1, wx.wxEXPAND, 0)
    dlg:SetSizerAndFit(sizer)



    dlg.downloads = {}
    dlg.activedownloads = {}
    dlg.pendingdownloads = {}



    -- ------------------------------------------------------------------------
    -- Layouts
    -- ------------------------------------------------------------------------
    P.layout.setLayout('Grid')



    -- ------------------------------------------------------------------------
    -- Closing: Make sure no download threads are running
    -- ------------------------------------------------------------------------

    function dlg.AbortDownloads()
        for _, dl in ipairs(dlg.activedownloads) do
            -- Kill the thread
            task.post(dl.task_id, 'blah', P.DL_ABORT)
        end
    end

    -- Hijack the xword Frame's close method so we can clean up our threads
    local frame = xword.GetFrame()
    frame:Connect(wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            -- We can't use Disconnect to remove a single function (in wxLua)
            -- so just make this event handler irrelevant after the dlg is
            -- closed.
            if not dlg then evt:Skip() return end

            -- Let the frame's event handling pass through if we have already
            -- killed all the threads
            if #dlg.activedownloads == 0 then
                evt:Skip()
            else
                -- We still have threads;
                -- kill them before we let the frame close.
                wx.wxBusyInfo('Ending downloads . . . please wait', frame)
                dlg.AbortDownloads()
                dlg.OnDownloadEnd(function() dlg:Close() frame:Close() end)
                evt:Veto()
            end
        end
    )

    -- Make sure the threads are killed before we close the dialog. We will
    -- call dlg:Close() again once the threads are killed.
    dlg:Connect(dlg:GetId(), wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            if #dlg.activedownloads == 0 then
                print 'destroying dialog'
                dlg:Disconnect(wx.wxEVT_IDLE)
                dlg:Destroy()
                -- Make this nil so the frame close evt handler knows the
                -- dlg is destroyed.
                dlg = nil
                P.dlg = nil
            else -- We have threads running
                print 'threads are running . . . aborting them'
                dlg:Hide()
                dlg.AbortDownloads()
                dlg.OnDownloadEnd(function() dlg:Close() end)
                evt:Veto()
            end
        end
    )


    -- ------------------------------------------------------------------------
    -- Idle Event:
    --     Cleanup completed downloads
    --     Execute events on completion of all downloads
    -- ------------------------------------------------------------------------

    dlg.completion_events = {}
    function dlg.OnDownloadEnd(func)
        table.insert(dlg.completion_events, func)
    end

    dlg:Connect(wx.wxEVT_IDLE,
        function(evt)
            -- Find newly active downloads
            local active = {}
            for i, dl in ipairs(dlg.pendingdownloads) do
                if dl.task_id then
                    table.insert(active, i)
                end
            end
            -- Reverse the order of the active table so that we remove downloads
            -- from the end of dlg.pendingdownloads instead of the beginning.
            table.sort(active, function(a,b) return a>b end)
            -- Add new downloads to the activedownloads table
            for _, i in ipairs(active) do
                table.insert(dlg.activedownloads, dlg.pendingdownloads[i])
                table.remove(dlg.pendingdownloads, i)
            end

            -- The layout may have changed if we have a new download
            if #active > 0 then
                dlg.panel:GetSizer():Layout()
                dlg.panel:FitInside()
            end

            -- Clean up any completed / aborted download threads

            -- Don't remove the threads from the table we're iterating over.
            -- Add them to a cleanup table to process afterward.
            local cleanup = {}
            for i, dl in ipairs(dlg.activedownloads) do
                if not dl.isrunning() then
                    table.insert(cleanup, i)
                end
            end

            -- Do the actual cleanup
            for _, i in ipairs(cleanup) do
                table.remove(dlg.activedownloads, i)
            end

            -- Look for download completion events
            if #dlg.activedownloads == 0 then
                -- It's possible that the dialog might be destroyed after
                -- calling func(), so do a check before we continue.
                while dlg do
                    local func = table.remove(dlg.completion_events)
                    if not func then break end
                    func()
                end
            end
        end
    )

    dlg:Fit()
    return dlg
end
