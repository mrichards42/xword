-- The main puzzle downloading task
local os = require 'os'
local date = require 'date'
local curl = require 'luacurl'
local puz = require 'luapuz'
require 'import' -- Additional puzzle file types

-- Messages
local mgr = require 'download.manager'

-- Expose pl.path for custom functions
path = require 'pl.path'

-- Allow custom functions to set the statusbar text in the download manager
function set_status(text)
    task.post(mgr.EVT_UPDATE_STATUS, text)
end

-- ---------------------------------------------------------------------------
-- Custom Download Function API
-- ---------------------------------------------------------------------------

--- Prompt the user for information.
-- @param msg The dialog message.
-- @param ... Field names
-- @return A table mapping field names to values
function prompt(msg, ...)
    task.post(mgr.PROMPT, msg, ...)
    return wait_for_message(mgr.PROMPT)
end

--- Check if this is a valid puzzle.
-- @param filename
-- @return true or nil, error
function check_puzzle(filename)
    if puz.Puzzle.CanLoad(filename) then
        local success, result = pcall(puz.Puzzle, filename)
        if success then
            result:__gc()
        else
            return nil, result
        end
    end
    return true
end

local CURRENT_PUZZLE
--- Get the cookie file for this puzzle.
-- @param[opt] puzzle The puzzle table, or CURRENT_PUZZLE
-- @return The cookie filename
function get_cookie_file(puzzle)
    puzzle = puzzle or CURRENT_PUZZLE
    assert(puzzle.auth and puzzle.auth.url, "Missing authentication url")
    -- Use authentication url as the cookie file, but strip invalid chars
    local name = puzzle.auth.url:gsub('[?<>:/\\%*|"%%\t\r\n\v\f]+', ".")
    return path.join(xword.configdir, 'download', name .. ".txt")
end

--- Does this puzzle have the associated cookie file?
-- @param[opt] puzzle The puzzle table, or CURRENT_PUZZLE
-- @return true/false
function has_cookie(puzzle)
    puzzle = puzzle or CURRENT_PUZZLE
    -- Look for the cookies
    local f = io.open(get_cookie_file(puzzle), 'rb')
    if not f then return false end
    local text = f:read("*a")
    f:close()
    return #text > 1
end

local function get_login_form(page, puzzle)
    local auth = (puzzle or CURRENT_PUZZLE).auth
    -- Find the login form
    for form in page:gmatch("<form(.-)</form>") do
        -- Check method == "POST"
        local method = form:match('method%s*=%s*"(.-)"')
        if method and method:upper() == "POST" then
            -- Get action url (or use the page url)
            local action = form:match('action%s*=%s*"(.-)"')
            if not action or action:sub(1,1) == "#" then
                action = auth.url
            end
            -- Look for the <inputs> and <selects> and keep default values (except username/password)
            local has_user = false -- Make sure this is the login form
            local has_pw = false
            local postdata = {}
            for input in form:gmatch("<input([^>]*)>") do
                local name = input:match('name%s*=%s*"(.-)"')
                if name == auth.user_id then
                    has_user = true
                elseif name == auth.password_id then
                    has_pw = true
                elseif name then
                    local value = input:match('value%s*=%s*"(.-)"')
                    if value then
                        table.insert(postdata, {name, value})
                    end
                end
            end
            for s in form:gmatch("<select(.-)</select>") do
                local name = s:match('name%s*=%s*"(.-)"')
                if name then
                    for option in s:gmatch("<option([^>]*)>") do
                        if option:match('selected') then
                            local value = option:match('value%s*=%s*"(.-)"')
                            if value then
                                table.insert(postdata, {name, value})
                            end
                        end
                    end
                end
            end
            if has_user and has_pw then
                return action, postdata
            end
        end
    end
end

