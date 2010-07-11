-- ============================================================================
-- Modifications to the LuaTask library
--     Allow LuaTask to be used with multiple add-ons:
--         - task.post prepends the id of the calling task to the message for
--           identification.
--         - task.receive takes an additional task_id parameter, and will only
--           return messages from the given task.
--           if task_id is omitted or is -1, return the first message from a
--           thread using LuaTask directly (not through this post/receive API).
--
--     This is sort of hackish and should incur a small performance penalty,
--     but otherwise, only one add-on could use LuaTask without the message
--     queues getting tangled.
--
--     Give unique ids to threads: [incrementing number].."wxtask"
--
--     Preserve package.path and package.cpath when creating a new thread.
--
--     Allow threads to tap into the wxWigets event handling system.
-- ============================================================================

require 'task'
require 'wxtask.queue'
require 'serialize'

-- ============================================================================
-- local functions and data
-- ============================================================================

local task_post = task.post
local task_receive = task.receive
local task_create = task.create
local task_id = task.id
local task_isrunning = task.isrunning

-- Queue ids
task.UNUSED_QUEUE = -10
task.UNKNOWN_QUEUE = -1
task.DEBUG_QUEUE = -2
task.ERROR_QUEUE = -3


-- ============================================================================
-- Unique thread ids
-- ============================================================================
-- Functions on the posting end need to convert using find_id
-- Functions on the receiving end need to convert using find_name

task.task_aliases = {}
local task_aliases = task.task_aliases
local last_task_id = 1
local function new_id()
    last_task_id = last_task_id + 1
    return tostring(last_task_id)..".wxtask"
end

local function find_id(name)
    if type(name) == "number" then return name end
    return task.find(name)
end

local function find_name(id)
    if type(id) == "string" then return id end
    return task_aliases[id]
end

local function register_name(id, name)
    task_aliases[id] = name
end

register_name(1, 1) -- main thread is always 1
register_name(task.UNKNOWN_QUEUE, "wxtask.unknown")
register_name(task.DEBUG_QUEUE, "wxtask.debug")
register_name(task.ERROR_QUEUE, "wxtask.error")
register_name(task.UNUSED_QUEUE, "wxtask.unused")

function task.isrunning(id)
    return task_isrunning(find_id(id))
end


-- ============================================================================
-- post functions
-- ============================================================================

-- Data is serialized before being posted.  When reconstructed, the data will
-- appear as follows: { 'id' = wxtask_id, 'data' = the_data }
-- data can be a number, string, or non-cyclic table of any valid data.
task.post = function(id, data, flag)
    return task_post(find_id(id), serialize({id=task.id(), data=data}), flag)
end

-- Post with id 1 to the specified queue.
-- This is used to circumvent custom task.post behavior, which posts with
-- a given id to the queue designated for the current task.
task.post_to_queue = function(queue_id, data, flag)
    return task_post(1, serialize({id=queue_id, data=data}), flag)
end

function task.error(data) task.post_to_queue(task.ERROR_QUEUE, data) end

function task.debug(data) task.post_to_queue(task.DEBUG_QUEUE, data) end



-- ============================================================================
-- receive functions
-- ============================================================================

-- A table that holds the messages for each task as we receive them.
local queues = {}
task.queues = queues

-- Recieve the next message in the queue, but don't remove it
function task.peek(timeout, task_id)
    local task_id = find_name(task_id) or task.UNKNOWN_QUEUE

    -- See if we already have a message in the queue for the specified task
    if not queues[task_id] then queues[task_id] = task.newQueue() end

    local ret = queues[task_id]:get_last()
    -- Return the message if there is one
    if ret then
        return unpack(ret)
    end

    -- No message in the local queue, look to the global task queue.

    -- Find the first message (using the timeout if specified)
    local msg, flags, rc = task_receive(timeout)

    -- Read messages until we get one from the specified task, or we run
    -- out of messages.
    while rc == 0 do
        local success, data
        -- Serialized messages must begin with "return "
        if msg:sub(1, 7) == 'return ' then
            -- Read the serialized message
            success, data = pcall(loadstring(msg))
        end
        -- If the message cannot be deserialized, assume it came from an
        -- unknown thread.
        if type(data) ~= 'table' then
            data = {id = task.UNKNOWN_QUEUE, data = msg}
        end

        -- Add the message to the local queue
        data.id = find_name(data.id)
        if not queues[data.id] then queues[data.id] = task.newQueue() end
        queues[data.id]:push({data.data, flags, rc})

        -- If this message is from the specified thread, return the message
        if data.id == task_id then
            return data.data, flags, rc
        end

        -- Get the next message
        msg, flags, rc = task_receive(0) -- No timeout
    end

    -- If we've gotten here, there are no messages from the specified thread,
    -- and all other messages have been read from the global queue into the
    -- local queue.
    return nil, nil, rc
end

-- Receive the next message in the queue and remove it
function task.receive(timeout, task_id)
    local task_id = find_name(task_id)
    local msg, flags, rc = task.peek(timeout, task_id)
    if rc == 0 then
        queues[task_id]:pop()
    end
    return msg, flags, rc
