-- This should only be required as part of download.layout
local P = download
local layout = P.layout

local function layoutByPuzzle()
    layout.createDownloadCtrls()
    local dlg = assert(P.dlg)
    local panel = dlg.panel

    local today = layout.today()

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    for _, source in ipairs(P.sources) do
        sizer:Add(layout.Header(panel, source.display),
                  0, wx.wxEXPAND + wx.wxALL, 5)

        local dlSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)

        -- Add the actual download controls to the grid
        for date_tbl, dl in pairs(dlg.downloads[source.display]) do
            local d = date(unpack(date_tbl))
            local datesizer = wx.wxBoxSizer(wx.wxVERTICAL)
                datesizer:Add(layout.Header(panel, d:fmt(P.dateformat), d == today),
                              0, wx.wxEXPAND + wx.wxALL, 5)
                datesizer:Add(dl.ctrl, 0, wx.wxEXPAND + wx.wxALL, 5)
            dlSizer:Add(datesizer, 0, wx.wxEXPAND + wx.wxALL, 5)
        end

        sizer:Add(dlSizer, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(wx.wxStaticLine(panel, wx.wxID_ANY),
                  0, wx.wxEXPAND + wx.wxALL, 5)
    end
    dlg.panel:SetSizerAndFit(sizer)
end

layout.addLayout("By Puzzle", layoutByPuzzle)
