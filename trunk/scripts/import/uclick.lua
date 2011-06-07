local lom = require 'lxp.lom'

-- Create a table mapping hex characters to their decimal values
local hex_codes = {}
local val = 0
for c in string.gmatch('0123456789abcdef', '.') do
    hex_codes[c] = val
    val = val + 1
end

-- Unscape the hex % codes.
-- We can't use wx.wxURI.Unescape because it returns a wxString, which is
-- subsequently converted to a lua string using wxConvCurrent.  We need to
-- preserve the actual bytes in order to convert the string from UTF-8 later.
local function unescape(str)
    -- Replace all %xx escapes with their value in hex
    return str:gsub('%%..', function(escape)
        local high = assert(hex_codes[escape:sub(2,2):lower()],
                            'Bad hex character: '..escape:sub(2,2))

        local low  = assert(hex_codes[escape:sub(3,3):lower()],
                            'Bad hex character: '..escape:sub(3,3))

        return string.char(high * 16 + low)
    end)
end

function import.UClick(p, filename)
    -- Open the file and parse it
    local f = assert(io.open(filename))
    local doc = assert(lom.parse(f:lines()))
    f:close()

    -- Find the root node
    assert(doc.tag == "crossword", 'Root node must be "crossword"')

    local width, height
    local solution
    local nAcross, nDown
    local across, down
    local across_count, down_count

    local function getattr(node, name, attr)
        attr = attr or 'v'
        assert(node.attr and node.attr[attr], "Missing "..name)
        return unescape(node.attr[attr])
    end

    -- Read the File
    for node in lom.inodes(doc) do
        local tag = node.tag
        if tag == 'Title' then
            p.Title = getattr(node, 'title')
        elseif tag == 'Author' then
            p.Author = getattr(node, 'author')
        elseif tag == 'Width' then
            width = tonumber(getattr(node, 'width'))
        elseif tag == 'Height' then
            height = tonumber(getattr(node, 'height'))
        elseif tag == 'AllAnswer' then
            solution = getattr(node, 'solution')
            solution = solution:gsub('-', '.') -- '-' is a black square
        elseif tag == 'across' then
            across = {}
            for clue in lom.inodes(node) do
                local number = getattr(clue, 'across clue number', 'cn')
                local text   = getattr(clue, 'across clue', 'c')
                across[tonumber(number)] = text
            end
        elseif tag == 'down' then
            down = {}
            for clue in lom.inodes(node) do
                local number = getattr(clue, 'down clue number', 'cn')
                local text   = getattr(clue, 'down clue', 'c')
                down[tonumber(number)] = text
            end
        end
    end
    -- Set the solution
    assert(width and height, "Missing puzzle size")
    assert(#solution == width * height, "Bad solution size")
    local grid = p.Grid
    grid:SetSize(width, height)
    local square = grid:First()
    for c in solution:gmatch('.') do -- Iterate characters
        square:SetSolution(c)
        square = square:Next(puz.ACROSS)
    end
    -- Set the clues
    p.Across = across
    p.Down   = down
    p:RenumberClues()
end

import.addHandler(import.UClick, "xml", "UClick XML")
