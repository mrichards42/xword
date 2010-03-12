-- ============================================================================
-- UTF-8 to Windows encoding conversion
--     Across Lite uses Windows encoding for all characters, so any character
--     saved to a .puz file must be within that character set.
-- ============================================================================

assert(table)
assert(bit) -- bitwise operator library (included in wxLua)

-- Mapping of Unicode code points to Windows encoding (for non-ASCII chars)
local replacement_table = {
    [0x20AC] = 0x80, -- EURO SIGN
    --[      ] = 0x81, --  UNDEFINED
    [0x201A] = 0x82, -- SINGLE LOW-9 QUOTATION MARK
    [0x0192] = 0x83, -- LATIN SMALL LETTER F WITH HOOK
    [0x201E] = 0x84, -- DOUBLE LOW-9 QUOTATION MARK
    [0x2026] = 0x85, -- HORIZONTAL ELLIPSIS
    [0x2020] = 0x86, -- DAGGER
    [0x2021] = 0x87, -- DOUBLE DAGGER
    [0x02C6] = 0x88, -- MODIFIER LETTER CIRCUMFLEX ACCENT
    [0x2030] = 0x89, -- PER MILLE SIGN
    [0x0160] = 0x8A, -- LATIN CAPITAL LETTER S WITH CARON
    [0x2039] = 0x8B, -- SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    [0x0152] = 0x8C, -- LATIN CAPITAL LIGATURE OE
    --[      ] = 0x8D, --  UNDEFINED
    [0x017D] = 0x8E, -- LATIN CAPITAL LETTER Z WITH CARON
    --[      ] = 0x8F, --  UNDEFINED
    --[      ] = 0x90, --  UNDEFINED
    [0x2018] = 0x91, -- LEFT SINGLE QUOTATION MARK
    [0x2019] = 0x92, -- RIGHT SINGLE QUOTATION MARK
    [0x201C] = 0x93, -- LEFT DOUBLE QUOTATION MARK
    [0x201D] = 0x94, -- RIGHT DOUBLE QUOTATION MARK
    [0x2022] = 0x95, -- BULLET
    [0x2013] = 0x96, -- EN DASH
    [0x2014] = 0x97, -- EM DASH
    [0x02DC] = 0x98, -- SMALL TILDE
    [0x2122] = 0x99, -- TRADE MARK SIGN
    [0x0161] = 0x9A, -- LATIN SMALL LETTER S WITH CARON
    [0x203A] = 0x9B, -- SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    [0x0153] = 0x9C, -- LATIN SMALL LIGATURE OE
    --[      ] = 0x9D, --  UNDEFINED
    [0x017E] = 0x9E, -- LATIN SMALL LETTER Z WITH CARON
    [0x0178] = 0x9F, -- LATIN CAPITAL LETTER Y WITH DIAERESIS
    [0x00A0] = 0xA0, -- NO-BREAK SPACE
    [0x00A1] = 0xA1, -- INVERTED EXCLAMATION MARK
    [0x00A2] = 0xA2, -- CENT SIGN
    [0x00A3] = 0xA3, -- POUND SIGN
    [0x00A4] = 0xA4, -- CURRENCY SIGN
    [0x00A5] = 0xA5, -- YEN SIGN
    [0x00A6] = 0xA6, -- BROKEN BAR
    [0x00A7] = 0xA7, -- SECTION SIGN
    [0x00A8] = 0xA8, -- DIAERESIS
    [0x00A9] = 0xA9, -- COPYRIGHT SIGN
    [0x00AA] = 0xAA, -- FEMININE ORDINAL INDICATOR
    [0x00AB] = 0xAB, -- LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    [0x00AC] = 0xAC, -- NOT SIGN
    [0x00AD] = 0xAD, -- SOFT HYPHEN
    [0x00AE] = 0xAE, -- REGISTERED SIGN
    [0x00AF] = 0xAF, -- MACRON
    [0x00B0] = 0xB0, -- DEGREE SIGN
    [0x00B1] = 0xB1, -- PLUS-MINUS SIGN
    [0x00B2] = 0xB2, -- SUPERSCRIPT TWO
    [0x00B3] = 0xB3, -- SUPERSCRIPT THREE
    [0x00B4] = 0xB4, -- ACUTE ACCENT
    [0x00B5] = 0xB5, -- MICRO SIGN
    [0x00B6] = 0xB6, -- PILCROW SIGN
    [0x00B7] = 0xB7, -- MIDDLE DOT
    [0x00B8] = 0xB8, -- CEDILLA
    [0x00B9] = 0xB9, -- SUPERSCRIPT ONE
    [0x00BA] = 0xBA, -- MASCULINE ORDINAL INDICATOR
    [0x00BB] = 0xBB, -- RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    [0x00BC] = 0xBC, -- VULGAR FRACTION ONE QUARTER
    [0x00BD] = 0xBD, -- VULGAR FRACTION ONE HALF
    [0x00BE] = 0xBE, -- VULGAR FRACTION THREE QUARTERS
    [0x00BF] = 0xBF, -- INVERTED QUESTION MARK
    [0x00C0] = 0xC0, -- LATIN CAPITAL LETTER A WITH GRAVE
    [0x00C1] = 0xC1, -- LATIN CAPITAL LETTER A WITH ACUTE
    [0x00C2] = 0xC2, -- LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    [0x00C3] = 0xC3, -- LATIN CAPITAL LETTER A WITH TILDE
    [0x00C4] = 0xC4, -- LATIN CAPITAL LETTER A WITH DIAERESIS
    [0x00C5] = 0xC5, -- LATIN CAPITAL LETTER A WITH RING ABOVE
    [0x00C6] = 0xC6, -- LATIN CAPITAL LETTER AE
    [0x00C7] = 0xC7, -- LATIN CAPITAL LETTER C WITH CEDILLA
    [0x00C8] = 0xC8, -- LATIN CAPITAL LETTER E WITH GRAVE
    [0x00C9] = 0xC9, -- LATIN CAPITAL LETTER E WITH ACUTE
    [0x00CA] = 0xCA, -- LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    [0x00CB] = 0xCB, -- LATIN CAPITAL LETTER E WITH DIAERESIS
    [0x00CC] = 0xCC, -- LATIN CAPITAL LETTER I WITH GRAVE
    [0x00CD] = 0xCD, -- LATIN CAPITAL LETTER I WITH ACUTE
    [0x00CE] = 0xCE, -- LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    [0x00CF] = 0xCF, -- LATIN CAPITAL LETTER I WITH DIAERESIS
    [0x00D0] = 0xD0, -- LATIN CAPITAL LETTER ETH
    [0x00D1] = 0xD1, -- LATIN CAPITAL LETTER N WITH TILDE
    [0x00D2] = 0xD2, -- LATIN CAPITAL LETTER O WITH GRAVE
    [0x00D3] = 0xD3, -- LATIN CAPITAL LETTER O WITH ACUTE
    [0x00D4] = 0xD4, -- LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    [0x00D5] = 0xD5, -- LATIN CAPITAL LETTER O WITH TILDE
    [0x00D6] = 0xD6, -- LATIN CAPITAL LETTER O WITH DIAERESIS
    [0x00D7] = 0xD7, -- MULTIPLICATION SIGN
    [0x00D8] = 0xD8, -- LATIN CAPITAL LETTER O WITH STROKE
    [0x00D9] = 0xD9, -- LATIN CAPITAL LETTER U WITH GRAVE
    [0x00DA] = 0xDA, -- LATIN CAPITAL LETTER U WITH ACUTE
    [0x00DB] = 0xDB, -- LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    [0x00DC] = 0xDC, -- LATIN CAPITAL LETTER U WITH DIAERESIS
    [0x00DD] = 0xDD, -- LATIN CAPITAL LETTER Y WITH ACUTE
    [0x00DE] = 0xDE, -- LATIN CAPITAL LETTER THORN
    [0x00DF] = 0xDF, -- LATIN SMALL LETTER SHARP S
    [0x00E0] = 0xE0, -- LATIN SMALL LETTER A WITH GRAVE
    [0x00E1] = 0xE1, -- LATIN SMALL LETTER A WITH ACUTE
    [0x00E2] = 0xE2, -- LATIN SMALL LETTER A WITH CIRCUMFLEX
    [0x00E3] = 0xE3, -- LATIN SMALL LETTER A WITH TILDE
    [0x00E4] = 0xE4, -- LATIN SMALL LETTER A WITH DIAERESIS
    [0x00E5] = 0xE5, -- LATIN SMALL LETTER A WITH RING ABOVE
    [0x00E6] = 0xE6, -- LATIN SMALL LETTER AE
    [0x00E7] = 0xE7, -- LATIN SMALL LETTER C WITH CEDILLA
    [0x00E8] = 0xE8, -- LATIN SMALL LETTER E WITH GRAVE
    [0x00E9] = 0xE9, -- LATIN SMALL LETTER E WITH ACUTE
    [0x00EA] = 0xEA, -- LATIN SMALL LETTER E WITH CIRCUMFLEX
    [0x00EB] = 0xEB, -- LATIN SMALL LETTER E WITH DIAERESIS
    [0x00EC] = 0xEC, -- LATIN SMALL LETTER I WITH GRAVE
    [0x00ED] = 0xED, -- LATIN SMALL LETTER I WITH ACUTE
    [0x00EE] = 0xEE, -- LATIN SMALL LETTER I WITH CIRCUMFLEX
    [0x00EF] = 0xEF, -- LATIN SMALL LETTER I WITH DIAERESIS
    [0x00F0] = 0xF0, -- LATIN SMALL LETTER ETH
    [0x00F1] = 0xF1, -- LATIN SMALL LETTER N WITH TILDE
    [0x00F2] = 0xF2, -- LATIN SMALL LETTER O WITH GRAVE
    [0x00F3] = 0xF3, -- LATIN SMALL LETTER O WITH ACUTE
    [0x00F4] = 0xF4, -- LATIN SMALL LETTER O WITH CIRCUMFLEX
    [0x00F5] = 0xF5, -- LATIN SMALL LETTER O WITH TILDE
    [0x00F6] = 0xF6, -- LATIN SMALL LETTER O WITH DIAERESIS
    [0x00F7] = 0xF7, -- DIVISION SIGN
    [0x00F8] = 0xF8, -- LATIN SMALL LETTER O WITH STROKE
    [0x00F9] = 0xF9, -- LATIN SMALL LETTER U WITH GRAVE
    [0x00FA] = 0xFA, -- LATIN SMALL LETTER U WITH ACUTE
    [0x00FB] = 0xFB, -- LATIN SMALL LETTER U WITH CIRCUMFLEX
    [0x00FC] = 0xFC, -- LATIN SMALL LETTER U WITH DIAERESIS
    [0x00FD] = 0xFD, -- LATIN SMALL LETTER Y WITH ACUTE
    [0x00FE] = 0xFE, -- LATIN SMALL LETTER THORN
    [0x00FF] = 0xFF, -- LATIN SMALL LETTER Y WITH DIAERESIS

}


