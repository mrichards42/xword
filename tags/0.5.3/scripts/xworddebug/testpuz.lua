-- Generate a bunch of test puzzles

require 'luapuz'

local function fillclues(p)
    local nacross, ndown = p.Grid:CountClues()
    local across, down = {}, {}
    for i=1,nacross do across[i] = 'across '..tostring(i) end
    for i=1,ndown   do down[i] = 'down '..tostring(i) end
    p.Across = across
    p.Down = down
end

-- A puzzle where all squares are of one character
local function genpuz(ch)
    local str = string.char(ch)
    local p = puz.Puzzle()
    p.Grid:SetSize(7,7)
    local square = p.Grid:First()
    while square do
        square:SetSolution(str, str)
        square = square:Next()
    end
    -- Make the middle square something else
    p.Grid[{4,4}].Solution = (str=='.' and 'A' or '.')
    p.Title = string.format('Test puz of character: %q (%d)', str, ch)
    fillclues(p)
    return p
end

if false then
    -- Create all the test puzzles
    for i=1,255 do
        local filename = string.format([[D:\puz_%d_test.puz]], i)
        local p = genpuz(i)
        p:Save(filename)
        p:__gc()
    end
elseif false then
    require 'pl.stringx'.import()
    -- Generate a master puzzle with all text
    p = puz.Puzzle()
    p.Grid:SetSize(17,17)

    p.Grid[{1,1}]:SetSolution('.')
    -- Set the sides to correspond with hex values
    local hex = '0123456789ABCDEF'
    for i=1,16 do
        local ch = hex:at(i)
        local s = p.Grid[{i+1,1}]
        s:SetSolution(ch, ch)
        s:SetText(ch)
        local s = p.Grid[{1,i+1}]
        s:SetSolution(ch, ch)
        s:SetText(ch)
        -- Set the grid itself to be the values
        for j=1,16 do
            local byte = (i-1)+(j-1)*16
            local ch = string.char(byte)
            if byte == 0 then ch = '.' end
            local s = p.Grid[{i+1,j+1}]
            s:SetSolution(ch, ch)
            s:SetText(ch)
        end
    end
    p.Grid[{2,2}]:SetSolution('.')

    fillclues(p)
    p.Title = 'Test puz of all characters'
    p:Save([[D:\puz_master_test.puz]])
    p:__gc()
elseif true then
    p = puz.Puzzle()
    p.Grid:SetSize(16,16)
    local s = p.Grid:First()
    while s do
        s:SetSolution('A')
        s = s:Next()
    end
    p.Title = "Blank Puzzle"
    fillclues(p)
    p:Save([[D:\puz_blank.puz]])
    p:__gc()
end