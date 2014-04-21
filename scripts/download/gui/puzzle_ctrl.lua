-- Current directory
local _R = (string.match(..., '^.+%.') or ... .. '.')
-- Up one directory
local _RR = (string.match(_R:sub(1,#_R-1), '^.+%.') or _R:sub(1,#_R-1) .. '.')

local stats = require(_RR .. 'stats')
local config = require(_RR .. 'config')

local TextButton = require(_R .. 'text_button')
local PuzzlePopup = require(_R .. 'puzzle_popup')

local ctrl_map = {} -- {filename = PuzzleCtrl}
setmetatable(ctrl_map, { __mode = 'v' }) -- Weak values

-- Return the style for the given status
local function get_style(flag)
    if flag == stats.MISSING then
        return config.styles.missing
    elseif flag == stats.EXISTS then
        return config.styles.downloaded
    elseif flag == stats.SOLVING then
        return config.styles.progress
    elseif flag == stats.COMPLETE then
        return config.styles.complete
    end
    return config.styles.unknown
end

-- Update the style of the PuzzleCtrl when stats are computed
stats:connect(stats.EVT_STATS, function(filename, flag)
    local ctrl = ctrl_map[filename]
    if not ctrl then return end
    -- Set the control style based on stats
    local style = get_style(flag)
    ctrl:SetForegroundColour(style.color)
    ctrl:SetFont(style.font)
    ctrl:Refresh()
end)

-- Copy text to the clipboard
local function copy_text(text)
    local clipBoard = wx.wxClipboard.Get()
    if clipBoard and clipBoard:Open() then
        clipBoard:SetData(wx.wxTextDataObject(tostring(text)))
        clipBoard:Flush() -- Make this available after we've exited
        clipBoard:Close()
    end
end

local function PuzzleCtrl(parent, text, puzzle)
    local self = TextButton(parent, wx.wxID_ANY, text)
    self:SetForegroundColour(config.styles.unknown.color)
    self:SetFont(config.styles.unknown.font)
    -- Keep track of this ctrl
    ctrl_map[puzzle.filename] = self
    -- Remove when the ctrl is destroyed
    self:Connect(wx.wxEVT_DESTROY, function()
        ctrl_map[puzzle.filename] = nil
    end)
    -- Fetch stats to set the initial style
    stats:fetch(puzzle.filename)

    -- Show popup on hover
    self:Connect(wx.wxEVT_ENTER_WINDOW, function (evt)
        PuzzlePopup(self, puzzle):Popup()
        evt:Skip()
    end)

    -- Open/download puzzle
    self:Connect(wx.wxEVT_LEFT_DOWN, function ()
        puzzle:open()
    end)

    -- Context menu:
    -- Copy URL
    -- Copy local filename
    -- Redownload
    self:Connect(wx.wxEVT_CONTEXT_MENU, function()
        local menu = wx.wxMenu()
        local item
        item = menu:Append(wx.wxID_ANY, "Copy URL")
        self:Connect(item:GetId(),
                     wx.wxEVT_COMMAND_MENU_SELECTED,
                     function () copy_text(puzzle.url) end)
        item = menu:Append(wx.wxID_ANY, "Copy local filename")
        self:Connect(item:GetId(),
                     wx.wxEVT_COMMAND_MENU_SELECTED,
                     function () copy_text(puzzle.filename) end)
        item = menu:Append(wx.wxID_ANY, "Redownload")
        self:Connect(item:GetId(),
                     wx.wxEVT_COMMAND_MENU_SELECTED,
                     function () puzzle:download() end)
        self:PopupMenu(menu)
        menu:delete()
        self:Disconnect(wx.wxEVT_COMMAND_MENU_SELECTED)
    end)
    return self
end

return PuzzleCtrl