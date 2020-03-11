local task = require 'task'
require 'serialize'

local dirname = require 'pl.path'.dirname
local join = require 'pl.path'.join
local makepath = require 'pl.dir'.makepath

if not xword.pkgmgr then
    require 'xword.pkgmgr'
end
local P = {}
xword.pkgmgr.updater = P

-- Messages
P.HAS_UPDATE = 1
P.DL_START = 2
P.DL_PROGRESS = 3
P.DL_END = 4
P.INSTALL = 5

-- Filenames
P.updates_filename = join(xword.configdir, 'updater', 'updates.lua')


if not task.is_main then return P end

-- Check for an update.  Callback is called after the task executes
local osname = wx.__WXMSW__ and 'windows' or wx.__WXMAC__ and 'mac' or 'linux'
P.packages_url = "https://mrichards42.github.io/xword/packages_"..osname..".lua"

function P.CheckForUpdates(callback)
    -- Make sure we have a directory for the updates file
    makepath(dirname(P.updates_filename))
    -- Run the task
    task.run{
        'xword.pkgmgr.updater.check_task', callback,
        args = {P.packages_url, P.updates_filename},
        name = "XWord Update Task"
    }
end

return P
