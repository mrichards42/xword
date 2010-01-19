--[[
An implementation of a download thread using LuaTask
]]

require 'mtask'
require 'download.defs'

-- Must be called with these args
local parent, url, filename = unpack(arg)
assert(parent)
assert(url)
assert(filename)

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
        if rc and flag == download.DL_ABORT then
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

-- Setup the cURL object
require 'luacurl'
local c = curl.easy_init()
c:setopt(curl.OPT_URL, url)
c:setopt(curl.OPT_FOLLOWLOCATION, 1)
c:setopt(curl.OPT_WRITEFUNCTION, writeTo(f))
c:setopt(curl.OPT_PROGRESSFUNCTION, progressFunc(linda))
c:setopt(curl.OPT_NOPROGRESS, 0)

-- Run the download
task.post(parent, '', download.DL_START)
local rc, errstr = c:perform()
task.post(parent, {rc, errstr}, download.DL_END)

-- Cleanup
f:close()
