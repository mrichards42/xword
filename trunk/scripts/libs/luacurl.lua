--- An easier luacurl.
-- @module curl

require 'c-luacurl'
local path = require 'pl.path'
local tablex = require 'pl.tablex'
local makepath = require 'pl.dir'.makepath

-- Get shortcurl names
-- e.g. curl.cookiejar == curl.OPT_COOKIEJAR
local named_opts = {
    write = curl.OPT_WRITEFUNCTION,
    progress = curl.OPT_PROGRESSFUNCTION,
    post = curl.OPT_POSTFIELDS,
}
local function get_curl_opt(key)
    if type(key) == 'string' then
        -- Look for key in named_opts or as a curl.OPT option
        local ukey = key:upper()
        return named_opts[key] or curl[ukey] or curl['OPT_' .. ukey]
    end
end

--- Parse an HTTP error message.
-- @param err An error message returned from curl_easy:perform().
-- @return A numeric HTTP error code.
local function get_http_error(err)
    return tonumber(err:match("The requested URL returned error: (%d+)"))
end

--- Progress function hook.
-- If this is not nil, called on progress.
-- This is an alternate way to specify a progress function.
-- @see curl.get
curl.progress_func = nil

--- Return a progress function
-- If in a secondary thread, make sure to try check_abort on progress.
local function get_progress_function(curlopts)
    local func = curlopts[curl.OPT_PROGRESSFUNCTION]
    local hook = curl.progress_func
    if task and not task.is_main and task.check_abort then
        if func then
            return function(...)
                local ret = func(...)
                local ret2 = hook and hook(...) or 0
                return task.check_abort() and 1 or ret == 0 and ret2 or ret
            end
        else
            return function(...)
                local ret = hook and hook(...) or 0
                return task.check_abort() and 1 or ret
            end
        end
    end
    -- Else we're in the main thread.
    -- Return the original progress function.
    if hook then
        return function(...)
            local ret = func(...)
            local ret2 = hook(...)
            return ret == 0 and ret2 or ret
        end
    else
        return func
    end
end

-- Encode and format a table of post data
local function get_post(curlopts)
    local data = curlopts[curl.OPT_POSTFIELDS]
    if type(data) ~= 'table' then
        return data
    end
    local encoded = {}
    for k,v in pairs(data) do
        -- Accept both { k = v, ... } and { {k,v}, ... }
        -- The second syntax is necessary for duplicate keys.
        if type(v) == 'table' then
            k,v = unpack(v)
        end
        table.insert(encoded, curl.escape(k) .. '=' .. curl.escape(v))
    end
    return table.concat(encoded, '&')
end

--- Download a URL, providing a callback function for the data.
-- @param url The URL
-- @param opts A table mapping curl options to values
-- @return[1] true on success
-- @return [2] nil, error message, curl return code
local function _perform(url, opts)
    -- Setup the cURL object
    local c = curl.easy_init()
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_FOLLOWLOCATION, 1)
    c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors
    c:setopt(curl.OPT_CONNECTTIMEOUT, 60) -- 60 seconds connection timeout
    c:setopt(curl.OPT_LOW_SPEED_TIME, 60) -- Abort after 60 seconds at < 100 bytes per sec
    c:setopt(curl.OPT_LOW_SPEED_LIMIT, 100)
    -- Authentication doesn't seem to work
    c:setopt(curl.OPT_SSL_VERIFYPEER, 0)
    -- Set user-defined options
    for k,v in pairs(opts or {}) do
        c:setopt(k, v)
    end
    -- Run the download
    local rc, err = c:perform()
    c:cleanup()
    -- Check the return code
    if rc == curl.HTTP_RETURNED_ERROR then
        local code = get_http_error(err)
        if code == 404 or code == 410 then
            err = "HTTP error " .. code .. " URL not found"
        else
            err = "HTTP error " .. code
        end
        return nil, err, rc
    end
    if rc == curl.OK then
        return true
    else
        return nil, err, rc
    end
end

--- Download to a file.
-- Internediate directories will be created.
-- @param url The URL
-- @param filename The file to write to
-- @param opts A table mapping curl options to values
-- @return [1] true on success
-- @return [2] nil, error message, curl return code
local function download_to_file(url, filename, opts)
    opts = opts or {}
    -- Open a file
    makepath(path.dirname(filename))
    local f, err = io.open(filename, 'wb')
    if not f then return nil, err end
    -- Callback function: write to a file
    local has_length = false
    opts[curl.OPT_WRITEFUNCTION] = function(str, length)
        if not has_length and length > 0 then has_length = true end
        f:write(str)
        return length -- Return length to continue download
    end
    -- Download
    local success, err, rc = _perform(url, opts)
    -- Cleanup
    f:close()
    if success and not has_length then
        success = false
        err = "Empty file"
    end
    -- Return
    if not success then
        os.remove(filename)
        return nil, err, rc
    else
        return true
    end
end

--- Download and return a string.
-- @param url The URL
-- @param opts A table mapping curl options to values
-- @return [1] The remote file as a string
-- @return [2] nil, error message, curl return code
local function download_to_string(url, opts)
    opts = opts or {}
    local t = {}
    -- Callback: write to the table
    opts[curl.OPT_WRITEFUNCTION] = function(str, length)
        table.insert(t, str)
        return length -- Return length to continue download
    end
    -- Download
    local success, err, rc = _perform(url, opts)
    -- Return the string
    if success then
        return table.concat(t)
    else
        return nil, err, rc
    end
