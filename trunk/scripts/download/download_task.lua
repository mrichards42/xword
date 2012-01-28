-- A task that downloads puzzles
require 'download.queue_task'
require 'luacurl'
require 'luapuz'
require 'import'
require 'os'
path = require 'pl.path' -- make this global for custom download functions
require 'lfs'
require 'date'

local function makedirs(dirname)
    if lfs.attributes(dirname, 'mode') then return true end
    -- Find the directories we need to make
    local to_make = {}
    local dir
    repeat
        dirname, dir = path.splitpath(dirname)
        table.insert(to_make, dir)
    until lfs.attributes(dirname, 'mode') ~= nil
    -- Make the directories
    for i=#to_make,1,-1 do
        dirname = path.join(dirname, to_make[i])
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
    c:setopt(curl.OPT_SSL_VERIFYPEER, 0) -- Authentication doesn't really work

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
    makedirs(path.dirname(filename))
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
    Download something
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
    elseif type(filename) == 'table' then
        curlopts = filename
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
        -- wrapped in a table so that location info isn't added
        error({'abort'})
    elseif not success then
        error({err})
    else
        return success
    end
end

local deepcopy = require 'pl.tablex'.deepcopy
local function download_puzzle(puzzle)
    -- If we don't copy the date, we could accidentally set the metatable
    -- twice (the date mt is "protected" and doesn't allow that).
    puzzle = deepcopy(puzzle)
    setmetatable(puzzle.date, getmetatable(date()))

    task.post(1, {puzzle}, download.START)

    local success, err = xpcall(
        function ()
            -- Download the puzzle
            if puzzle.func then
                local func, err = loadstring([[return function(puzzle, download) ]]..puzzle.func..[[ end]])
                if not err then
                    err = func()(puzzle, download.download)
                end
                if err and err ~= true then
                    return err
                end
            else
                download.download(puzzle.url, puzzle.filename, puzzle.curlopts)
            end
        end,
        -- error handler . . . separates user errors from programming errors
        function (e)
            if type(e) == 'table' and type(e[1]) == 'string' then
                return e[1]
            else
                return debug.traceback(e)
            end
        end
    )

    if err then
        os.remove(puzzle.filename)
        if err == 'abort' then
            error({'abort'})
        end
    else
        -- Try to open the file
        local success, result = pcall(puz.Puzzle, puzzle.filename)
        if success then
            result:__gc() -- This is a puzzle
        else
            os.remove(puzzle.filename)
            err = result
        end
    end

    task.post(1, {puzzle, err}, download.END)
end

loop_through_queue(download_puzzle, function(t) return t.filename end)
