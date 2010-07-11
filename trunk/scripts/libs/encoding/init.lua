
encoding = {}

-- Find the encoding function given either the table or a string
local function find_func(enc, kind)
    if type(enc) == "string" then
        enc = enc:lower()
        for k, v in pairs(encoding) do
            if k == enc or v.names[enc] then
                return find_func(v, kind)
            end
        end
        error(string.format("Could not find %sr: '%s'", kind, enc))
    end
    if type(enc) == "table" then
        return enc[kind]
    end
    if not enc then
        return encoding.unicode
    end
    assert(type(enc) == "function", "Expected function but got "..type(enc))
    return enc
end

function invalid_code_point(code_point)
    error(string.format("%d is not a valid unicode code point", code_point))
end

-- encoding.convert(str, "input encoding", "output encoding")
-- encoding.convert(str, encoding.input_encoding, encoding.output_encoding)
-- encoding.convert(str, function decoder(str), function encoder(byte))
function encoding.convert(str, decoder, encoder, on_error)
    decoder = find_func(decoder, "decode")
    encoder = find_func(encoder, "encode")

    if type(on_error) == "number" then
        on_error = string.char(on_error)
    end

    local ret = {}
    for code_point in decoder(str) do
        local encoded = encoder(code_point)
        if encoded then
            table.insert(ret, encoded)
        elseif on_error then
            -- Invalid code point for the output encoding, figure out what to do
            if type(on_error) == "string" then
                table.insert(ret, on_error)
            elseif type(on_error) == "function" then
                table.insert(ret, on_error(code_point))
            end
        end
        -- Else we skip errors
    end
    return table.concat(ret)
end

-- Assume the incoming string is unicode (the default input if none is
-- specified)
encoding.unicode = {
    decode = function(str)
        local i = 0
        return function()
            i = i + 1
            return str:byte(i)
        end
    end
}

-- Lazy loading of encoders and decoders
-- This assumes that each codec is created using the module() system, 
-- or that it at least defines a variable _NAME with the name of the codec,
-- which must correspond to the directory.
local codec_mt = {}
function codec_mt:__index(name)
    if type(name) == "string" then
        local modname = assert(rawget(self, "_NAME")).."."..name
        for _, loader in ipairs(package.loaders) do
            local load = loader(modname)
            if type(load) == "function" then
                load()
                return rawget(self, name)
            end
        end
    end
end

-- The encodings
setmetatable(require 'encoding.windows', codec_mt)
setmetatable(require 'encoding.utf8', codec_mt)


return encoding
