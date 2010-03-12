--[[
An implementation of a download thread using LuaTask
]]

require 'mtask'
require 'download.defs'

-- Arguments to this file:
-- { parent, url, filename, curl.OPT_NAME, val, curl.OPT_NAME, val, ... }
-- curl options and values are optional, but must come in pairs.
-- Only string and number values are allowed
local parent, url, filename = unpack(arg)
for k,v in pairs(arg) do task.debug(k.." = "..v) end
assert(parent)
assert(url)
assert(filename)

-- Find curlopts
-- Options must be sequential instead of a table because LuaTask only supports
-- passing string and number values to new threads.
local curlopts = {}
do
    local i = 4
    while true do
        local opt, val = arg[i], arg[i+1]
        if not (opt and val) then break end
        curlopts[opt] = val
        i = i + 2
    end
end

-- ----------------------------------------------------------------------------
-- cURL Callbacks
-- ----------------------------------------------------------------------------

-- Write to a file
local function writeTo(fp)
    return function(str, length)
        fp:write(str)
        return length -- Return length to continue download
    end
end

-- Progress callback function: Post '{dlnow, dltotal}' to the linda
local function progressFunc(linda)
    return function(dltotal, dlnow, ultotal, ulnow)
        task.post(parent, {dlnow, dltotal}, download.DL_PROGRESS)

        -- Check to see if we should abort the download
        local msg, flag, rc = task.receive(0, 1) -- no timeout; from main thread
        if rc == 0 and flag == download.DL_ABORT then
            task.post(parent, 'ABORTING', download.DL_MESSAGE)
            return 1
        else
            return 0
        end
    end
end



-- ----------------------------------------------------------------------------
-- The download thread
-- ----------------------------------------------------------------------------

local f = assert(io.open(filename, 'wb'))

task.debug("curl setup")
-- Setup the cURL object
require 'luacurl'
local c = curl.easy_init()
c:setopt(curl.OPT_URL, url)
c:setopt(curl.OPT_FOLLOWLOCATION, 1)
c:setopt(curl.OPT_WRITEFUNCTION, writeTo(f))
c:setopt(curl.OPT_PROGRESSFUNCTION, progressFunc(linda))
c:setopt(curl.OPT_NOPROGRESS, 0)

-- Set user-defined options
for k, v in pairs(curlopts) do c:setopt(k, v) end
task.debug("curl start")
-- Run the download
task.post(parent, '', download.DL_START)
local rc, errstr = c:perform()
task.post(parent, {rc, errstr}, download.DL_END)
task.debug("curl end")
-- Cleanup
f:close()
