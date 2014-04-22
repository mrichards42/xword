--- Download manager.
-- Implemented as a singleton class that derives from `task.QueueTask`.

-- Secondary threads just need these fields
local task = require 'task'
local QueueTask = require 'task.queue_task'

local M = {
    --- Start of a download.
    EVT_DOWNLOAD_START = QueueTask.EVT_ITEM_START,

    --- End of a download.
    EVT_DOWNLOAD_END = QueueTask.EVT_ITEM_END,

    --- The download queue has changed (other than START or END)
    -- @field M.EVT_QUEUE_UPDATED

    --- Progress on a long download.
    -- Sends dltotal, dlnow
    EVT_PROGRESS = 101,

    --- The statusbar should be updated.
    EVT_UPDATE_STATUS = 103,

    --- History cleared
    EVT_CLEAR_HISTORY = 104,

    -- Internal use
    PROMPT = 110,
}

if not task.is_main then return M end

local tablex = require 'pl.tablex'
local path = require 'pl.path'

local _R = mod_path(...)
local stats = require(_R .. 'stats')

-- The download manager
QueueTask.new{
    obj = M,
    name = 'Puzzle Downloader',
    script = _R .. 'download_task',
    -- Index puzzles by filename
    unique = true,
    key = 'filename'
}

--- @type Manager

--- Currently downloading `Puzzle`.
-- @field M.current

--- Download queue.
-- @field M.queue

--- Puzzles that have finished downloading (error or not).
-- @usage mgr.done[idx].puzzle, mgr.done[idx].status
M.done = {}

--- Error count
M.error_count = 0

-- Connect events
M:connect{
    [M.EVT_DOWNLOAD_START] = function(puzzle)
        puzzle.status = "Downloading"
    end,
    [M.EVT_DOWNLOAD_END] = function(puzzle, err)
        -- Add to done table and update errors/status
        table.insert(M.done, puzzle)
        if err == true then err = nil end
        if err then
            puzzle.status = err
            M.error_count = M.error_count + 1
            stats:update(puzzle.filename, err)
        else
            stats:update(puzzle.filename, stats.EXISTS)
            stats:fetch{puzzle.filename, force = true}
            puzzle.status = "Done"
        end
        -- Open if specified
        if M.open_after_download == puzzle.filename then
            M.open_after_download = nil
            if not err then
                local open_puzzle = require(_R .. 'sources').open
                open_puzzle(puzzle.filename)
            end
        end
    end,
    [M.PROMPT] = function(msg, ...)
        -- Prompt the user for fields specified in ... and return values
        local PromptDialog = require(_R..'gui.wxFB').PromptDialog
        local dlg = PromptDialog(wx.NULL)
        dlg.message:SetLabel(msg)
        -- Add fields
        local values = {}
        local sizer = dlg.message:GetContainingSizer():GetItem(1):GetSizer()
        for _, label in ipairs({...}) do
            sizer:Add(wx.wxStaticText(dlg, wx.wxID_ANY, label), 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxLEFT, 5)
            -- Make fields named "password" into password text boxes
            local flags = 0
            if label:lower():match("password") then
                flags = wx.wxTE_PASSWORD
            end
            local ctrl = wx.wxTextCtrl(
                dlg, wx.wxID_ANY, '',
                wx.wxDefaultPosition, wx.wxDefaultSize, flags)
            values[label] = ctrl
            ctrl:SetMinSize(wx.wxSize(200, -1))
            sizer:Add(ctrl, 1, wx.wxALIGN_CENTER_VERTICAL + wx.wxEXPAND)
        end
        -- Prompt
        dlg:Fit()
        if dlg:ShowModal() == wx.wxID_OK then
            for label, ctrl in pairs(values) do
                values[label] = ctrl:GetValue()
            end
            M:post(M.PROMPT, values)
        else
            M:post(M.PROMPT)
        end
        dlg:Destroy()
        dlg = nil
    end,
}

--- Add downloads to the task.
-- @param opts A table of options.
-- @param opts.1 table of `Puzzle` objects.
-- @param opts.prepend Download these puzzles before others in the queue.
-- @param opts.open Open the first puzzle in the list when complete.
-- @usage
-- local puzzles = {puzzle1, puzzle2, ...}
-- mgr:add(puzzles)
-- mgr:add{puzzles, prepend=true} -- Add to the top of the queue
-- mgr:add{puzzles, open=true} -- Open the first puzzle when it completes
--
-- -- Can also be called with a single puzzle instead of a list:
-- mgr:add(puzzle)
-- mgr:add{puzzle, prepend=true}
-- mgr:add{puzzle, open=true}
function M:add(opts)
    -- Get options
    local prepend, puzzles, open
    if #opts == 1 then -- More options exist
        puzzles = opts[1]
        prepend = opts.prepend
        open = opts.open
    else
        puzzles = opts
    end
    if puzzles[1] and type(puzzles[1]) ~= 'table' then -- single puzzle
        puzzles = {puzzles}
    end
    if #puzzles < 1 then return end
    if open then
        self.open_after_download = puzzles[1].filename
    end

    -- Post to the task
    local downloads = tablex.map_named_method('get_download_data', puzzles)
    if not self:is_running() then self:start() end
    if prepend then
        self:prepend(unpack(downloads))
    else
        self:append(unpack(downloads))
    end
    self.queue:normalize()
end

--- Clear download history.
-- Preserves download queue and sends `EVT_CLEAR_HISTORY`.
function M:clear_history()
    tablex.clear(self.done)
    self.error_count = 0
    self:send_event(M.EVT_CLEAR_HISTORY)
end

--- Clear download queue.
-- @function M:clear()

return M