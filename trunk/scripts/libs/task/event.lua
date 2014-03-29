--- Event passing module.
-- @classmod EvtHandler

local tablex = require 'pl.tablex'

local M = {}
M.__index = M

--- Create a new EvtHandler.
-- @return An EvtHandler
-- @usage
-- EvtHandler = require 'task.event'
-- EVT_PRINT = EvtHandler.new_event()
-- e = EvtHandler()
-- e:connect(EVT_PRINT, function(msg)
--   print(msg)
-- end)
-- -- prints 'hello world'
-- e:send_message(EVT_PRINT, 'hello world!')
function M.new()
    return setmetatable({}, M)
end

-- Auto create keys for EVENT_TABLE
local EVENT_TABLE = setmetatable({}, {
    __index = function(t, k)
        t[k] = setmetatable({}, getmetatable(t))
        return t[k]
    end})

--- Connect an event or an event table.
-- @param key The `EvtHandler` object or wx.wxID_ANY for all handlers
-- @param[opt] evt_id The event id, if connecting a single callback
-- @param callback The callback function, or a table mapping event ids to
--   callback functions
-- @usage
-- MY_CUSTOM_EVT = EvtHandler.new_event()
-- MY_CUSTOM_EVT2 = EvtHandler.new_event()
-- handler = EventHandler.new()
-- -- Connect an event
-- handler:connect(MY_CUSTOM_EVT, function() print('hello world') end)
-- -- Connect to events from any handler
-- EvtHandler.connect(wx.wxID_ANY, task.EVT_DEBUG, function(msg) print('debug:', msg) end)
function M.connect(key, evt_id, callback)
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

--- Remove an event callback.
-- @param key The EvtHandler object or wx.wxID_ANY for all handlers
-- @param[opt] evt_id The event id or wx.wxID_ANY for all events
-- @param[opt] callback The callback function, or nil for all callbacks
function M.disconnect(key, evt_id, callback)
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

--- The `EvtHandler` object that posted the current event.
--- @field M.evt_handler

--- The id of the current event.
-- @field M.evt_id

--- Process an event.
-- Calls each callback attached to this event.
-- @param evt_id The event id.
-- @param ... Event data
function M:send_event(evt_id, ...)
    -- Set current task and evt for task.evt_id/task.evt_task
    M.evt_handler = self
    M.evt_id = evt_id
    -- Find callbacks for this task/event combination
    -- Include wxID_ANY tasks/events
    local event_table_list = {
        EVENT_TABLE[self][evt_id],
        EVENT_TABLE[self][-1],
        EVENT_TABLE[-1][evt_id],
        EVENT_TABLE[-1][-1]
    }
    for _, callbacks in ipairs(event_table_list) do
        for _, callback in ipairs(callbacks) do
           callback(...)
        end
    end
    M.evt_handler = nil
    M.evt_id = nil
end

return M