-- The actual login function
-- See function login below for the public memorized version
local function _login(puzzle, msg)
    local auth = puzzle.auth
    -- Prompt for username and password
    local authdata = prompt((msg and ('*** '..msg..' ***\n') or '') .. 'Login for ' .. puzzle.name, 'Username', 'Password')
    if not authdata or type(authdata) ~= 'table' then
        return nil, msg or "Username or password not specified"
    elseif authdata.Username == '' or authdata.Password == '' then
        return _login(puzzle, "Username or password not specified")
    end
    -- Setup cookie options
    local cookie_file = get_cookie_file(puzzle)
    --os.remove(cookie_file) -- Start with a clean cookie file
    local opts = puzzle.curlopts or {}
    opts[curl.OPT_COOKIEJAR] = cookie_file -- Save cookies
    opts[curl.OPT_COOKIEFILE] = cookie_file -- Read cookies
    -- Download authentication page
    local page, err = curl.get(auth.url, opts)
    if not page then return nil, err end
    -- Find the login form
    local action, postdata = get_login_form(page, puzzle)
    if not action then
        return nil, "Could not find login form"
    end
    -- Add authentication info
    table.insert(postdata, {auth.user_id, authdata.Username})
    table.insert(postdata, {auth.password_id, authdata.Password})
    -- Send the POST request and save the cookie
    local str, err = curl.post(action, postdata, opts)
    if not str then
        return _login(puzzle, "HTTP POST error: " .. err)
    elseif get_login_form(str) or not has_cookie(puzzle) then
        return _login(puzzle, "Incorrect username or password")
    end
    -- Read the cookies file and remove expiration times
    local f = io.open(cookie_file, 'rb')
    if f then
        local text = {}
        for line in f:lines() do
            line = line:gsub("(.-)\t(.-)\t(.-)\t(.-)\t(.-)\t(.-)\t(.-)",
                              "%1\t%2\t%3\t%4\t0\t%6\t%7")
            table.insert(text, line)
        end
        f:close()
        -- Write the new cookies file
        f = io.open(cookie_file, 'wb')
        f:write(table.concat(text, '\n'))
        f:close()
    end
    return true
end

local LOGIN_RESULT = {} -- puzzle_id = true/false

-- Clear logins on task.CLEAR
local function on_clear()
   LOGIN_RESULT = {}
end

-- Clear specific saved logins
local function on_relogin(items)
    for _, id in ipairs(items) do
        LOGIN_RESULT[id] = nil
    end
end

--- Login for a subscription crossword.
-- @param[opt] puzzle The puzzle table, or CURRENT_PUZZLE
-- @param[opt] msg A message to show in the dialog
-- Uses puzzle.auth fields.
function login(puzzle, msg)
    -- A wrapper memorization function for _login so the user isn't bugged
    -- about logging in multiple times.
    puzzle = puzzle or CURRENT_PUZZLE
    local result = LOGIN_RESULT[puzzle.id]
    if result == true then
        return true
    elseif result then
        return nil, result
    else
        local success, err = _login(puzzle, msg)
        LOGIN_RESULT[puzzle.id] = success and true or err
        return success, err
    end
end


-- ---------------------------------------------------------------------------
-- QueueTask Functions
-- ---------------------------------------------------------------------------

-- Use this custom function in place of assert for custom download functions
-- so that the custom function can call, for instance, assert(curl.get())
-- and the error message will not have location information attached
local ASSERT_ERROR
local function myassert(v, msg, ...)
    if v then return v, msg, ... end
    ASSERT_ERROR = msg
    -- Error to jump out of this function
    error(msg)
end

-- Automatically clean up puzzles that are created during downloading
local mypuz = {}
local _puzzles = {}
setmetatable(mypuz, {__index = puz})
function mypuz.Puzzle(...)
    local p = puz.Puzzle(...)
    table.insert(_puzzles, p)
    return p
end

-- Garbage collect puzzles
local function cleanup_puzzles()
    for k, p in pairs(_puzzles) do
        _puzzles[k] = nil
        task.debug('Collecting puzzle:' .. tostring(p))
        p:__gc()
    end
