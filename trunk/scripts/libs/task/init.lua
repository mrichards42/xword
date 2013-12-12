------------------------------------------------------------------------------
-- Modifications to the LuaTask library for wxWidgets.
--
-- Implements an event loop using task.post and task.receive.
--
-- See `task.connect` for a usage example.
--
-- @module task

local _R = (string.match(..., '^.+%.') or ... .. '.') -- Relative require

require 'c-task' -- Load the luatask library
local serialize = require 'serialize'
local path = require 'pl.path'

local Queue = require(_R .. 'queue')

-- These functions will be overridden
local task_post = task.post
local task_receive = task.receive
local task_create = task.create
local task_isrunning = task.isrunning

-- Override the global task table
local _task = task
task = {sleep = _task.sleep, list = _task.list}

--- True once task.EVT_ABORT has been received
task.should_abort = false

-- Events

--- Start of a task
task.EVT_START = -100
--- End of a task.  Any values returned are passed to this handler.
task.EVT_END   = -101
--- Error messages
task.EVT_ERROR = -102
--- Debug messages
task.EVT_DEBUG = -103
--- Posted to abort a task
task.EVT_ABORT = -104


-- Post a message to the task with id, evt_id, and event data
local function _post(id, evt_id, ...)
    local data = serialize({id=_task.id(), data={...}})
    return task_post(id, data, evt_id)
end

-- Receive a message and return task_id, evt_id, a data table
-- Used by task.receive and task.peek
local function _receive(timeout)
    local msg, evt_id, rc = task_receive(timeout)
    if rc ~= 0 then return nil, 'timed out' end
    -- Note abort events
    if evt_id == task.EVT_ABORT then task.should_abort = true end
    -- Deserialize the data
    -- Serialized data must begin with "return "
    local success, data, task_id
    if msg:sub(1, 7) == 'return ' then
        success, data = pcall(loadstring(msg))
    end
    -- If the data cannot be deserialized, report this as an error
    if not success then
        task.error_handler(string.format("unable to deserialize task event data: %q", msg))
        return nil, 'unable to deserialize task event data'
    end
    -- Look for the sending thread's id
    if type(data) == 'table' and data.id then
        task_id = data.id
        data = data.data
    end
    return task_id, evt_id, data
end

if _task.id() == 1 then

-------------------------------------------------------------------------------
--- Main Thread.
-- These functions and fields are only available in the main thread.
-- @section main_thread

local tablex = require 'pl.tablex'

local Task = {} -- Declare the task object
Task.__index = Task

-- Add to the task table
task.Task = Task

--- Is this the main thread?
-- true in the main thread, nil in secondary threads.
task.is_main = true

local next_task = 2

--- Create a new `Task`.
-- If the first character of script is "=", run with `loadstring`, otherwise
-- try `package.loaders`.
-- *Does does not start a new thread.*  
-- If script is a module name, the module directory is added to package.path
-- @param opts Options table or script
-- @param opts.1 Lua string chunk or module name.
-- @param opts.script Alternative to opts[1]
-- @param opts.name[opt] A name for this task
-- @param opts.globals[opt] Table of globals set in the new task.
-- @param opts.events[opt] An event table for this task.
-- @return The `Task` object.
-- @see Task:start
-- @see Task:connect
-- @see Task.globals
-- @usage
-- -- Run a task that posts "Hello world!" to the main thread
-- task.new([[=task.post(MY_CUSTOM_EVT, "Hello world!")]]):start()
--
-- -- Run the threading.long_task script/module in a new thread
-- task.new("threading.long_task"):start()
--
-- -- Create a task with an existing talble
-- my_task = { EVT_CUSTOM = 500, some_key = "some value" }
-- task.new{"task_script", obj=my_task}
-- -- Existing values are preserved
-- print(my_task.EVT_CUSTOM, my_task.some_key) >> 500    "some_value"
function task.new(opts)
    if type(opts) == 'string' then
        opts = { script = opts }
    elseif opts[1] and not opts.script then
        opts.script = opts[1]
    end
    local self = opts.obj or {}
    self._script = opts.script
    self.name = self.name or opts.name or ('task.' .. next_task)
    next_task = next_task + 1
    self.globals = self.globals or opts.globals or {}
    setmetatable(self, Task)
    if opts.events then
        self:connect(opts.events)
    end
    return self
end

-- Create and run a one-off `Task`.
-- See below for usage and an option table overload.
-- @param script The script
-- @param callback A `task.EVT_END` callback.
-- @param args A table of arguments to start the task.
-- @function task.run

