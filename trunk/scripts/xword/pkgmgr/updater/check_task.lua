require 'xword.pkgmgr'
require 'xword.pkgmgr.updater'

require 'luacurl'

require 'serialize'

local P = xword.pkgmgr

-- Args
local packagesURL, updates_filename = unpack(arg)

-- Load the cached update file
local updates = serialize.loadfile(updates_filename) or {}

-- cURL Callbacks
local remote_version
-- Write to a table
local function write_to_table(t)
    local has_first_line = false
    return function(str, length)
        table.insert(t, str)
        if not has_first_line then
            local end_of_line = str:match("(.-)[\n\r\l]")
            if end_of_line then 
                has_first_line = true
                -- Get the version from the first line
                remote_version = table.concat(t, "", 1, #t-1)..end_of_line
                -- Version is a string "-- version x.y.z.r"
                remote_version = remote_version:match("-- *version *([%d%.]+)")
                -- Check to see if this is a new file
                if remote_version and updates.version then
                    if not P.is_newer(remote_version, updates.version) then
                        return 0 -- Abort download; we already have a file
                    end
                end
            end
        end
        return length -- Return length to continue download
    end
end

-- Progress callback function: Check for abort
local function progress_func()
    -- Check to see if we should abort the download
    if task.checkAbort() then return 1 end
    return 0
end


-- Download the table and load it
local t = {}

local c = curl.easy_init()
c:setopt(curl.OPT_URL, packagesURL)
c:setopt(curl.OPT_FOLLOWLOCATION, 1)
c:setopt(curl.OPT_WRITEFUNCTION, write_to_table(t))
c:setopt(curl.OPT_PROGRESSFUNCTION, progress_func)
c:setopt(curl.OPT_NOPROGRESS, 0)

local rc, err = c:perform()

-- Load the remote updates
local remote_updates = serialize.loadstring(table.concat(t))

-- Replace packages in updates with packages from remoted_updates, copying
-- "ignored" from packages in updates
for _, pkg in ipairs(remote_updates or {}) do
    -- Find the local package
    local local_pkg
    for _, p in ipairs(updates) do
        if p.name == pkg.name then
            local_pkg = p
        end
    end
    -- Copy "ignored"
    if local_pkg then
        pkg.ignored = local_pkg.ignored
    end
end

-- Save the updates to file
serialize.pdump(remote_updates, updates_filename)
