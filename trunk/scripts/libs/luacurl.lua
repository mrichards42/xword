--- An easier luacurl.
-- @module curl

require 'c-luacurl'
local path = require 'pl.path'
local makepath = require 'pl.dir'.makepath

--- Parse an HTTP error message.
-- @param err An error message returned from curl_easy:perform().
-- @return A numeric HTTP error code.
local function get_http_error(err)
    return tonumber(err:match("The requested URL returned error: (%d+)"))
end

--- Download a URL, providing a callback function for the data.
-- @param url The URL
-- @param opts A table mapping curl options to values
-- @return[1] true on success
-- @return [2] nil, curl return code, error message
local function _perform(url, opts)
    -- Setup the cURL object
    local c = curl.easy_init()
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_FOLLOWLOCATION, 1)
    c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors
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
    if rc == 22 then -- CURLE_HTTP_RETURNED_ERROR
        local code = get_http_error(err)
        if code == 404 or code == 410 then
            err = "HTTP error " .. code .. " URL not found"
        else
            err = "HTTP error " .. code
        end
        return nil, rc, err
    else
        return true
    end
end

--- Download to a file.
-- Internediate directories will be created.
-- @param url The URL
-- @param filename The file to write to
-- @param opts A table mapping curl options to values
-- @return [1] true on success
-- @return [2] nil, curl return code, error message
local function download_to_file(url, filename, opts)
    opts = opts or {}
    -- Open a file
    makepath(path.dirname(filename))
    local f, err = io.open(filename, 'wb')
    if not f then return nil, err end
    -- Callback function: write to a file
    opts[curl.OPT_WRITEFUNCTION] = function(str, length)
        f:write(str)
        return length -- Return length to continue download
    end
    -- Download
    local success, rc, err = _perform(url, opts)
    -- Cleanup
    f:close()
    -- Return
    if not success then
        os.remove(filename)
        return nil, rc, err
    else
        return true
    end
end

--- Download and return a string.
-- @param url The URL
-- @param opts A table mapping curl options to values
-- @return [1] The remote file as a string
-- @return [2] nil, curl return code, error message
local function download_to_string(url, opts)
    opts = opts or {}
    local t = {}
    -- Callback: write to the table
    opts[curl.OPT_WRITEFUNCTION] = function(str, length)
        table.insert(t, str)
        return length -- Return length to continue download
    end
    -- Download
    local success, rc, err = _perform(url, opts)
    -- Return the string
    if success then
        return table.concat(t)
    else
        return nil, rc, err
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
-- @param opts.write `function(data, length)` called as data is read.
--   Return `length` to continue.
--   Alias for `OPT_WRITEFUNCTION`.
-- @param opts.progress `function(dltotal, dlnow, uptotal, upnow)`
--   called periodically. Return 0 to continue.
--   Alias for `OPT_PROGRESSFUNCTION`.
-- @param opts.post A string or an associative table of post data.
--   Alias for `OPT_POSTFIELDS`.
--   Unlike cURL, if a table is sent, the data *will* be encoded and formatted.
-- @param opts.curlopts A table of curl options. An alternative `opts.OPT_XXX`.
-- @return[1] true
-- @return[2] A string of data if neither filename nor write are given
-- @return[3] nil, curl return code, error message
-- @usage
-- -- Return a string
-- str = curl.get("www.example.com")
--
-- -- Download to a file
-- curl.get("www.example.com", "output.htm")
--
-- -- Save cookies
-- curl.get{"www.example.com",
--               filename="output.htm",
--               OPT_COOKIEJAR="cookies.txt"}
-- -- or
-- local curlopts = { [curl.OPT_COOKIEJAR] = "cookies.txt" }
-- curl.get("www.example.com", "output.htm", curlopts)
-- curl.get{"www.example.com", filename="ouput.htm", curlopts=curlopts}
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
    end
    -- Read the opts table
    local url = opts[1] or opts.url
    local filename = opts.filename
    local curlopts = opts.curlopts or {}
    -- Named options
    if opts.write then
        curlopts[curl.OPT_WRITEFUNCTION] = opts.write
    end
    if opts.progress then
        curlopts[curl.OPT_PROGRESSFUNCTION] = opts.progress
    end
    if opts.post then
        curlopts[curl.OPT_POSTFIELDS] = opts.post
    end
    -- Copy opts.OPT_XXX to curlopts[curl.OPT_XXX]
    for k,v in pairs(opts) do
        if curl[k] then
            curlopts[curl[k]] = v
        elseif tostring(k):sub(1,4) == "OPT_" then
            error("No curl option: " .. k, 2)
        end
    end
    -- Set NOPROGRESS to 0 if we have a progess function
    if curlopts[curl.OPT_PROGRESSFUNCTION] then
        curlopts[curl.OPT_NOPROGRESS] = 0
    end
    -- Encode and format post fields
    local post = curlopts[curl.OPT_POSTFIELDS]
    if post and type(post) == 'table' then
        local encoded = {}
        for k,v in pairs(post) do
            -- Accept both { k = v, ... } and { {k,v}, ... }
            -- The second syntax is necessary for duplicate keys.
            if type(v) == 'table' then
                k,v = unpack(v)
            end
            table.insert(encoded, curl.escape(k) .. '=' .. curl.escape(k))
        end
        curlopts[curl.OPT_POSTFIELDS] = table.concat(encoded, '&')
    end
    -- If this is a secondary thread, check for abort on progress
    if task and not task.is_main and task.check_abort then
        if progress then
            curlopts[curl.OPT_PROGRESSFUNCTION] = function(...)
                return task.check_abort() and 1 or progress(...)
            end
        else
            curlopts[curl.OPT_PROGRESSFUNCTION] = function()
                return task.check_abort() and 1 or 0
            end
        end
    end
    -- Only one truly required argument
    assert(url)
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
function curl.post(url, post, ...)
    if type(opts) == 'table' then
        opts.post = opts.post or opts[2]
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