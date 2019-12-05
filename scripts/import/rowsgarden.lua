local join = require 'pl.path'.join
local lyaml = require 'lyaml'

local bloom_none = -1
local bloom_medium = 0
local bloom_dark = 1
local bloom_light = 2

-- TODO: Let users customize these colors.
local colors = {
    [bloom_light] = {255, 255, 255},
    [bloom_medium] = {195, 200, 250},
    [bloom_dark] = {87, 101, 247},
}

local number_prefixes = {
    [bloom_light] = "L",
    [bloom_medium] = "M",
    [bloom_dark] = "D",
}

local function getBloomType(x, y)
    local y_offset = 0
    if (x - 1) % 6 < 3 then
        if y == 1 or y == 12 then
            return bloom_none
        else
            y_offset = 3
        end
    end
    return math.floor((y + y_offset - 1) / 2) % 3
end

local function formatClue(word)
    local clue = word.clue

    -- Replace *{title}* with <i>title</i>.
    clue = clue:gsub('%*([^%*]+)%*', '<i>%1</i>')

    -- Add clue notations
    -- TODO: Let users customize which notations are added.
    local suffixes = {}

    -- Word count (e.g. "2 wds.")
    local _, space_count = word.answer:gsub(' ', '')
    local word_count = space_count + 1
    if word_count > 1 then
        table.insert(suffixes, word_count .. ' wds.')
    end

    -- Whether the answer is hyphenated
    local _, hyphen_count = word.answer:gsub('%-', '')
    if hyphen_count > 0 then
        table.insert(suffixes, 'hyph.')
    end

    if #suffixes > 0 then
        return clue .. ': ' .. table.concat(suffixes, ', ')
    else
        return clue
    end
end

-- Fix invalid values in the provided YAML text.
-- .rg files often contain invalid YAML due to values containing reserved
-- characters. This method attempts to adds quotes around any value, escaping
-- existing quotes as needed.
local function fixInvalidYamlValues(contents)
    local lines = {}
    for line in contents:gmatch('[^\r\n]+') do
        -- Find a value on this line - anything following a ":", ignoring any
        -- whitespace at the beginning and end.
        local value = line:match(':%s*([^%s].+[^%s])%s*')
        if value == nil then
            -- Nothing found; insert the line as is.
            table.insert(lines, line)
        else
            -- Insert the line with the value surrounded by quotes and any
            -- existing quotes escaped.
            local escaped_line =
                    line:gsub(':%s*.+', ': "' .. value:gsub('"', '\\"') .. '"')
            table.insert(lines, escaped_line)
        end
  end
  return table.concat(lines, '\n')
end

local function rowsGarden(p, contents)
    local success, doc = pcall(lyaml.load, fixInvalidYamlValues(contents))
    if not success or not (doc.rows and doc.light and doc.medium and doc.dark) then
        return false
    end

    -- Metadata
    p.Title = doc.title
    p.Author = doc.author

    p.Copyright = doc.copyright
    -- Add the copyright symbol (utf8)
    if #p.Copyright > 0 then p.Copyright = "\194\169 " .. p.Copyright end

    if type(doc.notes) == "string" then
        p.Notes = doc.notes
    end

    -- Grid and clues
    local g = p.Grid
    if #doc.rows ~= 12 then
        return false
    end
    g:SetSize(21, 12)
    local s = g:First()
    local blooms = {
        [bloom_light] = { count = 0, clues = doc.light, clue_list = {} },
        [bloom_medium] = { count = 0, clues = doc.medium, clue_list = {} },
        [bloom_dark] = { count = 0, clues = doc.dark, clue_list = {} },
    }
    local row_clues = {}
    for y, words in pairs(doc.rows) do
        local row_chars = {}
        for _, word in pairs(words) do
            -- Only include alphanumeric characters in the grid.
            for ch in word.answer:gmatch("%w") do
                table.insert(row_chars, ch)
            end
        end
        local i = 1
        for x = 1, 21 do
            local bloom_type = getBloomType(x, y)
            if bloom_type == bloom_none then
                s.Solution = "."
            else
                s.Solution = row_chars[i]
                i = i + 1

                local color = colors[bloom_type]
                s:SetColor(color[1], color[2], color[3])

                -- Rows
                if ((y == 1 or y == 12) and x == 4) or
                        ((y ~= 1 and y ~= 12) and x == 1) then
                    s.Number = string.char(string.byte("A") + y - 1)
                    local clue
                    local row_clue = ""
                    for _, word in pairs(doc.rows[y]) do
                        if #row_clue > 0 then
                            row_clue = row_clue .. " / "
                        end
                        row_clue = row_clue .. formatClue(word)
                    end
                    local row_word = {}
                    for row_x = 1, 21 do
                        if getBloomType(row_x, y) ~= bloom_none then
                            table.insert(row_word, g[{row_x, y}])
                        end
                    end
                    table.insert(row_clues, {
                        number = s.Number,
                        text = row_clue,
                        word = row_word,
                    })
                end

                -- Blooms
                if (y % 2 == 0 and x % 6 == 3) or (y % 2 == 1 and x % 6 == 0) then
                    local number = blooms[bloom_type].count + 1
                    blooms[bloom_type].count = number

                    s.Number = number_prefixes[bloom_type] .. tostring(number)
                    table.insert(blooms[bloom_type].clue_list, {
                        number = s.Number,
                        text = formatClue(blooms[bloom_type].clues[number]),
                        word = {
                            g[{x - 2, y}], g[{x - 1, y}], g[{x, y}],
                            g[{x, y + 1}], g[{x - 1, y + 1}], g[{x - 2, y + 1}]
                        }
                    })
                end
            end
            s = s:Next()
        end
    end

    p:SetClueList("Rows", row_clues)

    -- Join the bloom clues together into a single list
    local bloom_clues = {}
    for i, v in ipairs(blooms[bloom_light].clue_list) do
        bloom_clues[i] = v
    end
    for i, v in ipairs(blooms[bloom_medium].clue_list) do
        bloom_clues[blooms[bloom_light].count + i] = v
    end
    for i, v in ipairs(blooms[bloom_dark].clue_list) do
        bloom_clues[blooms[bloom_light].count + blooms[bloom_medium].count + i] = v
    end
    p:SetClueList("Blooms", bloom_clues)
end

-- Import a .rg file
function import.rowsGarden(p, filename)
    local f = assert(io.open(filename))
    local contents = f:read('*a')
    f:close()
    return rowsGarden(p, contents)
end

-- Import a .rgz file (a .zip file containing a single .rg file)
function import.rowsGardenCompressed(p, filename)
    local fs = xword.NewFs(filename .. "#zip:")
    if not fs then
        return false
    end
    local file = fs:FindFirst(join(fs.archive, '*'), wx.wxFILE)
    if not file then
        fs:delete()
        return false
    end
    local contents = fs:ReadFile(file)
    fs:delete()
    if type(contents) ~= "string" then
        return false
    end
    return rowsGarden(p, contents)
end

import.addHandler(import.rowsGarden, "rg", "Rows Garden")
import.addHandler(import.rowsGardenCompressed, "rgz", "Rows Garden")
