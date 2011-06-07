require 'wxtask'
require 'serialize'

local join = require 'pl.path'.join

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
    P.packages_url = "http://sourceforge.net/projects/wx-xword/files/packages_"..osname..".lua"
    P.packages_url = [[file:///D:\C++\XWord\trunk\scripts\xword\pkgmgr\updater\dummydata.lua]]

    function P.CheckForUpdates(callback)
        task.handleEvents(
            task.create(join(xword.scriptsdir, 'xword', 'pkgmgr', 'updater', 'check_task.lua'),
                        { P.packages_url, P.updates_filename } ),
            { [task.END] = callback }
        )
    end
end

return P