end

-- Download a puzzle, return the error code if any
local function download(puzzle)
    if puzzle.func then
        local func, err = loadstring([[return function(puzzle, download, assert, puz) ]]..puzzle.func..[[ end]])
        if err then return err end
        -- Compile this function
        func = func()
        -- We need to have a real date for custom functions
        puzzle.date = date(puzzle.date)
        -- This function can return error, nil + error, or throw an error, in
        -- which case we should differentiate between user-called asserts
        -- (using myassert and ASSERT_ERROR), or an actual programming error.
        ASSERT_ERROR = false
        local success, result, err = xpcall(function() return func(puzzle, curl.get, myassert, mypuz) end, debug.traceback)
        -- gc puzles
        cleanup_puzzles()
        -- Figure out what to return
        if ASSERT_ERROR then
            -- User called assert
            return ASSERT_ERROR
        elseif type(result) == 'string' and not err then
            -- User returned an error string, or this is a programming error
            return result
        else
            -- User returned nil, err OR the function returned true
            return err
        end
    else
        local success, err
        local opts = puzzle.curlopts or {}
        -- Check puzzle authentication
        if puzzle.auth and type(puzzle.auth) == 'table' then
            local cookie_file = get_cookie_file(puzzle)
            -- Add cookie file to curlopts
            opts[curl.OPT_COOKIEFILE] = cookie_file -- Read
            opts[curl.OPT_COOKIEJAR] = cookie_file -- Write
            if has_cookie(puzzle) then
                -- Download the puzle
                success, err = curl.get(puzzle.url, puzzle.filename, opts)
                if not success then return err end
                -- Check the result to make sure it's not an error/login page
                if LOGIN_RESULT[puzzle.id] == true then
                    -- If we have successfuly logged in, don't bother
                    -- checking this puzzle twice.
                    return
                elseif not puzzle.not_puzzle then
                    -- Check that this is a valid puzzle
                    if check_puzzle(puzzle.filename) then
                        -- If we have a good cookie and a good puzzle,
                        -- count this as a successful login.
                        LOGIN_RESULT[puzzle.id] = true
                        return
                    end
                else
                    -- If this is supposed to be a non-puzzle (e.g. PDF),
                    -- check for a login page
                    local f = io.open(puzzle.filename, 'rb')
                    local page = f:read('*a')
                    f:close()
                    -- If there is no login form, assume this is the correct
                    -- file type.
                    if not get_login_form(page) then
                        return
                    end
                end
            end
            -- If we haven't returned, login failed, so try to login.
            success, err = login(puzzle)
            if not success then return err end
        end
        -- Either no auth is necessary, or we have passed authentication.
        success, err = curl.get(puzzle.url, puzzle.filename, opts)
        if not success then return err end
    end
end

-- Send progress events if the dl is taking too long
local LONG_DL_SECS = 1
local time
curl.progress_func = function(dltotal, dlnow)
    if os.clock() - time > LONG_DL_SECS then
        task.post(mgr.EVT_PROGRESS, dltotal, dlnow)
    end
    return 0
end
curl.progress_func = nil -- Turn off for now

-- This function is called for each puzzle in the queue
local function func(puzzle)
    -- Download the puzzle
    time = os.clock()
    CURRENT_PUZZLE = puzzle
    local success, err = xpcall(function() return download(puzzle) end, debug.traceback)
    CURRENT_PUZZLE = nil
    -- Try to open to make sure the puzzle was completely downloaded
    if success and not err and not puzzle.not_puzzle then
        success, err = check_puzzle(puzzle.filename)
    end
    if err then
        -- Remove the puzzle if there was an error
        os.remove(puzzle.filename)
        -- Report errors if any
        return err
    end
end

-- Return main function and event handlers
return {
    func,
    [mgr.CLEAR]=on_clear,
    [mgr.LOGIN]=on_relogin
}