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

-- Download a file
function download.download(url, filename, opts)
    assert(url and filename)

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

    -- Setup the cURL object
    local c = curl.easy_init()
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_FOLLOWLOCATION, 1)
    c:setopt(curl.OPT_WRITEFUNCTION, write_to_file)
    c:setopt(curl.OPT_PROGRESSFUNCTION, progress)
    c:setopt(curl.OPT_NOPROGRESS, 0)
    c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors

    -- Set user-defined options
    for k,v in pairs(opts or {}) do c:setopt(k, v) end

    -- Run the download
    rc, err = c:perform()
    -- Cleanup
    f:close()

    -- Check the return code
    if rc ~= 0 then
        os.remove(filename)
        if rc == 22 then -- CURLE_HTTP_RETURNED_ERROR
            local code = get_http_error(err)
            if code == 404 or code == 410 then
                err = "URL not found: "..url
            end
        elseif rc == 42 then -- abort from the progress function
            return 'abort', err
        end
        return false, err
    else
        return true
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

    if success == 'abort' then return 'abort' end

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
