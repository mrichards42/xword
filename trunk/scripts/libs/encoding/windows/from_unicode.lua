module('encoding.windows')

-- All code points < 256 that have an odd translation from unicode
from_unicode = {
-- ASCII points are < 128
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
-- From here on out, windows code points are the same as unicode.
}