end

-- Print all messages from a given queue
function task.dump_queue(queue_id)
    task.flush_queues()
    local queue = queues[find_name(queue_id)]
    if not queue then return end
    for val in queue:iter() do
        local msg, flags, rc = unpack(val)
        print(msg)
    end
    task.clear_queue(queue_id)
end

-- Clear all messages from a given queue
function task.clear_queue(queue_id)
    task.queues[find_name(queue_id)]:clear()
end

-- Push all messages into their respecitve queues
function task.flush_queues()
    -- task.receive will read messages until it gets a message from the
    -- given queue / thread or it runs out of messages.  There should never
    -- be messages in the UNUSED_QUEUE, so this just reads until we run
    -- out of all messages.
    msg, flags, rc = task.receive(0, task.UNUSED_QUEUE)
end

-- Flush messages and return the count of messages for a given queue
function task.pending(queue_id)
    task.flush_queues()
    local q = task.queues[find_name(queue_id)]
    if q then return q:length() else return 0 end
end

-- Dump the debug queue
function task.dump_debug()
    print '===============      DEBUG       ==============='
    task.dump_queue(task.DEBUG_QUEUE)
    print '===============   END OF DEBUG   ==============='
end

-- Dump the error queue
function task.dump_errors()
    print '===============      ERRORS      ==============='
    task.dump_queue(task.ERROR_QUEUE)
    print '===============   END OF ERRORS  ==============='
end




-- ============================================================================
-- create thread function
-- ============================================================================

--- Create a new thread.
-- @param s Lua string chunk, module name, or filename.
--     <ul>
--         <li>If the first character of the string is "=" the code will be
--             executed using loadstring.</li>
--         <li>Otherwise the script will be loaded using <code>require</code>, or
--             <code>loadfile</code> if this fails.</li>
--     </ul>
-- @param args A table of arguments passed to the new task.
--     <ul>
--         <li>Arguments are available in the <code>arg</code> table.</li>
--         <li>String keys are available as global variables.</li>
--     </ul>
-- @return The task id.
task.create = function(s, args)
    local task_name = new_id()
    args = args or {}
    assert(type(args) == "table", "args must be a table")
    s = s or ""

    local command = [[=
        local success, err = pcall(function()
        -- Set the package.paths
        package.path = arg[1]
        package.cpath = arg[2]

        require "wxtask"

        -- Set this task's unique id
        function task.id()
            return "]]..task_name..[["
        end
        task.register(task.id())
    ]]

    -- Load the script
    if s:sub(1,1) == '=' then -- Script is a string
        -- Remove the equals sign
        s = s:sub(2)
        command = command .. [[
            -- Load the string that was passed to task.create
           local func, err = loadstring(arg[3])
        ]]
    else -- Script is a file name
        command = command .. [[
            -- Load the file that was passed to task.create

            -- Try to load the file using package.loaders
            local errors = {}
            local func, err
            for _, loader in ipairs(package.loaders) do
                func = loader(arg[3])
                if type(func) == "function" then
                    break
                else
                    table.insert(errors, func)
                end
            end

            -- Try to load the file itself
            if not func then
                func, err = loadfile(arg[3])
                if not func then
                    table.insert(errors, err)
                    err = table.concat(errors, '\n')
                end
            end
        ]]
    end

    -- Run the script with the remaining arguments
    command = command .. [[
            if not func then error(err) end

            -- Deserialize the function arguments
            local success, fargs = pcall(loadstring(arg[4]))

            -- The arg variable is a global variable that was initialized when
            -- the currently executing function started.
            -- We need to make sure that arg holds the arguments to our
            -- function, because we can't call a string or file with arguments.
            arg = fargs

            -- Add string keys to arg as global variables
            for k,v in pairs(arg) do
                -- Protect preexisting globals
                if type(k) == "string" and not _G[k] then
                    _G[k] = v
                end
            end

            -- Execute the script
            task.post(1, nil, task.START)
            func()
            task.post(1, nil, task.END)

        end)-- end of pcall function

        -- Report any errors
        if not success then
            task.error(err)
        end
        ]]

    args[0] = s
    -- Create the task
    local id = task_create(command,
                           {package.path, package.cpath, s, serialize(args)})
    if id > 0 then
        register_name(id, task_name)
        return task_name
    end
    -- If the task did not create, return the error code
    return id
end


-- ============================================================================
-- Frame closing
-- ============================================================================
task.START = -100
task.END   = -101
task.ABORT = -102

if task.id() == 1 then
    function task.abort(id)
        task.post(id, "", task.ABORT)
    end

    local function cleanup()
        local time = 0
        local isMsgShown = false
        while true do
            if not isMsgShown and time > 2000 then
                isMsgShown = true
                if wx then
                    wx.wxBusyInfo("Waiting for threads to complete...")
                end
            end
            -- Give our threads some time to exit before we kill them.
            if time > 5000 then break end
            local shouldBreak = true
            for id, _ in pairs(task.list()) do
                if id ~= 1 then
                    shouldBreak = false
                    print("ABORTING THREAD:"..tostring(id))
                    task.abort(id)
                end
            end
            -- Main thread is the only one left
            if shouldBreak then break end
            -- Wait a bit
            print("SLEEPING 10")
            task.sleep(10)
            time = time + 10
        end
         -- VC thinks that messages left in the queues are memory leaks
        task.flush_queues()
    end
    xword.OnCleanup(cleanup)
