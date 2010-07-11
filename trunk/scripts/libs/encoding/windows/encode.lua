local char = string.char

module("encoding.windows")

-- Conversion from unicode to windows encoding
local function encode_code_point(code_point)
    -- ASCII
    if code_point <= 127 then
        return code_point
    end
    if code_point > 255 then
        return from_unicode[code_point]
    end
    -- code points between 160 and 255 are the same in windows and unicode
    if code_point >= 160 then
        return code_point
    end

    -- Atypical stuff here: if the code point hasn't matched anything yet,
    -- see if it is already in windows encoding.
    if to_unicode[code_point] then
        return code_point
    end
end

-- Return the code point converted to a character if it exists
function encode(code_point)
    local conv = encode_code_point(code_point)
    if conv then
        return char(conv)
    end
end
