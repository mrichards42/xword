-- Errors
local function utf_error(pos)
    error("Invalid utf-8 string @ "..pos)
end
local function utf_eos(nbytes)
    error("Invalid utf-8 string: unterminated "..nbytes.."-byte sequence")
end

module('encoding.utf8')

seq_start = __seq_start
CONTINUATION = __CONTINUATION
max_byte = __max_byte


local to_unicode = {
    -- 1 byte sequence
    function(str, start)
        return str:byte(start)
    end,

    -- 2 byte sequence
    function (str, start)
        local b1, b2 = str:byte(start, start + 1)
        return b1 * 64 + b2 - 12416
    end,

    -- 3 byte sequence
    function (str, start)
        local b1, b2, b3 = str:byte(start, start + 2)
        return (b1 * 64 + b2) * 64 + b3 - 925824
    end,

    -- 4 byte sequence
    function (str, start)
        local b1, b2, b3, b4 = str:byte(start, start + 3)
        return ((b1 * 64 + b2) * 64 + b3) * 64 + b4 - 63447168
    end
}


-- An iterator over code points
function decode(str)
    local start = 1
    return function()
        -- Figure out how many bytes to expect
        local nbytes
        local b = str:byte(start)
        if not b then return end
        if b < seq_start[CONTINUATION] then
            nbytes = 1
        elseif b < seq_start[2] then
            utf_error(start)
        elseif b < seq_start[3] then
            nbytes = 2
        elseif b < seq_start[4] then
            nbytes = 3
        elseif b <= max_byte then
            nbytes = 4
        else
            utf_error(start)
        end

        -- Make sure the next several bytes are valid
        for pos=start+1,start+nbytes-1 do
            local b = str:byte(pos)
            if not b then utf_eos(nbytes) end -- Unterminated sequence error
            if b < seq_start[CONTINUATION] or b >= seq_start[2] then
                utf_error(pos)
            end
        end

        -- Return the unicode code point and advance the iterator
        local ret = to_unicode[nbytes](str, start)
        start = start + nbytes
        return ret
    end
end

