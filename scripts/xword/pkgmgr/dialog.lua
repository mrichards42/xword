require 'xword.pkgmgr.panel'

local P = xword.pkgmgr

function P.PackageDialog()
    local dlg = wx.wxDialog(xword.frame or wx.NULL, wx.wxID_ANY, "Installed Packages",
                            wx.wxDefaultPosition, wx.wxSize(450,350),
                            wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER)

    -- The package list
    local packages = P.PackagePanel(
        dlg,
        { {"Name"}, {"Kind"}, {"Version", align = wx.wxLIST_FORMAT_RIGHT}, {"Depends", "requires"} }
    )
    packages.list:SetDefaultCheck(false)

    -- Make an array of packages from the packages table
    local data = {}
    local pkgtable = P.GetPackagesTable()
    if pkgtable then
        for _, t in ipairs({"program", "scripts", "lualibs"}) do
            for _, pkg in ipairs(pkgtable[t] or {}) do
                table.insert(data, pkg)
            end
        end
    end
    packages:SetData(data)

    -- The buttons
    local buttons = wx.wxStdDialogButtonSizer()
    buttons:SetCancelButton(wx.wxButton(dlg, wx.wxID_CANCEL, "Close"))
    if updater then
        local checkbutton = wx.wxButton(dlg, wx.wxID_ANY, "Check for updates")
        checkbutton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
            updater.Update()
            evt:Skip()
        end)
        buttons:Add(checkbutton, 0)
    end
    buttons:Realize()

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    sizer:Add(packages, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:Add(buttons, 0, wx.wxEXPAND + wx.wxALL, 5)
    dlg:SetSizer(sizer)

    dlg:Center()
    return dlg
end