--- Create and run a one-off `Task`.
-- *Starts a new thread.*
-- @param opts See `task.new` for most options.
-- @param opts.2 A callback function used for `task.EVT_END`.
-- @param opts.callback Alternative to opt[2].
-- @param opts.args Table of arguments to pass to the task
-- @return The `Task` object.
-- @usage
-- -- Run a task that prints "Hello world!"
-- task.run([[=return "Hello world!"]], print)
function task.run(opts, callback, args)
    if type(opts) == 'table' then
        args = opts.args
        callback = opts[2] or opts.callback
    end
    local self = task.new(opts)
    -- Connect the EVT_END event
    if callback then
        self:connect(task.EVT_END, callback)
    end
    -- Run the task
    self:start(unpack(args or {}))
    return self
end

local TASK_LIST = {}

--- Find a `Task` by its internal id.
-- @param id The task id
-- @return A Task or nil
function task.find(id)
    return TASK_LIST[id]
end

-- Recursively replace keys in t1 with values from t2
local function update(t1, t2)
    for k,v in pairs(t2) do
        if type(v) == 'table' then
            if not t1[k] then t1[k] = {} end
            update(t1[k], v)
        else
            t1[k] = v
        end
    end
end

-- Auto create keys for EVENT_TABLE
local EVENT_TABLE = setmetatable({}, {
    __index = function(t, k)
        t[k] = setmetatable({}, getmetatable(t))
        return t[k]
    end})

--- Connect an event or an event table to a task.
-- Also a metamethod of `Task` objects
-- @param key The `Task` object or wx.wxID_ANY for all tasks
-- @param[opt] evt_id The event id, if connecting a single callback
-- @param callback The callback function, or a table mapping event ids to
--   callback functions
-- @usage
-- local MY_CUSTOM_EVT = 100
-- local MY_CUSTOM_EVT2 = 101
-- local the_task = task.new("my_module.my_long_task")
--
-- -- Connect an event
-- the_task:connect(MY_CUSTOM_EVT, function() print('hello world') end)
--
-- -- Start a task and connect all events
-- the_task:connect({
--     -- Connect one callback function to an event
--     [task.EVT_START] = function() print('starting task') end
--     [task.EVT_END] = function() print('task complete') end,
--     -- Data from `task.post` are passed as arguments to callbacks
--     [MY_CUSTOM_EVT] = function(str, num) print('My event', str, num) end
--     -- Connect several callback functions to an event
--     [MY_CUSTOM_EVT2] = {
--         function() print('Custom event 2') end,
--         function() print('Another callback') end,
--     },
-- })
-- the_task:start()
--
-- -- Connect to debug events from any thread
-- task.connect(wx.wxID_ANY, task.EVT_DEBUG,
--              function(msg) print('debug:', task.evt_task.name, msg) end)
function task.connect(key, evt_id, callback)
    local events = EVENT_TABLE[key]
    if evt_id and callback then -- Single callback
        table.insert(events[evt_id], callback)
    else -- Event table
        local event_table = evt_id
        for evt_id, callbacks in pairs(event_table) do
            if type(callbacks) ~= 'table' then
                table.insert(events[evt_id], callbacks)
            else
                tablex.insert_values(events[evt_id], callbacks)
            end
        end
    end
end

--- Remove an event handler from a task.
-- Also a metamethod of Task objects
-- If no callback is given, remove all callbacks for the event.
-- If no evt_id, remove callback from all events.
-- If neither is given, remove all callbacks for this event
-- @param key The Task object or wx.wxID_ANY for all tasks
-- @param[opt] evt_id The message type flag
-- @param callback[opt] The callback function
function task.disconnect(key, evt_id, callback)
    if not (evt_id or callback) then
        EVENT_TABLE[key] = nil
        return
    end
    local events = EVENT_TABLE[key]
    -- Single callback overload
    if not callback and type(evt_id) == 'function' then
        callback = evt_id
        evt_id = nil
    elseif evt_id then -- If we have an evt_id, just search its callback table
        events = { events[evt_id] }
    end
    -- Search through event tables and remove the callback function
    for _, callbacks in pairs(events) do
        if callback then
            -- Search for the callback and remove it
            local i = tablex.find(callbacks, callback)
            if i then table.remove(callbacks, i) end
        else -- No callback argument: clear all callbacks from this event
            tablex.clear(callbacks)
        end
    end
end

--- A table of globals set for all tasks.
task.globals = {}

--- The function called to handle error messages.
-- Defaults to `print`.
task.error_handler = print

