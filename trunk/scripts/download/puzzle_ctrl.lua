require 'lfs'
local join = require 'pl.path'.join
local PopupWindow = require 'download.popup'
local TextButton = require 'download.text_button'
require 'download.stats'
require 'download.download'

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

local function make_popup(parent, puzzle)
    local popup = PopupWindow(parent, wx.wxID_ANY)
    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    popup:SetSizer(border)
    
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    -- Load the puzzle and display various puzzle information
    local success, p = pcall(puz.Puzzle, puzzle.filename)
    local bmp
    if success then
        bmp = draw_puzzle(p)
        local title_label = p.Title
        if #p.Notes > 0 then
            title_label = title_label .. " [Notes]"
        end
        local title = wx.wxStaticText(popup, wx.wxID_ANY, title_label)
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
        sizer:Add(wx.wxStaticText(popup, wx.wxID_ANY, tostring(puzzle.url)))
    end

    popup:Fit()

    local destroy = popup.Destroy
    function popup:Destroy()
        if bmp then
            bmp:delete()
        end
        destroy(popup)
    end

    return popup
end

local function PuzzleCtrl(parent, text, puzzle)
    local ctrl = TextButton(parent, wx.wxID_ANY, text)
    ctrl.filename = filename
    ctrl.puzzle = puzzle

    -- Show popup on hover
    ctrl:Connect(wx.wxEVT_ENTER_WINDOW, function (evt)
        make_popup(ctrl, puzzle):Popup()
        evt:Skip()
    end)

    function ctrl:set_status(status)
        if status == download.MISSING then
            self.ForegroundColour = download.styles.missing.color
            self.Font = download.styles.missing.font
        elseif status == download.EXISTS then
            self.ForegroundColour = download.styles.downloaded.color
            self.Font = download.styles.downloaded.font
        elseif status == download.SOLVING then
            self.ForegroundColour = download.styles.progress.color
            self.Font = download.styles.progress.font
        elseif status == download.COMPLETE then
            self.ForegroundColour = download.styles.complete.color
            self.Font = download.styles.complete.font
        end
        self:Refresh()
    end

    -- Open the puzzle
    function ctrl:open_puzzle()
        if download.puzzle_exists(puzzle.filename) then
            xword.frame:LoadPuzzle(puzzle.filename)
        else
            download.add_download(puzzle, download.PREPEND)
            download.open_after_download = puzzle.filename
        end
    end
    ctrl:Connect(wx.wxEVT_LEFT_DOWN, function (evt)
        ctrl:open_puzzle()
    end)

    -- Context menu
    local function copy_text(text)
        -- Copy text to the clipboard
        local clipBoard = wx.wxClipboard.Get()
        if clipBoard and clipBoard:Open() then
            clipBoard:SetData(wx.wxTextDataObject(text))
            clipBoard:Flush() -- Make this available after we've exited
            clipBoard:Close()
        end
    end

    ctrl:Connect(wx.wxEVT_CONTEXT_MENU,
        function(evt)
            local menu = wx.wxMenu()
            local item
            item = menu:Append(wx.wxID_ANY, "Copy URL")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) copy_text(tostring(puzzle.url)) end)
            item = menu:Append(wx.wxID_ANY, "Copy local filename")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) copy_text(puzzle.filename) end)
            item = menu:Append(wx.wxID_ANY, "Redownload")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) download.add_download(puzzle) end)
            ctrl:PopupMenu(menu)
            menu:delete()
        end)

    -- Keep track of this ctrl
    download.add_ctrl(ctrl)

    return ctrl
end

return PuzzleCtrl
