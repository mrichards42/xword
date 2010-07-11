
module("encoding.utf8")

names = {
    ["utf-8"] = true,
}

-- Globals

-- Starting bytes for sequences
__seq_start = {
    128, -- continuation
    192, -- 2 byte
    224, -- 3 byte
    240, -- 4 byte
}
__CONTINUATION = 1
-- Highest possible byte in a utf string
__max_byte = 244
