require 'import.json'

-- iterate index, v1, v2, v[n]
local function izip(...)
    local tables = {...}
    local i = 0
    -- Find the max length
    local max_length = 0
    for _, t in ipairs(tables) do
        if #t > max_length then max_length = #t end
    end
    return function ()
        i = i + 1
        if i <= max_length then
            local vals = {}
            for _, v in ipairs(tables) do
                table.insert(vals, v[i])
            end
            return i, unpack(vals)
        end
    end
end

-- Decode xml entities
local entities = {
    --lt = "<", -- Apparently these shouldn't be unescaped
    --gt = ">",
    apos = "'",
    quot = '"',
    amp = "&"
}

local function decode_entities(str)
    return str:gsub("&(.-);", entities)
end


function import.xwordinfoJSON(p, filename)
    -- Open the file and parse it
    local f = assert(io.open(filename))
    local success, doc = pcall(json.decode, f:read('*a'))
    f:close()

    if not success or not (doc.size and doc.grid and doc.gridnums) then
        return false -- Not an xwordinfo JSON file
    end

    -- Metadata
    -- Sunday puzzles have a real title without the date, so add the date
    if doc.dow == "Sunday" then
        p.Title = date(doc.date):fmt("NY Times, %a, %b %d, %Y ") .. doc.title
    else
        p.Title = doc.title
    end
    p.Author = doc.author
    p.Copyright = doc.copyright
    p:SetMeta("editor", doc.editor)

    -- Add the copyright symbol (utf8)
    if #p.Copyright > 0 then p.Copyright = "\194\169 " .. p.Copyright end
    p.Notes = doc.notepad or ""

    -- Grid
    local g = p.Grid
    g:SetSize(doc.size.cols, doc.size.rows)

    local s = g:First()
    local shade = doc.shadecircles or false
    for _, letter, number, circle in izip(doc.grid, doc.gridnums,
                                                 doc.circles) do
        s.Solution = letter
        if circle == 1 then
            if shade then
                s:SetHighlight(true)
            else
                s:SetCircle(true)
            end
        end
        if number > 0 then s.Number = number end
        s = s:Next()
    end

    -- Clues
    for name, clues in pairs(doc.clues) do
        local list = {}
        for _, clue in ipairs(clues) do
            local number, text = clue:match("(.-)%. (.*)")
            list[tonumber(number)] = decode_entities(text)
        end
        p:SetClueList(name == "across" and "Across" or "Down", list)
    end
end

import.addHandler(import.xwordinfoJSON, "json", "xwordinfo JSON")
