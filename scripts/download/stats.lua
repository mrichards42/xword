require 'task'
require 'download.messages'
local join = require 'pl.path'.join

-- ------------------------------------------------------------------------
-- Stats thread
-- ------------------------------------------------------------------------

if not download then download = {} end

local stats_map = {} -- This will map filename to data
local ctrls = {} -- This maps filename to ctrl
local task_id

local function update_ctrl(ctrl, stats)
    if not stats.exists then
        ctrl:SetForegroundColour(wx.wxBLUE)
    elseif not stats.started then
        ctrl:SetForegroundColour(wx.wxColour(128, 0, 128))
    elseif stats.complete ~= 100 then
        ctrl:SetForegroundColour(wx.wxColour(34, 139, 34))
        local font = ctrl.Font
        font.Weight = wx.wxFONTWEIGHT_BOLD
        ctrl.Font = font
    else
        ctrl:SetForegroundColour(wx.wxColour(34, 139, 34))
    end
    ctrl:Refresh()
end

local function add_stats(data)
    local filename, stats = unpack(data)
    stats_map[filename] = stats
    local ctrl = ctrls[filename]
    if ctrl then
        update_ctrl(ctrl, stats)
    end
end

-- ---------------------------------------------------------------------------
-- Public functions
-- ---------------------------------------------------------------------------

function download.fetch_stats(filenames)
    if not type(filenames) == 'table' then
        filenames = { filenames }
    end
    -- See if we've already fetched this data
    local task_filenames = {}
    for _, fn in ipairs(filenames) do
        local stats = stats_map[fn]
        if stats then
            local ctrl = ctrls[filename]
            if ctrl then
                update_ctrl(ctrl, stats)
            end
        else
            table.insert(task_filenames, fn)
            print('no stats: '.. fn:match('[\\/]([^\\/]+)$'))
        end
    end
    -- Fetch the missing data
    if task_id then
        task.post(task_id, task_filenames,
                  now and download.PREPEND or download.APPEND)
    else
        task_id = task.create(
                join(xword.scriptsdir, 'download', 'stats_task.lua'),
                task_filenames)
        task.handleEvents(task_id,
            {
                [download.STATS] = add_stats,
                [task.END] = function () task_id = nil end
            }
        )
    end
end

function download.add_ctrl(ctrl)
    ctrls[ctrl.filename] = ctrl
    local stats = stats_map[ctrl.filename]
    if stats then
        update_ctrl(ctrl, stats)
    end
end

function download.clear_stats()
    if task_id then
        task.abort(task_id)
    end
    task_id = nil
    ctrls = {}
end

function download.erase_stats()
    download.clear_stats()
    stats_map = {}
end