-- Convert a multi-byte sequence to a unicode code point
local function multibyte_to_unicode(b)
    local len = #b

    -- Single-byte is always ASCII
    if len == 1 then
        return b[1]
    end

    -- Two-byte sequence
    if len == 2 then
        local high =
            bit.rshift(bit.band(b[1], 28), 2)      -- (byte1 & 00011100) >> 2

        local low =
            bit.lshift(bit.band(b[1],  3), 6) +    -- (byte1 & 00000011) << 6
                       bit.band(b[2], 63)          -- (byte2 & 00111111)

        return low + bit.lshift(high, 8)
    end

    -- Three-byte sequence
    if len == 3 then
        local high =
            bit.lshift(bit.band(b[1], 15), 4) +    -- (byte1 & 00001111) << 4
            bit.rshift(bit.band(b[2], 60), 2)      -- (byte2 & 00111100) >> 2

        local low =
            bit.lshift(bit.band(b[2],  3), 6) +    -- (byte2 & 00000011) << 6
                       bit.band(b[3], 63)          -- (byte3 & 00111111)

        return low + bit.lshift(high, 8)
    end

    -- Four-byte sequence
    if len == 4 then
        local high = 
            bit.lshift(bit.band(b[1],  7), 5) +    -- (byte1 & 00000111) << 3
            bit.rshift(bit.band(b[2], 48), 4)      -- (byte2 & 00110000) >> 4

        local mid =
            bit.lshift(bit.band(b[2], 15), 4) +    -- (byte2 & 00001111) << 4
            bit.rshift(bit.band(b[3], 60), 2)      -- (byte3 & 00111100) >> 2

        local low =
            bit.lshift(bit.band(b[3],  3), 6) +    -- (byte3 & 00000011) << 6
                       bit.band(b[4], 63)          -- (byte4 & 00111111)

        return low + bit.lshift(mid, 8) + bit.lshift(high, 16)
    end
