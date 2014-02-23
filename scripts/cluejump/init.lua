-- ============================================================================
-- Clue Jump
--     Searche the selected clue for the first clue-reference if can find, and
--     jump to that clue.  If there is no reference in the current clue, search
--     all clues for a reference to the current clue and jump to the first
--     clue that matches.
-- ============================================================================


local across_pattern = "(%d+)[- ]*across"
local down_pattern   = "(%d+)[- ]*down"

-- Find a reference to another clue in this clue
local function find_reference(clue)
    local clue = clue:lower()
    local number
    number = clue:match(across_pattern)
    if number then
        return number, puz.ACROSS
    end
    number = clue:match(down_pattern)
    if number then
        return number, puz.DOWN
    end
end

-- Find a reference to this clue in any other clue
local function search_clues(number, direction)
    local pattern = number.."[- ]*"..(direction == puz.ACROSS and "across" or "down")
    local p = xword.frame.Puzzle
    for _, clue in pairs(p:GetClueList('Across')) do
        if clue.text:lower():match(pattern) then
            return clue.number, puz.ACROSS
        end
    end
    for _, clue in pairs(p:GetClueList('Down')) do
        if clue.text:lower():match(pattern) then
            return clue.number, puz.DOWN
        end
    end
end


local function ClueJump()
    local focused_direction = xword.frame:GetFocusedDirection()
    -- Get the current clue
    local clue, _text = xword.frame:GetFocusedClue()
    if type(clue) == "string" then -- Accommodate the old API
        clue = { number = clue, text = text }
    end

    -- Find a match
    local number, direction
    number, direction = find_reference(clue.text)
    if not number then
        number, direction = search_clues(clue.number, focused_direction)
        if not number then
            xword.Message("Could not find a clue reference.")
            return
        end
    end

    -- Find the grid square for this clue
    local grid = xword.frame.Puzzle.Grid
    local square = grid:FindSquare(
        function(s)
            return s:HasWord(direction) and s:GetNumber() == number
        end
    )
    if square then
        xword.frame:SetFocusedSquare(square)
        xword.frame:SetFocusedDirection(direction)
    else
        xword.Message("%s-%s does not exist.",
                        number,
                        direction == puz.ACROSS and "Across" or "Down")
    end
end


local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Jump to clue reference\tCtrl+Shift+C',
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end
            ClueJump()
        end
    )
end

local function uninit()
    xword.frame:RemoveMenuItem("Tools", "Jump to clue reference")
end

return {init, uninit}
