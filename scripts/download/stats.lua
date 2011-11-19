require 'wxtask'
require 'download.messages'
local join = require 'pl.path'.join

-- ------------------------------------------------------------------------
-- Stats thread management
-- ------------------------------------------------------------------------

if not download then download = {} end
download.status_map = {} -- filename = status
download.puzzle_map = {} -- filename = PuzzlePanel
local ctrls = {} -- filename = PuzzleCtrl
local task_id

local function update_ctrl(filename, status)
    -- Update the status of the PuzzleCtrl
    ctrl = ctrls[filename]
    if not ctrl then return end
    status = status or download.status_map[ctrl.filename]
    if not status then return end
    if status == download.MISSING then
        ctrl:SetForegroundColour(wx.wxBLUE)
    elseif status == download.EXISTS then
        ctrl:SetForegroundColour(wx.wxColour(128, 0, 128))
    elseif status == download.SOLVING then
        ctrl:SetForegroundColour(wx.wxColour(34, 139, 34))
        local font = ctrl.Font
        font.Weight = wx.wxFONTWEIGHT_BOLD
        ctrl.Font = font
    elseif status == download.COMPLETE then
        ctrl:SetForegroundColour(wx.wxColour(34, 139, 34))
    end
    ctrl:Refresh()
end

-- ---------------------------------------------------------------------------
-- Public functions
-- ---------------------------------------------------------------------------

-- Fetch stats, with some options:
-- force = fetch even if we already have this file
-- prepend = get these stats before queued stats
function download.fetch_stats(filenames)
    if not type(filenames) == 'table' then
        filenames = { filenames }
    end
    -- This can be called as a table: fetch_stats{filenames, force = true}
    local force, prepend
    if type(filenames[1]) == 'table' then
        filenames, force, prepend =
            filenames[1], filenames.force, filenames.prepend
    end
    -- See if we've already fetched this data
    local task_filenames = {}
    for _, fn in ipairs(filenames) do
        local status = download.status_map[fn]
        if status and not force then
            update_ctrl(filename, status)
        else
            table.insert(task_filenames, fn)
        end
    end
    -- Fetch the missing data
    if task_id and task.isrunning(task_id) then
        task.post(task_id, task_filenames,
                  prepend and download.PREPEND or download.APPEND)
    else
        task_id = task.create(
                join(xword.scriptsdir, 'download', 'stats_task.lua'),
                task_filenames)
        task.handleEvents(task_id,
            {
                [download.STATS] = function (data)
                    local filename, status = unpack(data)
                    download.status_map[filename] = status
                    update_ctrl(filename, status)
                    -- Update the stats for the PuzzlePanel
                    local panel = download.puzzle_map[filename]
                    if panel then
                        panel:update_stats()
                    end
                end,
            }
        )
    end
end

function download.add_ctrl(ctrl)
    ctrls[ctrl.filename] = ctrl
    update_ctrl(ctrl.filename)
end

function download.clear_stats()
    if task_id and task.isrunning(task_id) then
        task.post(task_id, nil, download.CLEAR)
    end
    ctrls = {}
    --download.status_map = {}
end

function download.erase_stats()
    download.clear_stats()
    download.status_map = {}
end
