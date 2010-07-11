
local select = select
local tinsert = table.insert
local modf, mod = math.modf, math.mod
local char = string.char

module('encoding.utf8')

-- Return a,b where num = a * 64 + b
local function split64(num)
    return select(1, modf(num / 64)), mod(num, 64)
end


function encode(code_point)
    -- ASCII
    if code_point < 128 then return char(code_point) end

    -- Split the code point into a * 64 + b as many times as necessary
    -- The bytes table will be the resulting byte offsets in reverse order
    local bytes = {}
    local a,b = split64(code_point)
    while a > 0 do
        tinsert(bytes, b)
        a,b = split64(a)
    end
    tinsert(bytes, b)

    local nbytes = #bytes
    if nbytes > 4 then error("Invalid unicode code point: "..code_point) end

    -- Form the return string
    local ret

    -- The first byte will tell us how long to make the sequence
    local first_byte = seq_start[nbytes] + bytes[nbytes]
    -- This first byte could end up being too high for the given sequence
    -- length. If this is the case, increase the sequence length and give
    -- the first byte the lowest possible value for the next longest sequence
    if nbytes < 4 and first_byte > seq_start[nbytes + 1] then
        first_byte = seq_start[nbytes+1]
        nbytes = nbytes + 1
    end
    ret = char(first_byte)

    -- Iterate the rest of the bytes in reverse order and add them to the string
    for i=nbytes-1,1,-1 do
        ret = ret .. char(seq_start[CONTINUATION] + bytes[i])
    end

    return ret
end
