-- A task that downloads puzzles
require 'download.queue_task'
require 'luacurl'
require 'luapuz'
require 'import'
require 'os'

-- Parse the http error response message and spit out an error code
local function get_http_error(err)
    return tonumber(err:match("The requested URL returned error: (%d+)"))
end

local function do_download(args)
    local url, filename, curlopts = unpack(args)
    local f, rc, err
    f, err = io.open(filename, 'wb')
    if f then
        -- Write to a file
        local function write_to_file(str, length)
            f:write(str)
            return length -- Return length to continue download
        end

        -- Progress callback
        local function progress(dltotal, dlnow, ultotal, ulnow)
            -- Check to see if we should abort the download
            if task.checkAbort() then
                return 1 -- abort
            end
            return 0 -- continue
        end

        -- Setup the cURL object
        local c = curl.easy_init()
        c:setopt(curl.OPT_URL, url)
        c:setopt(curl.OPT_FOLLOWLOCATION, 1)
        c:setopt(curl.OPT_WRITEFUNCTION, write_to_file)
        c:setopt(curl.OPT_PROGRESSFUNCTION, progress)
        c:setopt(curl.OPT_NOPROGRESS, 0)
        c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors

        -- Set user-defined options
        for k,v in pairs(curlopts or {}) do c:setopt(k, v) end

        -- Run the download
        task.post(1, {url, filename}, download.START)
        rc, err = c:perform()
        -- Check the return code
        if rc ~= 0 then
            if rc == 22 then -- CURLE_HTTP_RETURNED_ERROR
                local code = get_http_error(err)
                if code == 404 or code == 410 then
                    err = "URL not found: "..url
                end
            elseif rc == 42 then -- abort from the progress function
                return 'abort'
            end
        else
            err = nil
        end

        -- Cleanup
        f:close()

        -- Try to open the file
        if not err then
            local success
            success, err = pcall(puz.Puzzle, filename)
            if success then
                err:__gc()
                err = nil
            else
                os.remove(filename)
            end
        else
            os.remove(filename)
        end
    end

    -- Thread is done, report errors if they exist
    task.post(1, {url, filename, err}, download.END)    
end

loop_through_queue(do_download, function(t) return t[1] end)
