--- Puzzle statistics.
-- Implemented as a singleton class that derives from `task.QueueTask`.

local task = require 'task'
local QueueTask = require 'task.queue_task'

local M = {
    --- Last download attempt was an error
    ERROR = 0,
    --- Puzzle does not exist.
    MISSING = 1,
    --- Puzzle exists, but has not been opened.
    EXISTS = 2,
    --- User has started solving the puzzle.
    SOLVING = 3,
    --- User has completely solved the puzzle
    COMPLETE = 4,
    --- Sent when stats have been fetched.
    -- Callback is passed filename and status value.
    EVT_STATS = QueueTask.EVT_ITEM_END,
}

-- Secondary threads just need these fields
if not task.is_main then return M end

local path = require 'pl.path'

local function clear(t)
    for k,_ in pairs(t) do
        t[k] = nil
    end
end

local _R = string.match(..., "^.+%.") or "" -- relative require

-- Create the stats thread
QueueTask.new{
    obj = M,
    script = _R .. 'stats_task',
    name = 'Puzzle Stats'
}

--- @type Stats

--- Table mapping filename to stats.
M.map = {}
--- Table mapping filename to download errors.
M.error = {}

-- Connect events
M:connect(M.EVT_STATS, function(filename, flag, err)
    if not filename then return end
    if flag == M.ERROR and not err then print(debug.traceback()) end
    M.map[filename] = flag
    M.error[filename] = err
end)

--- Fetch stats for the supplied files
-- @param opts Table of parameters, or filenames
-- @param opts.1 filenames
-- @param opts.force
-- @param opts.prepend
-- @usage
-- local filenames = {"file1", "file2", "file3", ...}
-- stats:fetch(filenames)
-- stats:fetch{filenames, force=true} -- Recheck any stats that are cached
-- stats:fetch{filenames, prepend=true} -- Add to the front of the queue
--
-- Can also be called with a single file:
-- stats:fetch("filename")
-- stats:fetch{"filename", force=true}
-- stats:fetch{"filename", prepend=true}
function M:fetch(opts)
    -- Gather options
    local force, prepend, filenames
    if type(opts) == 'table' and #opts == 1 then -- options
        filenames = opts[1]
        force, prepend = opts.force, opts.prepend
    else -- no options
        filenames = opts
    end
    if type(filenames) == 'string' then -- Single filename
        filenames = {filenames}
    end
    -- Make a list of uncached files (unless force=true)
    local task_filenames
    if force then
        task_filenames = filenames
    else
        task_filenames = {}
        for _, filename in ipairs(filenames) do
            local status = self.map[filename]
            if status then
                self:send_event(M.EVT_STATS, filename, status, self.error[filename])
            else
                table.insert(task_filenames, filename)
            end
        end
    end
    -- Fetch stats
    if not self:is_running() then self:start() end
    if prepend then
        self:prepend(unpack(task_filenames))
    else
        self:append(unpack(task_filenames))
    end
end

--- Clear and delete cached stats.
function M:erase()
    self:clear()
    clear(self.map)
    clear(self.error)
end

--- Set stats.
-- Sends stats.EVT_STATUS
-- @param filename The puzzle filename.
-- @param[opt] flag The status flag.
-- @param[opt] err An error message.  Sets status to stats.ERROR
function M:update(filename, flag, err)
    if type(flag) == 'string' then
        err = flag
        flag = M.ERROR
    end
    self:send_event(self.EVT_STATS, filename, flag, err)
end

-------------------------------------------------------------------------------
-- Static/module-level functions
-- @section static

--- Does this puzzle exist?
-- @param filename The puzzle
-- @return true/false
function M.exists(filename)
    local flag = M.map[filename]
    if flag then
        return flag > M.MISSING
    else
        local size = path.getsize(filename) or 0
        if size == 0 then
            M:update(filename, M.MISSING)
            return false
        end
        return true
    end
end

return M
