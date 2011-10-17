-- Run code under a protected environment
function safe_call(untrusted_function, env)
    setfenv(untrusted_function, env or {})
    local success, result = pcall(untrusted_function)
    if success then
        return result
    else
        return nil, result
    end
end

-- Safely load a string
function safe_dostring(untrusted_string, env)
    local func, err = loadstring(untrusted_string)
    if not func then return nil, err end
    return safe_call(func, env)
end

-- Safely load a table from a file
function safe_dofile(filename, env)
    local func, err = loadfile(filename)
    if not func then return nil, err end
    return safe_call(func, env)
end
