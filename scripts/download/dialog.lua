-- ============================================================================
-- The main download dialog
--     The dialog maintains a table of currently active downloads.  Before
--     application exit, the dialog terminates all active download threads.
-- ============================================================================


require 'mtask'
require 'download.panel'
require 'date'

assert(download, 'Must load download package first')

-- Integration with 'import' plugin for dlg.downloadAndOpen
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
    --if P.dlg then return P.dlg end

    local dlg = wx.wxDialog(
        xword.GetFrame(), wx.wxID_ANY, "Puzzle Download",
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )

    -- Controls
    dlg.scroller = wx.wxScrolledWindow(dlg, wx.wxID_ANY)
    dlg.panel = wx.wxPanel(dlg.scroller, wx.wxID_ANY)
    dlg.scroller:SetScrollRate(10,10)

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local panelSizer = wx.wxBoxSizer(wx.wxVERTICAL)
            panelSizer:Add(dlg.panel, 1, wx.wxEXPAND + wx.wxALL, 10)
        dlg.scroller:SetSizer(panelSizer)
        sizer:Add(dlg.scroller, 1, wx.wxEXPAND, 0)
    dlg:SetSizer(sizer)

    -- downloads owned by the dialog
    dlg.downloads = {}
    dlg.pendingdownloads = {}
    dlg.activedownloads = {}

    -- ------------------------------------------------------------------------
    -- Closing: Make sure no download threads are running
    -- ------------------------------------------------------------------------

    -- Hijack the xword Frame's close method so we can clean up our threads
    dlg.isClosing = false
    dlg.isFrameClosing = false
    local frame = xword.GetFrame()
    local function frameClose(evt)
        -- We can't use Disconnect to remove a single function (in wxLua)
        -- apparently, so just make the event handler irrelevant after the dlg
        -- is closed.
        if not dlg then evt:Skip() return end

        -- Let the frame's event handling pass through if we have already
        -- killed all the threads
        if #dlg.activedownloads == 0 then
            evt:Skip()
        else
            -- We still have threads; kill them before we let the frame close.
            wx.wxBusyInfo('Ending downloads . . . please wait', frame)
            dlg.isFrameClosing = true
            dlg:Close()
            evt:Veto()
        end
    end
    frame:Connect(wx.wxEVT_CLOSE_WINDOW, frameClose)

    dlg.closeThreads = function()
        for _, dl in pairs(dlg.activedownloads) do
            -- Kill the thread
            task.post(dl.task_id, 'abort', P.DL_ABORT)
        end
    end

    -- Make sure the threads are killed before we close the dialog. We will
    -- call dlg:Close() again once the threads are killed.
    dlg:Connect(dlg:GetId(), wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            if dlg.isClosing or #dlg.activedownloads == 0 then
                dlg:Disconnect(wx.wxEVT_IDLE)
                dlg:Destroy()
                -- Make this nil so the frame close evt handler knows the
                -- dlg is destroyed.
                dlg = nil
            else -- We have threads running
                dlg:Hide()
                dlg.isClosing = true
                dlg.closeThreads()
                evt:Veto()
            end
        end)


    -- ------------------------------------------------------------------------
    -- Idle Event: Cleanup completed downloads
    -- ------------------------------------------------------------------------
    dlg:Connect(wx.wxEVT_IDLE,
        function(evt)
            -- Find newly active downloads
            local active = {}
            for i, dl in ipairs(dlg.pendingdownloads) do
                if dl.task_id then
                    table.insert(active, i)
                end
            end
            -- The layout may have changed if we have a new download
            if #active > 0 then
                print('new layout')
                dlg.panel:GetSizer():Layout()
                dlg.panel:FitInside()
            end
            -- Add new downloads to the activedownloads table
            for _, i in ipairs(active) do
                table.insert(dlg.activedownloads, dlg.pendingdownloads[i])
                table.remove(dlg.pendingdownloads, i)
            end

            -- Clean up any completed / aborted download threads

            -- Don't remove the threads from the table we're iterating over.
            -- Add them to a cleanup table to process afterward.
            local cleanup = {}
            for i, dl in ipairs(dlg.activedownloads) do
                if not task.isrunning(dl.task_id) then
                    table.insert(cleanup, i)
                end
            end

            -- Do the actual cleanup
            for _, i in ipairs(cleanup) do
                table.remove(dlg.activedownloads, i)
            end

            -- Check to see if we're closing the dialog
            if dlg.isClosing and #dlg.activedownloads == 0 then
                if dlg.isFrameClosing then
                    frame:Close()
                else -- frame:Close() should also close the dialog
                    dlg:Close()
                end
            end
        end)


    -- ------------------------------------------------------------------------
    -- Initialize the dialog with a table of downloadable puzzles
    -- ------------------------------------------------------------------------
    local font = wx.wxFont(12, wx.wxFONTFAMILY_SWISS,
                           wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL)
    local boldfont = wx.wxFont(12, wx.wxFONTFAMILY_SWISS,
                               wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_BOLD)


    local sizer = wx.wxGridBagSizer(5, 5)

    local ctrl = wx.wxStaticText(dlg.panel, wx.wxID_ANY, 'Puzzle')
    ctrl:SetFont(font)
    sizer:Add(ctrl, wx.wxGBPosition(0,0), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

    -- Day headers (start with today's date)
    local d = date() -- today
    for i=1,7 do
        local ctrl = wx.wxStaticText(dlg.panel, wx.wxID_ANY, d:fmt(download.headerformat))
        -- Make today bold
        if i==1 then
            ctrl:SetFont(boldfont)
        else
            ctrl:SetFont(font)
        end
        sizer:Add(ctrl, wx.wxGBPosition(i,0), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)
        -- Previous day
        d:adddays(-1)
    end

    -- Add the sources to the grid
    for srcNum, source in pairs(download.sources) do
        local ctrl = wx.wxStaticText(dlg.panel, wx.wxID_ANY, source.display)
        ctrl:SetFont(font)
        sizer:Add(ctrl, wx.wxGBPosition(0, srcNum), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)

        local d = date() -- today
        for i=1,7 do
            if source.days[d:getisoweekday()] then
                local url = d:fmt(source.url)
                local filename = download.localfolder..'\\'..d:fmt(source.filename)

                -- Create the download ctrl
                local dl = P.newDownload(dlg.panel, url, filename)
                sizer:Add(dl.panel, wx.wxGBPosition(i,srcNum), wx.wxGBSpan(1,1), wx.wxALIGN_CENTER)
                -- Add to the downloads table
                table.insert(dlg.downloads, dl)
                table.insert(dlg.pendingdownloads, dl)
            end
            -- Previous day
            d:adddays(-1)
        end
    end

    dlg.panel:SetSizer(sizer)
    dlg.scroller:FitInside()

    download.dlg = dlg
    return dlg
end
