require 'download.messages'
pcall(require, 'import')

-- Arguments to this file:
-- { url, filename, { [curl.OPT_NAME] = val, ... } }
local url, filename, curlopts = unpack(arg)
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

-- Progress callback function: Post '{dlnow, dltotal}'
local function progressFunc(dltotal, dlnow, ultotal, ulnow)
    task.post(1, {dlnow, dltotal}, download.DL_PROGRESS)

    -- Check to see if we should abort the download
    local msg, flag, rc = task.receive(0, 1) -- no timeout; from main thread
    if task.checkAbort() then
        return 1
    end
    return 0
end


-- ---------------------------------------------------------------------------
-- Puzzle conversion
-- ---------------------------------------------------------------------------
function convertPuzzle() -- the filename argument is an upvalue
    -- Load the puzzle then save it as a .puz file
    require 'luapuz'
    -- Try to load the puzzle
    local success, p = pcall(puz.Puzzle, filename)
    if success then
        p:__gc()
    else
        return nil, (p or "Failed to open file: "..filename)
    end

    return true
end

-- ----------------------------------------------------------------------------
-- The download thread
-- ----------------------------------------------------------------------------

-- Parse the http error response message and spit out an error code
local function get_http_error(err)
    return tonumber(err:match("The requested URL returned error: (%d+)"))
end

local f, success, err
f, err = io.open(filename, 'wb')
if f then
    -- Setup the cURL object
    require 'luacurl'
    local c = curl.easy_init()
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_FOLLOWLOCATION, 1)
    c:setopt(curl.OPT_WRITEFUNCTION, writeTo(f))
    c:setopt(curl.OPT_PROGRESSFUNCTION, progressFunc)
    c:setopt(curl.OPT_NOPROGRESS, 0)
    c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors

    -- Set user-defined options
    for k,v in pairs(curlopts or {}) do c:setopt(k, v) end

    -- Run the download
    task.post(1, nil, download.DL_START)
    success, err = c:perform()
    -- Check the return code
    if success ~= 0 then
        if success == 22 then -- CURLE_HTTP_RETURNED_ERROR
            local code = get_http_error(err)
            if code == 404 or code == 410 then
                err = "URL not found: "..url
            end
        end
        success = false
    else
        success = true
    end
    -- Cleanup
    f:close()

    -- Convert the puzzle
    if success then
        success, err = convertPuzzle()
    end
end

-- Thread is done, report errors if they exist
task.post(1, err, download.DL_END)