end

--- Perform an http GET.
-- See below for usage and an option table overload.
-- @param url The URL.
-- @param ... A filename, write function, progress function, or table of curl
-- option and value pairs.  
-- If a progress function is passed it *must* follow either a filename or
-- a write function.  If you require other behavior, see below.
-- @function curl.get

--- Perform an http GET.
-- @param opts An option table.
-- @param opts.1 The URL.
-- @param opts.url Alternative to opts.1
-- @param opts.filename A file to save to.
-- @param opts.OPT_XXX additional curl options.  
--   e.g. `opts.OPT_COOKIEJAR`.  Can be given without the prefix as `opts.cookiejar`.
-- @param opts.write Synonym for `OPT_WRITEFUNCTION`.
-- @param opts.progress  Synonym for `OPT_PROGRESSFUNCTION`.
-- @param opts.post Synonym for `OPT_POSTFIELDS`.
--   If this is a table the data *will* be encoded and formatted.
-- @param opts.curlopts A table of curl options.
--   An alternative to the above syntax.
-- @return[1] true
-- @return[2] A string of data if neither filename nor write are given
-- @return[3] nil, error message, curl return code
-- @usage
-- -- Return a string
-- str = curl.get("www.example.com")
--
-- -- Download to a file
-- curl.get("www.example.com", "output.htm")
--
-- -- Save cookies using OPT_COOKIEJAR
-- curl.get{"www.example.com", filename="output.htm", OPT_COOKIEJAR="cookies.txt"}
-- -- . . . using the "cookiejar" key . . .
-- curl.get{"www.example.com", filename="ouput.htm", cookiejar="cookies.txt"}
-- -- . . . or using a curlopts table.
-- local curlopts = { [curl.OPT_COOKIEJAR] = "cookies.txt" }
-- curl.get("www.example.com", "output.htm", curlopts)
--
-- -- Using a custom callback function
-- function print_data(data, length)
--     print(data)
--     return length -- continue downloading
-- end
-- curl.get("www.example.com", print_data)
-- curl.get{"www.example.com", write=print_data}
function curl.get(opts, ...)
    -- Build an opts table if we got multiple arguments
    if type(opts) ~= 'table' then
        opts = {url=opts}
    end
    -- Process varargs
    for _, v in ipairs({...}) do
        local type_ = type(v)
        if type_ == 'table' then
            opts.curlopts = v
        elseif type_ == 'string' then
            opts.filename = v
        elseif type_ == 'function' then
            if opts.filename or opts.write then
                opts.progress = v
            else
                opts.write = v
            end
        end
    end
    -- Read the opts table (and remove options as we go so we don't get errors)
    local url = opts[1] or opts.url;
    local filename = opts[2] or opts.filename;
    -- Make of copy of curlopts since we're going to manipulate it
    -- Copy values from opts table to curlopts.
    -- Keys may be one of the following:
    --   A named opt (e.g. "write"),
    --   A curl option (e.g. "OPT_COOKIEJAR")
    --   Text following OPT_ (e.g. "cookiejar")
    local curlopts = {}
    local function add_opts(t)
        for k,v in pairs(t or {}) do
            if type(k) == 'string' then
                local id = get_curl_opt(k)
                if id then
                    curlopts[id] = v
                else -- We don't want to fail silently
                    error("No cURL option: " .. k, 2)
                end
            elseif type(k) == 'number' then
                curlopts[k] = v
            end
        end
    end
    add_opts(opts.curlopts)
    -- Remove the already processed options
    opts.url = nil; opts.filename = nil; opts.curlopts = nil
    add_opts(opts)
    -- Encode and format post fields
    curlopts[curl.OPT_POSTFIELDS] = get_post(curlopts)
    -- Get the progress function (add task.check_abort if necessary)
    local progress = get_progress_function(curlopts)
    if progress then
        curlopts[curl.OPT_PROGRESSFUNCTION] = progress
        curlopts[curl.OPT_NOPROGRESS] = 0
    end
    -- Only one required argument
    assert(url, "Missing required url argument")
    -- Figure out which variant to call
    local has_write_callback = curlopts[curl.OPT_WRITEFUNCTION]
    if has_write_callback then
        return _perform(url, curlopts)
    elseif filename then
        return download_to_file(url, filename, curlopts)
    else
        return download_to_string(url, curlopts)
    end
end

--- Perform an http POST.
-- See below for usage and an option table overload.
-- @param url The URL.
-- @param post A string or table of post data.
-- @param ... A filename, write function, progress function, or table of curl
-- option and value pairs.
function curl.post(opts, post, ...)
    if type(opts) == 'table' then
        opts.post = opts.post or opts[2]
        opts[2] = nil -- curl.get expects the second opt to be a filename
    else
        opts = {url=opts, post=post}
    end
    return curl.get(opts, ...)
end

--- Perform an http POST.
-- A shortcut for `curl.get` with a `post` parameter. Uses the same options.
-- @param opts An option table.
-- @param opts.1 The URL.
-- @param opts.2 A string or table of post data.
-- @usage
-- curl.post("www.example.com", { name = "John Doe", address = "123 Main St." })
---
-- -- Duplicate keys
-- data = { {"name", "John Doe"}, {"name", "Jane Doe"} }
-- curl.post("www.example.com", data)
-- @function curl.post

return curl