--- The function called to handle debug messages.
-- Defaults to `print`.
task.debug_handler = print

--- The `Task` object that posted the current event.
--- @field task.evt_task

--- The id of the current event.
-- @field task.evt_id

-- Process an event.  Used in the EVT_IDLE handler below
local function _process_event(t, evt_id, data)
    -- Set current task and evt for task.evt_id/task.evt_task
    task.evt_task = t
    task.evt_id = evt_id
    -- Find callbacks for this task/event combination
    -- Include wxID_ANY tasks/events
    local event_table_list = {
        EVENT_TABLE[task.evt_task][evt_id],
        EVENT_TABLE[task.evt_task][-1],
        EVENT_TABLE[-1][evt_id],
        EVENT_TABLE[-1][-1]
    }
    for _, callbacks in ipairs(event_table_list) do
        for _, callback in ipairs(callbacks) do
           callback(unpack(data))
        end
    end
    task.evt_task = nil
    task.evt_id = nil
end

-- Insert a message loop as an idle event
wx.wxGetApp():Connect(wx.wxEVT_IDLE, function()
    -- Check for messages
    while true do
        local task_id, evt_id, data = _receive(0)
        if not task_id then break end
        if evt_id ~= task.EVT_DEBUG then
            print(task.find(task_id).name, evt_id, serialize.pprint(data))
        end
        _process_event(task.find(task_id), evt_id, data)
    end
end)

-------------------------------------------------------------------------------
--- The Task object.
-- @type Task

function Task:__tostring()
    return string.format('Task (%q).', tostring(self.name or self.id))
end

--- The internal task id.
-- @field Task.id

--- An optional user-readable name for the task.
-- @field Task.name Defaults to the task's id

--- The table of globals set for the task.
-- @field Task.globals

-- Get the filename of task_script.lua which is passed to _task.create
local CREATE_SCRIPT = path.package_path(_R .. 'task_create')

--- Start the task in a new thread.
-- If a task has completed, it can be restarted with this method.
-- @param ... Arguments passed to the new task.
-- @return[1] true on success
-- @return[2] nil
-- @return[2] error code
function Task:start(...)
    if self:is_running() then return true end
    -- Setup task globals
    local globals = {}
    update(globals, task.globals)
    update(globals, self.globals)
    -- Add package.path and package.cpath
    update(globals, {package = {path = package.path, cpath = package.cpath}})
    -- Create the task
    local id = task_create(CREATE_SCRIPT, {
        serialize(globals), self._script, serialize({...})
    })
    -- Return the error code
    if id < 0 then
        return nil, id
    end
    -- Setup the task id and name
    TASK_LIST[id] = self
    self.id = id
    -- If we have queued events, post them
    if self._post_queue then
        for data in self._post_queue:iter() do
            self:post(unpack(data))
        end
        self._post_queue = nil
    end
    return true
end

--- Post an event to the task.
-- @param evt_id The (user-defined) event id.
-- @param ...  data.  (numbers, strings, or tables)
function Task:post(evt_id, ...)
    if self.id then
        return _post(self.id, evt_id, ...)
    else
        -- If the task has not started, add events to a queue.
        if not self._post_queue then
            self._post_queue = Queue()
        end
        self._post_queue:push({evt_id, ...})
    end
end

--- Simulate an event being sent to each event handler.
-- @param evt_id The (user-defined) event id.
-- @param ...  data.  (numbers, strings, or tables)
function Task:send_event(evt_id, ...)
    _process_event(self, evt_id, {...})
end

--- Connect event handlers to a task.
-- @function Task:connect
-- @see task.connect
-- @param[opt] evt_id The event id, if connecting a single callback
-- @param callback The callback function, or a table mapping event ids to
--   callback functions
Task.connect = task.connect

--- Remove an event handler from a task.
-- @function Task:disconnect
-- @see task.disconnect
-- @param[opt] evt_id The message type flag
-- @param callback[opt] The callback function
Task.disconnect = task.disconnect

--- Is this task currently running?
-- @return true/false
function Task:is_running()
    return self.id and task_isrunning(self.id)
end

--- Request that a task be aborted.
-- The task is resposible for aborting itself by calling `task.check_abort`.
function Task:abort()
    self:post(task.EVT_ABORT)
end

--- @section end

-- Report errors and debug messages
task.connect(wx.wxID_ANY, task.EVT_ERROR, function(msg)
    task.error_handler(string.format('Task Error (%s): %s', task.evt_task.name, msg))
end)

