-- A status bar sort of thing
require 'download.download'
local PopupWindow = require 'download.popup'
local BmpButton = require 'download.bmp_button'
local bmp = require 'download.bmp'
local basename = require 'pl.path'.basename
local clear = require 'pl.tablex'.clear


local function abbrev(url, length)
    return select(1, url:gsub('^(http[^:]*://[^/]+/).-(/[^/]+)$', '%1...%2'))
end

local function make_spacer(parent)
    local spacer = wx.wxWindow(
        parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(1, -1))
    spacer:SetBackgroundColour(wx.wxColour(192, 192, 192))
    return spacer
end

local function display_text(puzzle)
    return puzzle.name .. ', ' .. puzzle.date:fmt('%a, %b %d, %Y')
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
        clear(ctrls)
        for i=1,math.min(#download.queue, 10) do
            local ctrl = wx.wxStaticText(popup, wx.wxID_ANY, display_text(download.queue[i]))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
        end
        local extra = #download.queue - 10
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
        clear(ctrls)
        for i=#download.errors,math.max(#download.errors-9, 1),-1 do
            local data = download.errors[i]
            local ctrl
            ctrl = wx.wxStaticText(popup, wx.wxID_ANY, tostring(basename(data[1].filename)))
            table.insert(ctrls, ctrl)
            sizer:Add(ctrl)
            ctrl = wx.wxStaticText(popup, wx.wxID_ANY, tostring(data[2]))
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

-- A ctrl that truncates the url based on size
local function CurrentText(parent)
    -- Make this not autoresize so that adjust_label works right
    local ctrl = wx.wxStaticText(parent, wx.wxID_ANY, 'Ready',
        wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxST_NO_AUTORESIZE)
    ctrl.url = 'Ready'

    local setlabel = ctrl.SetLabel

    function adjust_label(url)
        local max_width = ctrl.Size.Width
        local width, _ = ctrl:GetTextExtent(url)
        -- Shortcut?
        if width <= max_width then
            setlabel(ctrl, url)
            return
        end
        -- Remove the protocol
        local trimmed = url:match('^http[^:]*://(.+)$')
        if not trimmed then trimmed = url end
        -- Shortcut again?
        local width, _ = ctrl:GetTextExtent(trimmed)
        if width <= max_width then
            setlabel(ctrl, trimmed)
            return
        end
        -- Split by '/', then reassemble
        local parts = {}
        for part in trimmed:gmatch('[^/]+') do table.insert(parts, part) end
        if #parts > 2 then
            table.insert(parts, 2, '...')
            while width > max_width and #parts > 3 do
                table.remove(parts, 3)
                local label = table.concat(parts, '/')
                width, _ = ctrl:GetTextExtent(label)
                if width <= max_width then
                    setlabel(ctrl, label)
                    return
                end
            end
        end
        setlabel(ctrl, table.concat(parts, '/'))
    end

    ctrl.SetLabel = function(ctrl, label)
        ctrl.url = label
        adjust_label(ctrl.url)
    end

    ctrl:Connect(wx.wxEVT_SIZE, function (evt)
        adjust_label(ctrl.url)
        evt:Skip()
    end)
    return ctrl
end


local function Status(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    panel:SetSizer(sizer)

    panel.current = CurrentText(panel)
    sizer:Add(panel.current, 7, wx.wxEXPAND + wx.wxALL, 5)

    sizer:Add(make_spacer(panel), 0, wx.wxEXPAND)

    panel.queue = wx.wxStaticText(panel, wx.wxID_ANY, '0 Queued')
    panel.queue.WindowStyle = wx.wxALIGN_RIGHT
    sizer:Add(panel.queue, 0, wx.wxEXPAND + wx.wxALL, 5)

    sizer:Add(make_spacer(panel), 0, wx.wxEXPAND)

    panel.errors = wx.wxStaticText(panel, wx.wxID_ANY, '(No Errors)')
    panel.errors.WindowStyle = wx.wxALIGN_RIGHT
    sizer:Add(panel.errors, 0, wx.wxEXPAND + wx.wxALL, 5)

    sizer:Add(make_spacer(panel), 0, wx.wxEXPAND)

    panel.clear = BmpButton(panel, wx.wxID_ANY, bmp.x)
    panel.clear:SetToolTip("Cancel downloads")
    sizer:Add(panel.clear, 0, wx.wxALL + wx.wxALIGN_CENTER, 5)

    panel.clear:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        download.clear_downloads()
    end)

    panel.errors:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            if #download.errors > 0 then
                make_error_popup(panel):Popup()
            end
        end)

    panel.queue:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            if #download.queue > 0 then
                make_queue_popup(panel):Popup()
            end
        end)

    function panel:update_status()
        if download.current then
            self.current:SetLabel(display_text(download.current))
        else
            self.current:SetLabel("Ready")
        end
        self.queue.Label = string.format('%d Queued', #download.queue)
        self.errors.Label = string.format('%d Errors', #download.errors)
        self:Layout()
        if queue_popup then queue_popup:update() end
        if error_popup then error_popup:update() end
    end

    panel:update_status()

    return panel
end

return Status