else
    function task.checkAbort(timeout, cleanupFunc)
        local function doCheck(msg, flag, rc)
            if rc == 0 and flag == task.ABORT then
                if cleanupFunc then
                    cleanupFunc()
                end
                return true
            end
        end

        -- Check the first message in the queue (with a timeout)
        local msg, flag, rc = task.peek(timeout or 0, 1)
        if doCheck(msg, flag, rc) then return true end

        -- Check all other messages in the queue
        task.flush_queues()
        for val in task.queues[1]:iter() do
            local msg, flag, rc = unpack(val)
            if doCheck(msg, flag, rc) then return true end
        end
    end
end


-- ============================================================================
-- wxWidgets event handling
-- ============================================================================

if wx then
    task.evtHandlers = {}

    -- Create a new event handler to process task events
    function task.newEvtHandler(window)
        local self = wx.wxEvtHandler()
        self.tasks = {}     -- wxtask ids
        self.callbacks = {} -- { [msgId] = {function(data, [id]), ...}, ... }

        local function idleEvent(evt)
            local nTasks = #self.tasks

            -- No more tasks, disconnect this event handler
            if nTasks == 0 then
                print("Disconnect(wx.wxEVT_IDLE): "..tostring(self).." "..tostring(window))
                self:Disconnect(wx.wxEVT_IDLE)
            end

            -- Check the message queue for each task and dispatch events
            -- Iterate backwards so that we can remove dead tasks
            for i=nTasks,1,-1 do
                local id = self.tasks[i]
                local isRunning = task.isrunning(id)
                local data, flag, rc = task.receive(0, id)
                if rc == 0 then
                    local callbacks = self.callbacks[flag]
                    if callbacks then
                        for _, func in ipairs(callbacks) do
                            func(data)
                        end
                    end
                    if flag == task.END then
                        table.remove(self.tasks, i)
                    end
                end
            end
            evt:Skip()
        end

        function self.addTask(id)
            table.insert(self.tasks, id)
            -- If this is the first task, connect the idle event
            if #self.tasks == 1 then
                print("Connect(wx.wxEVT_IDLE): "..tostring(self).." "..tostring(window))
                self:Connect(wx.wxEVT_IDLE, idleEvent)
            end
        end

        function self.addCallback(flag, func)
            local t = self.callbacks[flag]
            if not t then
                t = {}
                self.callbacks[flag] = t
            end
            table.insert(t, func)
        end
        self.pushCallback = self.addCallback

        function self.removeCallback(flag, func)
            local t = self.callbacks[flag]
            if t then
                for i, f in ipairs(t) do 
                    if f == func then
                        table.remove(t, i)
                        break
                    end
                end
            end
        end

        function self.popCallback(flag)
            local t = self.callbacks[flag]
            if t then
                table.remove(t, i)
            end
        end

        -- Make sure to use this instead of wxWindow::PushEventHandler()
        -- so that we can clean up after ourselves.
        function self.startEventHandling(window)
            print("PushEventHandler: "..tostring(self).." "..tostring(window))
            window:PushEventHandler(self)
            self.window = window
        end

        function self.endEventHandling()
            print("RemoveEventHandler: "..tostring(self).." "..tostring(window))
            self.window:RemoveEventHandler(self)
            self.window = false
        end

        if window then
            self.startEventHandling(window)
        else
            self.window = false
        end

        table.insert(task.evtHandlers, self)
        return self
    end

    -- A shortcut method to run a thread and handle its callbacks.
    -- Callbacks is a table of callback functions:
    -- { [evtid] = { func1, ... }, [evtid] = func, ... }
    function task.handleEvents(task_id, callbacks, window)
        -- The window that will receive idle events.
        window = window or wx.wxGetApp():GetTopWindow()
        local evtHandler = task.newEvtHandler(window)

        -- Turn single callback functions into a table of callbacks, as
        -- expected by our evtHandler
        for evtid, funcs in pairs(callbacks) do
            if type(funcs) == "function" then
                callbacks[evtid] = { funcs }
            end
        end
        evtHandler.callbacks = callbacks

        -- Destroy this evtHandler when the thread is complete
        evtHandler.addCallback(task.END, function()
            evtHandler.endEventHandling()
            evtHandler = nil
        end)

        evtHandler.addTask(task_id)
    end

    xword.OnCleanup(function()
        -- Remove all event handlers from their windows
        for _, handler in ipairs(task.evtHandlers) do
            if handler.window then
                print("Removing event handler: "..tostring(handler).." from window: "..tostring(handler.window))
                handler.endEventHandling()
            end
        end
    end)
end

return task