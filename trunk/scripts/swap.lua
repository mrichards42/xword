-- ============================================================================
-- Swap Across and Down
--     An XWord add-on that demonstrates interaction with the gui and the
--     crossword puzzle data structures.
-- ============================================================================

-- Note that most XWord puzzle data structures can be accessed using either
-- puzzle.XXXX or puzzle:GetXXXX()
-- e.g. puzzle.Across / puzzle.GetAcross()
--      xword.frame.Puzzle / xword.frame:GetPuzzle()
-- Both approaches are demonstrated here.

local function SwapAcrossDown(puz)
    -- Swap the clues
    -- --------------
    local across = {}
    local down = {}

    -- Make a local copy because GetAcross / GetDown creates a lua table each
    -- time it is called.
    local oldAcross = puz:GetAcross()
    local oldDown   = puz:GetDown()

    -- Iterate the grid (downward), and look for all clue squares.
    -- These will all remain clue squares after the grid is swapped.

    -- Fill in the new clue numbers as we go.
    local clueNumber = 1

    -- Squares are accessed using Grid[{col, row}]
    -- lua uses 1-based table indices.
    local square = puz.Grid[{1, 1}]

    while(square) do
        -- Down clues will become across clues
        if square:HasClue(xword.DIR_DOWN) then
            across[clueNumber] = oldDown[square.Number]
        end
        -- Across clues will become down clues
        if square:HasClue(xword.DIR_ACROSS) then
            down[clueNumber] = oldAcross[square.Number]
        end
        -- Next clue number
        if square:HasClue() then clueNumber = clueNumber + 1 end
        -- Next square
        square = square:Next(xword.DIR_DOWN)
    end

    puz.Across = across
    puz.Down = down

    -- Swap the grid
    -- -------------
    local newGrid = xword.XGrid(puz.Grid.Height, puz.Grid.Width)

    for row = 1, puz.Grid.Height do
        for col = 1, puz.Grid.Width do
            -- Swap the square
            newGrid[{row, col}] = puz.Grid[{col, row}]
        end
    end

    -- Set the puzzle's grid.  The puzzle now has ownership of the grid.
    -- lua will not garbage collect the grid's userdata.
    puz.Grid = newGrid
end


local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Swap Across and Down',
        -- The menu callback function
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end

            local puz = xword.frame.Puzzle
            local frame = xword.frame

            -- Save the current square and direction focus
            local square = frame:GetFocusedSquare()
            local col, row = square.Col, square.Row
            local direction = frame:GetFocusedDirection()

            -- Swap the grid
            SwapAcrossDown(puz)

            -- Update the applicable portions of the gui
            frame:ShowGrid()
            frame:ShowClues()

            -- Restore the focused square (swapping direction)
            frame:SetFocusedSquare(puz.Grid[{row, col}])
            frame:SetFocusedDirection(not direction)
        end
    )
end


init()
