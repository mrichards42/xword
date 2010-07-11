local function windows_error(pos)
    error("Invalid windows encoded string @ "..pos)
end

module("encoding.windows")

-- Iterator over unicode code points in a windows encoded string
function decode(str)
    local pos = 0
    return function ()
        pos = pos + 1
        local b = str:byte(pos)
        if not b then return end
        if b < 128 then -- ASCII
            return b
        end
        -- Look up the encoded byte in the replacement table
        local code_point = to_unicode[b]
        if code_point == false then -- false indicates an invalid byte
            windows_error(pos)
        elseif code_point ~= nil then
            return code_point
        end
        if b > 255 then
            windows_error(pos)
        end
        return b
    end
end
