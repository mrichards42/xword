require 'wxtask'
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


if task.id() == 1 then
    -- Check for an update.  Callback is called after the task executes
    local osname = wx.__WXMSW__ and 'windows' or wx.__WXMAC__ and 'mac' or 'linux'
    P.packages_url = "http://sourceforge.net/projects/wx-xword/files/scripts/packages_"..osname..".lua"

    function P.CheckForUpdates(callback)
        -- Make sure we have a directory for the updates file
        makepath(dirname(P.updates_filename))
        task.handleEvents(
            task.create(join(xword.scriptsdir, 'xword', 'pkgmgr', 'updater', 'check_task.lua'),
                        { P.packages_url, P.updates_filename } ),
            { [task.END] = callback }
        )
    end
end

return P