end



-- Convert the string from utf-8 to windows encoding.
-- Characters that are unrepresentable are replaced with replace_char or
--     removed if replace_char is nil.
-- Invalid sequences will be replaced with error_char, or removed if
--     error_char is nil.
function xword.conv_utf8(str, replace_char, error_char)
    local replace_char = replace_char or ''
    local error_char   = error_char or ''

    -- Return string as a table
    local chars = {}

    local nBytes = -1 -- expected number of bytes in a multi-byte sequence
    local sequence = {} -- table for multi-byte sequences


    -- Add characters in table sequence to the return string
    local function add_pending_sequence()
        assert(#sequence == nBytes)

        -- Find the Windows encoded version of this string
        local code_point = multibyte_to_unicode(sequence)
        if code_point then
            if code_point < 128 then
                table.insert(chars, string.char(code_point))
            else
                local ch = replacement_table[code_point]
                if ch then
                    table.insert(chars, string.char(ch))
                elseif code_point < 256 then
                    -- If this doesn't have a direct mapping, but the code
                    -- point is within the range of windows encoding, try to
                    -- use that character directly.
                    table.insert(chars, string.char(code_point))
                else
                    table.insert(chars, replace_char)
                end
            end
        else
            table.insert(chars, error_char)
        end
        -- Reset the sequence table
        nBytes = -1
        sequence = {}
    end

    local function start_sequence(expected_bytes, first_byte)
        -- If there is a pending byte-sequence, it has to be malformed
        if #sequence ~= 0 then
            table.insert(chars, error_char)
        end
        nBytes = expected_bytes
        sequence = { first_byte }
    end

    -- Iterate all characters
    for char in str:gmatch('.') do
        local byte = char:byte()

         -- ASCII
        if byte <= 127 then
            start_sequence(1, byte)

        -- 2nd, 3rd, or 4th byte of a multi-byte sequence
        elseif byte <= 191 then
            table.insert(sequence, byte)

        -- Start of 2-byte sequence
        elseif byte <= 223 then
            start_sequence(2, byte)

        -- Start of 3-byte sequence
        elseif byte <= 239 then
            start_sequence(3, byte)

        -- Start of 4-byte sequence
        elseif byte <= 244 then
            start_sequence(4, byte)
            
        -- Anything else is invalid
        else
            -- This will clear the current byte-sequence
            start_sequence(-1, nil)
        end


        -- Check to see if we have the correct number of bytes for the
        -- current pending sequence.
        if #sequence == nBytes then
            add_pending_sequence()
        end
    end

    -- If we have a pending byte sequence it must be an invalid sequence
    if #sequence ~= 0 then
        table.insert(chars, error_char)
    end

    return table.concat(chars)
end
