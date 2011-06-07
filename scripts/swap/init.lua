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

local tablex = require 'pl.tablex'

local function SwapAcrossDown(p)
    -- Check to see if we can swap the grid
    if p.Grid:IsDiagramless() then
        xword.Message("Can't swap a diagramless puzzle.")
    elseif not p:UsesNumberAlgorithm() then
        xword.Message("Can't swap a puzzle with unclued squares.")
    end

    -- Swap the clues
    -- --------------
    local across = puz.ClueList()
    local down = puz.ClueList()

    -- Make a local copy because GetAcross / GetDown creates a lua table each
    -- time it is called.
    local oldAcross = p:GetAcross()
    local oldDown   = p:GetDown()

    -- Iterate the grid (downward), and look for all clue squares.
    -- These will all remain clue squares after the grid is swapped.

    -- Fill in the new clue numbers as we go.
    local clueNumber = 1

    -- Squares are accessed using Grid[{col, row}]
    -- lua uses 1-based table indices.
    local square = p.Grid[{1, 1}]

    while(square) do
        -- Down clues will become across clues
        if square:HasClue(puz.DOWN) then
            across:Insert(clueNumber, oldDown:Find(square.Number))
        end
        -- Across clues will become down clues
        if square:HasClue(puz.ACROSS) then
            down:Insert(clueNumber, oldAcross:Find(square.Number))
        end
        -- Next clue number
        if square:HasClue() then clueNumber = clueNumber + 1 end
        -- Next square
        square = square:Next(puz.DOWN)
    end

    p.Across = across
    p.Down = down


    -- Swap the grid
    -- -------------
    local newGrid = puz.Grid(p.Grid.Height, p.Grid.Width)

    for row = 1, p.Grid.Height do
        for col = 1, p.Grid.Width do
            -- Swap the square
            newGrid[{row, col}] = p.Grid[{col, row}]
        end
    end

    -- Copy over metadata
    newGrid.Type = p.Grid.Type
    newGrid.Flag = p.Grid.Flag
    newGrid.Cksum = p.Grid.Cksum
    newGrid.Key = p.Grid.Key

    -- Set the puzzle's grid.  The puzzle now has ownership of the grid.
    -- lua will not garbage collect the grid's userdata.
    p.Grid = newGrid

    p:NumberGrid()
    p:NumberClues()

    xword.frame:ShowPuzzle(p)
end


local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Swap Across and Down',
        -- The menu callback function
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end

            local p = xword.frame.Puzzle
            local frame = xword.frame

            -- Save the current square and direction focus
            local square = frame:GetFocusedSquare()
            local col, row = square.Col, square.Row
            local direction = frame:GetFocusedDirection()

            -- Swap the grid
            SwapAcrossDown(p)

            -- Update the applicable portions of the gui
            frame:ShowGrid()
            frame:ShowClues()

            -- Restore the focused square (swapping direction)
            frame:SetFocusedSquare(xword.frame.Puzzle.Grid[{row, col}])
            frame:SetFocusedDirection(puz.SwapDirection(direction))
        end
    )
end

function uninit()
    xword.frame:RemoveMenuItem("Tools", "Swap Across and Down")
end

return { init, uninit }
