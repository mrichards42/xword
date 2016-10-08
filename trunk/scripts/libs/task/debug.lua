--- Task debugging


task.EVT_DEBUG_START = -990
task.EVT_DEBUG_MSG = -991

if task.is_main then return end

function task.debug()
    -- Get locals
    _L = {}
    for i=1,100 do
        local name, value = debug.getlocal(2, i)
        if not name then break end
        _L[name] = value
    end
    task.post(task.EVT_DEBUG_START, require'serialize'.pprint(_L))
    -- Wait for commands
    while true do
        local task_id, evt_id, data = task.receive(-1)
        if evt_id == task.EVT_ABORT then
            return
        elseif evt_id == task.EVT_DEBUG_MSG then
            if data[1] == 'cont' then return end
            local func, err = loadstring("return " .. tostring(data[1]))
            if not func then
                func, err = loadstring(tostring(data[1]))
            end
            if func then
                local result = { pcall(func) }
                local success = table.remove(result, 1)
                success, err = pcall(task.post, task.EVT_DEBUG_MSG, unpack(result))
                if err then
                    task.log("Error:" .. err)
                    task.log(result)
                end
            else
                task.log("Error:" .. err)
            end
        else
            -- Throw away other messages
        end
    end
end

return task.debug