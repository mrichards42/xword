local tablex = require 'pl.tablex'

-- Return a table of clue references {{number, direction}, ...}
local function get_clue_references(text, directions)
    if not directions then directions = {"across", "down"} end
    local ret = {}
    local accumulator = {} -- Numbers without a direction
    local i = 0 -- string index of the last number
    while true do
        local _, last, number = text:find("(%d+)%-", i)
        if not number then break end
        i = last
        table.insert(accumulator, number)
        -- Look for the direction
        for _, direction in ipairs(directions) do
            if text:sub(i+1, i+#direction) == direction:lower() then
                -- If we found a direction apply it to the accumulate clues
                for _, number in ipairs(accumulator) do
                    table.insert(ret, {number, direction})
                end
                tablex.clear(accumulator)
                -- Move the string index
                i = i+#direction
                break
            end
        end
    end
    return ret
end

-- This function is called from Frame::LoadPuzzle to mark theme squares
return function(p)
    local theme_clues = {}
    local clues = p:GetClues()

    -- Mark all squares in each clue word as a theme square
    local function mark_squares()
        for _, clue in ipairs(theme_clues) do
            for _, square in ipairs(clue:GetWord()) do
                square:SetTheme(true)
            end
        end
    end

    -- Iterate over all clues
    -- func takes clue and direction name
    local function iter_clues(func)
        for direction, list in pairs(clues) do
            for _, clue in ipairs(list) do
                func(clue, direction)
            end
        end
    end

    -- Return a list of clues that match anything in patterns
    local function find_clues(...)
        local patterns = {...}
        iter_clues(function(clue)
            for _, pat in ipairs(patterns) do
                if clue.text:match(pat) then
                    table.insert(theme_clues, clue)
                end
            end
        end)
    end

    -- ------------------------------------------------------------------------
    -- Starred clues
    -- ------------------------------------------------------------------------
    find_clues("^%*", "%*$") -- Star at the start or end
    if #theme_clues > 0 then
        find_clues("starred clue")
        find_clues("starred.*puzzle") -- "starred ___ in this puzzle"
        return mark_squares()
    end
    
    -- ------------------------------------------------------------------------
    -- Clues with 3+ references
    -- ------------------------------------------------------------------------
    -- Setup a map of clue references
    local clue_map = {} -- {direction={number={clue=clue, to={}, from={}}}}
    for direction, list in pairs(clues) do
        local t = {}
        for _, clue in ipairs(list) do
            -- { clue_list, to_other_clues, from_other_clues }
            t[clue.number] = {clue=clue, to={}, from={}}
        end
        clue_map[direction] = t
    end
    -- Find clue references and add to the clue_map
    iter_clues(function (clue, direction)
        local this_clue = clue_map[direction][clue.number]
        for _, ref in ipairs(get_clue_references(clue.text:lower(), tablex.keys(clue_map))) do
            local number, direction = unpack(ref)
            local to_clue = clue_map[direction][number]
            if to_clue then
                table.insert(this_clue.to, to_clue)
                table.insert(to_clue.from, this_clue)
            end
        end
    end)

    -- Look for clues in this map that have at least 3 references (to or from)
    for _, list in pairs(clue_map) do
        for _, clue in pairs(list) do
            if #clue.to >= 3 or #clue.from >= 3 then
                -- Add this clue's complete reference tree
                local done = {}
                local add_clue
                add_clue = function (clue)
                    if done[clue] then return end
                    done[clue] = true
                    table.insert(theme_clues, clue.clue)
                    for _, c in ipairs(clue.from) do add_clue(c) end
                    for _, c in ipairs(clue.to) do add_clue(c) end
                end
                add_clue(clue)
            end
        end
    end
    if #theme_clues > 0 then
        return mark_squares()
    end
end