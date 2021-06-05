local base64 = require 'base64'
local yajl = require 'luayajl'
local date = require 'date'

local function make_word(grid, placedWord)
  local word = {}
  local x = placedWord.x + 1
  local y = placedWord.y + 1
  for i = 0,placedWord.nBoxes-1 do
    if placedWord.acrossNotDown then
      table.insert(word, grid[{x + i, y}])
    else
      table.insert(word, grid[{x, y + i}])
    end
  end
  return word
end

local function importJSON(p, doc)
  if not doc or not (doc.box and doc.h and doc.w) then
      return false -- Not an amuselabs JSON file
  end

  -- Metadata
  p.Title = doc.title
  p.Author = doc.author

  -- Grid
  local g = p.Grid
  g:SetSize(doc.w, doc.h)

  for x, col in ipairs(doc.box) do
    for y, solution in ipairs(col) do
      local s = g[{x, y}]
      -- solution
      if solution == '\0' then
        s.Solution = '.'
      else
        s.Solution = solution
      end
      -- clue number
      local number = doc.clueNums[x][y]
      if number > 0 then
        s.Number = number
      end
    end
  end

  -- circles, etc.
  for _, info in ipairs(doc.cellInfos or {}) do
    local s = g[{info.x + 1, info.y + 1}]
    if info.isCircled then
      s.Circle = true
    end
    if info.bgColor then
      local r, g, b = info.bgColor:match('#?(..)(..)(..)')
      if r then
        s:SetColor(tonumber(r, 16), tonumber(g, 16), tonumber(b, 16))
      elseif #info.bgColor > 1 then
        s.Highlight = true -- fallback
      end
    end
    if info.isVoid then
      if s:HasColor() then
        -- can't have a void colored square, but can have a black colored one
        s.Solution = '.'
      else
        s.Missing = true
      end
    end
    -- TODO: bars
  end

  -- Clues
  local clues = {across={}, down={}}
  for i, placedWord in ipairs(doc.placedWords) do
    local clue_k = placedWord.acrossNotDown and "across" or "down"
    table.insert(clues[clue_k], {
        number = placedWord.clueNum,
        text = placedWord.clue.clue,
        word = make_word(g, placedWord)
      })
  end
  p:SetClueList("Across", clues.across)
  p:SetClueList("Down", clues.down)
end


function import.amuselabsJSON(p, filename)
    local f = assert(io.open(filename))
    local success, doc = pcall(yajl.to_value, f:read('*a'))
    f:close()
    return importJSON(p, doc)
end

function import.amuselabsBase64(p, data)
  local success, json, doc
  success, json = pcall(base64.decode, data)
  if success then
    success, doc = pcall(yajl.to_value, json)
    if success then
      return importJSON(p, doc)
    end
  end
  return false
end

import.addHandler(import.amuselabsJSON, "json", "amuselabs JSON")
