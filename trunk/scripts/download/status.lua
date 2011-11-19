-- A status bar sort of thing
require 'download.download'
local PopupWindow = require 'download.popup'

local function make_spacer(parent)
    local spacer = wx.wxWindow(
        parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(1, -1))
    spacer:SetBackgroundColour(wx.wxColour(192, 192, 192))
    return spacer
end

local function make_queue_popup(parent)
    local popup = PopupWindow(parent, wx.wxID_ANY)
    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    popup:SetSizer(border)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    local ctrls = {}

    function popup:update()
        popup:Freeze()
        for _, ctrl in ipairs(ctrls) do
            sizer:Detach(ctrl)
            ctrl:Destroy()
        end
        ctrls = {}
        --[[
        for _, data in ipairs(download.queue) do
            local ctrl = wx.wxStaticText(popup, wx.wxID_ANY, tostring(data[1]))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
        end
        ]]
        local text = {}
        for _, data in ipairs(download.queue) do
            table.insert(text, data[1])
        end
        local ctrl = wx.wxStaticText(popup, wx.wxID_ANY, table.concat(text, '\n'))
        table.insert(ctrls, ctrl)
        sizer:Add(ctrl)
        sizer:Layout()
        popup:Fit()
        popup:Thaw()
    end

    popup:update()

    popup:Fit()
    return popup
end

local function make_error_popup(parent)
    local popup = PopupWindow(parent, wx.wxID_ANY)
    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    popup:SetSizer(border)

    local sizer = wx.wxFlexGridSizer(0, 2, 5, 5)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    local ctrls = {}

    function popup:update()
        popup:Freeze()
        for _, ctrl in ipairs(ctrls) do
            sizer:Detach(ctrl)
            ctrl:Destroy()
        end
        ctrls = {}
        for i=#download.errors,math.max(#download.errors-9, 1),-1 do
            local data = download.errors[i]
            local ctrl
            ctrl = wx.wxStaticText(popup, wx.wxID_ANY, tostring(data[1]))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
            ctrl = wx.wxStaticText(popup, wx.wxID_ANY, tostring(data[3]))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
        end
        local extra = #download.errors - 10
        if extra > 0 then
            ctrl = wx.wxStaticText(popup, wx.wxID_ANY, string.format("(%d more)", extra))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
        end
        sizer:Layout()
        popup:Fit()
        popup:Thaw()
    end

    popup:update()

    popup:Fit()
    return popup
end


local function Status(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    panel:SetSizer(sizer)

    panel.current = wx.wxStaticText(panel, wx.wxID_ANY, 'Ready')
    sizer:Add(panel.current, 5, wx.wxEXPAND + wx.wxALL, 5)

    sizer:Add(make_spacer(panel), 0, wx.wxEXPAND)

    panel.queue = wx.wxStaticText(panel, wx.wxID_ANY, '0 Queued')
    panel.queue.WindowStyle = wx.wxALIGN_RIGHT
    sizer:Add(panel.queue, 1, wx.wxEXPAND + wx.wxALL, 5)

    sizer:Add(make_spacer(panel), 0, wx.wxEXPAND)

    panel.errors = wx.wxStaticText(panel, wx.wxID_ANY, '(No Errors)')
    panel.errors.WindowStyle = wx.wxALIGN_RIGHT
    sizer:Add(panel.errors, 1, wx.wxEXPAND + wx.wxALL, 5)


    local error_popup
    panel.errors:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            if #download.errors > 0 then
                error_popup = make_error_popup(panel)
                error_popup:Popup()
            end
        end)

    panel.errors:Connect(wx.wxEVT_LEAVE_WINDOW,
        function (evt)
            if error_popup then
                error_popup:Destroy()
                error_popup = nil
            end
        end)

    local queue_popup
    panel.queue:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            if #download.queue > 0 then
                queue_popup = make_queue_popup(panel)
                queue_popup:Popup()
            end
        end)

    panel.queue:Connect(wx.wxEVT_LEAVE_WINDOW,
        function (evt)
            if queue_popup then
                queue_popup:Destroy()
                queue_popup = nil
            end
        end)

    function panel:update_status()
        self.current.Label = download.current or "Ready"
        self.queue.Label = string.format('%d Queued', #download.queue)
        self.errors.Label = string.format('%d Errors', #download.errors)
        if queue_popup then queue_popup:update() end
        if error_popup then error_popup:update() end
    end

    panel:update_status()

    return panel
end

return Status
