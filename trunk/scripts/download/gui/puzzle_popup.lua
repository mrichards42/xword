local puz = require 'luapuz'
local date = require 'date'

local _R = mod_path(...)
local _RR = mod_path(..., 2)
local PopupWindow = require(_R .. 'popup')
local stats = require(_RR .. 'stats')

-- draw_puzzle globals
local box_size = 13
local font_size = 8
local rebus_color = "#FAA07A"
-- Draw a puzzle to a wxBitmap
local function draw_puzzle(p)
    -- Setup DC/bitmap
    local bmp = wx.wxBitmap(p.Grid.Width * (box_size + 1) + 1, p.Grid.Height * (box_size + 1) + 1)
    local dc = wx.wxMemoryDC()
    dc:SelectObject(bmp)
    dc:SetBackground(wx.wxBLACK_BRUSH)
    dc:SetFont(wx.wxFont(font_size, wx.wxFONTFAMILY_SWISS, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL))
    dc:Clear()
    dc:SetBrush(wx.wxWHITE_BRUSH)
    dc:SetPen(wx.wxTRANSPARENT_PEN)
    -- Draw grid
    local s = p.Grid:First()
    while s do
        if s:IsWhite() then
            local text = s:GetText()
            -- Set background color
            if #text > 1 then
                local brush = wx.wxBrush(rebus_color, wx.wxSOLID)
                dc:SetBrush(brush)
                brush:delete()
            elseif s:HasColor() then
                local brush = wx.wxBrush(s:GetHtmlColor(), wx.wxSOLID)
                dc:SetBrush(brush)
                brush:delete()
            else
                dc:SetBrush(wx.wxWHITE_BRUSH)
            end
            -- Draw square (and circle if present)
            local x = (s.Col-1) * (box_size + 1)
            local y = (s.Row-1) * (box_size + 1)
            dc:DrawRectangle(x+1, y+1, box_size, box_size) -- Leave a border
            if s:HasCircle() then
                dc:SetPen(wx.wxBLACK_PEN)
                dc:DrawEllipse(x, y, box_size+1, box_size+1)
                dc:SetPen(wx.wxTRANSPARENT_PEN)
            end
            -- Draw text (just first letter if rebus)
            if #text > 0 then
                dc:DrawLabel(text:sub(1,1), wx.wxRect(x, y, box_size+1, box_size+1), wx.wxALIGN_CENTER)
            end
        end
        s = s:Next()
    end
    dc:delete()
    return bmp
end

local function PuzzlePopup(parent, puzzle)
    local self = PopupWindow(parent)
    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    self:SetSizer(border)
    
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    -- Load the puzzle and display info
    local bmp
    function self:set_puzzle(puzzle)
        if bmp then bmp:delete() end
        bmp = nil
        sizer:Clear(true) -- Delete all the windows
        if not puzzle then return end
        local success, p = pcall(puz.Puzzle, puzzle.filename)
        if success then
            bmp = draw_puzzle(p)
            local title_label = p.Title
            if #p.Notes > 0 then
                title_label = title_label .. " [Notes]"
            end
            local title = wx.wxStaticText(self, wx.wxID_ANY, title_label)
            local author = wx.wxStaticText(self, wx.wxID_ANY, p.Author)
            -- If title / author will fit on one line, make it
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
                sizer:Add(wx.wxStaticText(self, wx.wxID_ANY, time))
            end
            sizer:Add(wx.wxStaticBitmap(self, wx.wxID_ANY, bmp), 0, wx.wxTOP, 5)
            p:__gc()
        elseif puzzle:exists() then
            sizer:Add(wx.wxStaticText(self, wx.wxID_ANY, tostring(puzzle.filename)))
        else
            local msg = tostring(puzzle.url)
            local err = stats.error[puzzle.filename]
            if err then
                msg = msg .. '\n' .. err
            end
            local st = wx.wxStaticText(self, wx.wxID_ANY, msg)
            st:Wrap(250)
            sizer:Add(st)
        end
        self:Layout()
        self:Fit()
        --self:Refresh()
    end

    self:set_puzzle(puzzle)

    -- Make sure we delete the bitmap when PopupWindow destroys itself
    self:Connect(wx.wxEVT_DESTROY, function(evt)
        if bmp then bmp:delete() end
        evt:Skip()
    end)

    return self
end

return PuzzlePopup
