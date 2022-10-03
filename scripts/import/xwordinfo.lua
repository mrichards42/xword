local yajl = require 'luayajl'
local date = require 'date'

-- Yajl uses yajl.null to represent null values instead of nil
local function ornil(obj)
    return obj ~= yajl.null and obj or nil
end

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
    --lt = "<", -- These will be unescaped later
    --gt = ">",
    apos = "'",
    quot = '"',
    amp = "&"
}

local function replace_entity(entity)
    if entities[entity] then
        return entities[entity]
    end
    local decimal = entity:match("#(%d+)")
    if decimal then
        return string.char(tonumber(decimal))
    else
        return entity
    end
end

local function decode_entities(str)
    -- Decode entities and replace XHTML tags
    return str:gsub("&(.-);", replace_entity):gsub("&lt;([^%s]-)&gt;", "<%1>")
end


function import.xwordinfoJSON(p, filename)
    -- Open the file and parse it
    local f = assert(io.open(filename))
    local success, doc = pcall(yajl.to_value, f:read('*a'))
    f:close()

    if not success or not (doc.size and doc.grid and doc.gridnums) then
        return false -- Not an xwordinfo JSON file
    end

    -- Metadata
    local title = decode_entities(doc.title)
    if ornil(doc.type) then
        -- Add the variety type to the title
        local variety_type
        if doc.type == "panda" then
            variety_type = "PUNS AND ANAGRAMS"
        else
            variety_type = doc.type:upper()
        end
        if title:find(variety_type) then
            p:SetTitle(title, --[[ is_html ]] true)
        else
            p:SetTitle(title .. " " .. variety_type, --[[ is_html ]] true)
        end
    else
        -- Sunday puzzles have a real title without the date, so add the date
        if doc.dow == "Sunday" then
            p:SetTitle(date(doc.date):fmt("NY Times, %a, %b %d, %Y ") .. title, --[[ is_html ]] true)
        else
            p:SetTitle(title, --[[ is_html ]] true)
        end
    end
    p:SetAuthor(doc.author, --[[ is_html ]] true)
    p.Copyright = doc.copyright
    if doc.editor then
        p:SetMeta("editor", doc.editor, --[[ is_html ]] true)
    end

    -- Add the copyright symbol (utf8)
    if #p.Copyright > 0 then p.Copyright = "\194\169 " .. p.Copyright end
    p:SetNotes(decode_entities(ornil(doc.notepad) or ""), --[[ is_html ]] true)

    -- Grid
    local g = p.Grid
    g:SetSize(doc.size.cols, doc.size.rows)

    -- Set diagramless
    if doc.type == "diagramless" then
        g.Type = puz.TYPE_DIAGRAMLESS
    end

    local s = g:First()
    local shade = ornil(doc.shadecircles) or false
    for _, letter, number, circle in izip(doc.grid, doc.gridnums, ornil(doc.circles)) do
        s.Solution = letter
        if letter == "." and doc.type == "diagramless" then
            s.Text = ""
        end
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
            list[tonumber(number)] = {
                text = decode_entities(text),
                is_html = true
            }
        end
        p:SetClueList(name == "across" and "Across" or "Down", list)
    end
end

import.addHandler(import.xwordinfoJSON, "json", "xwordinfo JSON")
