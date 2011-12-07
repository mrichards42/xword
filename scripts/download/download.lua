require 'wxtask'
require 'download.messages'
require 'download.stats'

local join = require 'pl.path'.join
local OrderedSet = require 'download.ordered_set'
local clear = require 'pl.tablex'.clear

-- ------------------------------------------------------------------------
-- Download thread management
-- ------------------------------------------------------------------------

if not download then download = {} end


local task_id
download.queue = OrderedSet(function (t) return t.filename end)
download.current = nil
download.errors = {}
download.open_after_download = nil
 -- These are downloads that were added when the task was starting
local queued_messages = {}

local function start_task(downloads)
    task_id = task.create(
            join(xword.scriptsdir, 'download', 'download_task.lua'), downloads)

    task.handleEvents(task_id,{
        [task.START] = function ()
            -- Add the queued messages
            for _, data in ipairs(queued_messages) do
                task.post(task_id, unpack(data))
            end
            clear(queued_messages)
        end,
        [download.START] = function (data)
            local puzzle = unpack(data)
            puzzle.date = setmetatable(puzzle.date, getmetatable(date()))
            download.current = puzzle
            download.queue:remove(puzzle)
            if download.dialog then
                download.dialog:update_status()
            end
        end,
        [download.END] = function (data)
            local puzzle, err = unpack(data)
            puzzle.date = setmetatable(puzzle.date, getmetatable(date()))
            download.fetch_stats{{filename}, force = true}
            if err then
                table.insert(download.errors, data)
            end
            if #download.queue > 0 then
                download.current = download.queue[1]
            else
                download.current = nil
            end
            if download.dialog then
                download.dialog:update_status()
            end
            if download.open_after_download == filename then
                download.open_after_download = nil
                if not err and lfs.attributes(filename, 'mode') == 'file' then
                    xword.frame:LoadPuzzle(filename)
                end
            end
        end,
        [task.END] = function ()
            download.current = nil
            task_id = nil
            if download.dialog then
                download.dialog:update_status()
            end
        end
    }
--        xword.frame
    )
end

local function post(id, data, flag)
    if task.post(id, data, flag) ~= 0 then
        table.insert(queued_messages, {data, flag})
    end
end

-- ---------------------------------------------------------------------------
-- Public functions
-- ---------------------------------------------------------------------------
require 'serialize'
function download.add_downloads(downloads, prepend)
    local prepend = (prepend and prepend == download.PREPEND)
    if prepend then
        download.queue:prepend(unpack(downloads))
    else
        download.queue:append(unpack(downloads))
    end
    if task_id then
        post(task_id, downloads, prepend and download.PREPEND or download.APPEND)
    else
        start_task(downloads)
    end
    if download.dialog then
        download.dialog:update_status()
    end
end

function download.add_download(data, prepend)
    download.add_downloads({ data }, prepend)
end

function download.clear_downloads()
    print 'hello world'
    if task_id then
        post(task_id, nil, download.CLEAR)
    end
    download.queue:clear()
    if download.dialog then
        download.dialog:update_status()
    end
end
