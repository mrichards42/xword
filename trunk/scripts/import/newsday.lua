-- Newsday files are not utf-8, but wx expects utf-8
-- They're either windows encoding or latin-1
local function do_read_line(f)
    local line = f:read('*l')
    return line:gsub("[\128-\255]", function(c)
        local n = string.byte(c)
        if n < 0xa0 then
            -- Below A0 each char has a different escape
            return "(?)"
        elseif n < 0xc0 then
            -- Below C2 each char is C0XX where XX is the original char
            return "\194" .. c
        else
            -- C0 and above, each char is C3XX where XX is the original char - 64
            return "\195" .. string.char(n - 64)
        end
    end)
end

-- Return a section as a table of lines
-- Sections are delimited by blank lines
local function read_section(f)
    local t = {}
    while true do
        local line = do_read_line(f)
        if line == "" then return t end
        table.insert(t, line)
    end
end

-- Return a single-line section
local function read_line(f)
    local line = do_read_line(f)
    -- blank line after each section
    if f:read("*l") ~= "" then 
        return ""
    end
    return line
end

local function parse(p, f)
    -- Header
    if read_line(f) ~= "ARCHIVE" then
        return false
    end
    -- Date: YYMMDD
    read_line(f)
    -- Title/Author
    p.Title = read_line(f)
    p.Author = read_line(f)
    -- Grid/Clues
    local width = tonumber(read_line(f))
    local height = tonumber(read_line(f))
    local nAcross = tonumber(read_line(f))
    local nDown = tonumber(read_line(f))
    local solution = table.concat(read_section(f))
    local across = read_section(f)
    local down = read_section(f)

    assert(width and height, "Missing puzzle size")
    assert(#solution == width * height, "Bad solution size")

    -- Set the solution
    local grid = p.Grid
    grid:SetSize(width, height)
    local square = grid:First()
    for c in solution:gmatch('.') do -- Iterate characters
        square:SetSolution(c == "#" and "." or c)
        square = square:Next(puz.ACROSS)
    end
    -- Set the clues
    p:SetClueList("Across", across)
    p:SetClueList("Down", down)
    p:NumberGrid()
    p:NumberClues()
end

function import.Newsday(p, filename)
    local f = assert(io.open(filename))
    local success, result = pcall(parse, p, f)
    f:close()
    assert(success, result)
    return result
end

import.addHandler(import.Newsday, "txt", "Newsday TXT")
