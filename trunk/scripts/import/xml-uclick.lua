assert(import)


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

-- Unscape the hex % codes then convert from UTF-8 to Windows encoding
local function convStr(str)
    return xword.conv_utf8(unescape(str), '?', '?')
end

local function importXML(filename, puz)
    -- wxXmlDocument() calls wxLogError on parsing errors.
    local lognull = wx.wxLogNull()
    local doc = wx.wxXmlDocument(filename)
    lognull:delete()
    if not doc:IsOk() then
        return false, 'Error parsing XML file'
    end

    -- Root node
    if doc.Root.Name ~= 'crossword' then
        return false, 'Root node must be "crossword"'
    end

    local width, height
    local nAcross, nDown

    -- Read the File
    local child = doc.Root.Children
    while child do
        if child.Name == 'Title' then
            local exists, title = child:GetPropVal('v')
            if not exists then return false, 'Missing title' end
            puz.Title = convStr(title)
        elseif child.Name == 'Author' then
            local exists, author = child:GetPropVal('v')
            if not exists then return false, 'Missing author' end
            puz.Author = convStr(author)
        elseif child.Name == 'Width' then
            local exists, value = child:GetPropVal('v')
            if not exists then return false, 'Missing width' end
            width = tonumber(value)
        elseif child.Name == 'Height' then
            local exists, value = child:GetPropVal('v')
            if not exists then return false, 'Missing height' end
            height = tonumber(value)
        elseif child.Name == 'AllAnswer' then
            puz.Grid:SetSize(width, height)
            local exists, solution = child:GetPropVal('v')
            if not exists then return false, 'Missing solution' end
            if not #solution == width * height then
                return false, 'Bad solution size'
            end
            local square = puz.Grid:First()
            -- These XML files use '-' for black squares
            solution = solution:gsub('-', '.')
            for c in solution:gmatch('.') do -- Iterate characters
                square:SetSolution(c)
                square = square:Next(xword.DIR_ACROSS)
            end
            nAcross, nDown = puz.Grid:CountClues()
        elseif child.Name == 'across' then
            local across = {}
            local clue = child.Children
            while clue do
                local exists, text = clue:GetPropVal('c')
                if not exists then return false, 'Missing across clue' end
                table.insert(across, convStr(text))
                clue = clue:GetNext()
            end
            if not #across == nAcross then
                return false, 'Missing across clues'
            end
            puz.Across = across
        elseif child.Name == 'down' then
            local down = {}
            local clue = child.Children
            while clue do
                local exists, text = clue:GetPropVal('c')
                if not exists then return false, 'Missing down clue' end
                table.insert(down, convStr(text))
                clue = clue:GetNext()
            end
            if not #down == nDown then
                return false, 'Missing down clues'
            end
            puz.Down = down
        end
        child = child:GetNext()
    end
    puz:RenumberClues()
    return true
end

--             Description      ext = file desc    function
import.addType('XML (uclick)', {xml = 'XML file'}, importXML)
