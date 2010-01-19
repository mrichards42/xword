assert(import)

-- The XML strings can have % escape codes in them.
-- After decoding, the string is in UTF-8.  wxWidgets doesn't give us access
-- to the conversion functions, but xword performs a conversion from windows
-- encoding to unicode when its functions are used.  Thus, we need to convert
-- from UTF-8 to windows encoding here.  This isn't difficult because we can
-- only represent codes below 256 anyways.
local function convStr(str)
    -- First convert the entities to real bytes
    local ret = ''
    for chars, escape in str:gmatch('([^%%]*)%%?(%d*)') do
        ret = ret..chars
        if #escape > 0 then
            ret = ret..string.char(tonumber(escape))
        end
    end
    -- Next scan through the string to catch bytes > 127 (mult-byte codes
    -- in UTF-8)
    str = ret
    ret = ''
    for c in str:gmatch('.') do
        local b = c:byte()
        if b <= 127 then
            ret = ret..c
        else
            
        end
    end
end


local function importXML(filename, puz)
    local doc = wx.wxXmlDocument(filename)
    if not doc:IsOk() then
        error('Error opening file: '..filename)
    end

    -- Root node
    if doc.Root.Name ~= 'crossword' then
        error'Root node should be "crossword"'
    end

    local width, height
    local nAcross, nDown

    -- Read the File
    local child = doc.Root.Children
    while child do
        if child.Name == 'Title' then
            local exists, title = child:GetPropVal('v')
            assert(exists,'Missing title')
            puz.Title = title
        elseif child.Name == 'Author' then
            local exists, author = child:GetPropVal('v')
            assert(exists,'Missing author')
            puz.Author = author
        elseif child.Name == 'Width' then
            local exists, value = child:GetPropVal('v')
            assert(exists,'Missing width')
            width = tonumber(value)
        elseif child.Name == 'Height' then
            local exists, value = child:GetPropVal('v')
            assert(exists,'Missing height')
            height = tonumber(value)
        elseif child.Name == 'AllAnswer' then
            puz.Grid:SetSize(width, height)
            local exists, solution = child:GetPropVal('v')
            assert(exists,'Missing solution')
            assert(#solution == width * height, 'Bad solution size')
            local square = puz.Grid:First()
            -- These XML files use '-' for black squares
            solution = solution:gsub('-', '.')
            for c in solution:gmatch('.') do -- Iterate characters
                square:SetSolution(c)
                square = square:Next(xword.DIR_ACROSS)
            end
            -- Don't forget to call this, or XWord will crash
            puz.Grid:SetupGrid()
            nAcross, nDown = puz.Grid:CountClues()
        elseif child.Name == 'across' then
            local across = {}
            local clue = child.Children
            while clue do
                local exists, text = clue:GetPropVal('c')
                assert(exists, 'Missing across clue')
                -- Clues may have % escape codes
                table.insert(across, wx.wxURI.Unescape(text))
                clue = clue:GetNext()
            end
            assert(#across == nAcross, 'Missing across clues: '..(#across)..' ~= '..nAcross)
            puz.Across = across
        elseif child.Name == 'down' then
            local down = {}
            local clue = child.Children
            while clue do
                local exists, text = clue:GetPropVal('c')
                assert(exists, 'Missing down clue')
                -- Clues may have % escape codes
                table.insert(down, wx.wxURI.Unescape(text))
                clue = clue:GetNext()
            end
            assert(#down == nDown, 'Missing down clues: '..(#down)..' ~= '..nDown)
            puz.Down = down
        end
        child = child:GetNext()
    end
    puz:RenumberClues()
end

--             Description      ext = file desc    function
import.addType('XML (uclick)', {xml = 'XML file'}, importXML)
