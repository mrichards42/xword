------------------------------------------------------------------------------
-- Modifications to the LuaTask library for wxWidgets.
--
-- Implements an event loop using task.post and task.receive.
--
-- See `task.new` for a usage example.
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
local task_list = task.list

-- Override the global task table
local _task = task
task = {sleep = _task.sleep}

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

local assert_arg = require 'pl.utils'.assert_arg

-- Post a message to the task with id, evt_id, and event data
local function _post(id, evt_id, ...)
    assert_arg(1, evt_id, 'number', nil, nil, 4) -- Make sure this is a number
    local data = serialize({id=task.id, data={...}})
    return task_post(id, data, evt_id)
end

-- Deserialize a message from task.receive
local function _deserialize_message(evt_id, msg)
    -- Note abort events
    if evt_id == task.EVT_ABORT then task.should_abort = true end
    -- Deserialize the data
    local data = serialize.loadstring(msg)
    -- If the data cannot be deserialized, report this as an error
    if not data then
        task.error_handler(("unable to deserialize task data: %q"):format(msg))
        return nil, 'unable to deserialize task data'
    end
    -- If we got the wrong info, report an error
    if type(data) ~= 'table' or not data.id then
        task.error_handler(("incorrect task data format: %q"):format(msg))
        return nil, 'incorrect task data format'
    end
    return data.id, evt_id, data.data
end

-- Receive a message and return task_id, evt_id, a data table
-- Used by task.receive and task.peek
local function _receive(timeout)
    local msg, evt_id, rc = task_receive(timeout)
    if rc ~= 0 then return nil, 'timed out' end
    return _deserialize_message(evt_id, msg)
end

if _task.id() == 1 then

-------------------------------------------------------------------------------
--- Main Thread.
-- These functions and fields are only available in the main thread.
-- @section main_thread

local tablex = require 'pl.tablex'
local EvtHandler = require(_R..'event')

local Task = {} -- Declare the task object
Task.__index = Task
setmetatable(Task, EvtHandler)

-- Add to the task table
task.Task = Task

-- Set the internal task id.  Only used for _post
task.id = 1

--- Is this the main thread?
-- true in the main thread, nil in secondary threads.
task.is_main = true

local NEXT_ID = 2
local TASK_LIST = {} -- {Task.id = Task}
local TASK_ID_LIST = {} -- {Task._task_id = Task}

--- List currently running Tasks.
-- Does not list the main thread.
-- @return a list of `Task` objects for currently running tasks.
function task.list()
    local tasks = task_list()
    -- Replace task info with actual Task objects
    for id=2,#tasks do
        tasks[id] = TASK_ID_LIST[id]
    end
    -- Remove the main task from this list
    table.remove(tasks, 1)
    return tasks
end

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
-- @param opts.obj[opt] An object to use as the table for this task.
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
    -- Create a new id.
    -- This is different from the id returned from task_create, and is always
    -- unique.
    self.id = NEXT_ID
    NEXT_ID = NEXT_ID + 1
    self.name = self.name or opts.name or ('task.' .. self.id)
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

--- A table of globals set for all tasks.
task.globals = {}

--- The function called to handle error messages.
-- Defaults to `print`.
task.error_handler = print

--- The function called to handle debug messages.
-- Defaults to `print`.
task.debug_handler = print

-- The task event loop.
-- xword creates an event xword.EVT_LUATASK that is used for passing messages
-- from secondary threads to the main thread.  This allows immediate processing
-- of events, whereas the old EVT_IDLE implementation required the user to
-- generate events in order to complete processing.
-- Note that the main thread still uses the standard task.post to send messages,
-- and secondary threads still use task.receive to get messages.
wx.wxGetApp():Connect(xword.EVT_LUATASK, function(evt)
    -- evt.Int == event id, evt.String == event message
    local id, evt_id, data = _deserialize_message(evt:GetInt(), evt:GetString())
    if not id then return end -- No message
    -- Find the task and process events
    local t = TASK_LIST[id]
    if t == nil then
        local msg = {}
        table.insert(msg, ("No task.  Task id: %d, evt id: %d, data: %s"):format(id, evt_id, serialize.pprint(data)))
        table.insert(msg, "Main running? " .. (wx.wxGetApp():IsMainLoopRunning() and "true" or "false"))
        table.insert(msg, "TASK_LIST = " .. serialize.pprint(TASK_LIST))
        table.insert(msg, "TASK_ID_LIST = " .. serialize.pprint(TASK_ID_LIST))
        task.error_handler(table.concat(msg, '\n'))
    else
        t:send_event(evt_id, unpack(data))
    end
end)

