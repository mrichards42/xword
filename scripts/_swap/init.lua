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

local function findClueText(cluelist, number)
    local idx, text = tablex.find_if(
        cluelist,
        function(clue)
            if clue.number == number then
                return clue.text
            end
        end)
    return text
end

local function SwapAcrossDown(p)

    -- Swap the clues
    -- --------------
    local across = {}
    local down = {}

    -- Make a local copy because GetClueList creates a lua table each
    -- time it is called.
    local oldAcross = p:GetClueList('Across')
    local oldDown   = p:GetClueList('Down')

    -- Iterate the grid (downward), and look for all clue squares.
    -- These will all remain clue squares after the grid is swapped.

    -- Fill in the new clue numbers as we go.
    local clueNumber = 1

    -- Squares are accessed using Grid[{col, row}]
    -- lua uses 1-based table indices.
    local square = p.Grid[{1, 1}]

    while(square) do
        local nextNumber = false
        -- Down clues will become across clues
        if square:WantsClue(puz.DOWN) then
            table.insert(across, { number=clueNumber,
                                   text=findClueText(oldDown, square.Number)})
            nextNumber = true
        end
        -- Across clues will become down clues
        if square:WantsClue(puz.ACROSS) then
            table.insert(down, { number=clueNumber,
                                 text=findClueText(oldAcross, square.Number)})
            nextNumber = true
        end
        -- Next clue number
        if nextNumber then clueNumber = clueNumber + 1 end
        -- Next square
        square = square:Next(puz.DOWN)
    end

    p:SetClueList('Across', across)
    p:SetClueList('Down', down)


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

    -- Set the puzzle's grid.  The puzzle now has ownership of the grid.
    -- lua will not garbage collect the grid's userdata.
    p.Grid = newGrid

    p:NumberGrid()
    p:GenerateWords()
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

            -- Check to see if we can swap the grid
            if p.Grid:IsDiagramless() then
                xword.Message("Can't swap a diagramless puzzle.")
                return
            elseif not p:UsesNumberAlgorithm() then
                xword.Message("Can't swap a puzzle with an irregular grid.")
                return
            end

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
