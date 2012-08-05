P = require 'xword.pkgmgr'
require 'xword.pkgmgr.updater'

require 'serialize'
require 'luacurl'

-- Args
local packages_url, updates_filename = unpack(arg)

-- cURL Callbacks
-- Write to a table
local function write_to_table(t)
    return function(str, length)
        table.insert(t, str)
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
c:setopt(curl.OPT_URL, packages_url)
c:setopt(curl.OPT_FOLLOWLOCATION, 1)
c:setopt(curl.OPT_WRITEFUNCTION, write_to_table(t))
c:setopt(curl.OPT_PROGRESSFUNCTION, progress_func)
c:setopt(curl.OPT_NOPROGRESS, 0)

local rc, err = c:perform()

-- Load the remote and local updates
local remote_updates = serialize.loadstring(table.concat(t)) or {}
local updates = serialize.loadfile(updates_filename) or {}

-- Replace packages in updates with packages from remoted_updates, copying
-- "ignored" from packages in updates
for _, pkg in ipairs(remote_updates or {}) do
    -- Find the local package
    for i, local_pkg in ipairs(updates) do
        if local_pkg.name == pkg.name and local_pkg.version == pkg.version then
            pkg.ignored = local_pkg.ignored -- Copy ignored status
            table.remove(updates, i)
        end
    end
end

-- Copy ignored only if this is an xword version that we have already seen.
if updates.xword and remote_updates.xword
    and updates.xword.version == remote_updates.xword.version
then
    remote_updates.xword.ignored = updates.xword.ignored
end

-- Save update status of packages that were not already in remote_updates
for _, p in ipairs(updates) do
    table.insert(remote_updates, p)
end

-- Save the updates to file
serialize.pdump(remote_updates, updates_filename)