-------------------------------------------------------------------------------
--- The Task object.
-- @type Task

function Task:__tostring()
    return ('%s (%s Task).'):format(
        tostring(self.name),
        self:is_running() and 'Running' or 'Not Running'
    )
end

--- The internal task id.
-- @field Task.id

--- An optional user-readable name for the task.
-- @field Task.name Defaults to the task's id

--- The table of globals set for the task.
-- @field Task.globals

-- Get the filename of task_script.lua which is passed to _task.create
local CREATE_SCRIPT = path.package_path(_R .. 'task_create')

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
        serialize(globals), self.id, self._script, serialize({...})
    })
    -- Return the error code
    if id < 0 then
        return nil, id
    end
    -- If there is another task with this id it has completed.
    -- Remove _task_id so that we can't post to it any more.
    local dead_task = TASK_ID_LIST[id]
    if dead_task then
        dead_task._task_id = nil
        TASK_ID_LIST[id] = nil
    end
    -- Set the internal id
    self._task_id = id
    -- Add to the global TASK_LISTs
    TASK_LIST[self.id] = self
    TASK_ID_LIST[self._task_id] = self
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
    if self:is_running() then
        return _post(self._task_id, evt_id, ...)
    else
        -- If the task has not started, add events to a queue.
        if not self._post_queue then
            self._post_queue = Queue()
        end
        self._post_queue:push({evt_id, ...})
    end
end

--- Simulate an event being sent to each event handler.
-- @function Task:send_event
-- @param evt_id The (user-defined) event id.
-- @param ... Data.  (numbers, strings, or tables)

--- Connect event handlers to a task.  
-- NB: Use extreme caution to avoid race conditions when wx.wxYield is called
--   from a callback function.  See usage for an example.
-- @function Task:connect
-- @see EvtHandler.connect
-- @param[opt] evt_id The event id, if connecting a single callback.
-- @param callback The callback function, or a table mapping event ids to
--   callback functions.
-- @usage
-- -- Careful! The following code may produce unexpected results.
-- t = Task.new([[=while true do task.post(EVT_START) task.post(EVT_END) end]])
-- t:connect(EVT_START, function() status = 'start' print(status) end)
-- t:connect(EVT_END, function() status = 'end' wx.wxYield() print(status) end)
-- task:run()
-- -- Expected result:
-- -- start  end  start  end  start  end
-- -- Possible result:
-- -- start  end  start  start  start  end  start  start  start  start  start
-- -- Since wx.wxYield() processes messages in the queue (including task.post)
-- -- status may be either 'start' or 'end' after the wx.wxYield() call.

--- Remove an event handler from a task.
-- @function Task:disconnect
-- @see EvtHandler.disconnect
-- @param[opt] evt_id The event id.
-- @param callback[opt] The callback function

--- Is this task currently running?
-- @return true/false
function Task:is_running()
    return self._task_id and task_isrunning(self._task_id)
end

--- Request that a task be aborted.
-- The task is resposible for aborting itself by calling `task.check_abort`.
function Task:abort()
    self:post(task.EVT_ABORT)
end

--- @section end

-- Report errors and debug messages
EvtHandler.connect(wx.wxID_ANY, task.EVT_ERROR, function(msg)
    task.error_handler(('Task Error (%s): %s'):format(EvtHandler.evt_handler.name, msg))
end)

EvtHandler.connect(wx.wxID_ANY, task.EVT_DEBUG, function(msg)
    task.debug_handler(('(%s): %s'):format(EvtHandler.evt_handler.name, msg))
end)

-- Cleanup when the task ends:
-- Remove from TASK_LISTs
-- Remove _task_id so we can't post to this task
EvtHandler.connect(wx.wxID_ANY, task.EVT_END, function()
    local t = EvtHandler.evt_handler
    t:disconnect() -- Remove event handlers
    TASK_LIST[t.id] = nil
    if t._task_id then
        TASK_ID_LIST[t._task_id] = nil
    end
    t._task_id = nil
end)

else -- _task.id() ~= 1

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

-- Override print for secondary threads
print = task.debug

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