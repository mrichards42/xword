require 'wxtask'
require 'download.messages'
require 'download.stats'

local join = require 'pl.path'.join
local OrderedSet = require 'download.ordered_set'

-- ------------------------------------------------------------------------
-- Download thread management
-- ------------------------------------------------------------------------

if not download then download = {} end

local task_id
download.queue = OrderedSet(function (t) return t[1] end)
download.current = nil
download.errors = {}
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
            queued_messages = {}
        end,
        [download.START] = function (data)
            local url, filename = unpack(data)
            download.current = url
            download.queue:remove({url})
            if download.dialog then
                download.dialog:update_status()
            end
        end,
        [download.END] = function (data)
            local url, filename, err = unpack(data)
            download.fetch_stats{{filename}, force = true}
            if err then
                table.insert(download.errors, data)
            end
            if #download.queue > 0 then
                download.current = download.queue[1][1]
            else
                download.current = nil
            end
            if download.dialog then
                download.dialog:update_status()
            end
        end,
        [task.END] = function ()
            download.current = nil
            task_id = nil
            if download.dialog then
                download.dialog:update_status()
            end
        end
    })
end

-- ---------------------------------------------------------------------------
-- Public functions
-- ---------------------------------------------------------------------------
require 'serialize'
function download.add_downloads(downloads)
    download.queue:append(unpack(downloads))
    if task_id then
        if task.post(task_id, downloads, download.APPEND) ~= 0 then
            table.insert(queued_messages, {downloads, download.APPEND})
        end
    else
        start_task(downloads)
    end
    if download.dialog then
        download.dialog:update_status()
    end
end

function download.add_download(url, filename, opts)
    download.add_downloads({ { url, filename, opts } })
end