task.connect(wx.wxID_ANY, task.EVT_DEBUG, function(msg)
    task.debug_handler(string.format('(%s): %s', task.evt_task.name, msg))
end)

-- Cleanup when the task ends:
-- Remove from TASK_LIST
-- Remove task.id so we can't post to this task
task.connect(wx.wxID_ANY, task.EVT_END, function()
    local t = task.evt_task
    TASK_LIST[t.id] = nil
    t.id = nil
end)

else -- task.id() ~= 1

-------------------------------------------------------------------------------
--- Secondary Thread.
-- These functions are not available in the main thread.
-- @section thread_functions

-- task.is_main is nil

--- Post an event to the main task.
-- @param evt_id The (user-defined) event id.
-- @param ...  data.  (numbers, strings, or tables)
function task.post(evt_id, ...)
    _post(1, evt_id, ...)
end

local peeked = Queue() -- Messages that have gone through peek but not receive

--- Receive an event from the main task.
-- @param[opt=forever] timeout Timeout in milliseconds; -1 = forever. 
-- @return Sending task's id
-- @return[1] Event id
-- @return[1] A table of event data
-- @return[2] nil
-- @return[2] error message
-- @see task.post
function task.receive(timeout)
    -- Look at peeked first
    local data = peeked:pop()
    if data then return unpack(data) end
    -- Otherwise we have to actually recieve
    return _receive(timeout)
end

--- Get at an event but leave it in the queue.
-- @return same as `task.receive`
function task.peek()
    -- Look at peeked first
    if not peeked:empty() then
        return unpack(peeked:top())
    end
    -- Receive and place into peeked queue
    local data = {_receive(0)}
    if not data[1] then return end
    peeked:push(data)
    return unpack(data)
end

--- Search the queue for an event.
-- Messages will be left in the peeked queue
-- @see task.check_abort
local function has_message(timeout, evt_id)
    -- Start with already peeked messages
    for data in peeked:iter() do
        if data[2] == evt_id then return true end
    end
    -- Receive messages and put them in the peeked queue
    while true do 
        local data = {_receive(timeout)}
        if not data[1] then return end -- No message
        peeked:push(data)
        timeout = 0 -- Only use a timeout the first time
        if data[2] == evt_id then return true end
    end
end

--- Are we being asked to abort?
-- @param[opt=0] timeout Check timeout
-- @param[opt] cleanup A fuction to execute before aborting
-- @return true/false
function task.check_abort(timeout, cleanup)
    -- Check for abort anywhere in the queue
    if task.should_abort or has_message(timeout or 0, task.EVT_ABORT) then
        if cleanup then
            cleanup()
        end
        return true
    end
end

-- Format a string for task.debug and task.error
local function format_string(fmt, ...)
    if type(fmt) == 'string' then
        if select('#', ...) > 0 then
            return string.format(fmt, ...)
        else
            return fmt
        end
    else
        return serialize.pprint(fmt)
    end
end

--- Report a task error.
-- Posts to the main task if called from a running task.
-- @param fmt The message.
-- @param ... Arguments passed to `string.format`
function task.error(fmt, ...)
    _post(1, task.EVT_ERROR, format_string(fmt, ...))
end

--- Log a debug message from a task.
-- Posts to the main task if called from a running task.
-- @param fmt The message.
-- @param ... Arguments passed to `string.format`
function task.debug(fmt, ...)
    _post(1, task.EVT_DEBUG, format_string(fmt, ...))
end

--- Load a script inside a task.
-- If the first character of script is "=", use `loadstring`, otherwise
-- try `package.loaders`.
-- If script is a module name, the module directory is added to package.path.
-- @param script A module name, or "=" followed by a string chunk.
function task.load(script)
    -- Try loadstring
    if script:sub(1,1) == '=' then
        -- Remove the equals sign and load the string
        return loadstring(script:sub(2))
    end
    -- Try package.loaders
    local errors = {}
    for _, loader in ipairs(package.loaders) do
        local success, result = pcall(loader, script)
        if not success then return nil, result end
        if type(result) == "function" then
            -- Add module directory to package.path
            local p = path.package_path(script)
            if p then
                local dir = path.dirname(p)
                package.path = path.join(dir, '?.lua') .. ';' ..
                               path.join(dir, '?', 'init.lua') .. ';' ..
                               package.path
            end
            -- Return the loader
            return result
        else
            table.insert(errors, result)
        end
    end
    -- Return nil, errors
    return nil, table.concat(errors, '\n')
end

end -- task.is_main / not task.is_main

return task