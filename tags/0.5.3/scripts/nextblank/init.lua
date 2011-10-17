-- ============================================================================
-- Next Blank
--     Jump to the next blank square
-- ============================================================================

local function find_next_blank(square)
    local grid = xword.frame.Puzzle.Grid
    return grid:FindSquare(
        square or grid:First(),
        function(s)
            return s:IsBlank()
        end
    )
end

local function NextBlank()
    -- Find the next blank square or the first blank square
    local square = find_next_blank(xword.frame:GetFocusedSquare():Next()) or
                   find_next_blank() -- first blank
    if not square then
        xword.Message("No more blank squares")
    else
        xword.frame:SetFocusedSquare(square)
    end
end


local function init()
    xword.frame:AddMenuItem({'Tools'}, 'Next blank square\tCtrl+B',
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end
            NextBlank()
        end
    )
end

function uninit()
    xword.frame:RemoveMenuItem("Tools", "Next blank square")
end

return { init, uninit }
