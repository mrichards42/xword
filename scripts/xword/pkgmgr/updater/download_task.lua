local P = require 'xword.pkgmgr.updater'
local curl = require 'luacurl'
local path = require 'pl.path'

-- Progress callback function: Post dlnow, dltotal
local function progress(dltotal, dlnow)
    task.post(P.DL_PROGRESS, dlnow, dltotal)
    -- Check to see if we should abort the download
    if task.check_abort() then return 1 end
    return 0
end

-- Make the download folder (temporary)
local dl_dir = path.join(xword.userdatadir, 'updates')
if not path.exists(dl_dir) then
    path.mkdir(dl_dir)
end

-- Download each package
for _, pkg in ipairs(arg) do
    local name, url = pkg.name, pkg.download
    local filename = path.join(dl_dir, path.basename(url))

    task.post(P.DL_START, url)
    local success, rc, err = curl.get(url, filename, progress)
    task.post(P.DL_END, success, name, err)
    if rc == curl.ABORTED_BY_CALLBACK then
        return
    end
end

