-- This file is a script to be passed to task.create
local globals, id, script, args = unpack(arg)

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

-- Deserialize (for args passed to this script)
local function deserialize(str)
    return select(2, pcall(loadstring(str)))
end

-- Set global variables (should include package.path/cpath)
update(_G, deserialize(globals) or {})

-- Load the task library
-- The C task library loads itself into package.loaded.
-- Move it to package.loaded['c-task'] so that we can load the lua library
package.loaded['c-task'] = package.loaded['task']
package.loaded['task'] = nil
task = require 'task'

-- Set the unique id
task.id = id

local function run_script()
    -- Load the script
    local func, err = task.load(script)
    -- Report loading errors immediately
    if not func then
        task.error(err)
        return
    end
    -- Set the global arg variable
    arg = deserialize(args)
    -- Run the script
    task.post(task.EVT_START)
     -- Pass the script argument (like require), then the args table
    return func(script, unpack(arg))
end

-- Load and run the script
local result = {xpcall(run_script, debug.traceback)}
if not result[1] then
    -- Report errors
    task.error(result[2])
    task.post(task.EVT_END)
else
    -- Pass EVT_END with the results of run_script
    task.post(task.EVT_END, select(2, unpack(result)))
end