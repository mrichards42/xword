P = require 'xword.pkgmgr'
require 'xword.pkgmgr.updater'

local serialize = require 'serialize'
local curl = require 'luacurl'

-- Args
local packages_url, updates_filename = unpack(arg)

-- Load the remote and local updates
local remote_updates = serialize.loadstring(curl.get(packages_url)) or {}
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
if updates.xword then
    if not remote_updates.xword then
        remote_updates.xword = updates.xword
    elseif updates.xword.version == remote_updates.xword.version then
        remote_updates.xword.ignored = updates.xword.ignored
    end
end

-- Save update status of packages that were not already in remote_updates
for _, p in ipairs(updates) do
    table.insert(remote_updates, p)
end

-- Save the updates to file
serialize.dump(remote_updates, updates_filename)
