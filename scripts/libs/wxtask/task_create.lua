-- This file is a script to be passed to task.create

local globals, script, args = unpack(arg)

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
task = require 'wxtask'

local path = require('pl.path')
-- Load a script using package.loaders
local function loadscript(script)
    local errors = {}
    -- Try package.loaders
    for _, loader in ipairs(package.loaders) do
        local result = loader(script)
        if type(result) == "function" then
            -- Add module directory to package.path
            local p = path.package_path(script)
            if p then
                package.path = package.path .. ';' .. path.dirname(p)
            end
            return result
        else
            table.insert(errors, result)
        end
    end
    -- Return nil, errors
    return nil, table.concat(errors, '\n')
end

local function run_script()
    -- Load the script
    local func, err
    if script:sub(1,1) == '=' then -- Script is a string
        -- Remove the equals sign and load the string
        func, err = loadstring(script:sub(2))
    else -- Script is a file name
        func, err = loadscript(script)
    end
    -- If we don't have a function from the loaders, we're done
    if not func then error(err) end
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