assert(import)


-- Convert from UTF-8 to Windows encoding
local function convStr(str)
    return xword.conv_utf8(str)
end

local function importXPF(filename, puz)
    -- wxXmlDocument() calls wxLogError on parsing errors.
    local lognull = wx.wxLogNull()
    local doc = wx.wxXmlDocument(filename)
    lognull:delete()
    if not doc:IsOk() then return false, 'Error parsing XML file' end

    -- Root node
    if doc.Root.Name ~= 'Puzzles' then
        return false, 'Root node must be "Puzzles"'
    end

    local width, height
    local nAcross, nDown

    -- XPF may contain multiple puzzle files.  Find the first one
    local puznode = doc.Root.Children
    while puznode.Name ~= "Puzzle" do
        puznode = puznode.Next
        if not puznode then return false, 'No "Puzzle" node' end
    end

    -- Read the puzzle
    local child = puznode.Children
    while child do
        if child.Name == 'Title' then
            puz.Title = convStr(child.Children.Content)
        elseif child.Name == 'Author' then
            puz.Author = convStr(child.Children.Content)
        elseif child.Name == 'Copyright' then
            -- Add the copyright symbol
            local copyright = convStr(child.Children.Content)
            if copyright ~= "" then
                puz.Copyright = string.char(169)..' '..copyright
            end
        elseif child.Name == 'Size' then
            local size = child.Children
            while size do
                if size.Name == 'Cols' then
                    width = tonumber(size.Children.Content)
                elseif size.Name == 'Rows' then
                    height = tonumber(size.Children.Content)
                end
                size = size.Next
            end
        elseif child.Name == 'Grid' then
            puz.Grid:SetSize(width, height)
            local square = puz.Grid:First()
            local row = child.Children
            while row do
                if not square:IsFirst(xword.DIR_ACROSS) then
                    return false, 'Not enough squares in a row'
                end
                for c in row.Children.Content:gmatch('.') do
                    square:SetSolution(c)
                    square = square:Next(xword.DIR_ACROSS)
                end
                row = row.Next
            end
            nAcross, nDown = puz.Grid:CountClues()
            print(nAcross, nDown)
        elseif child.Name == 'Clues' then
            local across, down = {}, {}
            local clue = child.Children
            while clue do
                local exists, dir = clue:GetPropVal('Dir')
                if not exists then return false, 'Missing clue direction' end
                local exists, num = clue:GetPropVal('Num')
                if not exists then return false, 'Missing clue number' end

                if dir == "Across" then
                    across[tonumber(num)] = clue.Children.Content
                    nAcross = nAcross - 1
                elseif dir == "Down" then
                    down[tonumber(num)] = clue.Children.Content
                    nDown = nDown - 1
                else
                    return false, 'Unknown clue direction: "'..dir..'"'
                end
                clue = clue:GetNext()
            end
            if nAcross > 0 then return false, "Missing across clues" end
            if nAcross < 0 then return false, "Extra across clues ("..nAcross..")" end
            if nDown > 0 then return false, "Missing down clues" end
            if nDown < 0 then return false, "Extra down clues" end
            puz.Across = across
            puz.Down = down
        elseif child.Name == "RebusEntries" then
            local rebus = child.Children
            while rebus do
                -- Gather parameters:
                -- <Rebus Row="1" Col="2" Short="Y">YELLOW</Rebus>
                local exists, row = rebus:GetPropVal('Row')
                if not exists then return false, 'Missing rebus entry row' end
                local exists, col = rebus:GetPropVal('Col')
                if not exists then return false, 'Missing rebus entry column' end
                local exists, short = rebus:GetPropVal('Short')
                if not exists then return false, 'Missing rebus entry short solution' end
                local long = rebus.Children.Content
                if long == '' then return false, 'Missing rebus entry long solution' end
                -- Set the square's rebus entry
                col = tonumber(col)
                row = tonumber(row)
                local square = puz.Grid[{col, row}]
                square:SetSolution(long, short)
                rebus = rebus.Next
            end
        elseif child.Name == 'Circles' or child.Name == 'Shades' then
            local circle = child.Children
            while circle do
                -- Gather parameters:
                -- <Circle Row="1" Col="8" />
                local exists, row = circle:GetPropVal('Row')
                if not exists then return false, 'Missing circle row' end
                local exists, col = circle:GetPropVal('Col')
                if not exists then return false, 'Missing circle column' end
                -- Set the square's circle flag
                col = tonumber(col)
                row = tonumber(row)
                local square = puz.Grid[{col, row}]
                square:AddFlag(xword.XFLAG_CIRCLE)
                circle = circle.Next
            end
        elseif child.Name == 'Notepad' then
            puz.Notes = child.Children.Content
        end
        child = child:GetNext()
    end
    puz:RenumberClues()
    return true
end

import.addType(
    'XPF',                           -- Description (label)
    {xml = 'XPF file'},              -- { ext = 'type description' }
    importXPF                        -- function(filename, puz)
)
