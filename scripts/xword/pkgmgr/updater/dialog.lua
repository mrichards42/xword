require 'xword.pkgmgr.updater'
local P = xword.pkgmgr.updater
local join = require 'xword.pkgmgr.join'
require 'lfs'

function P.DownloadDialog(parent)
    local dlg = wx.wxMiniFrame(parent or wx.NULL, wx.wxID_ANY, "Updating...",
                               wx.wxDefaultPosition, wx.wxSize(300,250))

    local panel = wx.wxPanel(dlg, wx.wxID_ANY)

    -- Info messages
    local filename = wx.wxStaticText(panel, wx.wxID_ANY, "filename")
    local status = wx.wxStaticText(panel, wx.wxID_ANY, "dlnow / dltotal")
    local gauge = wx.wxGauge(panel, wx.wxID_ANY, 100)
    gauge.knownLength = false
    local overallGauge = wx.wxGauge(panel, wx.wxID_ANY, 0)
    local overallStatus = wx.wxStaticText(panel, wx.wxID_ANY, "0 / "..tostring(overallGauge:GetRange()))

    -- Error dialog
    dlg.err = false
    -- Successful downloads
    dlg.to_install = {}
    dlg.restart = false

    local function add_error(name, msg)
        -- Add the error to the error dialog list
        if not dlg.err then
            dlg.err = dlg.CreateErrorDialog()
        end
        local i = dlg.err.list:GetItemCount()
        dlg.err.list:InsertItem(i, name)
        dlg.err.list:SetItem(i, 1, msg)
    end
    -- ------------------------------------------------------------------------
    -- Download callbacks
    -- ------------------------------------------------------------------------
    function dlg.OnDownloadStart(fn)
        filename:SetLabel(fn)
        gauge:SetValue(0)
        gauge.knownLength = false
    end

    function dlg.OnDownloadEnd(success, name, err)
        gauge:SetValue(gauge:GetRange())
        overallGauge:SetValue(overallGauge:GetValue() + 1)
        overallStatus:SetLabel(
            string.format("%d / %d",
                overallGauge:GetValue(),
                overallGauge:GetRange()))
        if success then
            table.insert(dlg.to_install, name)
        else
            add_error(name, err)
        end
    end

    function dlg.OnDownloadProgress(dlnow, dltotal)
        if not gauge.knownLength then
            if dltotal > 0 then -- We know the content-length
                gauge.knownLength = true
                gauge:SetRange(dltotal)
                gauge:SetValue(dlnow)
                status:SetLabel(string.format("%d / %d", dlnow, dltotal))
            else -- Unknown download size
                gauge:Pulse()
            end
        else
            gauge:SetValue(dlnow)
            status:SetLabel(string.format("%d / %d", dlnow, dltotal))
        end
    end

    function dlg.OnDownloadTaskEnd()
        -- Install the packages
        -- This can't be done in a thread because install_package uses wx
        for _, data in ipairs(dlg.to_install) do
            local name, filename = unpack(data)
            local result, err = xword.pkgmgr.install_package(filename)
            if result then
                local enabled = xword.pkgmgr.load_enabled_packages()
                if enabled[result] ~= false then
                    xword.pkgmgr.load_package(result)
                end
                os.remove(filename)
            elseif err then
                add_error(name, err)
            elseif result == false then
                dlg.restart = true
            end
        end
        lfs.rmdir(join(xword.userdatadir, 'updates'))
        -- Show applicable dialogs
        if dlg.err then
            dlg.err.list:SetColumnWidth(0, wx.wxLIST_AUTOSIZE)
            dlg.err.list:SetColumnWidth(1, wx.wxLIST_AUTOSIZE)
            dlg.err:ShowModal()
        end
        if dlg.restart then
            xword.Message("Restart XWord to complete installation")
        end
        dlg:Close()
    end


    -- Create and return a custom error dialog
    function dlg.CreateErrorDialog()
        local err = wx.wxDialog(dlg, -1, "XWord Error",
                                wx.wxDefaultPosition, wx.wxDefaultSize,
                                wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER)

        local bmp = wx.wxArtProvider.GetBitmap(
            wx.wxART_ERROR, wx.wxART_MESSAGE_BOX, wx.wxDefaultSize
        )
        local text = wx.wxStaticText(err, -1, "Download error(s) occurred:")

        local list = wx.wxListCtrl(err, -1, wx.wxDefaultPosition,
                                   wx.wxDefaultSize, wx.wxLC_REPORT)

        list:InsertColumn(0, "Package")
        list:InsertColumn(1, "Message")
        err.list = list

        -- Layout
        local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
            local top = wx.wxBoxSizer(wx.wxHORIZONTAL)
            top:Add(wx.wxStaticBitmap(err, -1, bmp), 0, wx.wxALIGN_CENTER_VERTICAL)
            top:Add(text, 1, wx.wxALIGN_CENTER_VERTICAL + wx.wxLEFT, 10)
        sizer:Add(top, 0, wx.wxEXPAND + wx.wxALL, 10)
        sizer:Add(list, 1, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT, 10)
        sizer:Add(err:CreateSeparatedButtonSizer(wx.wxOK), 0, wx.wxEXPAND + wx.wxALL, 10)
        err:SetSizer(sizer)

        return err
    end

    -- ------------------------------------------------------------------------
    -- Do the update
    -- ------------------------------------------------------------------------
    function dlg:DoUpdate(downloads)
        overallGauge:SetRange(#downloads)
        local t = task.new('xword.pkgmgr.updater.download_task')
        t:connect{
            [P.DL_START]    = dlg.OnDownloadStart,
            [P.DL_END]      = dlg.OnDownloadEnd,
            [P.DL_PROGRESS] = dlg.OnDownloadProgress,
            [task.EVT_END]  = dlg.OnDownloadTaskEnd,
        }
        t:start(unpack(downloads))
    end

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        sizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Current Progress:"), 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(filename, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(gauge, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(status, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Total Progress:"), 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(overallGauge, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(overallStatus, 0, wx.wxEXPAND + wx.wxALL, 5)
    panel:SetSizer(sizer)

    dlg:Center()

    return dlg
end
