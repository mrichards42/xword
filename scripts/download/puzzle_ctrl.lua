require 'lfs'
local PopupWindow = require 'download.popup'
require 'download.stats'

local box_size = 14
local font_size = 8
local function draw_puzzle(p)
    local bmp = wx.wxBitmap(p.Grid.Width * (box_size + 1) + 1, p.Grid.Height * (box_size + 1) + 1)
    local dc = wx.wxMemoryDC()
    dc:SelectObject(bmp)
    dc:SetBackground(wx.wxBLACK_BRUSH)
    dc:SetFont(wx.wxFont(font_size, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL))
    dc:Clear()
    dc:SetBrush(wx.wxWHITE_BRUSH)
    dc:SetPen(wx.wxTRANSPARENT_PEN)
    local s = p.Grid:First()
    while s do
        if s:IsWhite() then
            local x = (s.Col-1) * (box_size + 1) + 1
            local y = (s.Row-1) * (box_size + 1) + 1
            if #s.Text > 1 then
                dc:SetBrush(wx.wxBrush("#FAA07A", wx.wxSOLID))
            elseif s:HasColor() then
                dc:SetBrush(wx.wxBrush(s:GetHtmlColor(), wx.wxSOLID))
            else
                dc:SetBrush(wx.wxWHITE_BRUSH)
            end
            dc:DrawRectangle(x, y, box_size, box_size)
            if s:HasCircle() then
                dc:SetPen(wx.wxBLACK_PEN)
                dc:DrawCircle(x + box_size / 2, y + box_size / 2, box_size / 2 + 1)
                dc:SetPen(wx.wxTRANSPARENT_PEN)
            end
            if #s.Text > 0 then
                dc:DrawLabel(s.Text:sub(1,1), wx.wxRect(x, y, box_size, box_size), wx.wxALIGN_CENTER)
            end
        end
        s = s:Next()
    end
    dc:delete()
    return bmp
end

local function make_popup(parent, filename, url)
    local popup = PopupWindow(parent, wx.wxID_ANY, wx.wxPoint(0, 0))
    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    popup:SetSizer(border)
    
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    -- Load the puzzle and display various puzzle information
    local success, p = pcall(puz.Puzzle, filename)
    local bmp
    if success then
        bmp = draw_puzzle(p)
        local title = wx.wxStaticText(popup, wx.wxID_ANY, p.Title)
        local author = wx.wxStaticText(popup, wx.wxID_ANY, p.Author)
        -- If title / author is going to fit on one line, make it
        if title.Size.Width + 5 + author.Size.Width < bmp.Width then
            local heading = wx.wxBoxSizer(wx.wxHORIZONTAL)
            heading:Add(title, 1, wx.wxEXPAND + wx.wxRIGHT, 5)
            heading:Add(author, 0, wx.wxEXPAND)
            sizer:Add(heading, 0, wx.wxEXPAND)
        else
            sizer:Add(title)
            sizer:Add(author)
        end
        if p.Time > 0 then
            local time
            if p.Time > 60*60 then
                time = date(p.Time):fmt("%H:%M:%S")
            else
                time = date(p.Time):fmt("%M:%S")
            end
            sizer:Add(wx.wxStaticText(popup, wx.wxID_ANY, time))
        end
        sizer:Add(wx.wxStaticBitmap(popup, wx.wxID_ANY, bmp), 0, wx.wxTOP, 5)
        p:__gc()
    else
        sizer:Add(wx.wxStaticText(popup, wx.wxID_ANY, url))
    end

    popup:Fit()

    local destroy = popup.Destroy
    function popup:Destroy()
        if bmp then
            bmp:delete()
        end
        destroy(popup)
    end

    -- Move the popup off of the cursor, and make the popup fit on the screen
    local screenx = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_X)
    local screeny = wx.wxSystemSettings.GetMetric(wx.wxSYS_SCREEN_Y)
    local size = popup.Size
    local pos = popup.Position
    local offsetx, offsety = 20, 20
    -- Check right edge
    if pos.X + size.Width + 30 > screenx then
        offsetx = -size.Width
    end
    -- Check bottom edge
    if pos.Y + size.Height + 30 > screeny then
        offsety = -size.Height
    end
    popup:Move(pos.X + offsetx, pos.Y + offsety)

    return popup
end

local function PuzzleCtrl(parent, text, url, filename)
    local ctrl = wx.wxStaticText(parent, wx.wxID_ANY, text)
    ctrl.filename = filename
    ctrl.url = url

    ctrl:SetCursor(wx.wxCursor(wx.wxCURSOR_HAND))

    local popup

    -- Show underline on hover
    ctrl:Connect(wx.wxEVT_ENTER_WINDOW,
        function (evt)
            local font = ctrl.Font
            font:SetUnderlined(true)
            ctrl.Font = font
            popup = make_popup(ctrl, filename, url)
            popup:Show()
        end)

    ctrl:Connect(wx.wxEVT_LEAVE_WINDOW,
        function (evt)
            local font = ctrl.Font
            font:SetUnderlined(false)
            ctrl.Font = font
            if popup then
                popup:Destroy()
                popup = nil
            end
        end)

    -- Open the puzzle
    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function (evt)
            if lfs.attributes(filename, 'mode') then
                xword.frame:LoadPuzzle(filename)
            end
        end)

    -- Keep track of this ctrl
    download.add_ctrl(ctrl)

    return ctrl
end

return PuzzleCtrl
