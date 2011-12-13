-- A task that downloads puzzles
require 'download.queue_task'
require 'luacurl'
require 'luapuz'
require 'import'
require 'os'
require 'pl.path'
require 'lfs'
require 'date'

local function makedirs(dirname)
    if lfs.attributes(dirname, 'mode') then return true end
    -- Find the directories we need to make
    local to_make = {}
    local dir
    repeat
        dirname, dir = pl.path.splitpath(dirname)
        table.insert(to_make, dir)
    until lfs.attributes(dirname, 'mode') ~= nil
    -- Make the directories
    for i=#to_make,1,-1 do
        dirname = pl.path.join(dirname, to_make[i])
        local success, err = lfs.mkdir(dirname)
        if not success then return nil, err end
    end
    return true
end

-- Parse the http error response message and spit out an error code
local function get_http_error(err)
    return tonumber(err:match("The requested URL returned error: (%d+)"))
end

-- Progress callback
local function progress(dltotal, dlnow, ultotal, ulnow)
    -- Check to see if we should abort the download
    if task.checkAbort() then
        return 1 -- abort
    end
    return 0 -- continue
end

local function check_return_code(rc, err)
    if rc ~= 0 then
        if rc == 22 then -- CURLE_HTTP_RETURNED_ERROR
            local code = get_http_error(err)
            if code == 404 or code == 410 then
                err = "URL not found: "..url
            end
        elseif rc == 42 then -- abort from the progress function
            error({'abort'})
        end
        return nil, err
    else
        return true
    end
end

-- Download a file
local function do_download(url, callback, opts)
    -- Setup the cURL object
    local c = curl.easy_init()
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_FOLLOWLOCATION, 1)
    c:setopt(curl.OPT_WRITEFUNCTION, callback)
    c:setopt(curl.OPT_PROGRESSFUNCTION, progress)
    c:setopt(curl.OPT_NOPROGRESS, 0)
    c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors

    -- Set user-defined options
    for k,v in pairs(opts or {}) do c:setopt(k, v) end

    -- Run the download
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
        return nil, err
    else
        return true
    end
end

local function download_to_file(url, filename, opts)
    local f, rc, err
    makedirs(pl.path.dirname(filename))
    f, err = io.open(filename, 'wb')
    if not f then
        return nil, err
    end

    local f, rc, err
    makedirs(pl.path.dirname(filename))
    f, err = io.open(filename, 'wb')
    if not f then
        return nil, err
    end

    -- Write to a file
    local function write_to_file(str, length)
        f:write(str)
        return length -- Return length to continue download
    end

    -- Download
    rc, err = do_download(url, write_to_file, opts)

    -- Cleanup
    f:close()

    if not rc then
        os.remove(filename)
    end

    return rc, err
end

local function download_to_string(url, opts)
    local t = {}

    -- Write to the table
    local function write_to_table(str, length)
        table.insert(t, str)
        return length -- Return length to continue download
    end

    -- Download
    local success, err = do_download(url, write_to_table, opts)

    -- Return the string
    if success then
        return table.concat(t)
    else
        return success, err
    end
end

--[[
    Downlod something
    -----------------

    To a file
    ---------
    download.download{url = url, filename = filename, [curl]opts = opts}
        or
    download.download(url, filename, opts)
        -> true or nil, err

    With a callback function
    ------------------------

        function callback(str, length)
            return length -- to continue downloading
        end

    download.download{url = url, callback = callback, [curl]opts = opts}
        or 
    download.download(url, callback, opts)
        -> true or nil, err

    To a string
    -----------
    download.download{url = url, [curl]opts = opts}
        or 
    download.download(url, opts)
        -> str or nil, err
]]
function download.download(opts, filename, curlopts)
    -- Gather the arguments
    local url, callback
    if type(opts) == 'table' then
        url = opts.url or opts[1]
        filename = opts.filename or opts[2]
        callback = opts.callback
        curlopts = opts.opts or opts.curlopts or opts[3]
    else
        url = opts
    end
    if type(filename) == 'function' then
        callback = filename
        filename = nil
    end

    assert(url)

    -- Figure out which variant to call
    local success, err
    if callback then
        success, err = do_download(url, callback, curlopts)
    elseif filename then
        success, err = download_to_file(url, filename, curlopts)
    else
        success, err = download_to_string(url, curlopts)
    end
    -- Check for abort
    if success == 'abort' then
        -- wrapped in a table so that location info isn't addeds
        error({'abort'})
    else
        return success, err
    end
end


local function do_download(puzzle)
    setmetatable(puzzle.date, getmetatable(date()))
    local success, func, err

    task.post(1, {puzzle}, download.START)

    if puzzle.func then
        func, err = loadstring([[return function(puzzle) ]]..puzzle.func..[[ end]])
        if not err then
            success, err = func()(puzzle)
        end
    else
        success, err = download.download(puzzle.url, puzzle.filename, puzzle.curlopts)
    end

    if success == 'abort' then error({'abort'}) end

    -- Try to open the file
    if success then
        local success, p = pcall(puz.Puzzle, puzzle.filename)
        if success then
            p:__gc()
            err = nil
        else
            err = p
            os.remove(puzzle.filename)
        end
    else
        os.remove(puzzle.filename)
    end

    task.post(1, {puzzle, err}, download.END)
end

loop_through_queue(do_download, function(t) return t.filename end)
